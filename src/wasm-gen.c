#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "wasm.h"
#include "wasm-internal.h"
#include "wasm-parse.h"
#include "wasm-gen.h"

#define INITIAL_OUTPUT_BUFFER_CAPACITY (64 * 1024)
#define DEFAULT_MEMORY_EXPORT 1
#define DUMP_OCTETS_PER_LINE 16
#define DUMP_OCTETS_PER_GROUP 2

#define SEGMENT_SIZE 13
#define SEGMENT_ADDRESS_OFFSET 0
#define SEGMENT_OFFSET_OFFSET 4
#define SEGMENT_SIZE_OFFSET 8
#define SEGMENT_INIT_OFFSET 12

#define GLOBAL_SIZE 6
#define GLOBAL_NAME_OFFSET_OFFSET 0
#define GLOBAL_MEM_TYPE_OFFSET 4
#define GLOBAL_EXPORT_OFFSET 5

#define IMPORT_SIZE 7
#define IMPORT_FLAGS_OFFSET 0
#define IMPORT_SIGNATURE_OFFSET 1
#define IMPORT_NAME_OFFSET 3

#define FUNCTION_FLAGS_OFFSET 0
#define FUNCTION_SIGNATURE_OFFSET 1
#define FUNCTION_NAME_OFFSET 3
#define FUNCTION_BODY_SIZE_OFFSET(flags)             \
  (3 + (((flags)&WASM_FUNCTION_FLAG_NAME) ? 4 : 0) + \
   (((flags)&WASM_FUNCTION_FLAG_LOCALS) ? 8 : 0))

typedef enum WasmSectionType {
  WASM_SECTION_MEMORY = 0,
  WASM_SECTION_SIGNATURES = 1,
  WASM_SECTION_FUNCTIONS = 2,
  WASM_SECTION_GLOBALS = 3,
  WASM_SECTION_DATA_SEGMENTS = 4,
  WASM_SECTION_FUNCTION_TABLE = 5,
  WASM_SECTION_END = 6,
} WasmSectionType;

enum {
  WASM_FUNCTION_FLAG_NAME = 1,
  WASM_FUNCTION_FLAG_IMPORT = 2,
  WASM_FUNCTION_FLAG_LOCALS = 4,
  WASM_FUNCTION_FLAG_EXPORT = 8,
};
typedef uint8_t WasmFunctionFlags;

typedef enum WasmTypeV8 {
  WASM_TYPE_V8_VOID,
  WASM_TYPE_V8_I32,
  WASM_TYPE_V8_I64,
  WASM_TYPE_V8_F32,
  WASM_TYPE_V8_F64,
  WASM_NUM_V8_TYPES,
} WasmTypeV8;

typedef struct OutputBuffer {
  void* start;
  size_t size;
  size_t capacity;
  int log_writes;
} OutputBuffer;

typedef enum LabelType {
  LABEL_TYPE_BLOCK,
  LABEL_TYPE_LABEL,
  LABEL_TYPE_LOOP_OUTER,
  LABEL_TYPE_LOOP_INNER,
} LabelType;

typedef struct LabelInfo {
  LabelType type;
  struct LabelInfo* next_label;
  uint32_t offset;
  int block_depth;
  int with_expr;
} LabelInfo;

typedef struct AssertFunc {
  char* name;
  uint32_t name_offset;
} AssertFunc;

typedef struct Context {
  WasmGenOptions* options;
  OutputBuffer buf;
  OutputBuffer js_buf;
  WasmModule* module;
  uint32_t segment_section_offset;
  uint32_t global_section_offset;
  uint32_t signature_section_offset;
  uint32_t function_pre_section_offset;
  uint32_t function_section_offset;
  uint16_t nullary_signature_index[WASM_NUM_V8_TYPES];
  LabelInfo* top_label;
  int* remapped_locals;
  int in_assert;
  int block_depth;

  AssertFunc* assert_funcs;
  int num_assert_funcs;
} Context;

static const char* s_opcode_names[] = {
#define OPCODE(name, code) [code] = "OPCODE_" #name,
    OPCODES(OPCODE)
#undef OPCODE
};

static int is_power_of_two(uint32_t x) {
  return x && ((x & (x - 1)) == 0);
}

static uint32_t log_two_u32(uint32_t x) {
  if (!x)
    return 0;
  return sizeof(unsigned int) * 8 - __builtin_clz(x - 1);
}

static WasmTypeV8 wasm_type_to_v8_type(WasmType type) {
  switch (type) {
    case WASM_TYPE_VOID:
      return WASM_TYPE_V8_VOID;
    case WASM_TYPE_I32:
      return WASM_TYPE_V8_I32;
    case WASM_TYPE_I64:
      return WASM_TYPE_V8_I64;
    case WASM_TYPE_F32:
      return WASM_TYPE_V8_F32;
    case WASM_TYPE_F64:
      return WASM_TYPE_V8_F64;
    default:
      FATAL("v8-native does not support type %d\n", type);
  }
}

static WasmType v8_type_to_wasm_type(WasmTypeV8 type) {
  switch (type) {
    case WASM_TYPE_V8_VOID:
      return WASM_TYPE_VOID;
    case WASM_TYPE_V8_I32:
      return WASM_TYPE_I32;
    case WASM_TYPE_V8_I64:
      return WASM_TYPE_I64;
    case WASM_TYPE_V8_F32:
      return WASM_TYPE_F32;
    case WASM_TYPE_V8_F64:
      return WASM_TYPE_F64;
    default:
      FATAL("v8-native does not support type %d\n", type);
  }
}

static void init_output_buffer(OutputBuffer* buf,
                               size_t initial_capacity,
                               int log_writes) {
  /* We may be reusing the buffer, free it */
  free(buf->start);
  buf->start = malloc(initial_capacity);
  if (!buf->start)
    FATAL("unable to allocate %zd bytes\n", initial_capacity);
  buf->size = 0;
  buf->capacity = initial_capacity;
  buf->log_writes = log_writes;
}

static void ensure_output_buffer_capacity(OutputBuffer* buf,
                                          size_t ensure_capacity) {
  if (ensure_capacity > buf->capacity) {
    size_t new_capacity = buf->capacity * 2;
    while (new_capacity < ensure_capacity)
      new_capacity *= 2;
    buf->start = realloc(buf->start, new_capacity);
    if (!buf->start)
      FATAL("unable to allocate %zd bytes\n", new_capacity);
    buf->capacity = new_capacity;
  }
}

static void dump_memory(const void* start,
                        size_t size,
                        size_t offset,
                        int print_chars,
                        const char* desc) {
  /* mimic xxd output */
  const uint8_t* p = start;
  const uint8_t* end = p + size;
  while (p < end) {
    const uint8_t* line = p;
    const uint8_t* line_end = p + DUMP_OCTETS_PER_LINE;
    printf("%07x: ", (int)((void*)p - start + offset));
    while (p < line_end) {
      int i;
      for (i = 0; i < DUMP_OCTETS_PER_GROUP; ++i, ++p) {
        if (p < end) {
          printf("%02x", *p);
        } else {
          putchar(' ');
          putchar(' ');
        }
      }
      putchar(' ');
    }

    putchar(' ');
    p = line;
    int i;
    for (i = 0; i < DUMP_OCTETS_PER_LINE && p < end; ++i, ++p)
      if (print_chars)
        printf("%c", isprint(*p) ? *p : '.');
    /* if there are multiple lines, only print the desc on the last one */
    if (p >= end && desc)
      printf("  ; %s", desc);
    putchar('\n');
  }
}

static void move_data(OutputBuffer* buf,
                      size_t dest_offset,
                      size_t src_offset,
                      size_t size) {
  assert(src_offset + size <= buf->size);
  int dest_end = dest_offset + size;
  ensure_output_buffer_capacity(buf, dest_end);
  if (dest_end >= buf->size)
    buf->size = dest_end;
  memmove(buf->start + dest_offset, buf->start + src_offset, size);
  if (buf->log_writes) {
    printf("; move [%07zx,%07zx) -> [%07zx,%07zx)\n", src_offset,
           src_offset + size, dest_offset, dest_offset + size);
  }
}

static void print_header(Context* ctx, const char* name, int index) {
  if (ctx->options->verbose)
    printf("; %s %d\n", name, index);
}

static size_t out_data(OutputBuffer* buf,
                       size_t offset,
                       const void* src,
                       size_t size,
                       const char* desc) {
  assert(offset <= buf->size);
  ensure_output_buffer_capacity(buf, offset + size);
  memcpy(buf->start + offset, src, size);
  if (buf->log_writes)
    dump_memory(buf->start + offset, size, offset, 0, desc);
  return offset + size;
}

/* TODO(binji): endianness */
#define OUT_TYPE(name, ctype)                                                \
  static void out_##name(OutputBuffer* buf, ctype value, const char* desc) { \
    buf->size = out_data(buf, buf->size, &value, sizeof(value), desc);       \
  }

OUT_TYPE(u8, uint8_t)
OUT_TYPE(u16, uint16_t)
OUT_TYPE(u32, uint32_t)
OUT_TYPE(u64, uint64_t)
OUT_TYPE(f32, float)
OUT_TYPE(f64, double)

#undef OUT_TYPE

#define OUT_AT_TYPE(name, ctype)                                               \
  static void out_##name##_at(OutputBuffer* buf, uint32_t offset, ctype value, \
                              const char* desc) {                              \
    out_data(buf, offset, &value, sizeof(value), desc);                        \
  }

OUT_AT_TYPE(u8, uint8_t)
OUT_AT_TYPE(u16, uint16_t)
OUT_AT_TYPE(u32, uint32_t)

#undef OUT_AT_TYPE

#define READ_AT_TYPE(name, ctype)                                   \
  static ctype read_##name##_at(OutputBuffer* buf, size_t offset) { \
    assert(offset + sizeof(ctype) <= buf->size);                    \
    ctype value;                                                    \
    memcpy(&value, buf->start + offset, sizeof(value));             \
    return value;                                                   \
  }

READ_AT_TYPE(u8, uint8_t)
READ_AT_TYPE(u16, uint16_t)

#undef READ_AT_TYPE

static void out_opcode(OutputBuffer* buf, WasmOpcode opcode) {
  out_u8(buf, (uint8_t)opcode, s_opcode_names[opcode]);
}

static int leb128_size(uint32_t value) {
  int len = 0;
  do {
    value >>= 7;
    ++len;
  } while (value);
  return len;
}

/* returns the number of bytes written */
static int out_leb128_at(OutputBuffer* buf,
                         size_t offset,
                         uint32_t value,
                         const char* desc) {
  uint8_t data[5]; /* max 5 bytes */
  int i = 0;
  do {
    assert(i < 5);
    data[i] = value & 0x7f;
    value >>= 7;
    if (value)
      data[i] |= 0x80;
    ++i;
  } while (value);
  out_data(buf, offset, data, i, desc);
  return i;
}

static void out_leb128(OutputBuffer* buf, uint32_t value, const char* desc) {
  int num_bytes = out_leb128_at(buf, buf->size, value, desc);
  buf->size += num_bytes;
}

static void out_cstr(OutputBuffer* buf, const char* s, const char* desc) {
  buf->size = out_data(buf, buf->size, s, strlen(s) + 1, desc);
}

static void out_segment(OutputBuffer* buf,
                        WasmSegment* segment,
                        const char* desc) {
  size_t offset = buf->size;
  ensure_output_buffer_capacity(buf, offset + segment->size);
  void* dest = buf->start + offset;
  wasm_copy_segment_data(segment->data, dest, segment->size);
  if (buf->log_writes)
    dump_memory(buf->start + offset, segment->size, offset, 1, desc);
  buf->size += segment->size;
}

static void out_printf(OutputBuffer* buf, const char* format, ...) {
  va_list args;
  va_list args_copy;
  va_start(args, format);
  va_copy(args_copy, args);
  /* + 1 to account for the \0 that will be added automatically by vsnprintf */
  int len = vsnprintf(NULL, 0, format, args) + 1;
  va_end(args);
  ensure_output_buffer_capacity(buf, buf->size + len);
  char* buffer = buf->start + buf->size;
  vsnprintf(buffer, len, format, args_copy);
  va_end(args_copy);
  /* - 1 to remove the trailing \0 that was added by vsnprintf */
  buf->size += len - 1;
}

static void dump_output_buffer(OutputBuffer* buf) {
  dump_memory(buf->start, buf->size, 0, 1, NULL);
}

static void write_output_buffer(OutputBuffer* buf, const char* filename) {
  FILE* out = (strcmp(filename, "-") != 0) ? fopen(filename, "wb") : stdout;
  if (!out)
    FATAL("unable to open %s\n", filename);
  if (fwrite(buf->start, 1, buf->size, out) != buf->size)
    FATAL("unable to write %zd bytes\n", buf->size);
  fclose(out);
}

static void destroy_output_buffer(OutputBuffer* buf) {
  free(buf->start);
}

static void destroy_assert_funcs(Context* ctx) {
  int i;
  for (i = 0; i < ctx->num_assert_funcs; ++i)
    free(ctx->assert_funcs[i].name);
  ctx->num_assert_funcs = 0;
}

static void destroy_module(Context* ctx) {
  LabelInfo* label = ctx->top_label;
  while (label) {
    LabelInfo* next_label = label->next_label;
    free(label);
    label = next_label;
  }
  ctx->top_label = NULL;
  destroy_assert_funcs(ctx);
}

static void destroy_context(Context* ctx) {
  destroy_output_buffer(&ctx->buf);
  destroy_output_buffer(&ctx->js_buf);
  destroy_module(ctx);
  destroy_assert_funcs(ctx);
  free(ctx->remapped_locals);
  free(ctx->assert_funcs);
}

static void out_signature_generic(Context* ctx,
                                  WasmType result_type,
                                  WasmVariableVector* args,
                                  int num_args,
                                  int index,
                                  const char* desc) {
  int i;
  print_header(ctx, desc, index);
  out_u8(&ctx->buf, num_args, "num args");
  out_u8(&ctx->buf, wasm_type_to_v8_type(result_type), "result_type");
  for (i = 0; i < num_args; ++i)
    out_u8(&ctx->buf, wasm_type_to_v8_type(args->data[i].type), "arg type");
}

static void out_signature(Context* ctx,
                          WasmSignature* signature,
                          int index,
                          const char* desc) {
  out_signature_generic(ctx, signature->result_type, &signature->args,
                        signature->args.size, index, desc);
}

static void out_module_header(Context* ctx, WasmModule* module) {
  int i;
  OutputBuffer* buf = &ctx->buf;

  out_u8(buf, WASM_SECTION_MEMORY, "WASM_SECTION_MEMORY");
  out_u8(buf, log_two_u32(module->initial_memory_size), "min mem size log 2");
  out_u8(buf, log_two_u32(module->max_memory_size), "max mem size log 2");
  out_u8(buf, DEFAULT_MEMORY_EXPORT, "export mem");

  if (module->segments.size) {
    out_u8(buf, WASM_SECTION_DATA_SEGMENTS, "WASM_SECTION_DATA_SEGMENTS");
    out_leb128(buf, module->segments.size, "num data segments");
    ctx->segment_section_offset = ctx->buf.size;
    for (i = 0; i < module->segments.size; ++i) {
      WasmSegment* segment = &module->segments.data[i];
      print_header(ctx, "segment header", i);
      out_u32(buf, segment->address, "segment address");
      out_u32(buf, 0, "segment data offset");
      out_u32(buf, segment->size, "segment size");
      out_u8(buf, 1, "segment init");
    }
  }

  if (module->globals.size) {
    out_u8(buf, WASM_SECTION_GLOBALS, "WASM_SECTION_GLOBALS");
    out_leb128(buf, module->globals.size, "num globals");
    ctx->global_section_offset = ctx->buf.size;
    for (i = 0; i < module->globals.size; ++i) {
      WasmVariable* global = &module->globals.data[i];
      print_header(ctx, "global header", i);
      const uint8_t global_type_codes[WASM_NUM_V8_TYPES] = {-1, 4, 6, 8, 9};
      out_u32(buf, 0, "global name offset");
      out_u8(buf, global_type_codes[wasm_type_to_v8_type(global->type)],
             "global mem type");
      out_u8(buf, 0, "export global");
    }
  }

  uint32_t num_signatures = module->signatures.size;
  if (ctx->options->spec)
    num_signatures += WASM_NUM_V8_TYPES;

  out_u8(buf, WASM_SECTION_SIGNATURES, "WASM_SECTION_SIGNATURES");
  out_leb128(buf, num_signatures, "num signatures");
  ctx->signature_section_offset = ctx->buf.size;

  for (i = 0; i < module->signatures.size; ++i)
    out_signature(ctx, &module->signatures.data[i], i, "signature");

  if (ctx->options->spec) {
    /* Write out the signatures for various assert functions */
    WasmTypeV8 type;
    for (type = 0; type < WASM_NUM_V8_TYPES; ++type) {
      WasmSignatureIndex index = module->signatures.size + type;
      ctx->nullary_signature_index[type] = index;
      out_signature_generic(ctx, v8_type_to_wasm_type(type), NULL, 0, index,
                            "assert signature");
    }
  }

  int num_functions = module->imports.size + module->functions.size;
  if (num_functions) {
    ctx->function_pre_section_offset = ctx->buf.size;
    out_u8(buf, WASM_SECTION_FUNCTIONS, "WASM_SECTION_FUNCTIONS");
    out_leb128(buf, num_functions, "num functions");
    ctx->function_section_offset = ctx->buf.size;
    for (i = 0; i < module->imports.size; ++i) {
      WasmImport* import = &module->imports.data[i];
      print_header(ctx, "import header", i);
      WasmFunctionFlags flags =
          WASM_FUNCTION_FLAG_NAME | WASM_FUNCTION_FLAG_IMPORT;
      out_u8(buf, flags, "import flags");
      out_u16(buf, import->signature_index, "import signature index");
      out_u32(buf, 0, "import name offset");
    }
  }
}

static int has_i64_inputs_or_outputs(WasmFunction* function) {
  if (function->result_type == WASM_TYPE_I64) {
    return 1;
  } else {
    int i;
    for (i = 0; i < function->num_args; ++i) {
      if (function->locals.data[i].type == WASM_TYPE_I64) {
        return 1;
      }
    }
    return 0;
  }
}

static void out_module_footer(Context* ctx, WasmModule* module) {
  int i;
  size_t offset;
  OutputBuffer* buf = &ctx->buf;

  /* function table */
  if (module->function_table.size) {
    out_u8(buf, WASM_SECTION_FUNCTION_TABLE, "WASM_SECTION_FUNCTION_TABLE");
    out_leb128(buf, module->function_table.size, "num function table entries");
    for (i = 0; i < module->function_table.size; ++i) {
      WasmFunction* function = module->function_table.data[i];
      int index = function - module->functions.data;
      assert(index >= 0 && index < module->functions.size);
      out_u16(buf, index, "function table entry");
    }
  }

  out_u8(buf, WASM_SECTION_END, "WASM_SECTION_END");

  /* output assert function names. Do this first so we don't have to fixup any
   name/data offsets if we need to move data down */
  int num_functions = module->imports.size + module->functions.size;
  if (ctx->num_assert_funcs) {
    if (ctx->options->verbose)
      printf("; assert names\n");
    /* update the number of functions defined in the module to include the
     assert funcs */
    int old_num_functions = num_functions;
    num_functions = old_num_functions + ctx->num_assert_funcs;
    int old_leb128_size = leb128_size(old_num_functions);
    int new_leb128_size = leb128_size(num_functions);
    int diff_size = new_leb128_size - old_leb128_size;
    if (diff_size) {
      /* move everything down to make room for the longer function count */
      move_data(buf, ctx->function_section_offset + diff_size,
                ctx->function_section_offset,
                buf->size - ctx->function_section_offset);
      ctx->function_section_offset += diff_size;
    }
    out_leb128_at(buf, ctx->function_pre_section_offset + 1, num_functions,
                  "FIXUP num functions");

    for (i = 0; i < ctx->num_assert_funcs; ++i) {
      AssertFunc* assert_func = &ctx->assert_funcs[i];
      out_u32_at(buf, assert_func->name_offset + diff_size, buf->size,
                 "FIXUP func name offset");
      out_cstr(buf, assert_func->name, "export name");
    }
  }

  /* output segment data */
  offset = ctx->segment_section_offset;
  for (i = 0; i < module->segments.size; ++i) {
    print_header(ctx, "segment data", i);
    WasmSegment* segment = &module->segments.data[i];
    out_u32_at(buf, offset + SEGMENT_OFFSET_OFFSET, buf->size,
               "FIXUP segment data offset");
    out_segment(buf, segment, "segment data");
    offset += SEGMENT_SIZE;
  }

  /* output import names */
  if (ctx->options->verbose)
    printf("; names\n");
  offset = ctx->function_section_offset;
  for (i = 0; i < module->imports.size; ++i) {
    WasmImport* import = &module->imports.data[i];
    out_u32_at(buf, offset + IMPORT_NAME_OFFSET, buf->size,
               "FIXUP import name offset");
    out_cstr(buf, import->func_name, "import name");
    offset += IMPORT_SIZE;
  }

  /* output function names */
  for (i = 0; i < module->functions.size; ++i) {
    WasmFunction* function = &module->functions.data[i];
    uint8_t flags = read_u8_at(buf, offset + FUNCTION_FLAGS_OFFSET);
    size_t body_size_offset = FUNCTION_BODY_SIZE_OFFSET(flags);
    uint16_t body_size = read_u16_at(buf, offset + body_size_offset);

    if (function->exported) {
      out_u32_at(buf, offset + FUNCTION_NAME_OFFSET, buf->size,
                 "FIXUP func name offset");

      /* HACK(binji): v8-native-prototype crashes when you export functions
       that use i64 in the signature. This unfortunately prevents assert_return
       on functions that return i64. For now, don't mark those functions as
       exported in the generated output. */
      if (!has_i64_inputs_or_outputs(function)) {
        flags |= WASM_FUNCTION_FLAG_EXPORT;
        out_u8_at(buf, offset + FUNCTION_FLAGS_OFFSET, flags,
                  "FIXUP func exported");
      }

      /* TODO(binji): only exporting the first name for now,
       v8-native-prototype only allows associating one name per function. We
       could work around this by introducing a new function that forwards to
       the old one, or by using the same code start/end offsets. */
      out_cstr(buf, function->exported_name.name, "export name");
    }
    offset += body_size_offset + 2 + body_size;
  }
}

static void error(WasmParserCallbackInfo* info, const char* msg) {
  fprintf(stderr, "%s:%d:%d: %s", info->loc.source->filename, info->loc.line,
          info->loc.col, msg);
}

static void remap_locals(Context* ctx, WasmFunction* function) {
  int max[WASM_NUM_V8_TYPES] = {};
  int i;
  for (i = function->num_args; i < function->locals.size; ++i) {
    WasmVariable* variable = &function->locals.data[i];
    max[wasm_type_to_v8_type(variable->type)]++;
  }

  /* Args don't need remapping */
  for (i = 0; i < function->num_args; ++i)
    ctx->remapped_locals[i] = i;

  int start[WASM_NUM_V8_TYPES];
  start[WASM_TYPE_V8_I32] = function->num_args;
  start[WASM_TYPE_V8_I64] = start[WASM_TYPE_V8_I32] + max[WASM_TYPE_V8_I32];
  start[WASM_TYPE_V8_F32] = start[WASM_TYPE_V8_I64] + max[WASM_TYPE_V8_I64];
  start[WASM_TYPE_V8_F64] = start[WASM_TYPE_V8_F32] + max[WASM_TYPE_V8_F32];

  int seen[WASM_NUM_V8_TYPES] = {};
  for (i = function->num_args; i < function->locals.size; ++i) {
    WasmVariable* variable = &function->locals.data[i];
    WasmTypeV8 v8_type = wasm_type_to_v8_type(variable->type);
    ctx->remapped_locals[i] = start[v8_type] + seen[v8_type]++;
  }
}

static LabelInfo* push_label_info(Context* ctx, LabelType type) {
  LabelInfo* label_info = malloc(sizeof(LabelInfo));
  label_info->type = type;
  label_info->offset = ctx->buf.size;
  label_info->block_depth = ctx->block_depth++;
  label_info->next_label = ctx->top_label;
  label_info->with_expr = 0;
  ctx->top_label = label_info;
  return label_info;
}

static void pop_label_info(Context* ctx) {
  LabelInfo* label_info = ctx->top_label;
  ctx->block_depth--;
  ctx->top_label = label_info->next_label;
  free(label_info);
}

static void before_module(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  ctx->module = info->module;
  init_output_buffer(&ctx->buf, INITIAL_OUTPUT_BUFFER_CAPACITY,
                     ctx->options->verbose);
  out_module_header(ctx, info->module);
}

static void after_module(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  out_module_footer(ctx, info->module);
  if (ctx->options->dump_module)
    dump_output_buffer(&ctx->buf);
  destroy_module(ctx);
}

static void before_function(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  WasmModule* module = info->module;
  WasmFunction* function = info->function;
  ctx->remapped_locals =
      realloc(ctx->remapped_locals, function->locals.size * sizeof(int));
  remap_locals(ctx, function);

  int function_index = function - module->functions.data;
  print_header(ctx, "function", function_index);
  int has_locals = function->locals.size > function->num_args;
  /* Always write functions with a name. We don't want to have to do this, but
   if we don't, we won't have the space for the name offset if this function is
   later exported. */
  uint8_t flags = WASM_FUNCTION_FLAG_NAME;
  if (has_locals)
    flags |= WASM_FUNCTION_FLAG_LOCALS;
  out_u8(&ctx->buf, flags, "func flags");
  uint16_t signature_index = (uint16_t)function->signature_index;
  out_u16(&ctx->buf, signature_index, "func signature index");
  out_u32(&ctx->buf, 0, "func name offset");
  if (has_locals) {
    int num_locals[WASM_NUM_V8_TYPES] = {};
    int i;
    for (i = function->num_args; i < function->locals.size; ++i)
      num_locals[wasm_type_to_v8_type(function->locals.data[i].type)]++;

    out_u16(&ctx->buf, num_locals[WASM_TYPE_V8_I32], "num local i32");
    out_u16(&ctx->buf, num_locals[WASM_TYPE_V8_I64], "num local i64");
    out_u16(&ctx->buf, num_locals[WASM_TYPE_V8_F32], "num local f32");
    out_u16(&ctx->buf, num_locals[WASM_TYPE_V8_F64], "num local f64");
  }

  info->cookie = (WasmParserCookie)ctx->buf.size;
  out_u16(&ctx->buf, 0, "func body size");
}

static void after_function(WasmParserCallbackInfo* info, int num_exprs) {
  Context* ctx = info->user_data;
  size_t offset = (size_t)info->cookie;
  size_t body_size = ctx->buf.size - offset - sizeof(uint16_t);
  out_u16_at(&ctx->buf, offset, body_size, "FIXUP func body size");
}

static void before_block(WasmParserCallbackInfo* info, int with_label) {
  Context* ctx = info->user_data;
  /* TODO(binji): clean this up, it's a bit hacky right now. */
  if (with_label) {
    push_label_info(ctx, LABEL_TYPE_BLOCK);
    info->cookie = 0;
  } else {
    ctx->block_depth++;
    info->cookie = (WasmParserCookie)ctx->buf.size;
  }
  out_opcode(&ctx->buf, WASM_OPCODE_BLOCK);
  out_u8(&ctx->buf, 0, "num expressions");
}

static void after_block(WasmParserCallbackInfo* info,
                        WasmType type,
                        int num_exprs) {
  Context* ctx = info->user_data;
  uint32_t offset;
  if (info->cookie == 0) {
    offset = ctx->top_label->offset;
    pop_label_info(ctx);
  } else {
    ctx->block_depth--;
    offset = (uint32_t)info->cookie;
  }
  if (is_power_of_two(type))
    out_u8_at(&ctx->buf, offset, WASM_OPCODE_EXPR_BLOCK,
              "FIXUP OPCODE_EXPR_BLOCK");
  out_u8_at(&ctx->buf, offset + 1, num_exprs, "FIXUP num expressions");
}

static LabelInfo* label_info_at_depth(Context* ctx, int label_depth) {
  LabelInfo* label_info = ctx->top_label;
  for (; label_depth > 0; label_depth--)
    label_info = label_info->next_label;
  return label_info;
}

static int get_block_depth(Context* ctx, LabelInfo* label_info) {
  return (ctx->block_depth - 1) - label_info->block_depth;
}

static void before_br_if(WasmParserCallbackInfo* info, int label_depth) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_IF);
  info->cookie = (WasmParserCookie)label_depth;
}

static void after_br_if(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  int label_depth = (int)info->cookie;
  LabelInfo* label_info = label_info_at_depth(ctx, label_depth);
  WasmOpcode opcode = WASM_OPCODE_BREAK;
  /* In the br_if proposal, a branch to a loop continues the loop, instead of
   breaking. */
  if (ctx->options->br_if && label_info->type == LABEL_TYPE_LOOP_OUTER)
    opcode = WASM_OPCODE_CONTINUE;
  out_opcode(&ctx->buf, opcode);
  out_u8(&ctx->buf, get_block_depth(ctx, label_info), "break depth");
}

static void before_binary(WasmParserCallbackInfo* info,
                          enum WasmOpcode opcode) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, opcode);
}

static void before_break(WasmParserCallbackInfo* info,
                         int with_expr,
                         int label_depth) {
  Context* ctx = info->user_data;
  LabelInfo* label_info = label_info_at_depth(ctx, label_depth);
  label_info->with_expr = label_info->with_expr || with_expr;
  WasmOpcode opcode = with_expr ? WASM_OPCODE_EXPR_BREAK : WASM_OPCODE_BREAK;
  /* In the br_if proposal, a branch to a loop continues the loop, instead of
   breaking. */
  if (ctx->options->br_if && label_info->type == LABEL_TYPE_LOOP_OUTER)
    opcode = WASM_OPCODE_CONTINUE;
  out_opcode(&ctx->buf, opcode);
  out_u8(&ctx->buf, get_block_depth(ctx, label_info), "break depth");
}

static void before_call(WasmParserCallbackInfo* info, int function_index) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_CALL);
  /* defined functions are always after all imports */
  out_leb128(&ctx->buf, info->module->imports.size + function_index,
             "func index");
}

static void before_call_import(WasmParserCallbackInfo* info, int import_index) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_CALL);
  out_leb128(&ctx->buf, import_index, "import index");
}

static void before_call_indirect(WasmParserCallbackInfo* info,
                                 int signature_index) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_CALL_INDIRECT);
  out_leb128(&ctx->buf, signature_index, "signature index");
}

static void before_compare(WasmParserCallbackInfo* info,
                           enum WasmOpcode opcode) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, opcode);
}

static void after_const(WasmParserCallbackInfo* info,
                        enum WasmOpcode opcode,
                        WasmType type,
                        WasmNumber value) {
  Context* ctx = info->user_data;
  switch (type) {
    case WASM_TYPE_I32: {
      int32_t i32 = value.i32;
      if (i32 >= -128 && i32 < 127) {
        out_opcode(&ctx->buf, WASM_OPCODE_I8_CONST);
        out_u8(&ctx->buf, value.i32, "u8 literal");
      } else {
        out_opcode(&ctx->buf, opcode);
        out_u32(&ctx->buf, value.i32, "u32 literal");
      }
      break;
    }

    case WASM_TYPE_I64:
      out_opcode(&ctx->buf, opcode);
      out_u64(&ctx->buf, value.i64, "u64 literal");
      break;

    case WASM_TYPE_F32:
      out_opcode(&ctx->buf, opcode);
      out_f32(&ctx->buf, value.f32, "f32 literal");
      break;

    case WASM_TYPE_F64:
      out_opcode(&ctx->buf, opcode);
      out_f64(&ctx->buf, value.f64, "f64 literal");
      break;

    default:
      assert(0);
      break;
  }
}

static void before_convert(WasmParserCallbackInfo* info,
                           enum WasmOpcode opcode) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, opcode);
}

static void after_get_local(WasmParserCallbackInfo* info, int index) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_GET_LOCAL);
  out_leb128(&ctx->buf, ctx->remapped_locals[index], "remapped local index");
}

static void before_if(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  uint32_t offset = ctx->buf.size;
  out_opcode(&ctx->buf, WASM_OPCODE_IF);
  info->cookie = (WasmParserCookie)offset;
}

static void after_if(WasmParserCallbackInfo* info,
                     WasmType type,
                     int with_else) {
  Context* ctx = info->user_data;
  if (with_else) {
    uint32_t offset = (uint32_t)info->cookie;
    WasmOpcode opcode;
    const char* desc;
    if (is_power_of_two(type)) {
      opcode = WASM_OPCODE_EXPR_IF;
      desc = "FIXUP OPCODE_EXPR_IF";
    } else {
      opcode = WASM_OPCODE_IF_THEN;
      desc = "FIXUP OPCODE_IF_THEN";
    }
    out_u8_at(&ctx->buf, offset, opcode, desc);
  }
}

static void before_label(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  push_label_info(ctx, LABEL_TYPE_LABEL);
  out_opcode(&ctx->buf, WASM_OPCODE_BLOCK);
  out_u8(&ctx->buf, 1, "num expressions");
}

static void after_label(WasmParserCallbackInfo* info, WasmType type) {
  Context* ctx = info->user_data;
  uint32_t offset = ctx->top_label->offset;
  pop_label_info(ctx);
  if (is_power_of_two(type))
    out_u8_at(&ctx->buf, offset, WASM_OPCODE_EXPR_BLOCK,
              "FIXUP OPCODE_EXPR_BLOCK");
}

static void before_load(WasmParserCallbackInfo* info,
                        enum WasmOpcode opcode,
                        WasmMemType mem_type,
                        uint32_t alignment,
                        uint64_t offset,
                        int is_signed) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, opcode);
  uint8_t access = 0;
  switch (mem_type) {
    case WASM_MEM_TYPE_I8:
      access = is_signed ? 4 : 0;
      break;
    case WASM_MEM_TYPE_I16:
      access = is_signed ? 5 : 1;
      break;
    case WASM_MEM_TYPE_I32:
      if (opcode == WASM_OPCODE_I64_LOAD_I32)
        access = is_signed ? 6 : 2;
      else
        access = 6;
      break;
    case WASM_MEM_TYPE_I64:
      access = 7;
      break;
    default:
      break;
  }
  out_u8(&ctx->buf, access, "load access byte");
}

static void after_load_global(WasmParserCallbackInfo* info, int index) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_GET_GLOBAL);
  out_leb128(&ctx->buf, index, "global index");
}

static void before_loop(WasmParserCallbackInfo* info,
                        int with_inner_label,
                        int with_outer_label) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_LOOP);
  /* TODO(binji): clean this up, it's a bit hacky right now. */
  if (with_inner_label || with_outer_label) {
    /* The outer label must be defined if the inner label is defined */
    assert(with_outer_label);
    push_label_info(ctx, LABEL_TYPE_LOOP_OUTER);
    if (with_inner_label) {
      /* The loop only has one expression: this nested block */
      out_u8(&ctx->buf, 1, "num expressions");
      out_opcode(&ctx->buf, WASM_OPCODE_BLOCK);
      push_label_info(ctx, LABEL_TYPE_LOOP_INNER);
      out_u8(&ctx->buf, 0, "num expressions");
      info->cookie = 1;
    } else {
      out_u8(&ctx->buf, 0, "num expressions");
      info->cookie = 0;
    }
  } else {
    info->cookie = (WasmParserCookie)ctx->buf.size;
    out_u8(&ctx->buf, 0, "num expressions");
    ctx->block_depth++;
  }
}

static void after_loop(WasmParserCallbackInfo* info, int num_exprs) {
  Context* ctx = info->user_data;
  if (ctx->options->br_if) {
    /* In the br_if proposal, loops don't implicitly branch to the top, so to
     mimic this behavior we have to break at the end of every loop. */
    ++num_exprs;
    out_opcode(&ctx->buf, WASM_OPCODE_BREAK);
    out_u8(&ctx->buf, 0, "break out of loop");
  }

  if (info->cookie == 0 || info->cookie == 1) {
    LabelInfo* label_info = ctx->top_label;
    out_u8_at(&ctx->buf, label_info->offset, num_exprs,
              "FIXUP num expressions");
    if (info->cookie == 1) {
      /* inner and outer label */
      if (label_info->with_expr)
        out_u8_at(&ctx->buf, label_info->offset - 1, WASM_OPCODE_EXPR_BLOCK,
                  "FIXUP OPCODE_EXPR_BLOCK");
      pop_label_info(ctx);
    }
    pop_label_info(ctx);
  } else {
    /* no labels */
    ctx->block_depth--;
    uint32_t offset = (uint32_t)info->cookie;
    out_u8_at(&ctx->buf, offset, num_exprs, "FIXUP num expressions");
  }
}

static void after_memory_size(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  /* TODO(binji): not currently defined. Return 0 for now. */
  out_opcode(&ctx->buf, WASM_OPCODE_I8_CONST);
  out_u8(&ctx->buf, 0, "zero");
}

static void after_nop(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_NOP);
}

static void after_page_size(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_PAGE_SIZE);
}

static void before_grow_memory(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_RESIZE_MEMORY_I32);
}

static void before_return(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_RETURN);
}

static void before_set_local(WasmParserCallbackInfo* info, int index) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_SET_LOCAL);
  out_leb128(&ctx->buf, ctx->remapped_locals[index], "remapped local index");
}

static void before_store(WasmParserCallbackInfo* info,
                         enum WasmOpcode opcode,
                         WasmMemType mem_type,
                         uint32_t alignment,
                         uint64_t offset) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, opcode);
  uint8_t access = 0;
  switch (mem_type) {
    case WASM_MEM_TYPE_I8:
      access = 4;
      break;
    case WASM_MEM_TYPE_I16:
      access = 5;
      break;
    case WASM_MEM_TYPE_I32:
      access = 6;
      break;
    case WASM_MEM_TYPE_I64:
      access = 7;
      break;
    default:
      break;
  }
  out_u8(&ctx->buf, access, "store access byte");
}

static void before_store_global(WasmParserCallbackInfo* info, int index) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_SET_GLOBAL);
  out_leb128(&ctx->buf, index, "global index");
}

static void before_unary(WasmParserCallbackInfo* info, enum WasmOpcode opcode) {
  Context* ctx = info->user_data;
  out_opcode(&ctx->buf, opcode);
}

static void js_file_start(Context* ctx) {
  const char* quiet_str = ctx->options->spec_verbose ? "false" : "true";
  out_printf(&ctx->js_buf, "var quiet = %s;\n", quiet_str);
}

static void js_file_end(Context* ctx) {
  out_printf(&ctx->js_buf, "end();\n");
}

static void before_module_multi(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  out_printf(&ctx->js_buf, "var tests = function(m) {\n");
  before_module(info);
}

static void before_module_destroy(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  if (!ctx->buf.size)
    return;

  after_module(info);
  out_printf(&ctx->js_buf, "};\nvar m = createModule([\n");
  OutputBuffer* module_buf = &ctx->buf;
  const uint8_t* p = module_buf->start;
  const uint8_t* end = module_buf->start + module_buf->size;
  while (p < end) {
    out_printf(&ctx->js_buf, " ");
    const uint8_t* line_end = p + DUMP_OCTETS_PER_LINE;
    for (; p < line_end; ++p)
      out_printf(&ctx->js_buf, "%4d,", *p);
    out_printf(&ctx->js_buf, "\n");
  }
  out_printf(&ctx->js_buf, "]);\ntests(m);\n");
}

static void append_assert_function(Context* ctx,
                                   const char* name,
                                   uint32_t name_offset) {
  ctx->assert_funcs = realloc(ctx->assert_funcs,
                              (ctx->num_assert_funcs + 1) * sizeof(AssertFunc));
  AssertFunc* assert_func = &ctx->assert_funcs[ctx->num_assert_funcs++];
  memset(assert_func, 0, sizeof(*assert_func));
  assert_func->name = strdup(name);
  assert_func->name_offset = name_offset;
}

static void before_assert_return(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  ctx->in_assert = 1;
  print_header(ctx, "assert_return", ctx->num_assert_funcs);
  out_u8(&ctx->buf, WASM_FUNCTION_FLAG_NAME | WASM_FUNCTION_FLAG_EXPORT,
         "func flags");
  out_u16(&ctx->buf, ctx->nullary_signature_index[WASM_TYPE_V8_I32],
          "func signature index");
  uint32_t name_offset = ctx->buf.size;
  out_u32(&ctx->buf, 0, "func name offset");

  uint32_t body_size_offset = ctx->buf.size;
  out_u16(&ctx->buf, 0, "func body size");
  out_opcode(&ctx->buf, WASM_OPCODE_I32_EQ);

  char name[256];
  snprintf(name, 256, "$assert_return_%d", ctx->num_assert_funcs);
  append_assert_function(ctx, name, name_offset);

  out_printf(&ctx->js_buf, "  assertReturn(m, \"%s\", \"%s\", %d);\n", name,
             info->loc.source->filename, info->loc.line);

  info->cookie = (WasmParserCookie)body_size_offset;
}

static void after_assert_return(WasmParserCallbackInfo* info, WasmType type) {
  Context* ctx = info->user_data;
  WasmOpcode opcode;
  switch (type) {
    case WASM_TYPE_I32:
      opcode = WASM_OPCODE_I32_EQ;
      break;
    case WASM_TYPE_I64:
      opcode = WASM_OPCODE_I64_EQ;
      break;
    case WASM_TYPE_F32:
      opcode = WASM_OPCODE_F32_EQ;
      break;
    case WASM_TYPE_F64:
      opcode = WASM_OPCODE_F64_EQ;
      break;
    default: {
      opcode = WASM_OPCODE_NOP;
      /* The return type of the assert_return function is i32, but this invoked
       function has a return type of void, so we have nothing to compare to.
       Just return 1 to the caller, signifying everything is OK. */
      out_opcode(&ctx->buf, WASM_OPCODE_I8_CONST);
      out_u8(&ctx->buf, 1, "u8 literal");
      break;
    }
  }

  uint32_t body_size_offset = (uint32_t)info->cookie;
  size_t body_size = ctx->buf.size - body_size_offset - 2;
  out_u16_at(&ctx->buf, body_size_offset, body_size, "FIXUP body size");
  out_u8_at(&ctx->buf, body_size_offset + 2, opcode,
            "FIXUP assert_return opcode");
  ctx->in_assert = 0;
}

static void before_assert_return_nan(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  ctx->in_assert = 1;
  print_header(ctx, "assert_return_nan", ctx->num_assert_funcs);
  out_u8(&ctx->buf, WASM_FUNCTION_FLAG_NAME | WASM_FUNCTION_FLAG_EXPORT |
                        WASM_FUNCTION_FLAG_LOCALS,
         "func flags");
  out_u16(&ctx->buf, ctx->nullary_signature_index[WASM_TYPE_V8_I32],
          "func signature index");
  uint32_t name_offset = ctx->buf.size;
  out_u32(&ctx->buf, 0, "func name offset");
  out_u16(&ctx->buf, 0, "func num local i32");
  out_u16(&ctx->buf, 0, "func num local i64");

  uint32_t num_local_f32_offset = ctx->buf.size;
  out_u16(&ctx->buf, 0, "func num local f32");
  out_u16(&ctx->buf, 0, "func num local f64");
  out_u16(&ctx->buf, 0, "func body size");

  out_opcode(&ctx->buf, WASM_OPCODE_SET_LOCAL);
  out_u8(&ctx->buf, 0, "remapped local index");

  char name[256];
  snprintf(name, 256, "$assert_return_nan_%d", ctx->num_assert_funcs);
  append_assert_function(ctx, name, name_offset);

  out_printf(&ctx->js_buf, "  assertReturn(m, \"%s\", \"%s\", %d);\n", name,
             info->loc.source->filename, info->loc.line);

  info->cookie = (WasmParserCookie)num_local_f32_offset;
}

static void after_assert_return_nan(WasmParserCallbackInfo* info,
                                    WasmType type) {
  Context* ctx = info->user_data;

  uint32_t num_local_f32_offset = (uint32_t)info->cookie;
  switch (type) {
    case WASM_TYPE_F32:
      out_u8_at(&ctx->buf, num_local_f32_offset, 1, "FIXUP num local f32");
      out_opcode(&ctx->buf, WASM_OPCODE_F32_NE);
      break;
    case WASM_TYPE_F64:
      out_u8_at(&ctx->buf, num_local_f32_offset + 2, 1, "FIXUP num local f32");
      out_opcode(&ctx->buf, WASM_OPCODE_F64_NE);
      break;
    default:
      assert(0);
  }

  /* x != x is true iff x is NaN */
  out_opcode(&ctx->buf, WASM_OPCODE_GET_LOCAL);
  out_u8(&ctx->buf, 0, "remapped local index");
  out_opcode(&ctx->buf, WASM_OPCODE_GET_LOCAL);
  out_u8(&ctx->buf, 0, "remapped local index");

  uint32_t body_size_offset = num_local_f32_offset + 4;
  size_t body_size = ctx->buf.size - body_size_offset - 2;
  out_u16_at(&ctx->buf, body_size_offset, body_size, "FIXUP body size");
  ctx->in_assert = 0;
}

static void before_assert_trap(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  ctx->in_assert = 1;
  print_header(ctx, "assert_trap", ctx->num_assert_funcs);

  out_u8(&ctx->buf, WASM_FUNCTION_FLAG_NAME | WASM_FUNCTION_FLAG_EXPORT,
         "func flags");
  out_u16(&ctx->buf, ctx->nullary_signature_index[WASM_TYPE_V8_VOID],
          "func signature index");
  uint32_t name_offset = ctx->buf.size;
  out_u32(&ctx->buf, 0, "func name offset");
  uint32_t body_size_offset = ctx->buf.size;
  out_u16(&ctx->buf, 0, "func body size");

  char name[256];
  snprintf(name, 256, "$assert_trap_%d", ctx->num_assert_funcs);
  append_assert_function(ctx, name, name_offset);

  out_printf(&ctx->js_buf, "  assertTrap(m, \"%s\", \"%s\", %d);\n", name,
             info->loc.source->filename, info->loc.line);

  info->cookie = (WasmParserCookie)body_size_offset;
}

static void after_assert_trap(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  uint32_t body_size_offset = (uint32_t)info->cookie;
  size_t body_size = ctx->buf.size - body_size_offset - 2;
  out_u16_at(&ctx->buf, body_size_offset, body_size, "FIXUP body size");
  ctx->in_assert = 0;
}

static void before_invoke(WasmParserCallbackInfo* info,
                          const char* invoke_name,
                          int invoke_function_index) {
  Context* ctx = info->user_data;
  char name[256] = {};
  uint32_t body_size_offset = 0;

  if (!ctx->in_assert) {
    WasmFunction* function =
        &ctx->module->functions.data[invoke_function_index];

    print_header(ctx, "invoke", ctx->num_assert_funcs);
    out_u8(&ctx->buf, WASM_FUNCTION_FLAG_NAME | WASM_FUNCTION_FLAG_EXPORT,
           "func flags");
    out_u16(&ctx->buf, ctx->nullary_signature_index[wasm_type_to_v8_type(
                           function->result_type)],
            "func signature index");
    uint32_t name_offset = ctx->buf.size;
    out_u32(&ctx->buf, 0, "func name offset");
    body_size_offset = ctx->buf.size;
    out_u16(&ctx->buf, 0, "func body size");

    snprintf(name, 256, "$invoke_%d", ctx->num_assert_funcs);
    append_assert_function(ctx, name, name_offset);
  }

  out_opcode(&ctx->buf, WASM_OPCODE_CALL);
  /* defined functions are always after all imports */
  out_leb128(&ctx->buf, ctx->module->imports.size + invoke_function_index,
             "invoke func index");

  if (!ctx->in_assert) {
    out_printf(&ctx->js_buf, "  invoke(m, \"%s\");\n", name);
    info->cookie = (WasmParserCookie)body_size_offset;
  }
}

static void after_invoke(WasmParserCallbackInfo* info) {
  Context* ctx = info->user_data;
  if (ctx->in_assert)
    return;

  uint32_t body_size_offset = (uint32_t)info->cookie;
  size_t body_size = ctx->buf.size - body_size_offset - 2;
  out_u16_at(&ctx->buf, body_size_offset, body_size, "FIXUP body size");
}

static void assert_invalid_error(WasmParserCallbackInfo* info,
                                 const char* msg) {
  fprintf(stdout, "assert_invalid error:\n  %s:%d:%d: %s",
          info->loc.source->filename, info->loc.line, info->loc.col, msg);
}

int wasm_gen_file(WasmSource* source, WasmGenOptions* options) {
  Context ctx = {};
  ctx.options = options;

  WasmParserCallbacks callbacks = {};
  callbacks.user_data = &ctx;
  callbacks.error = error;
  callbacks.before_module = before_module;
  callbacks.after_module = after_module;
  callbacks.before_function = before_function;
  callbacks.after_function = after_function;
  callbacks.before_block = before_block;
  callbacks.after_block = after_block;
  callbacks.before_br_if = before_br_if;
  callbacks.after_br_if = after_br_if;
  callbacks.before_binary = before_binary;
  callbacks.before_break = before_break;
  callbacks.before_call = before_call;
  callbacks.before_call_import = before_call_import;
  callbacks.before_call_indirect = before_call_indirect;
  callbacks.before_compare = before_compare;
  callbacks.after_const = after_const;
  callbacks.before_convert = before_convert;
  callbacks.after_get_local = after_get_local;
  callbacks.before_if = before_if;
  callbacks.after_if = after_if;
  callbacks.before_label = before_label;
  callbacks.after_label = after_label;
  callbacks.before_load = before_load;
  callbacks.after_load_global = after_load_global;
  callbacks.before_loop = before_loop;
  callbacks.after_loop = after_loop;
  callbacks.after_memory_size = after_memory_size;
  callbacks.after_nop = after_nop;
  callbacks.after_page_size = after_page_size;
  callbacks.before_grow_memory = before_grow_memory;
  callbacks.before_return = before_return;
  callbacks.before_set_local = before_set_local;
  callbacks.before_store = before_store;
  callbacks.before_store_global = before_store_global;
  callbacks.before_unary = before_unary;
  callbacks.assert_invalid_error = assert_invalid_error;

  WasmParserOptions parser_options = {};
  parser_options.br_if = options->br_if;

  int result;
  if (options->spec) {
    if (options->outfile) {
      callbacks.before_module = before_module_multi;
      callbacks.after_module = NULL;
      callbacks.before_module_destroy = before_module_destroy;
      callbacks.before_assert_return = before_assert_return;
      callbacks.after_assert_return = after_assert_return;
      callbacks.before_assert_return_nan = before_assert_return_nan;
      callbacks.after_assert_return_nan = after_assert_return_nan;
      callbacks.before_assert_trap = before_assert_trap;
      callbacks.after_assert_trap = after_assert_trap;
      callbacks.before_invoke = before_invoke;
      callbacks.after_invoke = after_invoke;
      init_output_buffer(&ctx.js_buf, INITIAL_OUTPUT_BUFFER_CAPACITY,
                         options->verbose);
      js_file_start(&ctx);
    }
    result = wasm_parse_file(source, &callbacks, &parser_options);
    if (result== 0 && options->outfile) {
      js_file_end(&ctx);
      write_output_buffer(&ctx.js_buf, options->outfile);
    }
  } else {
    result = wasm_parse_module(source, &callbacks, &parser_options);
    if (result == 0 && options->outfile)
      write_output_buffer(&ctx.buf, options->outfile);
  }

  destroy_context(&ctx);
  return result;
}

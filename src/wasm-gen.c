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

#define GLOBAL_HEADERS_OFFSET 8
#define GLOBAL_HEADER_SIZE 6
#define FUNC_HEADERS_OFFSET(num_globals) \
  (GLOBAL_HEADERS_OFFSET + (num_globals)*GLOBAL_HEADER_SIZE)
#define FUNC_HEADER_SIZE(num_args) (24 + (num_args))
#define SEGMENT_HEADER_SIZE 13

/* offsets from the start of a global header */
#define GLOBAL_HEADER_NAME_OFFSET 0

/* offsets from the start of a function header */
#define FUNC_SIG_SIZE(num_args) (2 + (num_args))
#define FUNC_HEADER_RESULT_TYPE_OFFSET 1
#define FUNC_HEADER_NAME_OFFSET(num_args) (FUNC_SIG_SIZE(num_args) + 0)
#define FUNC_HEADER_CODE_START_OFFSET(num_args) (FUNC_SIG_SIZE(num_args) + 4)
#define FUNC_HEADER_CODE_END_OFFSET(num_args) (FUNC_SIG_SIZE(num_args) + 8)
#define FUNC_HEADER_NUM_LOCAL_I32_OFFSET(num_args) \
  (FUNC_SIG_SIZE(num_args) + 12)
#define FUNC_HEADER_NUM_LOCAL_I64_OFFSET(num_args) \
  (FUNC_SIG_SIZE(num_args) + 14)
#define FUNC_HEADER_NUM_LOCAL_F32_OFFSET(num_args) \
  (FUNC_SIG_SIZE(num_args) + 16)
#define FUNC_HEADER_NUM_LOCAL_F64_OFFSET(num_args) \
  (FUNC_SIG_SIZE(num_args) + 18)
#define FUNC_HEADER_EXPORTED_OFFSET(num_args) (FUNC_SIG_SIZE(num_args) + 20)

/* offsets from the start of a segment header */
#define SEGMENT_HEADER_DATA_OFFSET 4

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

typedef struct LabelInfo {
  struct LabelInfo* next_label;
  uint32_t offset;
  int block_depth;
  int with_expr;
} LabelInfo;

typedef struct Context {
  WasmGenOptions* options;
  OutputBuffer buf;
  OutputBuffer temp_buf;
  OutputBuffer js_buf;
  WasmModule* module;
  /* offset of each function/segment header in buf. */
  uint32_t* function_header_offsets;
  uint32_t* segment_header_offsets;
  LabelInfo* top_label;
  int* remapped_locals;
  int in_assert;
  int block_depth;
  uint32_t function_num_exprs_offset;
  int assert_return_count;
  int assert_return_nan_count;
  int assert_trap_count;
  int invoke_count;
  int in_js_module;
} Context;

static const char* s_opcode_names[] = {
#define OPCODE(name, code) [code] = "OPCODE_" #name,
    OPCODES(OPCODE)
#undef OPCODE
};

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
  assert(dest_offset + size <= buf->size);
  assert(src_offset + size <= buf->size);
  memmove(buf->start + dest_offset, buf->start + src_offset, size);
  if (buf->log_writes) {
    printf("; move [%07zx,%07zx) -> [%07zx,%07zx)\n", src_offset,
           src_offset + size, dest_offset, dest_offset + size);
  }
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
READ_AT_TYPE(u32, uint32_t)

#undef READ_AT_TYPE

static void add_u32_at(OutputBuffer* buf,
                       size_t offset,
                       uint32_t addend,
                       const char* desc) {
  out_u32_at(buf, offset, read_u32_at(buf, offset) + addend, desc);
}

void out_opcode(OutputBuffer* buf, WasmOpcode opcode) {
  out_u8(buf, (uint8_t)opcode, s_opcode_names[opcode]);
}

void out_leb128(OutputBuffer* buf, uint32_t value, const char* desc) {
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
  buf->size = out_data(buf, buf->size, data, i, desc);
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

static void destroy_context(Context* ctx) {
  LabelInfo* label = ctx->top_label;
  while (label) {
    LabelInfo* next_label = label->next_label;
    free(label);
    label = next_label;
  }
  destroy_output_buffer(&ctx->buf);
  destroy_output_buffer(&ctx->temp_buf);
  destroy_output_buffer(&ctx->js_buf);
  free(ctx->remapped_locals);
  free(ctx->function_header_offsets);
  free(ctx->segment_header_offsets);
}

static void out_module_header(Context* ctx,
                              WasmModule* module) {
  OutputBuffer* buf = &ctx->buf;
  out_u8(buf, log_two_u32(module->max_memory_size), "mem size log 2");
  out_u8(buf, DEFAULT_MEMORY_EXPORT, "export mem");
  out_u16(buf, module->globals.size, "num globals");
  out_u16(buf, module->imports.size + module->functions.size, "num funcs");
  out_u16(buf, module->segments.size, "num data segments");

  int i;
  for (i = 0; i < module->globals.size; ++i) {
    WasmVariable* global = &module->globals.data[i];
    if (ctx->options->verbose)
      printf("; global header %d\n", i);
    const uint8_t global_type_codes[WASM_NUM_V8_TYPES] = {-1, 4, 6, 8, 9};
    out_u32(buf, 0, "global name offset");
    out_u8(buf, global_type_codes[wasm_type_to_v8_type(global->type)],
           "global mem type");
    out_u8(buf, 0, "export global");
  }

  for (i = 0; i < module->imports.size; ++i) {
    WasmImport* import = &module->imports.data[i];
    if (ctx->options->verbose)
      printf("; import header %d\n", i);

    out_u8(buf, import->args.size, "import num args");
    out_u8(buf, wasm_type_to_v8_type(import->result_type),
           "import result_type");

    int j;
    for (j = 0; j < import->args.size; ++j)
      out_u8(buf, wasm_type_to_v8_type(import->args.data[j].type),
             "import arg type");

    out_u32(buf, 0, "import name offset");
    out_u32(buf, 0, "import code start offset");
    out_u32(buf, 0, "import code end offset");
    out_u16(buf, 0, "num local i32");
    out_u16(buf, 0, "num local i64");
    out_u16(buf, 0, "num local f32");
    out_u16(buf, 0, "num local f64");
    out_u8(buf, 0, "export func");
    out_u8(buf, 1, "import external");
  }

  for (i = 0; i < module->functions.size; ++i) {
    WasmFunction* function = &module->functions.data[i];
    if (ctx->options->verbose)
      printf("; function header %d\n", i);

    out_u8(buf, function->num_args, "func num args");
    out_u8(buf, wasm_type_to_v8_type(function->result_type),
           "func result type");
    int j;
    for (j = 0; j < function->num_args; ++j)
      out_u8(buf, wasm_type_to_v8_type(function->locals.data[j].type),
             "func arg type");
    out_u32(buf, 0, "func name offset");
    out_u32(buf, 0, "func code start offset");
    out_u32(buf, 0, "func code end offset");

    int num_locals[WASM_NUM_V8_TYPES] = {};
    for (j = function->num_args; j < function->locals.size; ++j)
      num_locals[wasm_type_to_v8_type(function->locals.data[j].type)]++;

    out_u16(buf, num_locals[WASM_TYPE_V8_I32], "num local i32");
    out_u16(buf, num_locals[WASM_TYPE_V8_I64], "num local i64");
    out_u16(buf, num_locals[WASM_TYPE_V8_F32], "num local f32");
    out_u16(buf, num_locals[WASM_TYPE_V8_F64], "num local f64");
    out_u8(buf, 0, "export func");
    out_u8(buf, 0, "func external");
  }

  for (i = 0; i < module->segments.size; ++i) {
    WasmSegment* segment = &module->segments.data[i];
    if (ctx->options->verbose)
      printf("; segment header %d\n", i);
    out_u32(buf, segment->address, "segment address");
    out_u32(buf, 0, "segment data offset");
    out_u32(buf, segment->size, "segment size");
    out_u8(buf, 1, "segment init");
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
  OutputBuffer* buf = &ctx->buf;

  int i;
  for (i = 0; i < module->segments.size; ++i) {
    if (ctx->options->verbose)
      printf("; segment data %d\n", i);
    WasmSegment* segment = &module->segments.data[i];
    out_u32_at(buf, ctx->segment_header_offsets[i] + SEGMENT_HEADER_DATA_OFFSET,
               buf->size, "FIXUP segment data offset");
    out_segment(buf, segment, "segment data");
  }

  /* output name table */
  if (ctx->options->verbose)
    printf("; names\n");
  uint32_t offset = FUNC_HEADERS_OFFSET(module->globals.size);
  for (i = 0; i < module->imports.size; ++i) {
    WasmImport* import = &module->imports.data[i];
    out_u32_at(buf, offset + FUNC_HEADER_NAME_OFFSET(import->args.size),
               buf->size, "FIXUP import name offset");
    out_cstr(buf, import->func_name, "import name");
    offset += FUNC_HEADER_SIZE(import->args.size);
  }
  for (i = 0; i < module->functions.size; ++i) {
    WasmFunction* function = &module->functions.data[i];
    if (function->exported) {
      out_u32_at(buf, offset + FUNC_HEADER_NAME_OFFSET(function->num_args),
                 buf->size, "FIXUP func name offset");

      /* HACK(binji): v8-native-prototype crashes when you export functions
       that use i64 in the signature. This unfortunately prevents assert_return on
       functions that return i64. For now, don't mark those functions as
       exported in the generated output. */
      if (!has_i64_inputs_or_outputs(function)) {
        out_u8_at(&ctx->buf,
                  ctx->function_header_offsets[i] +
                      FUNC_HEADER_EXPORTED_OFFSET(function->num_args),
                  1, "FIXUP func exported");
      }

      /* TODO(binji): only exporting the first name for now,
       v8-native-prototype only allows associating one name per function. We
       could work around this by introducing a new function that forwards to
       the old one, or by using the same code start/end offsets. */
      out_cstr(buf, function->exported_name.name, "export name");
    }
    offset += FUNC_HEADER_SIZE(function->num_args);
  }
}

static void error(WasmSourceLocation loc, const char* msg, void* user_data) {
  fprintf(stderr, "%s:%d:%d: %s", loc.source->filename, loc.line, loc.col, msg);
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

static LabelInfo* push_label_info(Context* ctx) {
  LabelInfo* label_info = malloc(sizeof(LabelInfo));
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

static void before_module(WasmModule* module, void* user_data) {
  Context* ctx = user_data;
  ctx->function_header_offsets = realloc(
      ctx->function_header_offsets, module->functions.size * sizeof(uint32_t));
  int i;
  uint32_t offset = FUNC_HEADERS_OFFSET(module->globals.size);
  /* skip past the import headers */
  for (i = 0; i < module->imports.size; ++i)
    offset += FUNC_HEADER_SIZE(module->imports.data[i].args.size);

  for (i = 0; i < module->functions.size; ++i) {
    WasmFunction* function = &module->functions.data[i];
    ctx->function_header_offsets[i] = offset;
    offset += FUNC_HEADER_SIZE(function->num_args);
  }

  ctx->segment_header_offsets = realloc(
      ctx->segment_header_offsets, module->segments.size * sizeof(uint32_t));
  for (i = 0; i < module->segments.size; ++i) {
    ctx->segment_header_offsets[i] = offset;
    offset += SEGMENT_HEADER_SIZE;
  }

  ctx->module = module;
  init_output_buffer(&ctx->buf, INITIAL_OUTPUT_BUFFER_CAPACITY,
                     ctx->options->verbose);
  out_module_header(ctx, module);
}

static void after_module(WasmModule* module, void* user_data) {
  Context* ctx = user_data;
  out_module_footer(ctx, module);
  if (ctx->options->dump_module)
    dump_output_buffer(&ctx->buf);
}

static void before_function(WasmModule* module,
                            WasmFunction* function,
                            void* user_data) {
  Context* ctx = user_data;
  ctx->remapped_locals =
      realloc(ctx->remapped_locals, function->locals.size * sizeof(int));
  remap_locals(ctx, function);

  int function_index = function - module->functions.data;
  if (ctx->options->verbose)
    printf("; function data %d\n", function_index);
  out_u32_at(&ctx->buf, ctx->function_header_offsets[function_index] +
                            FUNC_HEADER_CODE_START_OFFSET(function->num_args),
             ctx->buf.size, "FIXUP func code start offset");
}

static void after_function(WasmModule* module,
                           WasmFunction* function,
                           int num_exprs,
                           void* user_data) {
  Context* ctx = user_data;
  int function_index = function - module->functions.data;
  out_u32_at(&ctx->buf, ctx->function_header_offsets[function_index] +
                            FUNC_HEADER_CODE_END_OFFSET(function->num_args),
             ctx->buf.size, "FIXUP func code end offset");
}

static WasmParserCookie before_block(int with_label, void* user_data) {
  Context* ctx = user_data;
  WasmParserCookie cookie = 0;
  /* TODO(binji): clean this up, it's a bit hacky right now. */
  if (with_label) {
    push_label_info(ctx);
  } else {
    ctx->block_depth++;
    cookie = (WasmParserCookie)ctx->buf.size;
  }
  out_opcode(&ctx->buf, WASM_OPCODE_BLOCK);
  out_u8(&ctx->buf, 0, "num expressions");
  return cookie;
}

static void after_block(WasmType type,
                        int num_exprs,
                        WasmParserCookie cookie,
                        void* user_data) {
  Context* ctx = user_data;
  uint32_t offset;
  if (cookie == 0) {
    offset = ctx->top_label->offset;
    pop_label_info(ctx);
  } else {
    ctx->block_depth--;
    offset = (uint32_t)cookie;
  }
  if (type != WASM_TYPE_VOID)
    out_u8_at(&ctx->buf, offset, WASM_OPCODE_EXPR_BLOCK,
              "FIXUP OPCODE_EXPR_BLOCK");
  out_u8_at(&ctx->buf, offset + 1, num_exprs, "FIXUP num expressions");
}

static void before_binary(enum WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, opcode);
}

static WasmParserCookie before_break(int with_expr,
                                     int label_depth,
                                     void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, with_expr ? WASM_OPCODE_EXPR_BREAK : WASM_OPCODE_BREAK);
  LabelInfo* label_info = ctx->top_label;
  for (; label_depth > 0; label_depth--)
    label_info = label_info->next_label;
  label_info->with_expr = label_info->with_expr || with_expr;
  int block_depth = (ctx->block_depth - 1) - label_info->block_depth;
  out_u8(&ctx->buf, block_depth, "break depth");
  return 0;
}

static void after_break(WasmParserCookie cookie, void* user_data) {}

static void before_call(int function_index, void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_CALL);
  /* defined functions are always after all imports */
  out_leb128(&ctx->buf, ctx->module->imports.size + function_index,
             "func index");
}

static void before_call_import(int import_index, void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_CALL);
  out_leb128(&ctx->buf, import_index, "import index");
}

static void before_compare(enum WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, opcode);
}

static void after_const(enum WasmOpcode opcode,
                        WasmType type,
                        WasmNumber value,
                        void* user_data) {
  Context* ctx = user_data;
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

static void before_convert(enum WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, opcode);
}

static void after_get_local(int index, void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_GET_LOCAL);
  out_leb128(&ctx->buf, ctx->remapped_locals[index], "remapped local index");
}

static WasmParserCookie before_if(void* user_data) {
  Context* ctx = user_data;
  uint32_t offset = ctx->buf.size;
  out_opcode(&ctx->buf, WASM_OPCODE_IF);
  return (WasmParserCookie)offset;
}

static void after_if(WasmType type,
                     int with_else,
                     WasmParserCookie cookie,
                     void* user_data) {
  Context* ctx = user_data;
  if (with_else) {
    uint32_t offset = (uint32_t)cookie;
    WasmOpcode opcode;
    const char* desc;
    if (type == WASM_TYPE_VOID) {
      opcode = WASM_OPCODE_IF_THEN;
      desc = "FIXUP OPCODE_IF_THEN";
    } else {
      opcode = WASM_OPCODE_EXPR_IF;
      desc = "FIXUP OPCODE_EXPR_IF";
    }
    out_u8_at(&ctx->buf, offset, opcode, desc);
  }
}

static WasmParserCookie before_label(void* user_data) {
  Context* ctx = user_data;
  push_label_info(ctx);
  out_opcode(&ctx->buf, WASM_OPCODE_BLOCK);
  out_u8(&ctx->buf, 1, "num expressions");
  return 0;
}

static void after_label(WasmType type,
                        WasmParserCookie cookie,
                        void* user_data) {
  Context* ctx = user_data;
  uint32_t offset = ctx->top_label->offset;
  pop_label_info(ctx);
  if (type != WASM_TYPE_VOID)
    out_u8_at(&ctx->buf, offset, WASM_OPCODE_EXPR_BLOCK,
              "FIXUP OPCODE_EXPR_BLOCK");
}

static void before_load(enum WasmOpcode opcode,
                        uint8_t access,
                        void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, opcode);
  out_u8(&ctx->buf, access, "load access byte");
}

static void after_load_global(int index, void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_GET_GLOBAL);
  out_leb128(&ctx->buf, index, "global index");
}

static WasmParserCookie before_loop(int with_inner_label,
                                    int with_outer_label,
                                    void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_LOOP);
  WasmParserCookie cookie = 0;
  /* TODO(binji): clean this up, it's a bit hacky right now. */
  if (with_inner_label || with_outer_label) {
    /* The outer label must be defined if the inner label is defined */
    assert(with_outer_label);
    push_label_info(ctx);
    if (with_inner_label) {
      /* The loop only has one expression: this nested block */
      out_u8(&ctx->buf, 1, "num expressions");
      out_opcode(&ctx->buf, WASM_OPCODE_BLOCK);
      push_label_info(ctx);
      out_u8(&ctx->buf, 0, "num expressions");
      cookie = 1;
    } else {
      out_u8(&ctx->buf, 0, "num expressions");
    }
  } else {
    cookie = (WasmParserCookie)ctx->buf.size;
    out_u8(&ctx->buf, 0, "num expressions");
    ctx->block_depth++;
  }
  return cookie;
}

static void after_loop(int num_exprs,
                       WasmParserCookie cookie,
                       void* user_data) {
  Context* ctx = user_data;
  if (cookie == 0 || cookie == 1) {
    LabelInfo* label_info = ctx->top_label;
    out_u8_at(&ctx->buf, label_info->offset, num_exprs,
              "FIXUP num expressions");
    if (cookie == 1) {
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
    uint32_t offset = (uint32_t)cookie;
    out_u8_at(&ctx->buf, offset, num_exprs, "FIXUP num expressions");
  }
}

static void after_memory_size(void* user_data) {
  Context* ctx = user_data;
  /* TODO(binji): not currently defined. Return 0 for now. */
  out_opcode(&ctx->buf, WASM_OPCODE_I8_CONST);
  out_u8(&ctx->buf, 0, "zero");
}

static void after_nop(void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_NOP);
}

static void after_page_size(void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_PAGE_SIZE);
}

static void before_resize_memory(void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_RESIZE_MEMORY_I32);
}

static void before_return(void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_RETURN);
}

static void before_set_local(int index, void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_SET_LOCAL);
  out_leb128(&ctx->buf, ctx->remapped_locals[index], "remapped local index");
}

static void before_store(enum WasmOpcode opcode,
                         uint8_t access,
                         void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, opcode);
  out_u8(&ctx->buf, access, "store access byte");
}

static void before_store_global(int index, void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, WASM_OPCODE_SET_GLOBAL);
  out_leb128(&ctx->buf, index, "global index");
}

static void before_unary(enum WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  out_opcode(&ctx->buf, opcode);
}

static void js_file_start(Context* ctx) {
  const char* quiet_str = ctx->options->multi_module_verbose ? "false" : "true";
  out_printf(&ctx->js_buf, "var quiet = %s;\n", quiet_str);
}

static void js_file_end(Context* ctx) {
  out_printf(&ctx->js_buf, "end();\n");
}

static void js_module_start(Context* ctx) {
  out_printf(&ctx->js_buf, "var tests = function(m) {\n");
  ctx->in_js_module = 1;
}

static void js_module_end(Context* ctx) {
  if (ctx->in_js_module) {
    ctx->in_js_module = 0;
    out_printf(&ctx->js_buf, "};\n");
  }

  if (!ctx->temp_buf.size)
    return;

  out_printf(&ctx->js_buf, "var m = createModule([\n");
  OutputBuffer* module_buf = &ctx->temp_buf;
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

static void before_module_multi(WasmModule* module, void* user_data) {
  Context* ctx = user_data;
  js_module_end(ctx);
  js_module_start(ctx);
  before_module(module, user_data);
}

static void after_module_multi(WasmModule* module, void* user_data) {
  after_module(module, user_data);
  /* assert_return writes its commands directly into ctx->buf. This function
   data will then be added to the previous module. To make this work, we swap
   ctx->buf with ctx->temp_buf; then the functions above that write to ctx->buf
   will not modify the module (which will be saved in ctx->temp_buf).

   When the next module is processed, we don't need to swap back, because we
   don't care about the contents of either buffer */
  Context* ctx = user_data;
  OutputBuffer temp = ctx->buf;
  ctx->buf = ctx->temp_buf;
  ctx->temp_buf = temp;
  ctx->assert_return_nan_count = 0;
  ctx->assert_trap_count = 0;
  ctx->invoke_count = 0;
}

static void append_nullary_function(Context* ctx,
                                    const char* name,
                                    WasmType result_type,
                                    int16_t num_local_i32,
                                    int16_t num_local_i64,
                                    int16_t num_local_f32,
                                    int16_t num_local_f64) {
  /* We assume that the data for the function in ctx->buf. Add this as a
   new function to ctx->temp_buf. */
  const size_t header_size = FUNC_HEADER_SIZE(0);
  const size_t data_size = ctx->buf.size;
  const size_t name_size = strlen(name) + 1;
  const size_t total_added_size = header_size + data_size + name_size;
  const size_t new_size = ctx->temp_buf.size + total_added_size;
  const size_t old_size = ctx->temp_buf.size;

  if (ctx->options->verbose)
    printf("; after %s\n", name);

  ensure_output_buffer_capacity(&ctx->temp_buf, new_size);
  ctx->temp_buf.size = new_size;

  /* We need to add a new function header, data and name to the name table:
   OLD:                 NEW:
   module header        module header
   global headers       global headers
   function headers     function headers
                        NEW function header
   segment headers      segment headers
   function data        function data
                        NEW function data
   segment data         segment data
   name table           name table
                        NEW name
   */

  const uint16_t num_globals = read_u16_at(&ctx->temp_buf, 2);
  const uint16_t num_functions = read_u16_at(&ctx->temp_buf, 4);
  const uint16_t num_segments = read_u16_at(&ctx->temp_buf, 6);

  /* fixup the number of functions */
  out_u16_at(&ctx->temp_buf, 4, read_u16_at(&ctx->temp_buf, 4) + 1,
             "FIXUP num functions");

  /* fixup the global offsets */
  int i;
  uint32_t offset = GLOBAL_HEADERS_OFFSET;
  for (i = 0; i < num_globals; ++i) {
    add_u32_at(&ctx->temp_buf, offset + GLOBAL_HEADER_NAME_OFFSET,
               header_size + data_size, "FIXUP global name offset");
    offset += GLOBAL_HEADER_SIZE;
  }

  /* fixup the function offsets */
  for (i = 0; i < num_functions; ++i) {
    const uint8_t num_args = read_u8_at(&ctx->temp_buf, offset);
    add_u32_at(&ctx->temp_buf, offset + FUNC_HEADER_NAME_OFFSET(num_args),
               header_size + data_size, "FIXUP func name offset");
    add_u32_at(&ctx->temp_buf, offset + FUNC_HEADER_CODE_START_OFFSET(num_args),
               header_size, "FIXUP func code start offset");
    add_u32_at(&ctx->temp_buf, offset + FUNC_HEADER_CODE_END_OFFSET(num_args),
               header_size, "FIXUP func code end offset");
    offset += FUNC_HEADER_SIZE(num_args);
  }
  uint32_t old_func_header_end = offset;

  /* fixup the segment offsets */
  for (i = 0; i < num_segments; ++i) {
    add_u32_at(&ctx->temp_buf, offset + SEGMENT_HEADER_DATA_OFFSET,
               header_size + data_size, "FIXUP segment data offset");
    offset += SEGMENT_HEADER_SIZE;
  }

  /* if there are no functions, then the end of the function data is the end of
   the headers */
  uint32_t old_func_data_end = offset;
  if (num_functions) {
    /* we don't have to keep track of the number of args of the last function
     because it will be subtracted out, so we just use 0 */
    uint32_t last_func_code_end_offset = old_func_header_end -
                                         FUNC_HEADER_SIZE(0) +
                                         FUNC_HEADER_CODE_END_OFFSET(0);
    old_func_data_end =
        read_u32_at(&ctx->temp_buf, last_func_code_end_offset) - header_size;
  }

  /* move everything after the function data down, but leave room for the new
   function name */
  move_data(&ctx->temp_buf, old_func_data_end + header_size + data_size,
            old_func_data_end, old_size - old_func_data_end);

  /* write the new name */
  const uint32_t new_name_offset = new_size - name_size;
  out_data(&ctx->temp_buf, new_name_offset, name, name_size, "func name");

  /* write the new function data */
  const uint32_t new_data_offset = old_func_data_end + header_size;
  out_data(&ctx->temp_buf, new_data_offset, ctx->buf.start, ctx->buf.size,
           "func func data");

  /* move everything between the end of the function headers and the end of the
   function data down */
  move_data(&ctx->temp_buf, old_func_header_end + header_size,
            old_func_header_end, old_func_data_end - old_func_header_end);

  /* write the new header */
  const uint32_t new_header_offset = old_func_header_end;
  if (ctx->options->verbose) {
    printf("; clear [%07x,%07x)\n", new_header_offset,
           new_header_offset + FUNC_HEADER_SIZE(0));
  }
  memset(ctx->temp_buf.start + new_header_offset, 0, FUNC_HEADER_SIZE(0));
  out_u8_at(&ctx->temp_buf, new_header_offset + FUNC_HEADER_RESULT_TYPE_OFFSET,
            wasm_type_to_v8_type(result_type), "func result type");
  out_u32_at(&ctx->temp_buf, new_header_offset + FUNC_HEADER_NAME_OFFSET(0),
             new_name_offset, "func name offset");
  out_u32_at(&ctx->temp_buf,
             new_header_offset + FUNC_HEADER_CODE_START_OFFSET(0),
             new_data_offset, "func code start offset");
  out_u32_at(&ctx->temp_buf, new_header_offset + FUNC_HEADER_CODE_END_OFFSET(0),
             new_data_offset + data_size, "func code end offset");
  out_u16_at(&ctx->temp_buf,
             new_header_offset + FUNC_HEADER_NUM_LOCAL_I32_OFFSET(0),
             num_local_i32, "func num local i32");
  out_u16_at(&ctx->temp_buf,
             new_header_offset + FUNC_HEADER_NUM_LOCAL_I64_OFFSET(0),
             num_local_i64, "func num local i64");
  out_u16_at(&ctx->temp_buf,
             new_header_offset + FUNC_HEADER_NUM_LOCAL_F32_OFFSET(0),
             num_local_f32, "func num local f32");
  out_u16_at(&ctx->temp_buf,
             new_header_offset + FUNC_HEADER_NUM_LOCAL_F64_OFFSET(0),
             num_local_f64, "func num local f64");
  out_u8_at(&ctx->temp_buf, new_header_offset + FUNC_HEADER_EXPORTED_OFFSET(0),
            1, "func export");
}

static WasmParserCookie before_assert_return(WasmSourceLocation loc,
                                             void* user_data) {
  Context* ctx = user_data;
  ctx->in_assert = 1;
  if (ctx->options->verbose)
    printf("; before assert_return_%d\n", ctx->assert_return_count);
  init_output_buffer(&ctx->buf, INITIAL_OUTPUT_BUFFER_CAPACITY,
                     ctx->options->verbose);
  WasmParserCookie cookie = (WasmParserCookie)ctx->buf.size;
  out_opcode(&ctx->buf, WASM_OPCODE_I32_EQ);
  out_printf(&ctx->js_buf,
             "  assertReturn(m, \"$assert_return_%d\", \"%s\", %d);\n",
             ctx->assert_return_count, loc.source->filename, loc.line);
  return cookie;
}

static void after_assert_return(WasmType type,
                                WasmParserCookie cookie,
                                void* user_data) {
  Context* ctx = user_data;

  uint32_t offset = (uint32_t)cookie;
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

  out_u8_at(&ctx->buf, offset, opcode, "FIXUP assert_return opcode");

  char name[256];
  snprintf(name, 256, "$assert_return_%d", ctx->assert_return_count++);
  append_nullary_function(ctx, name, WASM_TYPE_I32, 0, 0, 0, 0);
  ctx->in_assert = 0;
}

static WasmParserCookie before_assert_return_nan(WasmSourceLocation loc,
                                                 void* user_data) {
  Context* ctx = user_data;
  ctx->in_assert = 1;
  if (ctx->options->verbose)
    printf("; before assert_return_nan_%d\n", ctx->assert_return_nan_count);
  init_output_buffer(&ctx->buf, INITIAL_OUTPUT_BUFFER_CAPACITY,
                     ctx->options->verbose);
  out_opcode(&ctx->buf, WASM_OPCODE_SET_LOCAL);
  out_u8(&ctx->buf, 0, "remapped local index");
  out_printf(&ctx->js_buf,
             "  assertReturn(m, \"$assert_return_nan_%d\", \"%s\", %d);\n",
             ctx->assert_return_nan_count, loc.source->filename, loc.line);
  return 0;
}

static void after_assert_return_nan(WasmType type,
                                    WasmParserCookie cookie,
                                    void* user_data) {
  Context* ctx = user_data;

  int num_local_f32 = 0;
  int num_local_f64 = 0;
  switch (type) {
    case WASM_TYPE_F32:
      num_local_f32++;
      out_opcode(&ctx->buf, WASM_OPCODE_F32_NE);
      break;
    case WASM_TYPE_F64:
      num_local_f64++;
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

  char name[256];
  snprintf(name, 256, "$assert_return_nan_%d", ctx->assert_return_nan_count++);
  append_nullary_function(ctx, name, WASM_TYPE_I32, 0, 0, num_local_f32,
                          num_local_f64);
  ctx->in_assert = 0;
}

static void before_assert_trap(WasmSourceLocation loc, void* user_data) {
  Context* ctx = user_data;
  ctx->in_assert = 1;
  if (ctx->options->verbose)
    printf("; before assert_trap_%d\n", ctx->assert_trap_count);
  init_output_buffer(&ctx->buf, INITIAL_OUTPUT_BUFFER_CAPACITY,
                     ctx->options->verbose);
  out_printf(&ctx->js_buf,
             "  assertTrap(m, \"$assert_trap_%d\", \"%s\", %d);\n",
             ctx->assert_trap_count, loc.source->filename, loc.line);
}

static void after_assert_trap( void* user_data) {
  Context* ctx = user_data;
  char name[256];
  snprintf(name, 256, "$assert_trap_%d", ctx->assert_trap_count++);
  append_nullary_function(ctx, name, WASM_TYPE_VOID, 0, 0, 0, 0);
  ctx->in_assert = 0;
}

static WasmParserCookie before_invoke(WasmSourceLocation loc,
                                      const char* invoke_name,
                                      int invoke_function_index,
                                      void* user_data) {
  Context* ctx = user_data;
  if (!ctx->in_assert) {
    if (ctx->options->verbose)
      printf("; before invoke_%d\n", ctx->invoke_count);
    init_output_buffer(&ctx->buf, INITIAL_OUTPUT_BUFFER_CAPACITY,
                       ctx->options->verbose);
  }

  out_opcode(&ctx->buf, WASM_OPCODE_CALL);
  /* defined functions are always after all imports */
  out_leb128(&ctx->buf, ctx->module->imports.size + invoke_function_index,
             "invoke func index");

  if (!ctx->in_assert) {
    out_printf(&ctx->js_buf, "  invoke(m, \"$invoke_%d\");\n",
               ctx->invoke_count);
  }

  return (WasmParserCookie)invoke_function_index;
}

static void after_invoke(WasmParserCookie cookie, void* user_data) {
  Context* ctx = user_data;
  if (ctx->in_assert)
    return;

  int invoke_function_index = (int)cookie;
  WasmFunction* function = &ctx->module->functions.data[invoke_function_index];
  char name[256];
  snprintf(name, 256, "$invoke_%d", ctx->invoke_count++);
  append_nullary_function(ctx, name, function->result_type, 0, 0, 0, 0);
}

static void assert_invalid_error(WasmSourceLocation loc,
                                 const char* msg,
                                 void* user_data) {
  fprintf(stdout, "assert_invalid error:\n  %s:%d:%d: %s", loc.source->filename,
          loc.line, loc.col, msg);
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
  callbacks.before_binary = before_binary;
  callbacks.before_break = before_break;
  callbacks.after_break = after_break;
  callbacks.before_call = before_call;
  callbacks.before_call_import = before_call_import;
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
  callbacks.before_resize_memory = before_resize_memory;
  callbacks.before_return = before_return;
  callbacks.before_set_local = before_set_local;
  callbacks.before_store = before_store;
  callbacks.before_store_global = before_store_global;
  callbacks.before_unary = before_unary;
  callbacks.assert_invalid_error = assert_invalid_error;

  int result;
  if (options->multi_module) {
    if (options->outfile) {
      callbacks.before_module = before_module_multi;
      callbacks.after_module = after_module_multi;
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
    result =
        wasm_parse_file(source, &callbacks, options->type_check);
    if (options->outfile) {
      js_module_end(&ctx);
      js_file_end(&ctx);
      write_output_buffer(&ctx.js_buf, options->outfile);
    }
  } else {
    result =
        wasm_parse_module(source, &callbacks, options->type_check);
    if (result == 0 && options->outfile)
      write_output_buffer(&ctx.buf, options->outfile);
  }

  destroy_context(&ctx);
  return result;
}

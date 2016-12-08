/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "binary-reader-objdump.h"

#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include "binary-reader.h"
#include "literal.h"

typedef struct Context {
  const WasmObjdumpOptions* options;
  WasmStream* out_stream;
  const uint8_t* data;
  size_t size;
  WasmOpcode current_opcode;
  size_t current_opcode_offset;
  size_t last_opcode_end;
  int indent_level;
  WasmBool print_details;
  WasmBool header_printed;
  int section_found;

  WasmStringSlice import_module_name;
  WasmStringSlice import_field_name;

  int function_index;
  int global_index;
} Context;


static WasmBool should_print_details(Context* ctx) {
  if (ctx->options->mode != WASM_DUMP_DETAILS)
    return WASM_FALSE;
  return ctx->print_details;
}

static void WASM_PRINTF_FORMAT(2, 3)
    print_details(Context* ctx, const char* fmt, ...) {
  if (!should_print_details(ctx))
    return;
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

#define SEGSTART(segname, name)                                             \
  static WasmResult begin_##segname##_section(WasmBinaryReaderContext* ctx, \
                                              uint32_t size) {              \
    return begin_section(ctx->user_data, name, ctx->offset, size);          \
  }

static WasmResult begin_section(Context* ctx,
                                const char* name,
                                size_t offset,
                                size_t size) {
  switch (ctx->options->mode) {
    case WASM_DUMP_HEADERS:
      printf("%9s start=%#010" PRIzx " end=%#010" PRIzx " (size=%#010" PRIzx
             ") ",
             name, offset, offset + size, size);
      break;
    case WASM_DUMP_DETAILS:
      if (!ctx->options->section_name || !strcasecmp(ctx->options->section_name, name)) {
        printf("%s:\n", name);
        ctx->print_details = WASM_TRUE;
        ctx->section_found = WASM_TRUE;
      } else {
        ctx->print_details = WASM_FALSE;
      }
      break;
    case WASM_DUMP_RAW_DATA:
      printf("\nContents of section %s:\n", name);
      wasm_write_memory_dump(ctx->out_stream, ctx->data + offset, size, offset,
                             WASM_PRINT_CHARS, NULL);
      break;
    case WASM_DUMP_DISASSEMBLE:
      break;
  }
  return WASM_OK;
}

static WasmResult begin_custom_section(WasmBinaryReaderContext* ctx,
                                       uint32_t size,
                                       WasmStringSlice section_name) {
  Context* context = ctx->user_data;
  if (begin_section(context, "CUSTOM", ctx->offset, size))
    return WASM_ERROR;
  print_details(context, " - name: \"" PRIstringslice "\"\n",
                WASM_PRINTF_STRING_SLICE_ARG(section_name));
  if (context->options->mode == WASM_DUMP_HEADERS)
    printf("\"" PRIstringslice "\"\n", WASM_PRINTF_STRING_SLICE_ARG(section_name));
  return WASM_OK;
}

SEGSTART(signature, "TYPE")
SEGSTART(import, "IMPORT")
SEGSTART(function_signatures, "FUNCTION")
SEGSTART(table, "TABLE")
SEGSTART(memory, "MEMORY")
SEGSTART(global, "GLOBAL")
SEGSTART(export, "EXPORT")
SEGSTART(start, "START")
SEGSTART(function_bodies, "CODE")
SEGSTART(elem, "ELEM")
SEGSTART(data, "DATA")

static WasmResult on_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  if (ctx->options->mode == WASM_DUMP_HEADERS) {
    printf("count: %d\n", count);
  }
  return WASM_OK;
}

static WasmResult begin_module(uint32_t version, void* user_data) {
  Context* ctx = user_data;
  if (ctx->options->print_header) {
    const char *basename = strrchr(ctx->options->infile, '/');
    if (basename)
      basename++;
    else
      basename = ctx->options->infile;
    printf("%s:\tfile format wasm %#08x\n", basename, version);
    ctx->header_printed = WASM_TRUE;
  }

  switch (ctx->options->mode) {
    case WASM_DUMP_HEADERS:
      printf("\n");
      printf("Sections:\n");
      break;
    case WASM_DUMP_DETAILS:
      printf("\n");
      printf("Section Details:\n");
      break;
    case WASM_DUMP_DISASSEMBLE:
      printf("\n");
      printf("Code Disassembly:\n");
      break;
    case WASM_DUMP_RAW_DATA:
      break;
  }

  return WASM_OK;
}

static WasmResult end_module(void *user_data) {
  Context* ctx = user_data;
  if (ctx->options->mode == WASM_DUMP_DETAILS && ctx->options->section_name) {
    if (!ctx->section_found) {
      printf("Section not found: %s\n", ctx->options->section_name);
      return WASM_ERROR;
    }
  }

  return WASM_OK;
}

static WasmResult on_opcode(WasmBinaryReaderContext* ctx, WasmOpcode opcode) {
  Context* context = ctx->user_data;

  if (context->options->debug) {
    const char* opcode_name = wasm_get_opcode_name(opcode);
    printf("on_opcode: %#" PRIzx ": %s\n", ctx->offset, opcode_name);
  }

  if (context->last_opcode_end) {
    if (ctx->offset != context->last_opcode_end + 1) {
      uint8_t missing_opcode = ctx->data[context->last_opcode_end];
      const char* opcode_name = wasm_get_opcode_name(missing_opcode);
      fprintf(stderr, "warning: %#" PRIzx " missing opcode callback at %#" PRIzx
                      " (%#02x=%s)\n",
              ctx->offset, context->last_opcode_end + 1,
              ctx->data[context->last_opcode_end], opcode_name);
      return WASM_ERROR;
    }
  }

  context->current_opcode_offset = ctx->offset;
  context->current_opcode = opcode;
  return WASM_OK;
}

#define IMMEDIATE_OCTET_COUNT 9

static void log_opcode(Context* ctx,
                       const uint8_t* data,
                       size_t data_size,
                       const char* fmt,
                       ...) {
  size_t offset = ctx->current_opcode_offset;

  // Print binary data
  printf(" %06" PRIzx ": %02x", offset - 1, ctx->current_opcode);
  size_t i;
  for (i = 0; i < data_size && i < IMMEDIATE_OCTET_COUNT; i++, offset++) {
    printf(" %02x", data[offset]);
  }
  for (i = data_size + 1; i < IMMEDIATE_OCTET_COUNT; i++) {
    printf("   ");
  }
  printf(" | ");

  // Print disassemble
  int j;
  int indent_level = ctx->indent_level;
  if (ctx->current_opcode == WASM_OPCODE_ELSE)
    indent_level--;
  for (j = 0; j < indent_level; j++) {
    printf("  ");
  }

  const char* opcode_name = wasm_get_opcode_name(ctx->current_opcode);
  printf("%s", opcode_name);
  if (fmt) {
    printf(" ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }

  printf("\n");

  ctx->last_opcode_end = ctx->current_opcode_offset + data_size;
}

static WasmResult on_opcode_bare(WasmBinaryReaderContext* ctx) {
  Context* context = ctx->user_data;
  log_opcode(context, ctx->data, 0, NULL);
  return WASM_OK;
}

static WasmResult on_opcode_uint32(WasmBinaryReaderContext* ctx,
                                   uint32_t value) {
  Context* context = ctx->user_data;
  size_t immediate_len = ctx->offset - context->current_opcode_offset;
  log_opcode(context, ctx->data, immediate_len, "%#x", value);
  return WASM_OK;
}

static WasmResult on_opcode_uint32_uint32(WasmBinaryReaderContext* ctx,
                                          uint32_t value,
                                          uint32_t value2) {
  Context* context = ctx->user_data;
  size_t immediate_len = ctx->offset - context->current_opcode_offset;
  log_opcode(context, ctx->data, immediate_len, "%lu %lu", value, value2);
  return WASM_OK;
}

static WasmResult on_opcode_uint64(WasmBinaryReaderContext* ctx,
                                   uint64_t value) {
  Context* context = ctx->user_data;
  size_t immediate_len = ctx->offset - context->current_opcode_offset;
  log_opcode(context, ctx->data, immediate_len, "%d", value);
  return WASM_OK;
}

static WasmResult on_opcode_f32(WasmBinaryReaderContext* ctx,
                                uint32_t value) {
  Context* context = ctx->user_data;
  size_t immediate_len = ctx->offset - context->current_opcode_offset;
  char buffer[WASM_MAX_FLOAT_HEX];
  wasm_write_float_hex(buffer, sizeof(buffer), value);
  log_opcode(context, ctx->data, immediate_len, buffer);
  return WASM_OK;
}

static WasmResult on_opcode_f64(WasmBinaryReaderContext* ctx,
                                uint64_t value) {
  Context* context = ctx->user_data;
  size_t immediate_len = ctx->offset - context->current_opcode_offset;
  char buffer[WASM_MAX_DOUBLE_HEX];
  wasm_write_double_hex(buffer, sizeof(buffer), value);
  log_opcode(context, ctx->data, immediate_len, buffer);
  return WASM_OK;
}

WasmResult on_br_table_expr(WasmBinaryReaderContext* ctx,
                            uint32_t num_targets,
                            uint32_t* target_depths,
                            uint32_t default_target_depth) {
  Context* context = ctx->user_data;
  size_t immediate_len = ctx->offset - context->current_opcode_offset;
  /* TODO(sbc): Print targets */
  log_opcode(context, ctx->data, immediate_len, NULL);
  return WASM_OK;
}

static WasmResult on_end_expr(void* user_data) {
  Context* context = user_data;
  context->indent_level--;
  assert(context->indent_level >= 0);
  log_opcode(context, NULL, 0, NULL);
  return WASM_OK;
}

static const char* wasm_type_name(WasmType type) {
  switch (type) {
    case WASM_TYPE_I32:
      return "i32";

    case WASM_TYPE_I64:
      return "i64";

    case WASM_TYPE_F32:
      return "f32";

    case WASM_TYPE_F64:
      return "f64";

    default:
      assert(0);
      return "INVALID TYPE";
  }
}

static WasmResult on_opcode_block_sig(WasmBinaryReaderContext* ctx,
                                      uint32_t num_types,
                                      WasmType* sig_types) {
  Context* context = ctx->user_data;
  if (num_types)
    log_opcode(context, ctx->data, 1, "%s", wasm_type_name(*sig_types));
  else
    log_opcode(context, ctx->data, 1, NULL);
  context->indent_level++;
  return WASM_OK;
}

static WasmResult on_signature(uint32_t index,
                               uint32_t param_count,
                               WasmType* param_types,
                               uint32_t result_count,
                               WasmType* result_types,
                               void* user_data) {
  Context* ctx = user_data;

  if (!should_print_details(ctx))
    return WASM_OK;
  printf(" - [%d] (", index);
  uint32_t i;
  for (i = 0; i < param_count; i++) {
    if (i != 0) {
      printf(", ");
    }
    printf("%s", wasm_type_name(param_types[i]));
  }
  printf(") -> ");
  if (result_count)
    printf("%s", wasm_type_name(result_types[0]));
  else
    printf("nil");
  printf("\n");
  return WASM_OK;
}

static WasmResult on_function_signature(uint32_t index,
                                        uint32_t sig_index,
                                        void* user_data) {
  Context* ctx = user_data;

  print_details(user_data, " - func[%d] sig=%d\n", ctx->function_index, sig_index);
  ctx->function_index++;
  return WASM_OK;
}

static WasmResult begin_function_body(uint32_t index, void* user_data) {
  Context* ctx = user_data;

  if (should_print_details(ctx))
    printf(" - func %d\n", index);
  if (ctx->options->mode == WASM_DUMP_DISASSEMBLE)
    printf("func %d\n", index);

  ctx->last_opcode_end = 0;
  return WASM_OK;
}

static WasmResult on_import(uint32_t index,
                            WasmStringSlice module_name,
                            WasmStringSlice field_name,
                            void* user_data) {
  Context* ctx = user_data;
  ctx->import_module_name = module_name;
  ctx->import_field_name = field_name;
  return WASM_OK;
}

static WasmResult on_import_func(uint32_t index,
                                 uint32_t sig_index,
                                 void* user_data) {
  Context* ctx = user_data;
  print_details(user_data,
                " - func[%d] sig=%d <- " PRIstringslice "." PRIstringslice "\n",
                ctx->function_index, sig_index,
                WASM_PRINTF_STRING_SLICE_ARG(ctx->import_module_name),
                WASM_PRINTF_STRING_SLICE_ARG(ctx->import_field_name));
  ctx->function_index++;
  return WASM_OK;
}

static WasmResult on_import_table(uint32_t index,
                                  WasmType elem_type,
                                  const WasmLimits* elem_limits,
                                  void* user_data) {
  Context* ctx = user_data;
  print_details(
      user_data, " - " PRIstringslice "." PRIstringslice
                 " -> table elem_type=%s init=%" PRId64 " max=%" PRId64 "\n",
      WASM_PRINTF_STRING_SLICE_ARG(ctx->import_module_name),
      WASM_PRINTF_STRING_SLICE_ARG(ctx->import_field_name),
      wasm_get_type_name(elem_type), elem_limits->initial, elem_limits->max);
  return WASM_OK;
}

static WasmResult on_import_memory(uint32_t index,
                                   const WasmLimits* page_limits,
                                   void* user_data) {
  Context* ctx = user_data;
  print_details(user_data,
                " - " PRIstringslice "." PRIstringslice " -> memory\n",
                WASM_PRINTF_STRING_SLICE_ARG(ctx->import_module_name),
                WASM_PRINTF_STRING_SLICE_ARG(ctx->import_field_name));
  return WASM_OK;
}

static WasmResult on_import_global(uint32_t index,
                                   WasmType type,
                                   WasmBool mutable_,
                                   void* user_data) {
  Context* ctx = user_data;
  print_details(user_data, " - global[%d] %s mutable=%d <- " PRIstringslice
                           "." PRIstringslice "\n",
                ctx->global_index, wasm_get_type_name(type), mutable_,
                WASM_PRINTF_STRING_SLICE_ARG(ctx->import_module_name),
                WASM_PRINTF_STRING_SLICE_ARG(ctx->import_field_name));
  ctx->global_index++;
  return WASM_OK;
}

static WasmResult on_memory(uint32_t index,
                            const WasmLimits* page_limits,
                            void* user_data) {
  print_details(user_data, " - memory %d\n", index);
  return WASM_OK;
}

static WasmResult on_table(uint32_t index,
                           WasmType elem_type,
                           const WasmLimits* elem_limits,
                           void* user_data) {
  print_details(user_data,
      " - [%d] type=%s init=%" PRId64 " max=%" PRId64 "\n",
      index,
      wasm_get_type_name(elem_type),
      elem_limits->initial,
      elem_limits->has_max ? elem_limits->max : 0);
  return WASM_OK;
}

static WasmResult on_export(uint32_t index,
                            WasmExternalKind kind,
                            uint32_t item_index,
                            WasmStringSlice name,
                            void* user_data) {
  print_details(user_data, " - %s[%d] ", wasm_get_kind_name(kind), item_index);
  print_details(user_data, PRIstringslice, WASM_PRINTF_STRING_SLICE_ARG(name));
  print_details(user_data, "\n");
  return WASM_OK;
}

static WasmResult on_elem_segment_function_index(uint32_t index,
                                                 uint32_t func_index,
                                                 void* user_data) {
  print_details(user_data, "  - func[%d]\n", func_index);
  return WASM_OK;
}

static WasmResult begin_elem_segment(uint32_t index,
                                     uint32_t table_index,
                                     void* user_data) {
  print_details(user_data, " - segment[%d] table=%d\n", index, table_index);
  return WASM_OK;
}

static WasmResult begin_global(uint32_t index, WasmType type, WasmBool mutable, void* user_data) {
  Context* ctx = user_data;
  print_details(user_data, " - global[%d] %s mutable=%d", ctx->global_index,
                wasm_get_type_name(type), mutable);
  ctx->global_index++;
  return WASM_OK;
}

static WasmResult on_init_expr_f32_const_expr(uint32_t index,
                                              uint32_t value,
                                              void* user_data) {
  char buffer[WASM_MAX_FLOAT_HEX];
  wasm_write_float_hex(buffer, sizeof(buffer), value);
  print_details(user_data, " - init f32=%s\n", buffer);
  return WASM_OK;
}

static WasmResult on_init_expr_f64_const_expr(uint32_t index,
                                              uint64_t value,
                                              void* user_data) {
  char buffer[WASM_MAX_DOUBLE_HEX];
  wasm_write_float_hex(buffer, sizeof(buffer), value);
  print_details(user_data, " - init f64=%s\n", buffer);
  return WASM_OK;
}

static WasmResult on_init_expr_get_global_expr(uint32_t index,
                                               uint32_t global_index,
                                               void* user_data) {
  print_details(user_data, " - init global=%d\n", global_index);
  return WASM_OK;
}

static WasmResult on_init_expr_i32_const_expr(uint32_t index,
                                              uint32_t value,
                                              void* user_data) {
  print_details(user_data, " - init i32=%d\n", value);
  return WASM_OK;
}

static WasmResult on_init_expr_i64_const_expr(uint32_t index,
                                              uint64_t value,
                                              void* user_data) {
  print_details(user_data, " - init i64=%" PRId64 "\n", value);
  return WASM_OK;
}

static WasmResult on_function_name(uint32_t index,
                                   WasmStringSlice name,
                                   void* user_data) {
  print_details(user_data, " - func[%d] " PRIstringslice "\n", index,
                WASM_PRINTF_STRING_SLICE_ARG(name));
  return WASM_OK;
}

static WasmResult on_local_name(uint32_t func_index,
                                uint32_t local_index,
                                WasmStringSlice name,
                                void* user_data) {
  if (name.length) {
    print_details(user_data, "  - local[%d] " PRIstringslice "\n", local_index,
                  WASM_PRINTF_STRING_SLICE_ARG(name));
  }
  return WASM_OK;
}

static void on_error(WasmBinaryReaderContext* ctx, const char* message) {
  WasmDefaultErrorHandlerInfo info;
  info.header = "error reading binary";
  info.out_file = stdout;
  info.print_header = WASM_PRINT_ERROR_HEADER_ONCE;
  wasm_default_binary_error_callback(ctx->offset, message, &info);
}

#define MAX_DATA_PRINT 30
static WasmResult on_data_segment_data(uint32_t index,
                                       const void* src_data,
                                       uint32_t size,
                                       void* user_data) {
  Context* ctx = user_data;
  print_details(user_data, "  - ");
  if (should_print_details(ctx))
    wasm_write_memory_dump(ctx->out_stream, src_data, size, 0, WASM_PRINT_CHARS,
                           NULL);
  return WASM_OK;
}

static WasmBinaryReader s_binary_reader = {
    .user_data = NULL,

    .begin_module = begin_module,
    .end_module = end_module,
    .on_error = on_error,

    // User section
    .begin_custom_section = begin_custom_section,

    // Signature section
    .begin_signature_section = begin_signature_section,
    .on_signature_count = on_count,
    .on_signature = on_signature,

    // Import section
    .begin_import_section = begin_import_section,
    .on_import_count = on_count,
    .on_import = on_import,
    .on_import_func = on_import_func,
    .on_import_table = on_import_table,
    .on_import_memory = on_import_memory,
    .on_import_global = on_import_global,

    // Function sigs section
    .begin_function_signatures_section = begin_function_signatures_section,
    .on_function_signatures_count = on_count,
    .on_function_signature = on_function_signature,

    // Table section
    .begin_table_section = begin_table_section,
    .on_table_count = on_count,
    .on_table = on_table,

    // Memory section
    .begin_memory_section = begin_memory_section,
    .on_memory_count = on_count,
    .on_memory = on_memory,

    // Globl seciont
    .begin_global_section = begin_global_section,
    .begin_global = begin_global,
    .on_global_count = on_count,

    // Export section
    .begin_export_section = begin_export_section,
    .on_export_count = on_count,
    .on_export = on_export,

    // Start section
    .begin_start_section = begin_start_section,

    // Body section
    .begin_function_bodies_section = begin_function_bodies_section,
    .on_function_bodies_count = on_count,
    .begin_function_body = begin_function_body,

    // Elems section
    .begin_elem_section = begin_elem_section,
    .begin_elem_segment = begin_elem_segment,
    .on_elem_segment_count = on_count,
    .on_elem_segment_function_index = on_elem_segment_function_index,

    // Data section
    .begin_data_section = begin_data_section,
    .on_data_segment_data = on_data_segment_data,
    .on_data_segment_count = on_count,

    // Known "User" sections:
    // - Names section
    .on_function_name = on_function_name,
    .on_local_name = on_local_name,

    .on_init_expr_i32_const_expr = on_init_expr_i32_const_expr,
    .on_init_expr_i64_const_expr = on_init_expr_i64_const_expr,
    .on_init_expr_f32_const_expr = on_init_expr_f32_const_expr,
    .on_init_expr_f64_const_expr = on_init_expr_f64_const_expr,
    .on_init_expr_get_global_expr = on_init_expr_get_global_expr,
};

WasmResult wasm_read_binary_objdump(struct WasmAllocator* allocator,
                                    const uint8_t* data,
                                    size_t size,
                                    const WasmObjdumpOptions* options) {
  WasmBinaryReader reader;
  WASM_ZERO_MEMORY(reader);
  reader = s_binary_reader;
  Context context;
  WASM_ZERO_MEMORY(context);
  context.header_printed = WASM_FALSE;
  context.print_details = WASM_FALSE;
  context.section_found = WASM_FALSE;
  context.data = data;
  context.size = size;
  context.options = options;
  context.out_stream = wasm_init_stdout_stream();

  if (options->mode == WASM_DUMP_DISASSEMBLE) {
    reader.on_opcode = on_opcode;
    reader.on_opcode_bare = on_opcode_bare;
    reader.on_opcode_uint32 = on_opcode_uint32;
    reader.on_opcode_uint32_uint32 = on_opcode_uint32_uint32;
    reader.on_opcode_uint64 = on_opcode_uint64;
    reader.on_opcode_f32 = on_opcode_f32;
    reader.on_opcode_f64 = on_opcode_f64;
    reader.on_opcode_block_sig = on_opcode_block_sig;
    reader.on_end_expr = on_end_expr;
    reader.on_br_table_expr = on_br_table_expr;
  }

  reader.user_data = &context;
  WasmReadBinaryOptions read_options = WASM_READ_BINARY_OPTIONS_DEFAULT;
  read_options.read_debug_names = WASM_TRUE;

  return wasm_read_binary(allocator, data, size, &reader, 1, &read_options);
}

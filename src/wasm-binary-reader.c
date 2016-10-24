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

#include "wasm-binary-reader.h"

#include <assert.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "wasm-allocator.h"
#include "wasm-binary.h"
#include "wasm-config.h"
#include "wasm-stream.h"
#include "wasm-vector.h"

#if HAVE_ALLOCA
#include <alloca.h>
#endif

#define INDENT_SIZE 2

#define INITIAL_PARAM_TYPES_CAPACITY 128
#define INITIAL_BR_TABLE_TARGET_CAPACITY 1000

typedef uint32_t Uint32;
WASM_DEFINE_VECTOR(type, WasmType)
WASM_DEFINE_VECTOR(uint32, Uint32);

#define CALLBACK_CTX(member, ...)                                       \
  RAISE_ERROR_UNLESS(                                                   \
      WASM_SUCCEEDED(                                                   \
          ctx->reader->member                                           \
              ? ctx->reader->member(get_user_context(ctx), __VA_ARGS__) \
              : WASM_OK),                                               \
      #member " callback failed")

#define CALLBACK_CTX0(member)                                         \
  RAISE_ERROR_UNLESS(                                                 \
      WASM_SUCCEEDED(ctx->reader->member                              \
                         ? ctx->reader->member(get_user_context(ctx)) \
                         : WASM_OK),                                  \
      #member " callback failed")

#define CALLBACK_SECTION(member) CALLBACK_CTX(member, section_size)

#define CALLBACK0(member)                                              \
  RAISE_ERROR_UNLESS(                                                  \
      WASM_SUCCEEDED(ctx->reader->member                               \
                         ? ctx->reader->member(ctx->reader->user_data) \
                         : WASM_OK),                                   \
      #member " callback failed")

#define CALLBACK(member, ...)                                            \
  RAISE_ERROR_UNLESS(                                                    \
      WASM_SUCCEEDED(                                                    \
          ctx->reader->member                                            \
              ? ctx->reader->member(__VA_ARGS__, ctx->reader->user_data) \
              : WASM_OK),                                                \
      #member " callback failed")

#define FORWARD0(member)                                                   \
  return ctx->reader->member ? ctx->reader->member(ctx->reader->user_data) \
                             : WASM_OK

#define FORWARD_CTX0(member)                  \
  if (!ctx->reader->member)                   \
    return WASM_OK;                           \
  WasmBinaryReaderContext new_ctx = *context; \
  new_ctx.user_data = ctx->reader->user_data; \
  return ctx->reader->member(&new_ctx);

#define FORWARD_CTX(member, ...)              \
  if (!ctx->reader->member)                   \
    return WASM_OK;                           \
  WasmBinaryReaderContext new_ctx = *context; \
  new_ctx.user_data = ctx->reader->user_data; \
  return ctx->reader->member(&new_ctx, __VA_ARGS__);

#define FORWARD(member, ...)                                            \
  return ctx->reader->member                                            \
             ? ctx->reader->member(__VA_ARGS__, ctx->reader->user_data) \
             : WASM_OK

#define RAISE_ERROR(...) \
  (ctx->reader->on_error ? raise_error(ctx, __VA_ARGS__) : (void)0)

#define RAISE_ERROR_UNLESS(cond, ...) \
  if (!(cond))                        \
    RAISE_ERROR(__VA_ARGS__);

#define V(NAME, code) [code] = #NAME,
static const char* s_section_name[] = {WASM_FOREACH_BINARY_SECTION(V)};
#undef V

typedef struct Context {
  const uint8_t* data;
  size_t size;
  size_t offset;
  WasmBinaryReaderContext user_ctx;
  WasmBinaryReader* reader;
  jmp_buf error_jmp_buf;
  WasmTypeVector param_types;
  Uint32Vector target_depths;
  const WasmReadBinaryOptions* options;
  WasmBool name_section_ok;
  uint32_t num_signatures;
  uint32_t num_imports;
  uint32_t num_func_imports;
  uint32_t num_table_imports;
  uint32_t num_memory_imports;
  uint32_t num_global_imports;
  uint32_t num_function_signatures;
  uint32_t num_tables;
  uint32_t num_memories;
  uint32_t num_globals;
  uint32_t num_exports;
  uint32_t num_function_bodies;
} Context;

typedef struct LoggingContext {
  WasmStream* stream;
  WasmBinaryReader* reader;
  int indent;
} LoggingContext;

static WasmBinaryReaderContext* get_user_context(Context* ctx) {
  ctx->user_ctx.user_data = ctx->reader->user_data;
  ctx->user_ctx.data = ctx->data;
  ctx->user_ctx.size = ctx->size;
  ctx->user_ctx.offset = ctx->offset;
  return &ctx->user_ctx;
}

static void WASM_PRINTF_FORMAT(2, 3)
    raise_error(Context* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  assert(ctx->reader->on_error);
  ctx->reader->on_error(get_user_context(ctx), buffer);
  longjmp(ctx->error_jmp_buf, 1);
}

#define IN_SIZE(type)                                       \
  if (ctx->offset + sizeof(type) > ctx->size) {             \
    RAISE_ERROR("unable to read " #type ": %s", desc);      \
  }                                                         \
  memcpy(out_value, ctx->data + ctx->offset, sizeof(type)); \
  ctx->offset += sizeof(type)

static void in_u8(Context* ctx, uint8_t* out_value, const char* desc) {
  IN_SIZE(uint8_t);
}

static void in_u32(Context* ctx, uint32_t* out_value, const char* desc) {
  IN_SIZE(uint32_t);
}

static void in_f32(Context* ctx, uint32_t* out_value, const char* desc) {
  IN_SIZE(float);
}

static void in_f64(Context* ctx, uint64_t* out_value, const char* desc) {
  IN_SIZE(double);
}

#undef IN_SIZE

#define BYTE_AT(type, i, shift) (((type)p[i] & 0x7f) << (shift))

#define LEB128_1(type) (BYTE_AT(type, 0, 0))
#define LEB128_2(type) (BYTE_AT(type, 1, 7) | LEB128_1(type))
#define LEB128_3(type) (BYTE_AT(type, 2, 14) | LEB128_2(type))
#define LEB128_4(type) (BYTE_AT(type, 3, 21) | LEB128_3(type))
#define LEB128_5(type) (BYTE_AT(type, 4, 28) | LEB128_4(type))
#define LEB128_6(type) (BYTE_AT(type, 5, 35) | LEB128_5(type))
#define LEB128_7(type) (BYTE_AT(type, 6, 42) | LEB128_6(type))
#define LEB128_8(type) (BYTE_AT(type, 7, 49) | LEB128_7(type))
#define LEB128_9(type) (BYTE_AT(type, 8, 56) | LEB128_8(type))
#define LEB128_10(type) (BYTE_AT(type, 9, 63) | LEB128_9(type))

#define SHIFT_AMOUNT(type, sign_bit) (sizeof(type) * 8 - 1 - (sign_bit))
#define SIGN_EXTEND(type, value, sign_bit)            \
  ((type)((value) << SHIFT_AMOUNT(type, sign_bit)) >> \
   SHIFT_AMOUNT(type, sign_bit))

static void in_u32_leb128(Context* ctx, uint32_t* out_value, const char* desc) {
  const uint8_t* p = ctx->data + ctx->offset;
  const uint8_t* end = ctx->data + ctx->size;

  if (p < end && (p[0] & 0x80) == 0) {
    *out_value = LEB128_1(uint32_t);
    ctx->offset += 1;
  } else if (p + 1 < end && (p[1] & 0x80) == 0) {
    *out_value = LEB128_2(uint32_t);
    ctx->offset += 2;
  } else if (p + 2 < end && (p[2] & 0x80) == 0) {
    *out_value = LEB128_3(uint32_t);
    ctx->offset += 3;
  } else if (p + 3 < end && (p[3] & 0x80) == 0) {
    *out_value = LEB128_4(uint32_t);
    ctx->offset += 4;
  } else if (p + 4 < end && (p[4] & 0x80) == 0) {
    /* the top bits set represent values > 32 bits */
    if (p[4] & 0xf0)
      RAISE_ERROR("invalid u32 leb128: %s", desc);
    *out_value = LEB128_5(uint32_t);
    ctx->offset += 5;
  } else {
    /* past the end */
    *out_value = 0;
    RAISE_ERROR("unable to read u32 leb128: %s", desc);
  }
}

static void in_i32_leb128(Context* ctx, uint32_t* out_value, const char* desc) {
  const uint8_t* p = ctx->data + ctx->offset;
  const uint8_t* end = ctx->data + ctx->size;

  if (p < end && (p[0] & 0x80) == 0) {
    uint32_t result = LEB128_1(uint32_t);
    *out_value = SIGN_EXTEND(int32_t, result, 6);
    ctx->offset += 1;
  } else if (p + 1 < end && (p[1] & 0x80) == 0) {
    uint32_t result = LEB128_2(uint32_t);
    *out_value = SIGN_EXTEND(int32_t, result, 13);
    ctx->offset += 2;
  } else if (p + 2 < end && (p[2] & 0x80) == 0) {
    uint32_t result = LEB128_3(uint32_t);
    *out_value = SIGN_EXTEND(int32_t, result, 20);
    ctx->offset += 3;
  } else if (p + 3 < end && (p[3] & 0x80) == 0) {
    uint32_t result = LEB128_4(uint32_t);
    *out_value = SIGN_EXTEND(int32_t, result, 27);
    ctx->offset += 4;
  } else if (p + 4 < end && (p[4] & 0x80) == 0) {
    /* the top bits should be a sign-extension of the sign bit */
    WasmBool sign_bit_set = (p[4] & 0x8);
    int top_bits = p[4] & 0xf0;
    if ((sign_bit_set && top_bits != 0x70) ||
        (!sign_bit_set && top_bits != 0)) {
      RAISE_ERROR("invalid i32 leb128: %s", desc);
    }
    uint32_t result = LEB128_5(uint32_t);
    *out_value = result;
    ctx->offset += 5;
  } else {
    /* past the end */
    RAISE_ERROR("unable to read i32 leb128: %s", desc);
  }
}

static void in_i64_leb128(Context* ctx, uint64_t* out_value, const char* desc) {
  const uint8_t* p = ctx->data + ctx->offset;
  const uint8_t* end = ctx->data + ctx->size;

  if (p < end && (p[0] & 0x80) == 0) {
    uint64_t result = LEB128_1(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 6);
    ctx->offset += 1;
  } else if (p + 1 < end && (p[1] & 0x80) == 0) {
    uint64_t result = LEB128_2(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 13);
    ctx->offset += 2;
  } else if (p + 2 < end && (p[2] & 0x80) == 0) {
    uint64_t result = LEB128_3(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 20);
    ctx->offset += 3;
  } else if (p + 3 < end && (p[3] & 0x80) == 0) {
    uint64_t result = LEB128_4(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 27);
    ctx->offset += 4;
  } else if (p + 4 < end && (p[4] & 0x80) == 0) {
    uint64_t result = LEB128_5(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 34);
    ctx->offset += 5;
  } else if (p + 5 < end && (p[5] & 0x80) == 0) {
    uint64_t result = LEB128_6(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 41);
    ctx->offset += 6;
  } else if (p + 6 < end && (p[6] & 0x80) == 0) {
    uint64_t result = LEB128_7(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 48);
    ctx->offset += 7;
  } else if (p + 7 < end && (p[7] & 0x80) == 0) {
    uint64_t result = LEB128_8(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 55);
    ctx->offset += 8;
  } else if (p + 8 < end && (p[8] & 0x80) == 0) {
    uint64_t result = LEB128_9(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 62);
    ctx->offset += 9;
  } else if (p + 9 < end && (p[9] & 0x80) == 0) {
    /* the top bits should be a sign-extension of the sign bit */
    WasmBool sign_bit_set = (p[9] & 0x1);
    int top_bits = p[9] & 0xfe;
    if ((sign_bit_set && top_bits != 0x7e) ||
        (!sign_bit_set && top_bits != 0)) {
      RAISE_ERROR("invalid i64 leb128: %s", desc);
    }
    uint64_t result = LEB128_10(uint64_t);
    *out_value = result;
    ctx->offset += 10;
  } else {
    /* past the end */
    RAISE_ERROR("unable to read i64 leb128: %s", desc);
  }
}

#undef BYTE_AT
#undef LEB128_1
#undef LEB128_2
#undef LEB128_3
#undef LEB128_4
#undef LEB128_5
#undef LEB128_6
#undef LEB128_7
#undef LEB128_8
#undef LEB128_9
#undef LEB128_10
#undef SHIFT_AMOUNT
#undef SIGN_EXTEND

static void in_type(Context* ctx, WasmType* out_value, const char* desc) {
  uint32_t type = 0;
  in_i32_leb128(ctx, &type, desc);
  /* Must be in the vs7 range: [-128, 127). */
  if ((int32_t)type < -128 || (int32_t)type > 127)
    RAISE_ERROR("invalid type: %d", type);
  *out_value = type;
}

static void in_str(Context* ctx, WasmStringSlice* out_str, const char* desc) {
  uint32_t str_len = 0;
  in_u32_leb128(ctx, &str_len, "string length");

  if (ctx->offset + str_len > ctx->size)
    RAISE_ERROR("unable to read string: %s", desc);

  out_str->start = (const char*)ctx->data + ctx->offset;
  out_str->length = str_len;
  ctx->offset += str_len;
}

static void in_bytes(Context* ctx,
                     const void** out_data,
                     uint32_t* out_data_size,
                     const char* desc) {
  uint32_t data_size = 0;
  in_u32_leb128(ctx, &data_size, "data size");

  if (ctx->offset + data_size > ctx->size)
    RAISE_ERROR("unable to read data: %s", desc);

  *out_data = (const uint8_t*)ctx->data + ctx->offset;
  *out_data_size = data_size;
  ctx->offset += data_size;
}

static WasmBool is_valid_external_kind(uint8_t kind) {
  return kind < WASM_NUM_EXTERNAL_KINDS;
}

static WasmBool is_concrete_type(WasmType type) {
  switch (type) {
    case WASM_TYPE_I32:
    case WASM_TYPE_I64:
    case WASM_TYPE_F32:
    case WASM_TYPE_F64:
      return WASM_TRUE;

    default:
      return WASM_FALSE;
  }
}

static WasmBool is_inline_sig_type(WasmType type) {
  return is_concrete_type(type) || type == WASM_TYPE_VOID;
}

static uint32_t num_total_funcs(Context* ctx) {
  return ctx->num_func_imports + ctx->num_function_signatures;
}

static uint32_t num_total_tables(Context* ctx) {
  return ctx->num_table_imports + ctx->num_tables;
}

static uint32_t num_total_memories(Context* ctx) {
  return ctx->num_memory_imports + ctx->num_memories;
}

static uint32_t num_total_globals(Context* ctx) {
  return ctx->num_global_imports + ctx->num_globals;
}

static WasmBool handle_unknown_section(Context* ctx,
                                       WasmStringSlice* section_name,
                                       uint32_t section_size) {
  if (ctx->options->read_debug_names && ctx->name_section_ok &&
      strncmp(section_name->start, WASM_BINARY_SECTION_NAME,
              section_name->length) == 0) {
    CALLBACK_SECTION(begin_names_section);
    uint32_t i, num_functions;
    in_u32_leb128(ctx, &num_functions, "function name count");
    RAISE_ERROR_UNLESS(num_functions <= num_total_funcs(ctx),
                       "function name count > function signature count");
    CALLBACK(on_function_names_count, num_functions);
    for (i = 0; i < num_functions; ++i) {
      WasmStringSlice function_name;
      in_str(ctx, &function_name, "function name");
      CALLBACK(on_function_name, i, function_name);

      uint32_t num_locals;
      in_u32_leb128(ctx, &num_locals, "local name count");
      CALLBACK(on_local_names_count, i, num_locals);
      uint32_t j;
      for (j = 0; j < num_locals; ++j) {
        WasmStringSlice local_name;
        in_str(ctx, &local_name, "local name");
        CALLBACK(on_local_name, i, j, local_name);
      }
    }
    CALLBACK0(end_names_section);
    return WASM_TRUE;
  }

  return WASM_FALSE;
}

static WasmBool skip_until_section(Context* ctx,
                                   WasmBinarySection expected_code,
                                   uint32_t* section_size) {
  uint32_t section_start_offset = ctx->offset;
  uint32_t section_code;
  if (ctx->offset == ctx->size) {
    /* ok, no more sections */
    return WASM_FALSE;
  }

  in_u32_leb128(ctx, &section_code, "section code");
  in_u32_leb128(ctx, section_size, "section size");
  uint32_t payload_start_offset = ctx->offset;
  if (ctx->offset + *section_size > ctx->size)
    RAISE_ERROR("invalid section size: extends past end");

  if (section_code == WASM_BINARY_SECTION_UNKNOWN) {
    WasmStringSlice section_name;
    in_str(ctx, &section_name, "section name");
    if (!handle_unknown_section(ctx, &section_name, *section_size)) {
      /* section wasn't handled, so skip it and try again. */
      ctx->offset = payload_start_offset + *section_size;
      return skip_until_section(ctx, expected_code, section_size);
    }
    return WASM_FALSE;
  } else {
    /* section is known, check if it is valid. */
    if (section_code >= WASM_NUM_BINARY_SECTIONS) {
      RAISE_ERROR("invalid section code: %u; max is %u", section_code,
                  WASM_NUM_BINARY_SECTIONS - 1);
    }

    if (section_code == expected_code) {
      return WASM_TRUE;
    } else if (section_code < expected_code) {
      RAISE_ERROR("section %s out of order", s_section_name[section_code]);
      return WASM_FALSE;
    } else {
      /* ok, future section. Reset the offset. */
      ctx->offset = section_start_offset;
      return WASM_FALSE;
    }
  }
}

static void destroy_context(WasmAllocator* allocator, Context* ctx) {
  wasm_destroy_type_vector(allocator, &ctx->param_types);
  wasm_destroy_uint32_vector(allocator, &ctx->target_depths);
}

/* Logging */

static void indent(LoggingContext* ctx) {
  ctx->indent += INDENT_SIZE;
}

static void dedent(LoggingContext* ctx) {
  ctx->indent -= INDENT_SIZE;
  assert(ctx->indent >= 0);
}
static void write_indent(LoggingContext* ctx) {
  static char s_indent[] =
      "                                                                       "
      "                                                                       ";
  static size_t s_indent_len = sizeof(s_indent) - 1;
  size_t indent = ctx->indent;
  while (indent > s_indent_len) {
    wasm_write_data(ctx->stream, s_indent, s_indent_len, NULL);
    indent -= s_indent_len;
  }
  if (indent > 0) {
    wasm_write_data(ctx->stream, s_indent, indent, NULL);
  }
}

#define LOGF_NOINDENT(...) wasm_writef(ctx->stream, __VA_ARGS__)

#define LOGF(...)               \
  do {                          \
    write_indent(ctx);          \
    LOGF_NOINDENT(__VA_ARGS__); \
  } while (0)

static void logging_on_error(WasmBinaryReaderContext* ctx,
                             const char* message) {
  LoggingContext* logging_ctx = ctx->user_data;
  if (logging_ctx->reader->on_error) {
    WasmBinaryReaderContext new_ctx = *ctx;
    new_ctx.user_data = logging_ctx->reader->user_data;
    logging_ctx->reader->on_error(&new_ctx, message);
  }
}

#define LOGGING_BEGIN(name)                                                \
  static WasmResult logging_begin_##name(WasmBinaryReaderContext* context, \
                                         uint32_t size) {                  \
    LoggingContext* ctx = context->user_data;                              \
    LOGF("begin_" #name "\n");                                             \
    indent(ctx);                                                           \
    FORWARD_CTX(begin_##name, size);                                       \
  }

#define LOGGING_END(name)                                                  \
  static WasmResult logging_end_##name(WasmBinaryReaderContext* context) { \
    LoggingContext* ctx = context->user_data;                              \
    dedent(ctx);                                                           \
    LOGF("end_" #name "\n");                                               \
    FORWARD_CTX0(end_##name);                                              \
  }

#define LOGGING_UINT32(name)                                          \
  static WasmResult logging_##name(uint32_t value, void* user_data) { \
    LoggingContext* ctx = user_data;                                  \
    LOGF(#name "(%u)\n", value);                                      \
    FORWARD(name, value);                                             \
  }

#define LOGGING_UINT32_DESC(name, desc)                               \
  static WasmResult logging_##name(uint32_t value, void* user_data) { \
    LoggingContext* ctx = user_data;                                  \
    LOGF(#name "(" desc ": %u)\n", value);                            \
    FORWARD(name, value);                                             \
  }

#define LOGGING_UINT32_UINT32(name, desc0, desc1)                    \
  static WasmResult logging_##name(uint32_t value0, uint32_t value1, \
                                   void* user_data) {                \
    LoggingContext* ctx = user_data;                                 \
    LOGF(#name "(" desc0 ": %u, " desc1 ": %u)\n", value0, value1);  \
    FORWARD(name, value0, value1);                                   \
  }

#define LOGGING_OPCODE(name)                                             \
  static WasmResult logging_##name(WasmOpcode opcode, void* user_data) { \
    LoggingContext* ctx = user_data;                                     \
    LOGF(#name "(\"%s\" (%u))\n", wasm_get_opcode_name(opcode), opcode); \
    FORWARD(name, opcode);                                               \
  }

#define LOGGING0(name)                                \
  static WasmResult logging_##name(void* user_data) { \
    LoggingContext* ctx = user_data;                  \
    LOGF(#name "\n");                                 \
    FORWARD0(name);                                   \
  }

LOGGING0(begin_module)
LOGGING0(end_module)
LOGGING_BEGIN(signature_section)
LOGGING_UINT32(on_signature_count)
LOGGING_END(signature_section)
LOGGING_BEGIN(import_section)
LOGGING_UINT32(on_import_count)
LOGGING_UINT32_UINT32(on_import_func, "index", "sig_index")
LOGGING_END(import_section)
LOGGING_BEGIN(function_signatures_section)
LOGGING_UINT32(on_function_signatures_count)
LOGGING_UINT32_UINT32(on_function_signature, "index", "sig_index")
LOGGING_END(function_signatures_section)
LOGGING_BEGIN(table_section)
LOGGING_UINT32(on_table_count)
LOGGING_END(table_section)
LOGGING_BEGIN(memory_section)
LOGGING_UINT32(on_memory_count)
LOGGING_END(memory_section)
LOGGING_BEGIN(global_section)
LOGGING_UINT32(on_global_count)
LOGGING_UINT32(begin_global_init_expr)
LOGGING_UINT32(end_global_init_expr)
LOGGING_UINT32(end_global)
LOGGING_END(global_section)
LOGGING_BEGIN(export_section)
LOGGING_UINT32(on_export_count)
LOGGING_END(export_section)
LOGGING_BEGIN(start_section)
LOGGING_UINT32(on_start_function)
LOGGING_END(start_section)
LOGGING_BEGIN(function_bodies_section)
LOGGING_UINT32(on_function_bodies_count)
LOGGING_UINT32(begin_function_body)
LOGGING_UINT32(end_function_body)
LOGGING_UINT32(on_local_decl_count)
LOGGING_OPCODE(on_binary_expr)
LOGGING_UINT32_DESC(on_call_expr, "func_index")
LOGGING_UINT32_DESC(on_call_import_expr, "import_index")
LOGGING_UINT32_DESC(on_call_indirect_expr, "sig_index")
LOGGING_OPCODE(on_compare_expr)
LOGGING_OPCODE(on_convert_expr)
LOGGING0(on_current_memory_expr)
LOGGING0(on_drop_expr)
LOGGING0(on_else_expr)
LOGGING0(on_end_expr)
LOGGING_UINT32_DESC(on_get_global_expr, "index")
LOGGING_UINT32_DESC(on_get_local_expr, "index")
LOGGING0(on_grow_memory_expr)
LOGGING0(on_nop_expr)
LOGGING0(on_return_expr)
LOGGING0(on_select_expr)
LOGGING_UINT32_DESC(on_set_global_expr, "index")
LOGGING_UINT32_DESC(on_set_local_expr, "index")
LOGGING_UINT32_DESC(on_tee_local_expr, "index")
LOGGING0(on_unreachable_expr)
LOGGING_OPCODE(on_unary_expr)
LOGGING_END(function_bodies_section)
LOGGING_BEGIN(elem_section)
LOGGING_UINT32(on_elem_segment_count)
LOGGING_UINT32_UINT32(begin_elem_segment, "index", "table_index")
LOGGING_UINT32(begin_elem_segment_init_expr)
LOGGING_UINT32(end_elem_segment_init_expr)
LOGGING_UINT32_UINT32(on_elem_segment_function_index_count, "index", "count")
LOGGING_UINT32_UINT32(on_elem_segment_function_index, "index", "func_index")
LOGGING_UINT32(end_elem_segment)
LOGGING_END(elem_section)
LOGGING_BEGIN(data_section)
LOGGING_UINT32(on_data_segment_count)
LOGGING_UINT32_UINT32(begin_data_segment, "index", "memory_index")
LOGGING_UINT32(begin_data_segment_init_expr)
LOGGING_UINT32(end_data_segment_init_expr)
LOGGING_UINT32(end_data_segment)
LOGGING_END(data_section)
LOGGING_BEGIN(names_section)
LOGGING_UINT32(on_function_names_count)
LOGGING_UINT32_UINT32(on_local_names_count, "index", "count")
LOGGING_END(names_section)
LOGGING_UINT32_UINT32(on_init_expr_get_global_expr, "index", "global_index")

static void sprint_limits(char* dst, size_t size, const WasmLimits* limits) {
  int result;
  if (limits->has_max) {
    result = wasm_snprintf(dst, size, "initial: %" PRIu64 ", max: %" PRIu64,
                           limits->initial, limits->max);
  } else {
    result = wasm_snprintf(dst, size, "initial: %" PRIu64, limits->initial);
  }
  WASM_USE(result);
  assert((size_t)result < size);
}

static void log_types(LoggingContext* ctx,
                      uint32_t type_count,
                      WasmType* types) {
  uint32_t i;
  LOGF_NOINDENT("[");
  for (i = 0; i < type_count; ++i) {
    LOGF_NOINDENT("%s", wasm_get_type_name(types[i]));
    if (i != type_count - 1)
      LOGF_NOINDENT(", ");
  }
  LOGF_NOINDENT("]");
}

static WasmResult logging_on_signature(uint32_t index,
                                       uint32_t param_count,
                                       WasmType* param_types,
                                       uint32_t result_count,
                                       WasmType* result_types,
                                       void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_signature(index: %u, params: ", index);
  log_types(ctx, param_count, param_types);
  LOGF_NOINDENT(", results: ");
  log_types(ctx, result_count, result_types);
  LOGF_NOINDENT(")\n");
  FORWARD(on_signature, index, param_count, param_types, result_count,
          result_types);
}

static WasmResult logging_on_import(uint32_t index,
                                    WasmStringSlice module_name,
                                    WasmStringSlice field_name,
                                    void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_import(index: %u, module: \"" PRIstringslice
       "\", field: \"" PRIstringslice "\")\n",
       index, WASM_PRINTF_STRING_SLICE_ARG(module_name),
       WASM_PRINTF_STRING_SLICE_ARG(field_name));
  FORWARD(on_import, index, module_name, field_name);
}

static WasmResult logging_on_import_table(uint32_t index,
                                          WasmType elem_type,
                                          const WasmLimits* elem_limits,
                                          void* user_data) {
  LoggingContext* ctx = user_data;
  char buf[100];
  sprint_limits(buf, sizeof(buf), elem_limits);
  LOGF("on_import_table(index: %u, elem_type: %s, %s)\n", index,
       wasm_get_type_name(elem_type), buf);
  FORWARD(on_import_table, index, elem_type, elem_limits);
}

static WasmResult logging_on_import_memory(uint32_t index,
                                           const WasmLimits* page_limits,
                                           void* user_data) {
  LoggingContext* ctx = user_data;
  char buf[100];
  sprint_limits(buf, sizeof(buf), page_limits);
  LOGF("on_import_memory(index: %u, %s)\n", index, buf);
  FORWARD(on_import_memory, index, page_limits);
}

static WasmResult logging_on_import_global(uint32_t index,
                                           WasmType type,
                                           WasmBool mutable_,
                                           void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_import_global(index: %u, type: %s, mutable: %s)\n", index,
       wasm_get_type_name(type), mutable_ ? "true" : "false");
  FORWARD(on_import_global, index, type, mutable_);
}

static WasmResult logging_on_table(uint32_t index,
                                   WasmType elem_type,
                                   const WasmLimits* elem_limits,
                                   void* user_data) {
  LoggingContext* ctx = user_data;
  char buf[100];
  sprint_limits(buf, sizeof(buf), elem_limits);
  LOGF("on_table(index: %u, elem_type: %s, %s)\n", index,
       wasm_get_type_name(elem_type), buf);
  FORWARD(on_table, index, elem_type, elem_limits);
}

static WasmResult logging_on_memory(uint32_t index,
                                    const WasmLimits* page_limits,
                                    void* user_data) {
  LoggingContext* ctx = user_data;
  char buf[100];
  sprint_limits(buf, sizeof(buf), page_limits);
  LOGF("on_memory(index: %u, %s)\n", index, buf);
  FORWARD(on_memory, index, page_limits);
}

static WasmResult logging_begin_global(uint32_t index,
                                       WasmType type,
                                       WasmBool mutable_,
                                       void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("begin_global(index: %u, type: %s, mutable: %s)\n", index,
       wasm_get_type_name(type), mutable_ ? "true" : "false");
  FORWARD(begin_global, index, mutable_, type);
}

static WasmResult logging_on_export(uint32_t index,
                                    WasmExternalKind kind,
                                    uint32_t item_index,
                                    WasmStringSlice name,
                                    void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_export(index: %u, kind: %s, item_index: %u, name: \"" PRIstringslice
       "\")\n",
       index, wasm_get_kind_name(kind), item_index,
       WASM_PRINTF_STRING_SLICE_ARG(name));
  FORWARD(on_export, index, kind, item_index, name);
}

static WasmResult logging_begin_function_body_pass(uint32_t index,
                                                   uint32_t pass,
                                                   void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("begin_function_body_pass(index: %u, pass: %u)\n", index, pass);
  indent(ctx);
  FORWARD(begin_function_body_pass, index, pass);
}

static WasmResult logging_on_local_decl(uint32_t decl_index,
                                        uint32_t count,
                                        WasmType type,
                                        void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_local_decl(index: %u, count: %u, type: %s)\n", decl_index, count,
       wasm_get_type_name(type));
  FORWARD(on_local_decl, decl_index, count, type);
}

static WasmResult logging_on_block_expr(uint32_t num_types,
                                        WasmType* sig_types,
                                        void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_block_expr(sig: ");
  log_types(ctx, num_types, sig_types);
  LOGF_NOINDENT(")\n");
  FORWARD(on_block_expr, num_types, sig_types);
}

static WasmResult logging_on_br_expr(uint32_t depth, void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_br_expr(depth: %u)\n", depth);
  FORWARD(on_br_expr, depth);
}

static WasmResult logging_on_br_if_expr(uint32_t depth, void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_br_if_expr(depth: %u)\n", depth);
  FORWARD(on_br_if_expr, depth);
}

static WasmResult logging_on_br_table_expr(WasmBinaryReaderContext* context,
                                           uint32_t num_targets,
                                           uint32_t* target_depths,
                                           uint32_t default_target_depth) {
  LoggingContext* ctx = context->user_data;
  LOGF("on_br_table_expr(num_targets: %u, depths: [", num_targets);
  uint32_t i;
  for (i = 0; i < num_targets; ++i) {
    LOGF_NOINDENT("%u", target_depths[i]);
    if (i != num_targets - 1)
      LOGF_NOINDENT(", ");
  }
  LOGF_NOINDENT("], default: %u)\n", default_target_depth);
  FORWARD_CTX(on_br_table_expr, num_targets, target_depths,
              default_target_depth);
}

static WasmResult logging_on_f32_const_expr(uint32_t value_bits,
                                            void* user_data) {
  LoggingContext* ctx = user_data;
  float value;
  memcpy(&value, &value_bits, sizeof(value));
  LOGF("on_f32_const_expr(%g (0x04%x))\n", value, value_bits);
  FORWARD(on_f32_const_expr, value_bits);
}

static WasmResult logging_on_f64_const_expr(uint64_t value_bits,
                                            void* user_data) {
  LoggingContext* ctx = user_data;
  double value;
  memcpy(&value, &value_bits, sizeof(value));
  LOGF("on_f64_const_expr(%g (0x08%" PRIx64 "))\n", value, value_bits);
  FORWARD(on_f64_const_expr, value_bits);
}

static WasmResult logging_on_i32_const_expr(uint32_t value, void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_i32_const_expr(%u (0x%x))\n", value, value);
  FORWARD(on_i32_const_expr, value);
}

static WasmResult logging_on_i64_const_expr(uint64_t value, void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_i64_const_expr(%" PRIu64 " (0x%" PRIx64 "))\n", value, value);
  FORWARD(on_i64_const_expr, value);
}

static WasmResult logging_on_if_expr(uint32_t num_types,
                                     WasmType* sig_types,
                                     void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_if_expr(sig: ");
  log_types(ctx, num_types, sig_types);
  LOGF_NOINDENT(")\n");
  FORWARD(on_if_expr, num_types, sig_types);
}

static WasmResult logging_on_load_expr(WasmOpcode opcode,
                                       uint32_t alignment_log2,
                                       uint32_t offset,
                                       void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_load_expr(opcode: \"%s\" (%u), align log2: %u, offset: %u)\n",
       wasm_get_opcode_name(opcode), opcode, alignment_log2, offset);
  FORWARD(on_load_expr, opcode, alignment_log2, offset);
}

static WasmResult logging_on_loop_expr(uint32_t num_types,
                                       WasmType* sig_types,
                                       void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_loop_expr(sig: ");
  log_types(ctx, num_types, sig_types);
  LOGF_NOINDENT(")\n");
  FORWARD(on_loop_expr, num_types, sig_types);
}

static WasmResult logging_on_store_expr(WasmOpcode opcode,
                                        uint32_t alignment_log2,
                                        uint32_t offset,
                                        void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_store_expr(opcode: \"%s\" (%u), align log2: %u, offset: %u)\n",
       wasm_get_opcode_name(opcode), opcode, alignment_log2, offset);
  FORWARD(on_store_expr, opcode, alignment_log2, offset);
}

static WasmResult logging_end_function_body_pass(uint32_t index,
                                                 uint32_t pass,
                                                 void* user_data) {
  LoggingContext* ctx = user_data;
  dedent(ctx);
  LOGF("end_function_body_pass(index: %u, pass: %u)\n", index, pass);
  FORWARD(end_function_body_pass, index, pass);
}

static WasmResult logging_on_data_segment_data(uint32_t index,
                                               const void* data,
                                               uint32_t size,
                                               void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_data_segment_data(index:%u, size:%u)\n", index, size);
  FORWARD(on_data_segment_data, index, data, size);
}

static WasmResult logging_on_function_name(uint32_t index,
                                           WasmStringSlice name,
                                           void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_function_name(index: %u, name: \"" PRIstringslice "\")\n", index,
       WASM_PRINTF_STRING_SLICE_ARG(name));
  FORWARD(on_function_name, index, name);
}

static WasmResult logging_on_local_name(uint32_t func_index,
                                        uint32_t local_index,
                                        WasmStringSlice name,
                                        void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_local_name(func_index: %u, local_index: %u, name: \"" PRIstringslice
       "\")\n",
       func_index, local_index, WASM_PRINTF_STRING_SLICE_ARG(name));
  FORWARD(on_local_name, func_index, local_index, name);
}

static WasmResult logging_on_init_expr_f32_const_expr(uint32_t index,
                                                      uint32_t value_bits,
                                                      void* user_data) {
  LoggingContext* ctx = user_data;
  float value;
  memcpy(&value, &value_bits, sizeof(value));
  LOGF("on_init_expr_f32_const_expr(index: %u, value: %g (0x04%x))\n", index,
       value, value_bits);
  FORWARD(on_init_expr_f32_const_expr, index, value_bits);
}

static WasmResult logging_on_init_expr_f64_const_expr(uint32_t index,
                                                      uint64_t value_bits,
                                                      void* user_data) {
  LoggingContext* ctx = user_data;
  double value;
  memcpy(&value, &value_bits, sizeof(value));
  LOGF("on_init_expr_f64_const_expr(index: %u value: %g (0x08%" PRIx64 "))\n",
       index, value, value_bits);
  FORWARD(on_init_expr_f64_const_expr, index, value_bits);
}

static WasmResult logging_on_init_expr_i32_const_expr(uint32_t index,
                                                      uint32_t value,
                                                      void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_init_expr_i32_const_expr(index: %u, value: %u)\n", index, value);
  FORWARD(on_init_expr_i32_const_expr, index, value);
}

static WasmResult logging_on_init_expr_i64_const_expr(uint32_t index,
                                                      uint64_t value,
                                                      void* user_data) {
  LoggingContext* ctx = user_data;
  LOGF("on_init_expr_i64_const_expr(index: %u, value: %" PRIu64 ")\n", index,
       value);
  FORWARD(on_init_expr_i64_const_expr, index, value);
}

static WasmBinaryReader s_logging_binary_reader = {
    .user_data = NULL,
    .on_error = logging_on_error,
    .begin_module = logging_begin_module,
    .end_module = logging_end_module,

    .begin_signature_section = logging_begin_signature_section,
    .on_signature_count = logging_on_signature_count,
    .on_signature = logging_on_signature,
    .end_signature_section = logging_end_signature_section,

    .begin_import_section = logging_begin_import_section,
    .on_import_count = logging_on_import_count,
    .on_import = logging_on_import,
    .on_import_func = logging_on_import_func,
    .on_import_table = logging_on_import_table,
    .on_import_memory = logging_on_import_memory,
    .on_import_global = logging_on_import_global,
    .end_import_section = logging_end_import_section,

    .begin_function_signatures_section =
        logging_begin_function_signatures_section,
    .on_function_signatures_count = logging_on_function_signatures_count,
    .on_function_signature = logging_on_function_signature,
    .end_function_signatures_section = logging_end_function_signatures_section,

    .begin_table_section = logging_begin_table_section,
    .on_table_count = logging_on_table_count,
    .on_table = logging_on_table,
    .end_table_section = logging_end_table_section,

    .begin_memory_section = logging_begin_memory_section,
    .on_memory_count = logging_on_memory_count,
    .on_memory = logging_on_memory,
    .end_memory_section = logging_end_memory_section,

    .begin_global_section = logging_begin_global_section,
    .on_global_count = logging_on_global_count,
    .begin_global = logging_begin_global,
    .begin_global_init_expr = logging_begin_global_init_expr,
    .end_global_init_expr = logging_end_global_init_expr,
    .end_global = logging_end_global,
    .end_global_section = logging_end_global_section,

    .begin_export_section = logging_begin_export_section,
    .on_export_count = logging_on_export_count,
    .on_export = logging_on_export,
    .end_export_section = logging_end_export_section,

    .begin_start_section = logging_begin_start_section,
    .on_start_function = logging_on_start_function,
    .end_start_section = logging_end_start_section,

    .begin_function_bodies_section = logging_begin_function_bodies_section,
    .on_function_bodies_count = logging_on_function_bodies_count,
    .begin_function_body_pass = logging_begin_function_body_pass,
    .begin_function_body = logging_begin_function_body,
    .on_local_decl_count = logging_on_local_decl_count,
    .on_local_decl = logging_on_local_decl,
    .on_binary_expr = logging_on_binary_expr,
    .on_block_expr = logging_on_block_expr,
    .on_br_expr = logging_on_br_expr,
    .on_br_if_expr = logging_on_br_if_expr,
    .on_br_table_expr = logging_on_br_table_expr,
    .on_call_expr = logging_on_call_expr,
    .on_call_import_expr = logging_on_call_import_expr,
    .on_call_indirect_expr = logging_on_call_indirect_expr,
    .on_compare_expr = logging_on_compare_expr,
    .on_convert_expr = logging_on_convert_expr,
    .on_drop_expr = logging_on_drop_expr,
    .on_else_expr = logging_on_else_expr,
    .on_end_expr = logging_on_end_expr,
    .on_f32_const_expr = logging_on_f32_const_expr,
    .on_f64_const_expr = logging_on_f64_const_expr,
    .on_get_global_expr = logging_on_get_global_expr,
    .on_get_local_expr = logging_on_get_local_expr,
    .on_grow_memory_expr = logging_on_grow_memory_expr,
    .on_i32_const_expr = logging_on_i32_const_expr,
    .on_i64_const_expr = logging_on_i64_const_expr,
    .on_if_expr = logging_on_if_expr,
    .on_load_expr = logging_on_load_expr,
    .on_loop_expr = logging_on_loop_expr,
    .on_current_memory_expr = logging_on_current_memory_expr,
    .on_nop_expr = logging_on_nop_expr,
    .on_return_expr = logging_on_return_expr,
    .on_select_expr = logging_on_select_expr,
    .on_set_global_expr = logging_on_set_global_expr,
    .on_set_local_expr = logging_on_set_local_expr,
    .on_store_expr = logging_on_store_expr,
    .on_tee_local_expr = logging_on_tee_local_expr,
    .on_unary_expr = logging_on_unary_expr,
    .on_unreachable_expr = logging_on_unreachable_expr,
    .end_function_body = logging_end_function_body,
    .end_function_body_pass = logging_end_function_body_pass,
    .end_function_bodies_section = logging_end_function_bodies_section,

    .begin_elem_section = logging_begin_elem_section,
    .on_elem_segment_count = logging_on_elem_segment_count,
    .begin_elem_segment = logging_begin_elem_segment,
    .begin_elem_segment_init_expr = logging_begin_elem_segment_init_expr,
    .end_elem_segment_init_expr = logging_end_elem_segment_init_expr,
    .on_elem_segment_function_index_count =
        logging_on_elem_segment_function_index_count,
    .on_elem_segment_function_index = logging_on_elem_segment_function_index,
    .end_elem_segment = logging_end_elem_segment,
    .end_elem_section = logging_end_elem_section,

    .begin_data_section = logging_begin_data_section,
    .on_data_segment_count = logging_on_data_segment_count,
    .begin_data_segment = logging_begin_data_segment,
    .begin_data_segment_init_expr = logging_begin_data_segment_init_expr,
    .end_data_segment_init_expr = logging_end_data_segment_init_expr,
    .on_data_segment_data = logging_on_data_segment_data,
    .end_data_segment = logging_end_data_segment,
    .end_data_section = logging_end_data_section,

    .begin_names_section = logging_begin_names_section,
    .on_function_names_count = logging_on_function_names_count,
    .on_function_name = logging_on_function_name,
    .on_local_names_count = logging_on_local_names_count,
    .on_local_name = logging_on_local_name,
    .end_names_section = logging_end_names_section,

    .on_init_expr_f32_const_expr = logging_on_init_expr_f32_const_expr,
    .on_init_expr_f64_const_expr = logging_on_init_expr_f64_const_expr,
    .on_init_expr_get_global_expr = logging_on_init_expr_get_global_expr,
    .on_init_expr_i32_const_expr = logging_on_init_expr_i32_const_expr,
    .on_init_expr_i64_const_expr = logging_on_init_expr_i64_const_expr,
};

static void read_init_expr(Context* ctx, uint32_t index) {
  uint8_t opcode;
  in_u8(ctx, &opcode, "opcode");
  switch (opcode) {
    case WASM_OPCODE_I32_CONST: {
      uint32_t value = 0;
      in_i32_leb128(ctx, &value, "init_expr i32.const value");
      CALLBACK(on_init_expr_i32_const_expr, index, value);
      break;
    }

    case WASM_OPCODE_I64_CONST: {
      uint64_t value = 0;
      in_i64_leb128(ctx, &value, "init_expr i64.const value");
      CALLBACK(on_init_expr_i64_const_expr, index, value);
      break;
    }

    case WASM_OPCODE_F32_CONST: {
      uint32_t value_bits = 0;
      in_f32(ctx, &value_bits, "init_expr f32.const value");
      CALLBACK(on_init_expr_f32_const_expr, index, value_bits);
      break;
    }

    case WASM_OPCODE_F64_CONST: {
      uint64_t value_bits = 0;
      in_f64(ctx, &value_bits, "init_expr f64.const value");
      CALLBACK(on_init_expr_f64_const_expr, index, value_bits);
      break;
    }

    case WASM_OPCODE_GET_GLOBAL: {
      uint32_t global_index;
      in_u32_leb128(ctx, &global_index, "init_expr get_global index");
      CALLBACK(on_init_expr_get_global_expr, index, global_index);
      break;
    }

    case WASM_OPCODE_END:
      return;

    default:
      RAISE_ERROR("unexpected opcode in initializer expression: %d (0x%x)",
                  opcode, opcode);
      break;
  }

  in_u8(ctx, &opcode, "opcode");
  RAISE_ERROR_UNLESS(opcode == WASM_OPCODE_END,
                     "expected END opcode after initializer expression");
}

static void read_table(Context* ctx,
                       WasmType* out_elem_type,
                       WasmLimits* out_elem_limits) {
  in_type(ctx, out_elem_type, "table elem type");
  RAISE_ERROR_UNLESS(*out_elem_type == WASM_TYPE_ANYFUNC,
                     "table elem type must by anyfunc");

  uint32_t flags;
  uint32_t initial;
  uint32_t max = 0;
  in_u32_leb128(ctx, &flags, "table flags");
  in_u32_leb128(ctx, &initial, "table initial elem count");
  WasmBool has_max = flags & WASM_BINARY_LIMITS_HAS_MAX_FLAG;
  if (has_max) {
    in_u32_leb128(ctx, &max, "table max elem count");
    RAISE_ERROR_UNLESS(initial <= max,
                       "table initial elem count must be <= max elem count");
  }

  out_elem_limits->has_max = has_max;
  out_elem_limits->initial = initial;
  out_elem_limits->max = max;
}

static void read_memory(Context* ctx, WasmLimits* out_page_limits) {
  uint32_t flags;
  uint32_t initial;
  uint32_t max = 0;
  in_u32_leb128(ctx, &flags, "memory flags");
  in_u32_leb128(ctx, &initial, "memory initial page count");
  WasmBool has_max = flags & WASM_BINARY_LIMITS_HAS_MAX_FLAG;
  RAISE_ERROR_UNLESS(initial <= WASM_MAX_PAGES, "invalid memory initial size");
  if (has_max) {
    in_u32_leb128(ctx, &max, "memory max page count");
    RAISE_ERROR_UNLESS(max <= WASM_MAX_PAGES, "invalid memory max size");
    RAISE_ERROR_UNLESS(initial <= max,
                       "memory initial size must be <= max size");
  }

  out_page_limits->has_max = has_max;
  out_page_limits->initial = initial;
  out_page_limits->max = max;
}

static void read_global_header(Context* ctx,
                               WasmType* out_type,
                               WasmBool* out_mutable) {
  WasmType global_type;
  uint8_t mutable_;
  in_type(ctx, &global_type, "global type");
  RAISE_ERROR_UNLESS(is_concrete_type(global_type),
                     "expected valid global type");

  in_u8(ctx, &mutable_, "global mutability");
  RAISE_ERROR_UNLESS(mutable_ <= 1, "global mutability must be 0 or 1");

  *out_type = global_type;
  *out_mutable = mutable_;
}

static void read_function_body(Context* ctx,
                               WasmAllocator* allocator,
                               uint32_t end_offset) {
  WasmBool seen_end_opcode = WASM_FALSE;
  while (ctx->offset < end_offset) {
    uint8_t opcode;
    in_u8(ctx, &opcode, "opcode");
    CALLBACK_CTX(on_opcode, opcode);
    switch (opcode) {
      case WASM_OPCODE_UNREACHABLE:
        CALLBACK0(on_unreachable_expr);
        CALLBACK_CTX0(on_opcode_bare);
        break;

      case WASM_OPCODE_BLOCK: {
        WasmType sig_type;
        in_type(ctx, &sig_type, "block signature type");
        RAISE_ERROR_UNLESS(is_inline_sig_type(sig_type),
                           "expected valid block signature type");
        uint32_t num_types = sig_type == WASM_TYPE_VOID ? 0 : 1;
        CALLBACK(on_block_expr, num_types, &sig_type);
        CALLBACK_CTX(on_opcode_block_sig, num_types, &sig_type);
        break;
      }

      case WASM_OPCODE_LOOP: {
        WasmType sig_type;
        in_type(ctx, &sig_type, "loop signature type");
        RAISE_ERROR_UNLESS(is_inline_sig_type(sig_type),
                           "expected valid block signature type");
        uint32_t num_types = sig_type == WASM_TYPE_VOID ? 0 : 1;
        CALLBACK(on_loop_expr, num_types, &sig_type);
        CALLBACK_CTX(on_opcode_block_sig, num_types, &sig_type);
        break;
      }

      case WASM_OPCODE_IF: {
        WasmType sig_type;
        in_type(ctx, &sig_type, "if signature type");
        RAISE_ERROR_UNLESS(is_inline_sig_type(sig_type),
                           "expected valid block signature type");
        uint32_t num_types = sig_type == WASM_TYPE_VOID ? 0 : 1;
        CALLBACK(on_if_expr, num_types, &sig_type);
        CALLBACK_CTX(on_opcode_block_sig, num_types, &sig_type);
        break;
      }

      case WASM_OPCODE_ELSE:
        CALLBACK0(on_else_expr);
        CALLBACK_CTX0(on_opcode_bare);
        break;

      case WASM_OPCODE_SELECT:
        CALLBACK0(on_select_expr);
        CALLBACK_CTX0(on_opcode_bare);
        break;

      case WASM_OPCODE_BR: {
        uint32_t depth;
        in_u32_leb128(ctx, &depth, "br depth");
        CALLBACK(on_br_expr, depth);
        CALLBACK_CTX(on_opcode_uint32, depth);
        break;
      }

      case WASM_OPCODE_BR_IF: {
        uint32_t depth;
        in_u32_leb128(ctx, &depth, "br_if depth");
        CALLBACK(on_br_if_expr, depth);
        CALLBACK_CTX(on_opcode_uint32, depth);
        break;
      }

      case WASM_OPCODE_BR_TABLE: {
        uint32_t num_targets;
        in_u32_leb128(ctx, &num_targets, "br_table target count");
        if (num_targets > ctx->target_depths.capacity) {
          wasm_reserve_uint32s(allocator, &ctx->target_depths, num_targets);
          ctx->target_depths.size = num_targets;
        }

        uint32_t i;
        for (i = 0; i < num_targets; ++i) {
          uint32_t target_depth;
          in_u32_leb128(ctx, &target_depth, "br_table target depth");
          ctx->target_depths.data[i] = target_depth;
        }

        uint32_t default_target_depth;
        in_u32_leb128(ctx, &default_target_depth,
                      "br_table default target depth");

        CALLBACK_CTX(on_br_table_expr, num_targets, ctx->target_depths.data,
                     default_target_depth);
        break;
      }

      case WASM_OPCODE_RETURN:
        CALLBACK0(on_return_expr);
        CALLBACK_CTX0(on_opcode_bare);
        break;

      case WASM_OPCODE_NOP:
        CALLBACK0(on_nop_expr);
        CALLBACK_CTX0(on_opcode_bare);
        break;

      case WASM_OPCODE_DROP:
        CALLBACK0(on_drop_expr);
        CALLBACK_CTX0(on_opcode_bare);
        break;

      case WASM_OPCODE_END:
        if (ctx->offset == end_offset)
          seen_end_opcode = WASM_TRUE;
        else
          CALLBACK0(on_end_expr);
        break;

      case WASM_OPCODE_I32_CONST: {
        uint32_t value = 0;
        in_i32_leb128(ctx, &value, "i32.const value");
        CALLBACK(on_i32_const_expr, value);
        CALLBACK_CTX(on_opcode_uint32, value);
        break;
      }

      case WASM_OPCODE_I64_CONST: {
        uint64_t value = 0;
        in_i64_leb128(ctx, &value, "i64.const value");
        CALLBACK(on_i64_const_expr, value);
        CALLBACK_CTX(on_opcode_uint64, value);
        break;
      }

      case WASM_OPCODE_F32_CONST: {
        uint32_t value_bits = 0;
        in_f32(ctx, &value_bits, "f32.const value");
        CALLBACK(on_f32_const_expr, value_bits);
        CALLBACK_CTX(on_opcode_uint32, value_bits);
        break;
      }

      case WASM_OPCODE_F64_CONST: {
        uint64_t value_bits = 0;
        in_f64(ctx, &value_bits, "f64.const value");
        CALLBACK(on_f64_const_expr, value_bits);
        CALLBACK_CTX(on_opcode_uint64, value_bits);
        break;
      }

      case WASM_OPCODE_GET_GLOBAL: {
        uint32_t global_index;
        in_u32_leb128(ctx, &global_index, "get_global global index");
        CALLBACK(on_get_global_expr, global_index);
        CALLBACK_CTX(on_opcode_uint32, global_index);
        break;
      }

      case WASM_OPCODE_GET_LOCAL: {
        uint32_t local_index;
        in_u32_leb128(ctx, &local_index, "get_local local index");
        CALLBACK(on_get_local_expr, local_index);
        CALLBACK_CTX(on_opcode_uint32, local_index);
        break;
      }

      case WASM_OPCODE_SET_GLOBAL: {
        uint32_t global_index;
        in_u32_leb128(ctx, &global_index, "set_global global index");
        CALLBACK(on_set_global_expr, global_index);
        CALLBACK_CTX(on_opcode_uint32, global_index);
        break;
      }

      case WASM_OPCODE_SET_LOCAL: {
        uint32_t local_index;
        in_u32_leb128(ctx, &local_index, "set_local local index");
        CALLBACK(on_set_local_expr, local_index);
        CALLBACK_CTX(on_opcode_uint32, local_index);
        break;
      }

      case WASM_OPCODE_CALL: {
        uint32_t func_index;
        in_u32_leb128(ctx, &func_index, "call function index");
        RAISE_ERROR_UNLESS(func_index < num_total_funcs(ctx),
                           "invalid call function index");
        CALLBACK(on_call_expr, func_index);
        CALLBACK_CTX(on_opcode_uint32, func_index);
        break;
      }

      case WASM_OPCODE_CALL_INDIRECT: {
        uint32_t sig_index;
        in_u32_leb128(ctx, &sig_index, "call_indirect signature index");
        RAISE_ERROR_UNLESS(sig_index < ctx->num_signatures,
                           "invalid call_indirect signature index");
        CALLBACK(on_call_indirect_expr, sig_index);
        CALLBACK_CTX(on_opcode_uint32, sig_index);
        break;
      }

      case WASM_OPCODE_TEE_LOCAL: {
        uint32_t local_index;
        in_u32_leb128(ctx, &local_index, "tee_local local index");
        CALLBACK(on_tee_local_expr, local_index);
        CALLBACK_CTX(on_opcode_uint32, local_index);
        break;
      }

      case WASM_OPCODE_I32_LOAD8_S:
      case WASM_OPCODE_I32_LOAD8_U:
      case WASM_OPCODE_I32_LOAD16_S:
      case WASM_OPCODE_I32_LOAD16_U:
      case WASM_OPCODE_I64_LOAD8_S:
      case WASM_OPCODE_I64_LOAD8_U:
      case WASM_OPCODE_I64_LOAD16_S:
      case WASM_OPCODE_I64_LOAD16_U:
      case WASM_OPCODE_I64_LOAD32_S:
      case WASM_OPCODE_I64_LOAD32_U:
      case WASM_OPCODE_I32_LOAD:
      case WASM_OPCODE_I64_LOAD:
      case WASM_OPCODE_F32_LOAD:
      case WASM_OPCODE_F64_LOAD: {
        uint32_t alignment_log2;
        in_u32_leb128(ctx, &alignment_log2, "load alignment");
        uint32_t offset;
        in_u32_leb128(ctx, &offset, "load offset");

        CALLBACK(on_load_expr, opcode, alignment_log2, offset);
        CALLBACK_CTX(on_opcode_uint32_uint32, alignment_log2, offset);
        break;
      }

      case WASM_OPCODE_I32_STORE8:
      case WASM_OPCODE_I32_STORE16:
      case WASM_OPCODE_I64_STORE8:
      case WASM_OPCODE_I64_STORE16:
      case WASM_OPCODE_I64_STORE32:
      case WASM_OPCODE_I32_STORE:
      case WASM_OPCODE_I64_STORE:
      case WASM_OPCODE_F32_STORE:
      case WASM_OPCODE_F64_STORE: {
        uint32_t alignment_log2;
        in_u32_leb128(ctx, &alignment_log2, "store alignment");
        uint32_t offset;
        in_u32_leb128(ctx, &offset, "store offset");

        CALLBACK(on_store_expr, opcode, alignment_log2, offset);
        CALLBACK_CTX(on_opcode_uint32_uint32, alignment_log2, offset);
        break;
      }

      case WASM_OPCODE_CURRENT_MEMORY:
        CALLBACK0(on_current_memory_expr);
        CALLBACK_CTX0(on_opcode_bare);
        break;

      case WASM_OPCODE_GROW_MEMORY:
        CALLBACK0(on_grow_memory_expr);
        CALLBACK_CTX0(on_opcode_bare);
        break;

      case WASM_OPCODE_I32_ADD:
      case WASM_OPCODE_I32_SUB:
      case WASM_OPCODE_I32_MUL:
      case WASM_OPCODE_I32_DIV_S:
      case WASM_OPCODE_I32_DIV_U:
      case WASM_OPCODE_I32_REM_S:
      case WASM_OPCODE_I32_REM_U:
      case WASM_OPCODE_I32_AND:
      case WASM_OPCODE_I32_OR:
      case WASM_OPCODE_I32_XOR:
      case WASM_OPCODE_I32_SHL:
      case WASM_OPCODE_I32_SHR_U:
      case WASM_OPCODE_I32_SHR_S:
      case WASM_OPCODE_I32_ROTR:
      case WASM_OPCODE_I32_ROTL:
      case WASM_OPCODE_I64_ADD:
      case WASM_OPCODE_I64_SUB:
      case WASM_OPCODE_I64_MUL:
      case WASM_OPCODE_I64_DIV_S:
      case WASM_OPCODE_I64_DIV_U:
      case WASM_OPCODE_I64_REM_S:
      case WASM_OPCODE_I64_REM_U:
      case WASM_OPCODE_I64_AND:
      case WASM_OPCODE_I64_OR:
      case WASM_OPCODE_I64_XOR:
      case WASM_OPCODE_I64_SHL:
      case WASM_OPCODE_I64_SHR_U:
      case WASM_OPCODE_I64_SHR_S:
      case WASM_OPCODE_I64_ROTR:
      case WASM_OPCODE_I64_ROTL:
      case WASM_OPCODE_F32_ADD:
      case WASM_OPCODE_F32_SUB:
      case WASM_OPCODE_F32_MUL:
      case WASM_OPCODE_F32_DIV:
      case WASM_OPCODE_F32_MIN:
      case WASM_OPCODE_F32_MAX:
      case WASM_OPCODE_F32_COPYSIGN:
      case WASM_OPCODE_F64_ADD:
      case WASM_OPCODE_F64_SUB:
      case WASM_OPCODE_F64_MUL:
      case WASM_OPCODE_F64_DIV:
      case WASM_OPCODE_F64_MIN:
      case WASM_OPCODE_F64_MAX:
      case WASM_OPCODE_F64_COPYSIGN:
        CALLBACK(on_binary_expr, opcode);
        CALLBACK_CTX0(on_opcode_bare);
        break;

      case WASM_OPCODE_I32_EQ:
      case WASM_OPCODE_I32_NE:
      case WASM_OPCODE_I32_LT_S:
      case WASM_OPCODE_I32_LE_S:
      case WASM_OPCODE_I32_LT_U:
      case WASM_OPCODE_I32_LE_U:
      case WASM_OPCODE_I32_GT_S:
      case WASM_OPCODE_I32_GE_S:
      case WASM_OPCODE_I32_GT_U:
      case WASM_OPCODE_I32_GE_U:
      case WASM_OPCODE_I64_EQ:
      case WASM_OPCODE_I64_NE:
      case WASM_OPCODE_I64_LT_S:
      case WASM_OPCODE_I64_LE_S:
      case WASM_OPCODE_I64_LT_U:
      case WASM_OPCODE_I64_LE_U:
      case WASM_OPCODE_I64_GT_S:
      case WASM_OPCODE_I64_GE_S:
      case WASM_OPCODE_I64_GT_U:
      case WASM_OPCODE_I64_GE_U:
      case WASM_OPCODE_F32_EQ:
      case WASM_OPCODE_F32_NE:
      case WASM_OPCODE_F32_LT:
      case WASM_OPCODE_F32_LE:
      case WASM_OPCODE_F32_GT:
      case WASM_OPCODE_F32_GE:
      case WASM_OPCODE_F64_EQ:
      case WASM_OPCODE_F64_NE:
      case WASM_OPCODE_F64_LT:
      case WASM_OPCODE_F64_LE:
      case WASM_OPCODE_F64_GT:
      case WASM_OPCODE_F64_GE:
        CALLBACK(on_compare_expr, opcode);
        CALLBACK_CTX0(on_opcode_bare);
        break;

      case WASM_OPCODE_I32_CLZ:
      case WASM_OPCODE_I32_CTZ:
      case WASM_OPCODE_I32_POPCNT:
      case WASM_OPCODE_I64_CLZ:
      case WASM_OPCODE_I64_CTZ:
      case WASM_OPCODE_I64_POPCNT:
      case WASM_OPCODE_F32_ABS:
      case WASM_OPCODE_F32_NEG:
      case WASM_OPCODE_F32_CEIL:
      case WASM_OPCODE_F32_FLOOR:
      case WASM_OPCODE_F32_TRUNC:
      case WASM_OPCODE_F32_NEAREST:
      case WASM_OPCODE_F32_SQRT:
      case WASM_OPCODE_F64_ABS:
      case WASM_OPCODE_F64_NEG:
      case WASM_OPCODE_F64_CEIL:
      case WASM_OPCODE_F64_FLOOR:
      case WASM_OPCODE_F64_TRUNC:
      case WASM_OPCODE_F64_NEAREST:
      case WASM_OPCODE_F64_SQRT:
        CALLBACK(on_unary_expr, opcode);
        CALLBACK_CTX0(on_opcode_bare);
        break;

      case WASM_OPCODE_I32_TRUNC_S_F32:
      case WASM_OPCODE_I32_TRUNC_S_F64:
      case WASM_OPCODE_I32_TRUNC_U_F32:
      case WASM_OPCODE_I32_TRUNC_U_F64:
      case WASM_OPCODE_I32_WRAP_I64:
      case WASM_OPCODE_I64_TRUNC_S_F32:
      case WASM_OPCODE_I64_TRUNC_S_F64:
      case WASM_OPCODE_I64_TRUNC_U_F32:
      case WASM_OPCODE_I64_TRUNC_U_F64:
      case WASM_OPCODE_I64_EXTEND_S_I32:
      case WASM_OPCODE_I64_EXTEND_U_I32:
      case WASM_OPCODE_F32_CONVERT_S_I32:
      case WASM_OPCODE_F32_CONVERT_U_I32:
      case WASM_OPCODE_F32_CONVERT_S_I64:
      case WASM_OPCODE_F32_CONVERT_U_I64:
      case WASM_OPCODE_F32_DEMOTE_F64:
      case WASM_OPCODE_F32_REINTERPRET_I32:
      case WASM_OPCODE_F64_CONVERT_S_I32:
      case WASM_OPCODE_F64_CONVERT_U_I32:
      case WASM_OPCODE_F64_CONVERT_S_I64:
      case WASM_OPCODE_F64_CONVERT_U_I64:
      case WASM_OPCODE_F64_PROMOTE_F32:
      case WASM_OPCODE_F64_REINTERPRET_I64:
      case WASM_OPCODE_I32_REINTERPRET_F32:
      case WASM_OPCODE_I64_REINTERPRET_F64:
      case WASM_OPCODE_I32_EQZ:
      case WASM_OPCODE_I64_EQZ:
        CALLBACK(on_convert_expr, opcode);
        CALLBACK_CTX0(on_opcode_bare);
        break;

      default:
        RAISE_ERROR("unexpected opcode: %d (0x%x)", opcode, opcode);
    }
  }
  RAISE_ERROR_UNLESS(ctx->offset == end_offset,
                     "function body longer than given size");
  RAISE_ERROR_UNLESS(seen_end_opcode, "function body must end with END opcode");
}

WasmResult wasm_read_binary(WasmAllocator* allocator,
                            const void* data,
                            size_t size,
                            WasmBinaryReader* reader,
                            uint32_t num_function_passes,
                            const WasmReadBinaryOptions* options) {
  LoggingContext logging_context;
  WASM_ZERO_MEMORY(logging_context);
  logging_context.reader = reader;
  logging_context.stream = options->log_stream;

  WasmBinaryReader logging_reader = s_logging_binary_reader;
  logging_reader.user_data = &logging_context;

  Context context;
  WASM_ZERO_MEMORY(context);
  /* all the macros assume a Context* named ctx */
  Context* ctx = &context;
  ctx->data = data;
  ctx->size = size;
  ctx->reader = options->log_stream ? &logging_reader : reader;
  ctx->options = options;

  if (setjmp(ctx->error_jmp_buf) == 1) {
    destroy_context(allocator, ctx);
    return WASM_ERROR;
  }

  wasm_reserve_types(allocator, &ctx->param_types,
                     INITIAL_PARAM_TYPES_CAPACITY);
  wasm_reserve_uint32s(allocator, &ctx->target_depths,
                       INITIAL_BR_TABLE_TARGET_CAPACITY);

  CALLBACK0(begin_module);

  uint32_t magic;
  in_u32(ctx, &magic, "magic");
  RAISE_ERROR_UNLESS(magic == WASM_BINARY_MAGIC, "bad magic value");
  uint32_t version;
  in_u32(ctx, &version, "version");
  RAISE_ERROR_UNLESS(version == WASM_BINARY_VERSION,
                     "bad wasm file version: %#x (expected %#x)", version,
                     WASM_BINARY_VERSION);

  /* type */
  uint32_t section_size;
  if (skip_until_section(ctx, WASM_BINARY_SECTION_TYPE, &section_size)) {
    CALLBACK_SECTION(begin_signature_section);
    uint32_t i;
    in_u32_leb128(ctx, &ctx->num_signatures, "type count");
    CALLBACK(on_signature_count, ctx->num_signatures);

    for (i = 0; i < ctx->num_signatures; ++i) {
      WasmType form;
      in_type(ctx, &form, "type form");
      RAISE_ERROR_UNLESS(form == WASM_TYPE_FUNC, "unexpected type form");

      uint32_t num_params;
      in_u32_leb128(ctx, &num_params, "function param count");

      if (num_params > ctx->param_types.capacity)
        wasm_reserve_types(allocator, &ctx->param_types, num_params);

      uint32_t j;
      for (j = 0; j < num_params; ++j) {
        WasmType param_type;
        in_type(ctx, &param_type, "function param type");
        RAISE_ERROR_UNLESS(is_concrete_type(param_type),
                           "expected valid param type");
        ctx->param_types.data[j] = param_type;
      }

      uint32_t num_results;
      in_u32_leb128(ctx, &num_results, "function result count");
      RAISE_ERROR_UNLESS(num_results <= 1, "result count must be 0 or 1");

      WasmType result_type = WASM_TYPE_VOID;
      if (num_results) {
        in_type(ctx, &result_type, "function result type");
        RAISE_ERROR_UNLESS(is_concrete_type(result_type),
                           "expected valid result type");
      }

      CALLBACK(on_signature, i, num_params, ctx->param_types.data, num_results,
               &result_type);
    }
    CALLBACK_CTX0(end_signature_section);
  }

  /* import */
  if (skip_until_section(ctx, WASM_BINARY_SECTION_IMPORT, &section_size)) {
    CALLBACK_SECTION(begin_import_section);
    uint32_t i;
    in_u32_leb128(ctx, &ctx->num_imports, "import count");
    CALLBACK(on_import_count, ctx->num_imports);
    for (i = 0; i < ctx->num_imports; ++i) {
      WasmStringSlice module_name;
      in_str(ctx, &module_name, "import module name");
      WasmStringSlice field_name;
      in_str(ctx, &field_name, "import field name");
      CALLBACK(on_import, i, module_name, field_name);

      uint32_t kind;
      in_u32_leb128(ctx, &kind, "import kind");
      switch (kind) {
        case WASM_EXTERNAL_KIND_FUNC: {
          uint32_t sig_index;
          in_u32_leb128(ctx, &sig_index, "import signature index");
          RAISE_ERROR_UNLESS(sig_index < ctx->num_signatures,
                             "invalid import signature index");
          CALLBACK(on_import_func, i, sig_index);
          ctx->num_func_imports++;
          break;
        }

        case WASM_EXTERNAL_KIND_TABLE: {
          WasmType elem_type;
          WasmLimits elem_limits;
          read_table(ctx, &elem_type, &elem_limits);
          CALLBACK(on_import_table, i, elem_type, &elem_limits);
          ctx->num_table_imports++;
          break;
        }

        case WASM_EXTERNAL_KIND_MEMORY: {
          WasmLimits page_limits;
          read_memory(ctx, &page_limits);
          CALLBACK(on_import_memory, i, &page_limits);
          ctx->num_memory_imports++;
          break;
        }

        case WASM_EXTERNAL_KIND_GLOBAL: {
          WasmType type;
          WasmBool mutable_;
          read_global_header(ctx, &type, &mutable_);
          CALLBACK(on_import_global, i, type, mutable_);
          ctx->num_global_imports++;
          break;
        }

        default:
          RAISE_ERROR("invalid import kind: %d", kind);
      }
    }
    CALLBACK_CTX0(end_import_section);
  }

  /* function */
  if (skip_until_section(ctx, WASM_BINARY_SECTION_FUNCTION, &section_size)) {
    CALLBACK_SECTION(begin_function_signatures_section);
    uint32_t i;
    in_u32_leb128(ctx, &ctx->num_function_signatures,
                  "function signature count");
    CALLBACK(on_function_signatures_count, ctx->num_function_signatures);
    for (i = 0; i < ctx->num_function_signatures; ++i) {
      uint32_t sig_index;
      in_u32_leb128(ctx, &sig_index, "function signature index");
      RAISE_ERROR_UNLESS(sig_index < ctx->num_signatures,
                         "invalid function signature index");
      CALLBACK(on_function_signature, i, sig_index);
    }
    CALLBACK_CTX0(end_function_signatures_section);
  }

  /* only allow the name section to come after the function signatures have
   * been specified */
  ctx->name_section_ok = WASM_TRUE;

  /* table */
  if (skip_until_section(ctx, WASM_BINARY_SECTION_TABLE, &section_size)) {
    CALLBACK_SECTION(begin_table_section);
    uint32_t i;
    in_u32_leb128(ctx, &ctx->num_tables, "table count");
    RAISE_ERROR_UNLESS(ctx->num_tables <= 1, "table count must be 0 or 1");
    CALLBACK(on_table_count, ctx->num_tables);
    for (i = 0; i < ctx->num_tables; ++i) {
      WasmType elem_type;
      WasmLimits elem_limits;
      read_table(ctx, &elem_type, &elem_limits);
      CALLBACK(on_table, i, elem_type, &elem_limits);
    }
    CALLBACK_CTX0(end_table_section);
  }

  /* memory */
  if (skip_until_section(ctx, WASM_BINARY_SECTION_MEMORY, &section_size)) {
    CALLBACK_SECTION(begin_memory_section);
    uint32_t i;
    in_u32_leb128(ctx, &ctx->num_memories, "memory count");
    RAISE_ERROR_UNLESS(ctx->num_memories, "memory count must be 0 or 1");
    CALLBACK(on_memory_count, ctx->num_memories);
    for (i = 0; i < ctx->num_memories; ++i) {
      WasmLimits page_limits;
      read_memory(ctx, &page_limits);
      CALLBACK(on_memory, i, &page_limits);
    }
    CALLBACK_CTX0(end_memory_section);
  }

  if (skip_until_section(ctx, WASM_BINARY_SECTION_GLOBAL, &section_size)) {
    CALLBACK_SECTION(begin_global_section);
    uint32_t i;
    in_u32_leb128(ctx, &ctx->num_globals, "global count");
    CALLBACK(on_global_count, ctx->num_globals);
    for (i = 0; i < ctx->num_globals; ++i) {
      WasmType global_type;
      WasmBool mutable_;
      read_global_header(ctx, &global_type, &mutable_);
      CALLBACK(begin_global, i, global_type, mutable_);
      CALLBACK(begin_global_init_expr, i);
      read_init_expr(ctx, i);
      CALLBACK(end_global_init_expr, i);
      CALLBACK(end_global, i);
    }
    CALLBACK_CTX0(end_global_section);
  }

  /* export */
  if (skip_until_section(ctx, WASM_BINARY_SECTION_EXPORT, &section_size)) {
    CALLBACK_SECTION(begin_export_section);
    uint32_t i;
    in_u32_leb128(ctx, &ctx->num_exports, "export count");
    CALLBACK(on_export_count, ctx->num_exports);
    for (i = 0; i < ctx->num_exports; ++i) {
      WasmStringSlice name;
      in_str(ctx, &name, "export item name");

      uint8_t external_kind;
      in_u8(ctx, &external_kind, "export external kind");
      RAISE_ERROR_UNLESS(is_valid_external_kind(external_kind),
                         "invalid export external kind");

      uint32_t item_index;
      in_u32_leb128(ctx, &item_index, "export item index");
      switch (external_kind) {
        case WASM_EXTERNAL_KIND_FUNC:
          RAISE_ERROR_UNLESS(item_index < num_total_funcs(ctx),
                             "invalid export func index");
          break;
        case WASM_EXTERNAL_KIND_TABLE:
          RAISE_ERROR_UNLESS(item_index < num_total_tables(ctx),
                             "invalid export table index");
          break;
        case WASM_EXTERNAL_KIND_MEMORY:
          RAISE_ERROR_UNLESS(item_index < num_total_memories(ctx),
                             "invalid export memory index");
          break;
        case WASM_EXTERNAL_KIND_GLOBAL:
          RAISE_ERROR_UNLESS(item_index < num_total_globals(ctx),
                             "invalid export global index");
          break;
        case WASM_NUM_EXTERNAL_KINDS:
          assert(0);
          break;
      }

      CALLBACK(on_export, i, external_kind, item_index, name);
    }
    CALLBACK_CTX0(end_export_section);
  }

  /* start */
  if (skip_until_section(ctx, WASM_BINARY_SECTION_START, &section_size)) {
    CALLBACK_SECTION(begin_start_section);
    uint32_t func_index;
    in_u32_leb128(ctx, &func_index, "start function index");
    RAISE_ERROR_UNLESS(func_index < num_total_funcs(ctx),
                       "invalid start function index");
    CALLBACK(on_start_function, func_index);
    CALLBACK_CTX0(end_start_section);
  }

  /* elem */
  if (skip_until_section(ctx, WASM_BINARY_SECTION_ELEM, &section_size)) {
    RAISE_ERROR_UNLESS(num_total_tables(ctx) > 0,
                       "elem section without table section");
    CALLBACK_SECTION(begin_elem_section);
    uint32_t i, num_elem_segments;
    in_u32_leb128(ctx, &num_elem_segments, "elem segment count");
    CALLBACK(on_elem_segment_count, num_elem_segments);
    for (i = 0; i < num_elem_segments; ++i) {
      uint32_t table_index;
      in_u32_leb128(ctx, &table_index, "elem segment table index");
      CALLBACK(begin_elem_segment, i, table_index);
      CALLBACK(begin_elem_segment_init_expr, i);
      read_init_expr(ctx, i);
      CALLBACK(end_elem_segment_init_expr, i);

      uint32_t j, num_function_indexes;
      in_u32_leb128(ctx, &num_function_indexes,
                    "elem segment function index count");
      CALLBACK(on_elem_segment_function_index_count, i, num_function_indexes);
      for (j = 0; j < num_function_indexes; ++j) {
        uint32_t func_index;
        in_u32_leb128(ctx, &func_index, "elem segment function index");
        CALLBACK(on_elem_segment_function_index, i, func_index);
      }
      CALLBACK(end_elem_segment, i);
    }
    CALLBACK_CTX0(end_elem_section);
  }

  /* code */
  if (skip_until_section(ctx, WASM_BINARY_SECTION_CODE, &section_size)) {
    CALLBACK_SECTION(begin_function_bodies_section);
    uint32_t i;
    in_u32_leb128(ctx, &ctx->num_function_bodies, "function body count");
    RAISE_ERROR_UNLESS(ctx->num_function_signatures == ctx->num_function_bodies,
                       "function signature count != function body count");
    CALLBACK(on_function_bodies_count, ctx->num_function_bodies);
    for (i = 0; i < ctx->num_function_bodies; ++i) {
      uint32_t j;
      uint32_t func_offset = ctx->offset;
      for (j = 0; j < num_function_passes; ++j) {
        ctx->offset = func_offset;
        CALLBACK(begin_function_body_pass, i, j);
        CALLBACK(begin_function_body, i);
        uint32_t body_size;
        in_u32_leb128(ctx, &body_size, "function body size");
        uint32_t body_start_offset = ctx->offset;
        uint32_t end_offset = body_start_offset + body_size;

        uint32_t num_local_decls;
        in_u32_leb128(ctx, &num_local_decls, "local declaration count");
        CALLBACK(on_local_decl_count, num_local_decls);
        uint32_t k;
        for (k = 0; k < num_local_decls; ++k) {
          uint32_t num_local_types;
          in_u32_leb128(ctx, &num_local_types, "local type count");
          WasmType local_type;
          in_type(ctx, &local_type, "local type");
          RAISE_ERROR_UNLESS(is_concrete_type(local_type),
                             "expected valid local type");
          CALLBACK(on_local_decl, k, num_local_types, local_type);
        }

        read_function_body(ctx, allocator, end_offset);

        CALLBACK(end_function_body, i);
        CALLBACK(end_function_body_pass, i, j);
      }
    }
    CALLBACK_CTX0(end_function_bodies_section);
  }

  /* data */
  if (skip_until_section(ctx, WASM_BINARY_SECTION_DATA, &section_size)) {
    RAISE_ERROR_UNLESS(num_total_memories(ctx) > 0,
                       "data section without memory section");
    CALLBACK_SECTION(begin_data_section);
    uint32_t i, num_data_segments;
    in_u32_leb128(ctx, &num_data_segments, "data segment count");
    CALLBACK(on_data_segment_count, num_data_segments);
    for (i = 0; i < num_data_segments; ++i) {
      uint32_t memory_index;
      in_u32_leb128(ctx, &memory_index, "data segment memory index");
      CALLBACK(begin_data_segment, i, memory_index);
      CALLBACK(begin_data_segment_init_expr, i);
      read_init_expr(ctx, i);
      CALLBACK(end_data_segment_init_expr, i);

      uint32_t data_size;
      const void* data;
      in_bytes(ctx, &data, &data_size, "data segment data");
      CALLBACK(on_data_segment_data, i, data, data_size);
      CALLBACK(end_data_segment, i);
    }
    CALLBACK_CTX0(end_data_section);
  }

  CALLBACK0(end_module);
  destroy_context(allocator, ctx);
  return WASM_OK;
}

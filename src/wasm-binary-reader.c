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
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "wasm-allocator.h"
#include "wasm-binary.h"
#include "wasm-config.h"
#include "wasm-vector.h"

#if HAVE_ALLOCA
#include <alloca.h>
#endif

#define INITIAL_PARAM_TYPES_CAPACITY 128
#define INITIAL_BR_TABLE_TARGET_CAPACITY 1000

typedef uint32_t Uint32;
WASM_DEFINE_VECTOR(type, WasmType)
WASM_DEFINE_VECTOR(uint32, Uint32);

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

#define RAISE_ERROR(...) \
  (ctx->reader->on_error ? raise_error(ctx, __VA_ARGS__) : (void)0)

#define RAISE_ERROR_UNLESS(cond, ...) \
  if (!(cond))                        \
    RAISE_ERROR(__VA_ARGS__);

#define CHECK_ALLOC_(cond) \
  RAISE_ERROR_UNLESS(cond, "%s:%d: allocation failed\n", __FILE__, __LINE__)

#define CHECK_ALLOC(e) CHECK_ALLOC_(WASM_SUCCEEDED(e))

/* clang-format off */
enum {
#define V(name) WASM_SECTION_INDEX_##name,
  WASM_FOREACH_SECTION(V)
#undef V
  WASM_NUM_SECTIONS
};
/* clang-format on */

typedef struct Context {
  const uint8_t* data;
  size_t size;
  size_t offset;
  WasmBinaryReader* reader;
  jmp_buf error_jmp_buf;
  WasmTypeVector param_types;
  Uint32Vector target_depths;
} Context;

static void WASM_PRINTF_FORMAT(2, 3)
    raise_error(Context* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  assert(ctx->reader->on_error);
  ctx->reader->on_error(ctx->offset, buffer, ctx->reader->user_data);
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

static WasmBool is_non_void_type(uint8_t type) {
  return type != 0 && type < WASM_NUM_TYPES;
}

static WasmBool is_bool(uint8_t value) {
  return value < 2;
}

static WasmBool skip_until_section(Context* ctx, int section_index) {
  uint32_t section_start_offset = ctx->offset;
  uint32_t section_size = 0;
  if (ctx->offset == ctx->size) {
    /* ok, no more sections */
    return WASM_FALSE;
  }

  WasmStringSlice section_name;
  in_str(ctx, &section_name, "section name");
  in_u32_leb128(ctx, &section_size, "section size");

  if (ctx->offset + section_size > ctx->size)
    RAISE_ERROR("invalid section size: extends past end");

  int index = -1;
#define V(name)                                                  \
  else if (strncmp(section_name.start, WASM_SECTION_NAME_##name, \
                   section_name.length) == 0) {                  \
    index = WASM_SECTION_INDEX_##name;                           \
  }
  if (0) {
  }
  WASM_FOREACH_SECTION(V)
#undef V

  if (index == -1) {
    /* ok, unknown section, skip it. */
    ctx->offset += section_size;
    return 0;
  } else if (index < section_index) {
    RAISE_ERROR("section " PRIstringslice " out of order",
                WASM_PRINTF_STRING_SLICE_ARG(section_name));
  } else if (index > section_index) {
    /* ok, future section. Reset the offset. */
    /* TODO(binji): slightly inefficient to re-read the section later. But
     there aren't many so it probably doesn't matter */
    ctx->offset = section_start_offset;
    return WASM_FALSE;
  }

  assert(index == section_index);
  return WASM_TRUE;
}

static void destroy_context(WasmAllocator* allocator, Context* ctx) {
  wasm_destroy_type_vector(allocator, &ctx->param_types);
  wasm_destroy_uint32_vector(allocator, &ctx->target_depths);
}

WasmResult wasm_read_binary(WasmAllocator* allocator,
                            const void* data,
                            size_t size,
                            WasmBinaryReader* reader,
                            uint32_t num_function_passes,
                            const WasmReadBinaryOptions* options) {
  Context context;
  WASM_ZERO_MEMORY(context);
  /* all the macros assume a Context* named ctx */
  Context* ctx = &context;
  ctx->data = data;
  ctx->size = size;
  ctx->reader = reader;

  if (setjmp(ctx->error_jmp_buf) == 1) {
    destroy_context(allocator, ctx);
    return WASM_ERROR;
  }

  CHECK_ALLOC(wasm_reserve_types(allocator, &ctx->param_types,
                                 INITIAL_PARAM_TYPES_CAPACITY));
  CHECK_ALLOC(wasm_reserve_uint32s(allocator, &ctx->target_depths,
                                   INITIAL_BR_TABLE_TARGET_CAPACITY));

  CALLBACK0(begin_module);

  uint32_t magic;
  in_u32(ctx, &magic, "magic");
  RAISE_ERROR_UNLESS(magic == WASM_BINARY_MAGIC, "magic value mismatch");
  uint32_t version;
  in_u32(ctx, &version, "version");
  RAISE_ERROR_UNLESS(version == WASM_BINARY_VERSION, "version mismatch");

  /* type */
  uint32_t num_signatures = 0;
  if (skip_until_section(ctx, WASM_SECTION_INDEX_TYPE)) {
    CALLBACK0(begin_signature_section);
    uint32_t i;
    in_u32_leb128(ctx, &num_signatures, "type count");
    CALLBACK(on_signature_count, num_signatures);

    for (i = 0; i < num_signatures; ++i) {
      uint8_t form;
      in_u8(ctx, &form, "type form");
      RAISE_ERROR_UNLESS(form == WASM_BINARY_TYPE_FORM_FUNCTION,
                         "unexpected type form");

      uint32_t num_params;
      in_u32_leb128(ctx, &num_params, "function param count");

      if (num_params > ctx->param_types.capacity) {
        CHECK_ALLOC(
            wasm_reserve_types(allocator, &ctx->param_types, num_params));
      }

      uint32_t j;
      for (j = 0; j < num_params; ++j) {
        uint8_t param_type;
        in_u8(ctx, &param_type, "function param type");
        RAISE_ERROR_UNLESS(is_non_void_type(param_type),
                           "expected valid param type");
        ctx->param_types.data[j] = param_type;
      }

      uint32_t num_results;
      in_u32_leb128(ctx, &num_results, "function result count");
      RAISE_ERROR_UNLESS(num_results <= 1, "result count must be 0 or 1");

      uint8_t result_type = WASM_TYPE_VOID;
      if (num_results) {
        in_u8(ctx, &result_type, "function result type");
        RAISE_ERROR_UNLESS(is_non_void_type(result_type),
                           "expected valid result type");
      }

      CALLBACK(on_signature, i, (WasmType)result_type, num_params,
               ctx->param_types.data);
    }
    CALLBACK0(end_signature_section);
  }

  /* import */
  uint32_t num_imports = 0;
  if (skip_until_section(ctx, WASM_SECTION_INDEX_IMPORT)) {
    CALLBACK0(begin_import_section);
    uint32_t i;
    in_u32_leb128(ctx, &num_imports, "import count");
    CALLBACK(on_import_count, num_imports);
    for (i = 0; i < num_imports; ++i) {
      uint32_t sig_index;
      in_u32_leb128(ctx, &sig_index, "import signature index");
      RAISE_ERROR_UNLESS(sig_index < num_signatures,
                         "invalid import signature index");

      WasmStringSlice module_name;
      in_str(ctx, &module_name, "import module name");

      WasmStringSlice function_name;
      in_str(ctx, &function_name, "import function name");

      CALLBACK(on_import, i, sig_index, module_name, function_name);
    }
    CALLBACK0(end_import_section);
  }

  /* function */
  uint32_t num_function_signatures = 0;
  if (skip_until_section(ctx, WASM_SECTION_INDEX_FUNCTION)) {
    CALLBACK0(begin_function_signatures_section);
    uint32_t i;
    in_u32_leb128(ctx, &num_function_signatures, "function signature count");
    CALLBACK(on_function_signatures_count, num_function_signatures);
    for (i = 0; i < num_function_signatures; ++i) {
      uint32_t sig_index;
      in_u32_leb128(ctx, &sig_index, "function signature index");
      RAISE_ERROR_UNLESS(sig_index < num_signatures,
                         "invalid function signature index");
      CALLBACK(on_function_signature, i, sig_index);
    }
    CALLBACK0(end_function_signatures_section);
  }

  /* table */
  uint32_t num_function_table_entries;
  if (skip_until_section(ctx, WASM_SECTION_INDEX_TABLE)) {
    CALLBACK0(begin_function_table_section);
    uint32_t i;
    in_u32_leb128(ctx, &num_function_table_entries,
                  "function table entry count");
    CALLBACK(on_function_table_count, num_function_table_entries);
    for (i = 0; i < num_function_table_entries; ++i) {
      uint32_t func_index;
      in_u32_leb128(ctx, &func_index, "function table function index");
      RAISE_ERROR_UNLESS(func_index < num_function_signatures,
                         "invalid function table function index");
      CALLBACK(on_function_table_entry, i, func_index);
    }
    CALLBACK0(end_function_table_section);
  }

  /* memory */
  WasmBool seen_memory_section = WASM_FALSE;
  if (skip_until_section(ctx, WASM_SECTION_INDEX_MEMORY)) {
    seen_memory_section = WASM_TRUE;
    CALLBACK0(begin_memory_section);
    uint32_t initial_size_pages;
    in_u32_leb128(ctx, &initial_size_pages, "memory initial size");
    CALLBACK(on_memory_initial_size_pages, initial_size_pages);
    RAISE_ERROR_UNLESS(initial_size_pages <= WASM_MAX_PAGES,
                       "invalid memory initial size");

    uint32_t max_size_pages;
    in_u32_leb128(ctx, &max_size_pages, "memory max size");
    CALLBACK(on_memory_max_size_pages, max_size_pages);
    RAISE_ERROR_UNLESS(max_size_pages <= WASM_MAX_PAGES,
                       "invalid memory max size");

    RAISE_ERROR_UNLESS(initial_size_pages <= max_size_pages,
                       "memory initial size must be <= max size");

    uint8_t mem_exported;
    in_u8(ctx, &mem_exported, "memory export");
    RAISE_ERROR_UNLESS(is_bool(mem_exported), "expected valid mem export flag");
    CALLBACK(on_memory_exported, mem_exported);
    CALLBACK0(end_memory_section);
  }

  /* export */
  uint32_t num_exports = 0;
  if (skip_until_section(ctx, WASM_SECTION_INDEX_EXPORT)) {
    CALLBACK0(begin_export_section);
    uint32_t i;
    in_u32_leb128(ctx, &num_exports, "export count");
    CALLBACK(on_export_count, num_exports);
    for (i = 0; i < num_exports; ++i) {
      uint32_t func_index;
      in_u32_leb128(ctx, &func_index, "export function index");
      RAISE_ERROR_UNLESS(func_index < num_function_signatures,
                         "invalid export function index");

      WasmStringSlice name;
      in_str(ctx, &name, "export function name");

      CALLBACK(on_export, i, func_index, name);
    }
    CALLBACK0(end_export_section);
  }

  /* start */
  if (skip_until_section(ctx, WASM_SECTION_INDEX_START)) {
    CALLBACK0(begin_start_section);
    uint32_t func_index;
    in_u32_leb128(ctx, &func_index, "start function index");
    RAISE_ERROR_UNLESS(func_index < num_function_signatures,
                       "invalid start function index");
    CALLBACK(on_start_function, func_index);
    CALLBACK0(end_start_section);
  }

  /* code */
  uint32_t num_function_bodies = 0;
  if (skip_until_section(ctx, WASM_SECTION_INDEX_CODE)) {
    CALLBACK0(begin_function_bodies_section);
    uint32_t i;
    in_u32_leb128(ctx, &num_function_bodies, "function body count");
    RAISE_ERROR_UNLESS(num_function_signatures == num_function_bodies,
                       "function signature count != function body count");
    CALLBACK(on_function_bodies_count, num_function_bodies);
    for (i = 0; i < num_function_bodies; ++i) {
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
          uint8_t local_type;
          in_u8(ctx, &local_type, "local type");
          RAISE_ERROR_UNLESS(is_non_void_type(local_type),
                             "expected valid local type");
          CALLBACK(on_local_decl, k, num_local_types, local_type);
        }

        while (ctx->offset < end_offset) {
          uint8_t opcode;
          in_u8(ctx, &opcode, "opcode");
          switch (opcode) {
            case WASM_OPCODE_NOP:
              CALLBACK0(on_nop_expr);
              break;

            case WASM_OPCODE_BLOCK:
              CALLBACK0(on_block_expr);
              break;

            case WASM_OPCODE_LOOP:
              CALLBACK0(on_loop_expr);
              break;

            case WASM_OPCODE_IF:
              CALLBACK0(on_if_expr);
              break;

            case WASM_OPCODE_ELSE:
              CALLBACK0(on_else_expr);
              break;

            case WASM_OPCODE_SELECT:
              CALLBACK0(on_select_expr);
              break;

            case WASM_OPCODE_BR: {
              uint8_t arity;
              in_u8(ctx, &arity, "br arity");
              uint32_t depth;
              in_u32_leb128(ctx, &depth, "br depth");
              CALLBACK(on_br_expr, arity, depth);
              break;
            }

            case WASM_OPCODE_BR_IF: {
              uint8_t arity;
              in_u8(ctx, &arity, "br_if arity");
              uint32_t depth;
              in_u32_leb128(ctx, &depth, "br_if depth");
              CALLBACK(on_br_if_expr, arity, depth);
              break;
            }

            case WASM_OPCODE_BR_TABLE: {
              uint8_t arity;
              in_u8(ctx, &arity, "br_table arity");
              uint32_t num_targets;
              in_u32_leb128(ctx, &num_targets, "br_table target count");
              if (num_targets > ctx->target_depths.capacity) {
                CHECK_ALLOC(wasm_reserve_uint32s(allocator, &ctx->target_depths,
                                                 num_targets));
                ctx->target_depths.size = num_targets;
              }

              uint32_t i;
              for (i = 0; i < num_targets; ++i) {
                uint32_t target_depth;
                in_u32(ctx, &target_depth, "br_table target depth");
                ctx->target_depths.data[i] = target_depth;
              }

              uint32_t default_target_depth;
              in_u32(ctx, &default_target_depth,
                     "br_table default target depth");

              CALLBACK(on_br_table_expr, arity, num_targets,
                       ctx->target_depths.data, default_target_depth);
              break;
            }

            case WASM_OPCODE_RETURN: {
              uint8_t arity;
              in_u8(ctx, &arity, "return arity");
              CALLBACK(on_return_expr, arity);
              break;
            }

            case WASM_OPCODE_UNREACHABLE:
              CALLBACK0(on_unreachable_expr);
              break;

            case WASM_OPCODE_END:
              CALLBACK0(on_end_expr);
              break;

            case WASM_OPCODE_I32_CONST: {
              uint32_t value = 0;
              in_i32_leb128(ctx, &value, "i32.const value");
              CALLBACK(on_i32_const_expr, value);
              break;
            }

            case WASM_OPCODE_I64_CONST: {
              uint64_t value = 0;
              in_i64_leb128(ctx, &value, "i64.const value");
              CALLBACK(on_i64_const_expr, value);
              break;
            }

            case WASM_OPCODE_F32_CONST: {
              uint32_t value_bits = 0;
              in_f32(ctx, &value_bits, "f32.const value");
              CALLBACK(on_f32_const_expr, value_bits);
              break;
            }

            case WASM_OPCODE_F64_CONST: {
              uint64_t value_bits = 0;
              in_f64(ctx, &value_bits, "f64.const value");
              CALLBACK(on_f64_const_expr, value_bits);
              break;
            }

            case WASM_OPCODE_GET_LOCAL: {
              uint32_t local_index;
              in_u32_leb128(ctx, &local_index, "get_local local index");
              CALLBACK(on_get_local_expr, local_index);
              break;
            }

            case WASM_OPCODE_SET_LOCAL: {
              uint32_t local_index;
              in_u32_leb128(ctx, &local_index, "set_local local index");
              CALLBACK(on_set_local_expr, local_index);
              break;
            }

            case WASM_OPCODE_CALL_FUNCTION: {
              uint32_t arity;
              in_u32_leb128(ctx, &arity, "call_function arity");
              uint32_t func_index;
              in_u32_leb128(ctx, &func_index, "call_function function index");
              RAISE_ERROR_UNLESS(func_index < num_function_signatures,
                                 "invalid call_function function index");
              CALLBACK(on_call_expr, arity, func_index);
              break;
            }

            case WASM_OPCODE_CALL_INDIRECT: {
              uint32_t arity;
              in_u32_leb128(ctx, &arity, "call_indirect arity");
              uint32_t sig_index;
              in_u32_leb128(ctx, &sig_index, "call_indirect signature index");
              RAISE_ERROR_UNLESS(sig_index < num_function_signatures,
                                 "invalid call_indirect signature index");
              CALLBACK(on_call_indirect_expr, arity, sig_index);
              break;
            }

            case WASM_OPCODE_CALL_IMPORT: {
              uint32_t arity;
              in_u32_leb128(ctx, &arity, "call_import arity");
              uint32_t import_index;
              in_u32_leb128(ctx, &import_index, "call_import import index");
              RAISE_ERROR_UNLESS(import_index < num_imports,
                                 "invalid call_import import index");
              CALLBACK(on_call_import_expr, arity, import_index);
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
              break;
            }

            case WASM_OPCODE_CURRENT_MEMORY:
              CALLBACK0(on_current_memory_expr);
              break;

            case WASM_OPCODE_GROW_MEMORY:
              CALLBACK0(on_grow_memory_expr);
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
              break;

            default:
              RAISE_ERROR("unexpected opcode: %d (0x%x)", opcode, opcode);
          }
        }
        RAISE_ERROR_UNLESS(ctx->offset == end_offset,
                           "function body longer than given size");
        CALLBACK(end_function_body, i);
        CALLBACK(end_function_body_pass, i, j);
      }
    }
    CALLBACK0(end_function_bodies_section);
  }

  /* data */
  if (skip_until_section(ctx, WASM_SECTION_INDEX_DATA)) {
    RAISE_ERROR_UNLESS(seen_memory_section,
                       "data segment section without memory section");
    CALLBACK0(begin_data_segment_section);
    uint32_t i, num_data_segments;
    in_u32_leb128(ctx, &num_data_segments, "data segment count");
    CALLBACK(on_data_segment_count, num_data_segments);
    for (i = 0; i < num_data_segments; ++i) {
      uint32_t address;
      in_u32_leb128(ctx, &address, "data segment address");

      uint32_t data_size;
      const void* data;
      in_bytes(ctx, &data, &data_size, "data segment data");

      CALLBACK(on_data_segment, i, address, data, data_size);
    }
    CALLBACK0(end_data_segment_section);
  }

  /* name */
  if (options->read_debug_names &&
      skip_until_section(ctx, WASM_SECTION_INDEX_NAME)) {
    CALLBACK0(begin_names_section);
    uint32_t i, num_functions;
    in_u32_leb128(ctx, &num_functions, "function name count");
    RAISE_ERROR_UNLESS(num_functions <= num_function_signatures,
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
  }

  CALLBACK0(end_module);
  destroy_context(allocator, ctx);
  return WASM_OK;
}

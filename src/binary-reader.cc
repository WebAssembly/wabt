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

#include "binary-reader.h"

#include <assert.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include "binary.h"
#include "binary-reader-logging.h"
#include "config.h"
#include "stream.h"

#if HAVE_ALLOCA
#include <alloca.h>
#endif

namespace wabt {

namespace {

#define CALLBACK0(member)                                   \
  RAISE_ERROR_UNLESS(WABT_SUCCEEDED(ctx->reader->member()), \
                     #member " callback failed")

#define CALLBACK(member, ...)                                          \
  RAISE_ERROR_UNLESS(WABT_SUCCEEDED(ctx->reader->member(__VA_ARGS__)), \
                     #member " callback failed")

#define RAISE_ERROR(...) raise_error(ctx, __VA_ARGS__)

#define RAISE_ERROR_UNLESS(cond, ...) \
  if (!(cond))                        \
    RAISE_ERROR(__VA_ARGS__);

struct Context {
  size_t read_end = 0; /* Either the section end or data_size. */
  BinaryReader* reader = nullptr;
  BinaryReader::State state;
  jmp_buf error_jmp_buf;
  TypeVector param_types;
  std::vector<uint32_t> target_depths;
  const ReadBinaryOptions* options = nullptr;
  BinarySection last_known_section = BinarySection::Invalid;
  uint32_t num_signatures = 0;
  uint32_t num_imports = 0;
  uint32_t num_func_imports = 0;
  uint32_t num_table_imports = 0;
  uint32_t num_memory_imports = 0;
  uint32_t num_global_imports = 0;
  uint32_t num_function_signatures = 0;
  uint32_t num_tables = 0;
  uint32_t num_memories = 0;
  uint32_t num_globals = 0;
  uint32_t num_exports = 0;
  uint32_t num_function_bodies = 0;
};

}  // namespace

static void WABT_PRINTF_FORMAT(2, 3)
    raise_error(Context* ctx, const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  bool handled = ctx->reader->OnError(buffer);

  if (!handled) {
    /* Not great to just print, but we don't want to eat the error either. */
    fprintf(stderr, "*ERROR*: @0x%08zx: %s\n", ctx->state.offset, buffer);
  }
  longjmp(ctx->error_jmp_buf, 1);
}

#define IN_SIZE(type)                                                   \
  if (ctx->state.offset + sizeof(type) > ctx->read_end) {               \
    RAISE_ERROR("unable to read " #type ": %s", desc);                  \
  }                                                                     \
  memcpy(out_value, ctx->state.data + ctx->state.offset, sizeof(type)); \
  ctx->state.offset += sizeof(type)

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

#define BYTE_AT(type, i, shift) ((static_cast<type>(p[i]) & 0x7f) << (shift))

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
#define SIGN_EXTEND(type, value, sign_bit)                       \
  (static_cast<type>((value) << SHIFT_AMOUNT(type, sign_bit)) >> \
   SHIFT_AMOUNT(type, sign_bit))

size_t read_u32_leb128(const uint8_t* p,
                       const uint8_t* end,
                       uint32_t* out_value) {
  if (p < end && (p[0] & 0x80) == 0) {
    *out_value = LEB128_1(uint32_t);
    return 1;
  } else if (p + 1 < end && (p[1] & 0x80) == 0) {
    *out_value = LEB128_2(uint32_t);
    return 2;
  } else if (p + 2 < end && (p[2] & 0x80) == 0) {
    *out_value = LEB128_3(uint32_t);
    return 3;
  } else if (p + 3 < end && (p[3] & 0x80) == 0) {
    *out_value = LEB128_4(uint32_t);
    return 4;
  } else if (p + 4 < end && (p[4] & 0x80) == 0) {
    /* the top bits set represent values > 32 bits */
    if (p[4] & 0xf0)
      return 0;
    *out_value = LEB128_5(uint32_t);
    return 5;
  } else {
    /* past the end */
    *out_value = 0;
    return 0;
  }
}

static void in_u32_leb128(Context* ctx, uint32_t* out_value, const char* desc) {
  const uint8_t* p = ctx->state.data + ctx->state.offset;
  const uint8_t* end = ctx->state.data + ctx->read_end;
  size_t bytes_read = read_u32_leb128(p, end, out_value);
  if (!bytes_read)
    RAISE_ERROR("unable to read u32 leb128: %s", desc);
  ctx->state.offset += bytes_read;
}

size_t read_i32_leb128(const uint8_t* p,
                       const uint8_t* end,
                       uint32_t* out_value) {
  if (p < end && (p[0] & 0x80) == 0) {
    uint32_t result = LEB128_1(uint32_t);
    *out_value = SIGN_EXTEND(int32_t, result, 6);
    return 1;
  } else if (p + 1 < end && (p[1] & 0x80) == 0) {
    uint32_t result = LEB128_2(uint32_t);
    *out_value = SIGN_EXTEND(int32_t, result, 13);
    return 2;
  } else if (p + 2 < end && (p[2] & 0x80) == 0) {
    uint32_t result = LEB128_3(uint32_t);
    *out_value = SIGN_EXTEND(int32_t, result, 20);
    return 3;
  } else if (p + 3 < end && (p[3] & 0x80) == 0) {
    uint32_t result = LEB128_4(uint32_t);
    *out_value = SIGN_EXTEND(int32_t, result, 27);
    return 4;
  } else if (p + 4 < end && (p[4] & 0x80) == 0) {
    /* the top bits should be a sign-extension of the sign bit */
    bool sign_bit_set = (p[4] & 0x8);
    int top_bits = p[4] & 0xf0;
    if ((sign_bit_set && top_bits != 0x70) ||
        (!sign_bit_set && top_bits != 0)) {
      return 0;
    }
    uint32_t result = LEB128_5(uint32_t);
    *out_value = result;
    return 5;
  } else {
    /* past the end */
    return 0;
  }
}

static void in_i32_leb128(Context* ctx, uint32_t* out_value, const char* desc) {
  const uint8_t* p = ctx->state.data + ctx->state.offset;
  const uint8_t* end = ctx->state.data + ctx->read_end;
  size_t bytes_read = read_i32_leb128(p, end, out_value);
  if (!bytes_read)
    RAISE_ERROR("unable to read i32 leb128: %s", desc);
  ctx->state.offset += bytes_read;
}

static void in_i64_leb128(Context* ctx, uint64_t* out_value, const char* desc) {
  const uint8_t* p = ctx->state.data + ctx->state.offset;
  const uint8_t* end = ctx->state.data + ctx->read_end;

  if (p < end && (p[0] & 0x80) == 0) {
    uint64_t result = LEB128_1(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 6);
    ctx->state.offset += 1;
  } else if (p + 1 < end && (p[1] & 0x80) == 0) {
    uint64_t result = LEB128_2(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 13);
    ctx->state.offset += 2;
  } else if (p + 2 < end && (p[2] & 0x80) == 0) {
    uint64_t result = LEB128_3(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 20);
    ctx->state.offset += 3;
  } else if (p + 3 < end && (p[3] & 0x80) == 0) {
    uint64_t result = LEB128_4(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 27);
    ctx->state.offset += 4;
  } else if (p + 4 < end && (p[4] & 0x80) == 0) {
    uint64_t result = LEB128_5(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 34);
    ctx->state.offset += 5;
  } else if (p + 5 < end && (p[5] & 0x80) == 0) {
    uint64_t result = LEB128_6(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 41);
    ctx->state.offset += 6;
  } else if (p + 6 < end && (p[6] & 0x80) == 0) {
    uint64_t result = LEB128_7(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 48);
    ctx->state.offset += 7;
  } else if (p + 7 < end && (p[7] & 0x80) == 0) {
    uint64_t result = LEB128_8(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 55);
    ctx->state.offset += 8;
  } else if (p + 8 < end && (p[8] & 0x80) == 0) {
    uint64_t result = LEB128_9(uint64_t);
    *out_value = SIGN_EXTEND(int64_t, result, 62);
    ctx->state.offset += 9;
  } else if (p + 9 < end && (p[9] & 0x80) == 0) {
    /* the top bits should be a sign-extension of the sign bit */
    bool sign_bit_set = (p[9] & 0x1);
    int top_bits = p[9] & 0xfe;
    if ((sign_bit_set && top_bits != 0x7e) ||
        (!sign_bit_set && top_bits != 0)) {
      RAISE_ERROR("invalid i64 leb128: %s", desc);
    }
    uint64_t result = LEB128_10(uint64_t);
    *out_value = result;
    ctx->state.offset += 10;
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

static void in_type(Context* ctx, Type* out_value, const char* desc) {
  uint32_t type = 0;
  in_i32_leb128(ctx, &type, desc);
  /* Must be in the vs7 range: [-128, 127). */
  if (static_cast<int32_t>(type) < -128 || static_cast<int32_t>(type) > 127)
    RAISE_ERROR("invalid type: %d", type);
  *out_value = static_cast<Type>(type);
}

static void in_str(Context* ctx, StringSlice* out_str, const char* desc) {
  uint32_t str_len = 0;
  in_u32_leb128(ctx, &str_len, "string length");

  if (ctx->state.offset + str_len > ctx->read_end)
    RAISE_ERROR("unable to read string: %s", desc);

  out_str->start =
      reinterpret_cast<const char*>(ctx->state.data) + ctx->state.offset;
  out_str->length = str_len;
  ctx->state.offset += str_len;
}

static void in_bytes(Context* ctx,
                     const void** out_data,
                     uint32_t* out_data_size,
                     const char* desc) {
  uint32_t data_size = 0;
  in_u32_leb128(ctx, &data_size, "data size");

  if (ctx->state.offset + data_size > ctx->read_end)
    RAISE_ERROR("unable to read data: %s", desc);

  *out_data =
      static_cast<const uint8_t*>(ctx->state.data) + ctx->state.offset;
  *out_data_size = data_size;
  ctx->state.offset += data_size;
}

static bool is_valid_external_kind(uint8_t kind) {
  return kind < kExternalKindCount;
}

static bool is_concrete_type(Type type) {
  switch (type) {
    case Type::I32:
    case Type::I64:
    case Type::F32:
    case Type::F64:
      return true;

    default:
      return false;
  }
}

static bool is_inline_sig_type(Type type) {
  return is_concrete_type(type) || type == Type::Void;
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

static void read_init_expr(Context* ctx, uint32_t index) {
  uint8_t opcode;
  in_u8(ctx, &opcode, "opcode");
  switch (static_cast<Opcode>(opcode)) {
    case Opcode::I32Const: {
      uint32_t value = 0;
      in_i32_leb128(ctx, &value, "init_expr i32.const value");
      CALLBACK(OnInitExprI32ConstExpr, index, value);
      break;
    }

    case Opcode::I64Const: {
      uint64_t value = 0;
      in_i64_leb128(ctx, &value, "init_expr i64.const value");
      CALLBACK(OnInitExprI64ConstExpr, index, value);
      break;
    }

    case Opcode::F32Const: {
      uint32_t value_bits = 0;
      in_f32(ctx, &value_bits, "init_expr f32.const value");
      CALLBACK(OnInitExprF32ConstExpr, index, value_bits);
      break;
    }

    case Opcode::F64Const: {
      uint64_t value_bits = 0;
      in_f64(ctx, &value_bits, "init_expr f64.const value");
      CALLBACK(OnInitExprF64ConstExpr, index, value_bits);
      break;
    }

    case Opcode::GetGlobal: {
      uint32_t global_index;
      in_u32_leb128(ctx, &global_index, "init_expr get_global index");
      CALLBACK(OnInitExprGetGlobalExpr, index, global_index);
      break;
    }

    case Opcode::End:
      return;

    default:
      RAISE_ERROR("unexpected opcode in initializer expression: %d (0x%x)",
                  opcode, opcode);
      break;
  }

  in_u8(ctx, &opcode, "opcode");
  RAISE_ERROR_UNLESS(static_cast<Opcode>(opcode) == Opcode::End,
                     "expected END opcode after initializer expression");
}

static void read_table(Context* ctx,
                       Type* out_elem_type,
                       Limits* out_elem_limits) {
  in_type(ctx, out_elem_type, "table elem type");
  RAISE_ERROR_UNLESS(*out_elem_type == Type::Anyfunc,
                     "table elem type must by anyfunc");

  uint32_t flags;
  uint32_t initial;
  uint32_t max = 0;
  in_u32_leb128(ctx, &flags, "table flags");
  in_u32_leb128(ctx, &initial, "table initial elem count");
  bool has_max = flags & WABT_BINARY_LIMITS_HAS_MAX_FLAG;
  if (has_max) {
    in_u32_leb128(ctx, &max, "table max elem count");
    RAISE_ERROR_UNLESS(initial <= max,
                       "table initial elem count must be <= max elem count");
  }

  out_elem_limits->has_max = has_max;
  out_elem_limits->initial = initial;
  out_elem_limits->max = max;
}

static void read_memory(Context* ctx, Limits* out_page_limits) {
  uint32_t flags;
  uint32_t initial;
  uint32_t max = 0;
  in_u32_leb128(ctx, &flags, "memory flags");
  in_u32_leb128(ctx, &initial, "memory initial page count");
  bool has_max = flags & WABT_BINARY_LIMITS_HAS_MAX_FLAG;
  RAISE_ERROR_UNLESS(initial <= WABT_MAX_PAGES, "invalid memory initial size");
  if (has_max) {
    in_u32_leb128(ctx, &max, "memory max page count");
    RAISE_ERROR_UNLESS(max <= WABT_MAX_PAGES, "invalid memory max size");
    RAISE_ERROR_UNLESS(initial <= max,
                       "memory initial size must be <= max size");
  }

  out_page_limits->has_max = has_max;
  out_page_limits->initial = initial;
  out_page_limits->max = max;
}

static void read_global_header(Context* ctx,
                               Type* out_type,
                               bool* out_mutable) {
  Type global_type;
  uint8_t mutable_;
  in_type(ctx, &global_type, "global type");
  RAISE_ERROR_UNLESS(is_concrete_type(global_type),
                     "invalid global type: %#x", static_cast<int>(global_type));

  in_u8(ctx, &mutable_, "global mutability");
  RAISE_ERROR_UNLESS(mutable_ <= 1, "global mutability must be 0 or 1");

  *out_type = global_type;
  *out_mutable = mutable_;
}

static void read_function_body(Context* ctx, uint32_t end_offset) {
  bool seen_end_opcode = false;
  while (ctx->state.offset < end_offset) {
    uint8_t opcode_u8;
    in_u8(ctx, &opcode_u8, "opcode");
    Opcode opcode = static_cast<Opcode>(opcode_u8);
    CALLBACK(OnOpcode, opcode);
    switch (opcode) {
      case Opcode::Unreachable:
        CALLBACK0(OnUnreachableExpr);
        CALLBACK0(OnOpcodeBare);
        break;

      case Opcode::Block: {
        Type sig_type;
        in_type(ctx, &sig_type, "block signature type");
        RAISE_ERROR_UNLESS(is_inline_sig_type(sig_type),
                           "expected valid block signature type");
        uint32_t num_types = sig_type == Type::Void ? 0 : 1;
        CALLBACK(OnBlockExpr, num_types, &sig_type);
        CALLBACK(OnOpcodeBlockSig, num_types, &sig_type);
        break;
      }

      case Opcode::Loop: {
        Type sig_type;
        in_type(ctx, &sig_type, "loop signature type");
        RAISE_ERROR_UNLESS(is_inline_sig_type(sig_type),
                           "expected valid block signature type");
        uint32_t num_types = sig_type == Type::Void ? 0 : 1;
        CALLBACK(OnLoopExpr, num_types, &sig_type);
        CALLBACK(OnOpcodeBlockSig, num_types, &sig_type);
        break;
      }

      case Opcode::If: {
        Type sig_type;
        in_type(ctx, &sig_type, "if signature type");
        RAISE_ERROR_UNLESS(is_inline_sig_type(sig_type),
                           "expected valid block signature type");
        uint32_t num_types = sig_type == Type::Void ? 0 : 1;
        CALLBACK(OnIfExpr, num_types, &sig_type);
        CALLBACK(OnOpcodeBlockSig, num_types, &sig_type);
        break;
      }

      case Opcode::Else:
        CALLBACK0(OnElseExpr);
        CALLBACK0(OnOpcodeBare);
        break;

      case Opcode::Select:
        CALLBACK0(OnSelectExpr);
        CALLBACK0(OnOpcodeBare);
        break;

      case Opcode::Br: {
        uint32_t depth;
        in_u32_leb128(ctx, &depth, "br depth");
        CALLBACK(OnBrExpr, depth);
        CALLBACK(OnOpcodeUint32, depth);
        break;
      }

      case Opcode::BrIf: {
        uint32_t depth;
        in_u32_leb128(ctx, &depth, "br_if depth");
        CALLBACK(OnBrIfExpr, depth);
        CALLBACK(OnOpcodeUint32, depth);
        break;
      }

      case Opcode::BrTable: {
        uint32_t num_targets;
        in_u32_leb128(ctx, &num_targets, "br_table target count");
        ctx->target_depths.resize(num_targets);

        for (uint32_t i = 0; i < num_targets; ++i) {
          uint32_t target_depth;
          in_u32_leb128(ctx, &target_depth, "br_table target depth");
          ctx->target_depths[i] = target_depth;
        }

        uint32_t default_target_depth;
        in_u32_leb128(ctx, &default_target_depth,
                      "br_table default target depth");

        uint32_t* target_depths =
            num_targets ? ctx->target_depths.data() : nullptr;

        CALLBACK(OnBrTableExpr, num_targets, target_depths,
                 default_target_depth);
        break;
      }

      case Opcode::Return:
        CALLBACK0(OnReturnExpr);
        CALLBACK0(OnOpcodeBare);
        break;

      case Opcode::Nop:
        CALLBACK0(OnNopExpr);
        CALLBACK0(OnOpcodeBare);
        break;

      case Opcode::Drop:
        CALLBACK0(OnDropExpr);
        CALLBACK0(OnOpcodeBare);
        break;

      case Opcode::End:
        if (ctx->state.offset == end_offset) {
          seen_end_opcode = true;
          CALLBACK0(OnEndFunc);
        } else {
          CALLBACK0(OnEndExpr);
        }
        break;

      case Opcode::I32Const: {
        uint32_t value = 0;
        in_i32_leb128(ctx, &value, "i32.const value");
        CALLBACK(OnI32ConstExpr, value);
        CALLBACK(OnOpcodeUint32, value);
        break;
      }

      case Opcode::I64Const: {
        uint64_t value = 0;
        in_i64_leb128(ctx, &value, "i64.const value");
        CALLBACK(OnI64ConstExpr, value);
        CALLBACK(OnOpcodeUint64, value);
        break;
      }

      case Opcode::F32Const: {
        uint32_t value_bits = 0;
        in_f32(ctx, &value_bits, "f32.const value");
        CALLBACK(OnF32ConstExpr, value_bits);
        CALLBACK(OnOpcodeF32, value_bits);
        break;
      }

      case Opcode::F64Const: {
        uint64_t value_bits = 0;
        in_f64(ctx, &value_bits, "f64.const value");
        CALLBACK(OnF64ConstExpr, value_bits);
        CALLBACK(OnOpcodeF64, value_bits);
        break;
      }

      case Opcode::GetGlobal: {
        uint32_t global_index;
        in_u32_leb128(ctx, &global_index, "get_global global index");
        CALLBACK(OnGetGlobalExpr, global_index);
        CALLBACK(OnOpcodeUint32, global_index);
        break;
      }

      case Opcode::GetLocal: {
        uint32_t local_index;
        in_u32_leb128(ctx, &local_index, "get_local local index");
        CALLBACK(OnGetLocalExpr, local_index);
        CALLBACK(OnOpcodeUint32, local_index);
        break;
      }

      case Opcode::SetGlobal: {
        uint32_t global_index;
        in_u32_leb128(ctx, &global_index, "set_global global index");
        CALLBACK(OnSetGlobalExpr, global_index);
        CALLBACK(OnOpcodeUint32, global_index);
        break;
      }

      case Opcode::SetLocal: {
        uint32_t local_index;
        in_u32_leb128(ctx, &local_index, "set_local local index");
        CALLBACK(OnSetLocalExpr, local_index);
        CALLBACK(OnOpcodeUint32, local_index);
        break;
      }

      case Opcode::Call: {
        uint32_t func_index;
        in_u32_leb128(ctx, &func_index, "call function index");
        RAISE_ERROR_UNLESS(func_index < num_total_funcs(ctx),
                           "invalid call function index: %d", func_index);
        CALLBACK(OnCallExpr, func_index);
        CALLBACK(OnOpcodeUint32, func_index);
        break;
      }

      case Opcode::CallIndirect: {
        uint32_t sig_index;
        in_u32_leb128(ctx, &sig_index, "call_indirect signature index");
        RAISE_ERROR_UNLESS(sig_index < ctx->num_signatures,
                           "invalid call_indirect signature index");
        uint32_t reserved;
        in_u32_leb128(ctx, &reserved, "call_indirect reserved");
        RAISE_ERROR_UNLESS(reserved == 0,
                           "call_indirect reserved value must be 0");
        CALLBACK(OnCallIndirectExpr, sig_index);
        CALLBACK(OnOpcodeUint32Uint32, sig_index, reserved);
        break;
      }

      case Opcode::TeeLocal: {
        uint32_t local_index;
        in_u32_leb128(ctx, &local_index, "tee_local local index");
        CALLBACK(OnTeeLocalExpr, local_index);
        CALLBACK(OnOpcodeUint32, local_index);
        break;
      }

      case Opcode::I32Load8S:
      case Opcode::I32Load8U:
      case Opcode::I32Load16S:
      case Opcode::I32Load16U:
      case Opcode::I64Load8S:
      case Opcode::I64Load8U:
      case Opcode::I64Load16S:
      case Opcode::I64Load16U:
      case Opcode::I64Load32S:
      case Opcode::I64Load32U:
      case Opcode::I32Load:
      case Opcode::I64Load:
      case Opcode::F32Load:
      case Opcode::F64Load: {
        uint32_t alignment_log2;
        in_u32_leb128(ctx, &alignment_log2, "load alignment");
        uint32_t offset;
        in_u32_leb128(ctx, &offset, "load offset");

        CALLBACK(OnLoadExpr, opcode, alignment_log2, offset);
        CALLBACK(OnOpcodeUint32Uint32, alignment_log2, offset);
        break;
      }

      case Opcode::I32Store8:
      case Opcode::I32Store16:
      case Opcode::I64Store8:
      case Opcode::I64Store16:
      case Opcode::I64Store32:
      case Opcode::I32Store:
      case Opcode::I64Store:
      case Opcode::F32Store:
      case Opcode::F64Store: {
        uint32_t alignment_log2;
        in_u32_leb128(ctx, &alignment_log2, "store alignment");
        uint32_t offset;
        in_u32_leb128(ctx, &offset, "store offset");

        CALLBACK(OnStoreExpr, opcode, alignment_log2, offset);
        CALLBACK(OnOpcodeUint32Uint32, alignment_log2, offset);
        break;
      }

      case Opcode::CurrentMemory: {
        uint32_t reserved;
        in_u32_leb128(ctx, &reserved, "current_memory reserved");
        RAISE_ERROR_UNLESS(reserved == 0,
                           "current_memory reserved value must be 0");
        CALLBACK0(OnCurrentMemoryExpr);
        CALLBACK(OnOpcodeUint32, reserved);
        break;
      }

      case Opcode::GrowMemory: {
        uint32_t reserved;
        in_u32_leb128(ctx, &reserved, "grow_memory reserved");
        RAISE_ERROR_UNLESS(reserved == 0,
                           "grow_memory reserved value must be 0");
        CALLBACK0(OnGrowMemoryExpr);
        CALLBACK(OnOpcodeUint32, reserved);
        break;
      }

      case Opcode::I32Add:
      case Opcode::I32Sub:
      case Opcode::I32Mul:
      case Opcode::I32DivS:
      case Opcode::I32DivU:
      case Opcode::I32RemS:
      case Opcode::I32RemU:
      case Opcode::I32And:
      case Opcode::I32Or:
      case Opcode::I32Xor:
      case Opcode::I32Shl:
      case Opcode::I32ShrU:
      case Opcode::I32ShrS:
      case Opcode::I32Rotr:
      case Opcode::I32Rotl:
      case Opcode::I64Add:
      case Opcode::I64Sub:
      case Opcode::I64Mul:
      case Opcode::I64DivS:
      case Opcode::I64DivU:
      case Opcode::I64RemS:
      case Opcode::I64RemU:
      case Opcode::I64And:
      case Opcode::I64Or:
      case Opcode::I64Xor:
      case Opcode::I64Shl:
      case Opcode::I64ShrU:
      case Opcode::I64ShrS:
      case Opcode::I64Rotr:
      case Opcode::I64Rotl:
      case Opcode::F32Add:
      case Opcode::F32Sub:
      case Opcode::F32Mul:
      case Opcode::F32Div:
      case Opcode::F32Min:
      case Opcode::F32Max:
      case Opcode::F32Copysign:
      case Opcode::F64Add:
      case Opcode::F64Sub:
      case Opcode::F64Mul:
      case Opcode::F64Div:
      case Opcode::F64Min:
      case Opcode::F64Max:
      case Opcode::F64Copysign:
        CALLBACK(OnBinaryExpr, opcode);
        CALLBACK0(OnOpcodeBare);
        break;

      case Opcode::I32Eq:
      case Opcode::I32Ne:
      case Opcode::I32LtS:
      case Opcode::I32LeS:
      case Opcode::I32LtU:
      case Opcode::I32LeU:
      case Opcode::I32GtS:
      case Opcode::I32GeS:
      case Opcode::I32GtU:
      case Opcode::I32GeU:
      case Opcode::I64Eq:
      case Opcode::I64Ne:
      case Opcode::I64LtS:
      case Opcode::I64LeS:
      case Opcode::I64LtU:
      case Opcode::I64LeU:
      case Opcode::I64GtS:
      case Opcode::I64GeS:
      case Opcode::I64GtU:
      case Opcode::I64GeU:
      case Opcode::F32Eq:
      case Opcode::F32Ne:
      case Opcode::F32Lt:
      case Opcode::F32Le:
      case Opcode::F32Gt:
      case Opcode::F32Ge:
      case Opcode::F64Eq:
      case Opcode::F64Ne:
      case Opcode::F64Lt:
      case Opcode::F64Le:
      case Opcode::F64Gt:
      case Opcode::F64Ge:
        CALLBACK(OnCompareExpr, opcode);
        CALLBACK0(OnOpcodeBare);
        break;

      case Opcode::I32Clz:
      case Opcode::I32Ctz:
      case Opcode::I32Popcnt:
      case Opcode::I64Clz:
      case Opcode::I64Ctz:
      case Opcode::I64Popcnt:
      case Opcode::F32Abs:
      case Opcode::F32Neg:
      case Opcode::F32Ceil:
      case Opcode::F32Floor:
      case Opcode::F32Trunc:
      case Opcode::F32Nearest:
      case Opcode::F32Sqrt:
      case Opcode::F64Abs:
      case Opcode::F64Neg:
      case Opcode::F64Ceil:
      case Opcode::F64Floor:
      case Opcode::F64Trunc:
      case Opcode::F64Nearest:
      case Opcode::F64Sqrt:
        CALLBACK(OnUnaryExpr, opcode);
        CALLBACK0(OnOpcodeBare);
        break;

      case Opcode::I32TruncSF32:
      case Opcode::I32TruncSF64:
      case Opcode::I32TruncUF32:
      case Opcode::I32TruncUF64:
      case Opcode::I32WrapI64:
      case Opcode::I64TruncSF32:
      case Opcode::I64TruncSF64:
      case Opcode::I64TruncUF32:
      case Opcode::I64TruncUF64:
      case Opcode::I64ExtendSI32:
      case Opcode::I64ExtendUI32:
      case Opcode::F32ConvertSI32:
      case Opcode::F32ConvertUI32:
      case Opcode::F32ConvertSI64:
      case Opcode::F32ConvertUI64:
      case Opcode::F32DemoteF64:
      case Opcode::F32ReinterpretI32:
      case Opcode::F64ConvertSI32:
      case Opcode::F64ConvertUI32:
      case Opcode::F64ConvertSI64:
      case Opcode::F64ConvertUI64:
      case Opcode::F64PromoteF32:
      case Opcode::F64ReinterpretI64:
      case Opcode::I32ReinterpretF32:
      case Opcode::I64ReinterpretF64:
      case Opcode::I32Eqz:
      case Opcode::I64Eqz:
        CALLBACK(OnConvertExpr, opcode);
        CALLBACK0(OnOpcodeBare);
        break;

      default:
        RAISE_ERROR("unexpected opcode: %d (0x%x)", static_cast<int>(opcode),
                    static_cast<unsigned>(opcode));
    }
  }
  RAISE_ERROR_UNLESS(ctx->state.offset == end_offset,
                     "function body longer than given size");
  RAISE_ERROR_UNLESS(seen_end_opcode, "function body must end with END opcode");
}

static void read_names_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginNamesSection, section_size);
  uint32_t i = 0;
  size_t previous_read_end = ctx->read_end;
  uint32_t previous_subsection_type = 0;
  while (ctx->state.offset < ctx->read_end) {
    uint32_t name_type;
    uint32_t subsection_size;
    in_u32_leb128(ctx, &name_type, "name type");
    if (i != 0) {
      if (name_type == previous_subsection_type)
        RAISE_ERROR("duplicate sub-section");
      if (name_type < previous_subsection_type)
        RAISE_ERROR("out-of-order sub-section");
    }
    previous_subsection_type = name_type;
    in_u32_leb128(ctx, &subsection_size, "subsection size");
    size_t subsection_end = ctx->state.offset + subsection_size;
    if (subsection_end > ctx->read_end)
      RAISE_ERROR("invalid sub-section size: extends past end");
    ctx->read_end = subsection_end;

    switch (static_cast<NameSectionSubsection>(name_type)) {
    case NameSectionSubsection::Function:
      CALLBACK(OnFunctionNameSubsection, i, name_type, subsection_size);
      if (subsection_size) {
        uint32_t num_names;
        in_u32_leb128(ctx, &num_names, "name count");
        CALLBACK(OnFunctionNamesCount, num_names);
        for (uint32_t j = 0; j < num_names; ++j) {
          uint32_t function_index;
          StringSlice function_name;

          in_u32_leb128(ctx, &function_index, "function index");
          RAISE_ERROR_UNLESS(function_index < num_total_funcs(ctx),
                             "invalid function index: %u", function_index);
          in_str(ctx, &function_name, "function name");
          CALLBACK(OnFunctionName, function_index, function_name);
        }
      }
      break;
    case NameSectionSubsection::Local:
      CALLBACK(OnLocalNameSubsection, i, name_type, subsection_size);
      if (subsection_size) {
        uint32_t num_funcs;
        in_u32_leb128(ctx, &num_funcs, "function count");
        CALLBACK(OnLocalNameFunctionCount, num_funcs);
        for (uint32_t j = 0; j < num_funcs; ++j) {
          uint32_t function_index;
          in_u32_leb128(ctx, &function_index, "function index");
          uint32_t num_locals;
          in_u32_leb128(ctx, &num_locals, "local count");
          CALLBACK(OnLocalNameLocalCount, function_index, num_locals);
          for (uint32_t k = 0; k < num_locals; ++k) {
            uint32_t local_index;
            StringSlice local_name;

            in_u32_leb128(ctx, &local_index, "named index");
            in_str(ctx, &local_name, "name");
            CALLBACK(OnLocalName, function_index, local_index, local_name);
          }
        }
      }
      break;
    default:
      /* unknown subsection, skip it */
      ctx->state.offset = subsection_end;
      break;
    }
    ++i;
    if (ctx->state.offset != subsection_end) {
      RAISE_ERROR("unfinished sub-section (expected end: 0x%" PRIzx ")",
                  subsection_end);
    }
    ctx->read_end = previous_read_end;
  }
  CALLBACK0(EndNamesSection);
}

static void read_reloc_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginRelocSection, section_size);
  uint32_t num_relocs, section;
  in_u32_leb128(ctx, &section, "section");
  StringSlice section_name;
  WABT_ZERO_MEMORY(section_name);
  if (static_cast<BinarySection>(section) == BinarySection::Custom)
    in_str(ctx, &section_name, "section name");
  in_u32_leb128(ctx, &num_relocs, "relocation count");
  CALLBACK(OnRelocCount, num_relocs, static_cast<BinarySection>(section),
           section_name);
  for (uint32_t i = 0; i < num_relocs; ++i) {
    uint32_t reloc_type, offset, index, addend = 0;
    in_u32_leb128(ctx, &reloc_type, "relocation type");
    in_u32_leb128(ctx, &offset, "offset");
    in_u32_leb128(ctx, &index, "index");
    RelocType type = static_cast<RelocType>(reloc_type);
    switch (type) {
      case RelocType::MemoryAddressLEB:
      case RelocType::MemoryAddressSLEB:
      case RelocType::MemoryAddressI32:
        in_i32_leb128(ctx, &addend, "addend");
        break;
      default:
        break;
    }
    CALLBACK(OnReloc, type, offset, index, addend);
  }
  CALLBACK0(EndRelocSection);
}

static void read_custom_section(Context* ctx, uint32_t section_size) {
  StringSlice section_name;
  in_str(ctx, &section_name, "section name");
  CALLBACK(BeginCustomSection, section_size, section_name);

  bool name_section_ok = ctx->last_known_section >= BinarySection::Import;
  if (ctx->options->read_debug_names && name_section_ok &&
      strncmp(section_name.start, WABT_BINARY_SECTION_NAME,
              section_name.length) == 0) {
    read_names_section(ctx, section_size);
  } else if (strncmp(section_name.start, WABT_BINARY_SECTION_RELOC,
                     strlen(WABT_BINARY_SECTION_RELOC)) == 0) {
    read_reloc_section(ctx, section_size);
  } else {
    /* This is an unknown custom section, skip it. */
    ctx->state.offset = ctx->read_end;
  }
  CALLBACK0(EndCustomSection);
}

static void read_type_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginTypeSection, section_size);
  in_u32_leb128(ctx, &ctx->num_signatures, "type count");
  CALLBACK(OnTypeCount, ctx->num_signatures);

  for (uint32_t i = 0; i < ctx->num_signatures; ++i) {
    Type form;
    in_type(ctx, &form, "type form");
    RAISE_ERROR_UNLESS(form == Type::Func, "unexpected type form: %d",
                       static_cast<int>(form));

    uint32_t num_params;
    in_u32_leb128(ctx, &num_params, "function param count");

    ctx->param_types.resize(num_params);

    for (uint32_t j = 0; j < num_params; ++j) {
      Type param_type;
      in_type(ctx, &param_type, "function param type");
      RAISE_ERROR_UNLESS(is_concrete_type(param_type),
                         "expected valid param type (got %d)",
                         static_cast<int>(param_type));
      ctx->param_types[j] = param_type;
    }

    uint32_t num_results;
    in_u32_leb128(ctx, &num_results, "function result count");
    RAISE_ERROR_UNLESS(num_results <= 1, "result count must be 0 or 1");

    Type result_type = Type::Void;
    if (num_results) {
      in_type(ctx, &result_type, "function result type");
      RAISE_ERROR_UNLESS(is_concrete_type(result_type),
                         "expected valid result type: %d",
                         static_cast<int>(result_type));
    }

    Type* param_types = num_params ? ctx->param_types.data() : nullptr;

    CALLBACK(OnType, i, num_params, param_types, num_results, &result_type);
  }
  CALLBACK0(EndTypeSection);
}

static void read_import_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginImportSection, section_size);
  in_u32_leb128(ctx, &ctx->num_imports, "import count");
  CALLBACK(OnImportCount, ctx->num_imports);
  for (uint32_t i = 0; i < ctx->num_imports; ++i) {
    StringSlice module_name;
    in_str(ctx, &module_name, "import module name");
    StringSlice field_name;
    in_str(ctx, &field_name, "import field name");

    uint32_t kind;
    in_u32_leb128(ctx, &kind, "import kind");
    switch (static_cast<ExternalKind>(kind)) {
      case ExternalKind::Func: {
        uint32_t sig_index;
        in_u32_leb128(ctx, &sig_index, "import signature index");
        RAISE_ERROR_UNLESS(sig_index < ctx->num_signatures,
                           "invalid import signature index");
        CALLBACK(OnImport, i, module_name, field_name);
        CALLBACK(OnImportFunc, i, module_name, field_name,
                 ctx->num_func_imports, sig_index);
        ctx->num_func_imports++;
        break;
      }

      case ExternalKind::Table: {
        Type elem_type;
        Limits elem_limits;
        read_table(ctx, &elem_type, &elem_limits);
        CALLBACK(OnImport, i, module_name, field_name);
        CALLBACK(OnImportTable, i, module_name, field_name,
                 ctx->num_table_imports, elem_type, &elem_limits);
        ctx->num_table_imports++;
        break;
      }

      case ExternalKind::Memory: {
        Limits page_limits;
        read_memory(ctx, &page_limits);
        CALLBACK(OnImport, i, module_name, field_name);
        CALLBACK(OnImportMemory, i, module_name, field_name,
                 ctx->num_memory_imports, &page_limits);
        ctx->num_memory_imports++;
        break;
      }

      case ExternalKind::Global: {
        Type type;
        bool mutable_;
        read_global_header(ctx, &type, &mutable_);
        CALLBACK(OnImport, i, module_name, field_name);
        CALLBACK(OnImportGlobal, i, module_name, field_name,
                 ctx->num_global_imports, type, mutable_);
        ctx->num_global_imports++;
        break;
      }

      default:
        RAISE_ERROR("invalid import kind: %d", kind);
    }
  }
  CALLBACK0(EndImportSection);
}

static void read_function_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginFunctionSection, section_size);
  in_u32_leb128(ctx, &ctx->num_function_signatures, "function signature count");
  CALLBACK(OnFunctionCount, ctx->num_function_signatures);
  for (uint32_t i = 0; i < ctx->num_function_signatures; ++i) {
    uint32_t func_index = ctx->num_func_imports + i;
    uint32_t sig_index;
    in_u32_leb128(ctx, &sig_index, "function signature index");
    RAISE_ERROR_UNLESS(sig_index < ctx->num_signatures,
                       "invalid function signature index: %d", sig_index);
    CALLBACK(OnFunction, func_index, sig_index);
  }
  CALLBACK0(EndFunctionSection);
}

static void read_table_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginTableSection, section_size);
  in_u32_leb128(ctx, &ctx->num_tables, "table count");
  RAISE_ERROR_UNLESS(ctx->num_tables <= 1, "table count (%d) must be 0 or 1",
                     ctx->num_tables);
  CALLBACK(OnTableCount, ctx->num_tables);
  for (uint32_t i = 0; i < ctx->num_tables; ++i) {
    uint32_t table_index = ctx->num_table_imports + i;
    Type elem_type;
    Limits elem_limits;
    read_table(ctx, &elem_type, &elem_limits);
    CALLBACK(OnTable, table_index, elem_type, &elem_limits);
  }
  CALLBACK0(EndTableSection);
}

static void read_memory_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginMemorySection, section_size);
  in_u32_leb128(ctx, &ctx->num_memories, "memory count");
  RAISE_ERROR_UNLESS(ctx->num_memories <= 1, "memory count must be 0 or 1");
  CALLBACK(OnMemoryCount, ctx->num_memories);
  for (uint32_t i = 0; i < ctx->num_memories; ++i) {
    uint32_t memory_index = ctx->num_memory_imports + i;
    Limits page_limits;
    read_memory(ctx, &page_limits);
    CALLBACK(OnMemory, memory_index, &page_limits);
  }
  CALLBACK0(EndMemorySection);
}

static void read_global_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginGlobalSection, section_size);
  in_u32_leb128(ctx, &ctx->num_globals, "global count");
  CALLBACK(OnGlobalCount, ctx->num_globals);
  for (uint32_t i = 0; i < ctx->num_globals; ++i) {
    uint32_t global_index = ctx->num_global_imports + i;
    Type global_type;
    bool mutable_;
    read_global_header(ctx, &global_type, &mutable_);
    CALLBACK(BeginGlobal, global_index, global_type, mutable_);
    CALLBACK(BeginGlobalInitExpr, global_index);
    read_init_expr(ctx, global_index);
    CALLBACK(EndGlobalInitExpr, global_index);
    CALLBACK(EndGlobal, global_index);
  }
  CALLBACK0(EndGlobalSection);
}

static void read_export_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginExportSection, section_size);
  in_u32_leb128(ctx, &ctx->num_exports, "export count");
  CALLBACK(OnExportCount, ctx->num_exports);
  for (uint32_t i = 0; i < ctx->num_exports; ++i) {
    StringSlice name;
    in_str(ctx, &name, "export item name");

    uint8_t external_kind;
    in_u8(ctx, &external_kind, "export external kind");
    RAISE_ERROR_UNLESS(is_valid_external_kind(external_kind),
                       "invalid export external kind: %d", external_kind);

    uint32_t item_index;
    in_u32_leb128(ctx, &item_index, "export item index");
    switch (static_cast<ExternalKind>(external_kind)) {
      case ExternalKind::Func:
        RAISE_ERROR_UNLESS(item_index < num_total_funcs(ctx),
                           "invalid export func index: %d", item_index);
        break;
      case ExternalKind::Table:
        RAISE_ERROR_UNLESS(item_index < num_total_tables(ctx),
                           "invalid export table index: %d", item_index);
        break;
      case ExternalKind::Memory:
        RAISE_ERROR_UNLESS(item_index < num_total_memories(ctx),
                           "invalid export memory index: %d", item_index);
        break;
      case ExternalKind::Global:
        RAISE_ERROR_UNLESS(item_index < num_total_globals(ctx),
                           "invalid export global index: %d", item_index);
        break;
    }

    CALLBACK(OnExport, i, static_cast<ExternalKind>(external_kind), item_index,
             name);
  }
  CALLBACK0(EndExportSection);
}

static void read_start_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginStartSection, section_size);
  uint32_t func_index;
  in_u32_leb128(ctx, &func_index, "start function index");
  RAISE_ERROR_UNLESS(func_index < num_total_funcs(ctx),
                     "invalid start function index: %d", func_index);
  CALLBACK(OnStartFunction, func_index);
  CALLBACK0(EndStartSection);
}

static void read_elem_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginElemSection, section_size);
  uint32_t num_elem_segments;
  in_u32_leb128(ctx, &num_elem_segments, "elem segment count");
  CALLBACK(OnElemSegmentCount, num_elem_segments);
  RAISE_ERROR_UNLESS(num_elem_segments == 0 || num_total_tables(ctx) > 0,
                     "elem section without table section");
  for (uint32_t i = 0; i < num_elem_segments; ++i) {
    uint32_t table_index;
    in_u32_leb128(ctx, &table_index, "elem segment table index");
    CALLBACK(BeginElemSegment, i, table_index);
    CALLBACK(BeginElemSegmentInitExpr, i);
    read_init_expr(ctx, i);
    CALLBACK(EndElemSegmentInitExpr, i);

    uint32_t num_function_indexes;
    in_u32_leb128(ctx, &num_function_indexes,
                  "elem segment function index count");
    CALLBACK(OnElemSegmentFunctionIndexCount, i, num_function_indexes);
    for (uint32_t j = 0; j < num_function_indexes; ++j) {
      uint32_t func_index;
      in_u32_leb128(ctx, &func_index, "elem segment function index");
      CALLBACK(OnElemSegmentFunctionIndex, i, func_index);
    }
    CALLBACK(EndElemSegment, i);
  }
  CALLBACK0(EndElemSection);
}

static void read_code_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginCodeSection, section_size);
  in_u32_leb128(ctx, &ctx->num_function_bodies, "function body count");
  RAISE_ERROR_UNLESS(ctx->num_function_signatures == ctx->num_function_bodies,
                     "function signature count != function body count");
  CALLBACK(OnFunctionBodyCount, ctx->num_function_bodies);
  for (uint32_t i = 0; i < ctx->num_function_bodies; ++i) {
    uint32_t func_index = ctx->num_func_imports + i;
    uint32_t func_offset = ctx->state.offset;
    ctx->state.offset = func_offset;
    CALLBACK(BeginFunctionBody, func_index);
    uint32_t body_size;
    in_u32_leb128(ctx, &body_size, "function body size");
    uint32_t body_start_offset = ctx->state.offset;
    uint32_t end_offset = body_start_offset + body_size;

    uint32_t num_local_decls;
    in_u32_leb128(ctx, &num_local_decls, "local declaration count");
    CALLBACK(OnLocalDeclCount, num_local_decls);
    for (uint32_t k = 0; k < num_local_decls; ++k) {
      uint32_t num_local_types;
      in_u32_leb128(ctx, &num_local_types, "local type count");
      Type local_type;
      in_type(ctx, &local_type, "local type");
      RAISE_ERROR_UNLESS(is_concrete_type(local_type),
                         "expected valid local type");
      CALLBACK(OnLocalDecl, k, num_local_types, local_type);
    }

    read_function_body(ctx, end_offset);

    CALLBACK(EndFunctionBody, func_index);
  }
  CALLBACK0(EndCodeSection);
}

static void read_data_section(Context* ctx, uint32_t section_size) {
  CALLBACK(BeginDataSection, section_size);
  uint32_t num_data_segments;
  in_u32_leb128(ctx, &num_data_segments, "data segment count");
  CALLBACK(OnDataSegmentCount, num_data_segments);
  RAISE_ERROR_UNLESS(num_data_segments == 0 || num_total_memories(ctx) > 0,
                     "data section without memory section");
  for (uint32_t i = 0; i < num_data_segments; ++i) {
    uint32_t memory_index;
    in_u32_leb128(ctx, &memory_index, "data segment memory index");
    CALLBACK(BeginDataSegment, i, memory_index);
    CALLBACK(BeginDataSegmentInitExpr, i);
    read_init_expr(ctx, i);
    CALLBACK(EndDataSegmentInitExpr, i);

    uint32_t data_size;
    const void* data;
    in_bytes(ctx, &data, &data_size, "data segment data");
    CALLBACK(OnDataSegmentData, i, data, data_size);
    CALLBACK(EndDataSegment, i);
  }
  CALLBACK0(EndDataSection);
}

static void read_sections(Context* ctx) {
  while (ctx->state.offset < ctx->state.size) {
    uint32_t section_code;
    uint32_t section_size;
    /* Temporarily reset read_end to the full data size so the next section
     * can be read. */
    ctx->read_end = ctx->state.size;
    in_u32_leb128(ctx, &section_code, "section code");
    in_u32_leb128(ctx, &section_size, "section size");
    ctx->read_end = ctx->state.offset + section_size;
    if (section_code >= kBinarySectionCount) {
      RAISE_ERROR("invalid section code: %u; max is %u", section_code,
                  kBinarySectionCount - 1);
    }

    BinarySection section = static_cast<BinarySection>(section_code);

    if (ctx->read_end > ctx->state.size)
      RAISE_ERROR("invalid section size: extends past end");

    if (ctx->last_known_section != BinarySection::Invalid &&
        section != BinarySection::Custom &&
        section <= ctx->last_known_section) {
      RAISE_ERROR("section %s out of order", get_section_name(section));
    }

    CALLBACK(BeginSection, section, section_size);

#define V(Name, name, code)                   \
  case BinarySection::Name:                   \
    read_##name##_section(ctx, section_size); \
    break;

    switch (section) {
      WABT_FOREACH_BINARY_SECTION(V)

      default:
        assert(0);
        break;
    }

#undef V

    if (ctx->state.offset != ctx->read_end) {
      RAISE_ERROR("unfinished section (expected end: 0x%" PRIzx ")",
                  ctx->read_end);
    }

    if (section != BinarySection::Custom)
      ctx->last_known_section = section;
  }
}

Result read_binary(const void* data,
                   size_t size,
                   BinaryReader* reader,
                   const ReadBinaryOptions* options) {
  BinaryReaderLogging logging_reader(options->log_stream, reader);
  Context context;
  /* all the macros assume a Context* named ctx */
  Context* ctx = &context;
  ctx->state.data = static_cast<const uint8_t*>(data);
  ctx->state.size = ctx->read_end = size;
  ctx->state.offset = 0;
  ctx->reader = options->log_stream ? &logging_reader : reader;
  ctx->options = options;
  ctx->last_known_section = BinarySection::Invalid;

  if (setjmp(ctx->error_jmp_buf) == 1) {
    return Result::Error;
  }

  ctx->reader->OnSetState(&ctx->state);

  uint32_t magic;
  in_u32(ctx, &magic, "magic");
  RAISE_ERROR_UNLESS(magic == WABT_BINARY_MAGIC, "bad magic value");
  uint32_t version;
  in_u32(ctx, &version, "version");
  RAISE_ERROR_UNLESS(version == WABT_BINARY_VERSION,
                     "bad wasm file version: %#x (expected %#x)", version,
                     WABT_BINARY_VERSION);

  CALLBACK(BeginModule, version);
  read_sections(ctx);
  CALLBACK0(EndModule);
  return Result::Ok;
}

}  // namespace wabt

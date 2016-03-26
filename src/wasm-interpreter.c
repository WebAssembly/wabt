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

#include "wasm-interpreter.h"

#include <assert.h>
#include <math.h>

#define LOG 0

#if LOG
#define V(rtype, type1, type2, mem_size, code, NAME, text) [code] = text,
static const char* s_opcode_name[] = {
  WASM_FOREACH_OPCODE(V)
  [WASM_OPCODE_ALLOCA] = "alloca",
  [WASM_OPCODE_DISCARD] = "discard",
  [WASM_OPCODE_DISCARD_KEEP] = "discard_keep",
};
#undef V
#endif

#define CHECK_RESULT(expr) \
  do {                     \
    if ((expr) != WASM_OK) \
      return WASM_ERROR;   \
  } while (0)

WasmResult wasm_init_interpreter_thread(WasmAllocator* allocator,
                                        WasmInterpreterModule* module,
                                        WasmInterpreterThread* thread,
                                        WasmInterpreterThreadOptions* options) {
  CHECK_RESULT(wasm_new_interpreter_value_array(allocator, &thread->value_stack,
                                                options->value_stack_size));
  CHECK_RESULT(wasm_new_uint32_array(allocator, &thread->call_stack,
                                     options->call_stack_size));
  thread->value_stack_top = 0;
  thread->call_stack_top = 0;
  thread->pc = options->pc;

  /* allocate import_args based on the signature with the most params */
  /* TODO(binji): move this elsewhere? */
  uint32_t i;
  uint32_t max_import_params = 0;
  for (i = 0; i < module->imports.size; ++i) {
    WasmInterpreterImport* import = &module->imports.data[i];
    assert(import->sig_index < module->sigs.size);
    WasmInterpreterFuncSignature* sig = &module->sigs.data[import->sig_index];
    if (sig->param_types.size > max_import_params)
      max_import_params = sig->param_types.size;
  }
  CHECK_RESULT(wasm_new_interpreter_typed_value_array(
      allocator, &thread->import_args, max_import_params));
  return WASM_OK;
}

void wasm_destroy_interpreter_thread(WasmAllocator* allocator,
                                     WasmInterpreterThread* thread) {
  wasm_destroy_interpreter_value_array(allocator, &thread->value_stack);
  wasm_destroy_uint32_array(allocator, &thread->call_stack);
}

#define F32_SIGN_MASK 0x80000000U
#define F32_EXP_MASK 0x7f800000U
#define F32_SIG_MASK 0x007fffffU
#define F32_EXP_SHIFT 23
#define F32_EXP_BIAS 127
#define F64_SIGN_MASK 0x8000000000000000ULL
#define F64_EXP_MASK 0x7ff0000000000000ULL
#define F64_SIG_MASK 0x000fffffffffffffULL
#define F64_EXP_SHIFT 52
#define F64_EXP_BIAS 1023

static WASM_INLINE int is_nan_f32(uint32_t f32_bits) {
  return ((f32_bits & F32_EXP_MASK) == F32_EXP_MASK) &&
         ((f32_bits & F32_SIG_MASK) != 0);
}

static WASM_INLINE int is_nan_f64(uint64_t f64_bits) {
  return ((f64_bits & F64_EXP_MASK) == F64_EXP_MASK) &&
         ((f64_bits & F64_SIG_MASK) != 0);
}

static WASM_INLINE int get_exp_f32(uint32_t f32_bits) {
  return (int)((f32_bits & F32_EXP_MASK) >> F32_EXP_SHIFT) - F32_EXP_BIAS;
}

static WASM_INLINE int get_exp_f64(uint64_t f64_bits) {
  return (int)((f64_bits & F64_EXP_MASK) >> F64_EXP_SHIFT) - F64_EXP_BIAS;
}

static WASM_INLINE int is_signed_f32(uint32_t f32_bits) {
  return (f32_bits & F32_SIGN_MASK) != 0;
}

static WASM_INLINE int is_signed_f64(uint64_t f64_bits) {
  return (f64_bits & F64_SIGN_MASK) != 0;
}

#define IS_NAN_F32 is_nan_f32
#define IS_NAN_F64 is_nan_f64

#define DEFINE_BITCAST(name, src, dst) \
  static WASM_INLINE dst name(src x) { \
    dst result;                        \
    memcpy(&result, &x, sizeof(dst));  \
    return result;                     \
  }

DEFINE_BITCAST(bitcast_u32_to_i32, uint32_t, int32_t)
DEFINE_BITCAST(bitcast_u64_to_i64, uint64_t, int64_t)
DEFINE_BITCAST(bitcast_f32_to_u32, float, uint32_t)
DEFINE_BITCAST(bitcast_u32_to_f32, uint32_t, float)
DEFINE_BITCAST(bitcast_f64_to_u64, double, uint64_t)
DEFINE_BITCAST(bitcast_u64_to_f64, uint64_t, double)

#define bitcast_i32_to_u32(x) ((uint32_t)x)
#define bitcast_i64_to_u64(x) ((uint64_t)x)

#define VALUE_TYPE_I32 uint32_t
#define VALUE_TYPE_I64 uint64_t
#define VALUE_TYPE_F32 uint32_t
#define VALUE_TYPE_F64 uint64_t

#define FLOAT_TYPE_F32 float
#define FLOAT_TYPE_F64 double

#define MEM_TYPE_I8 int8_t
#define MEM_TYPE_U8 uint8_t
#define MEM_TYPE_I16 int16_t
#define MEM_TYPE_U16 uint16_t
#define MEM_TYPE_I32 int32_t
#define MEM_TYPE_U32 uint32_t
#define MEM_TYPE_I64 int64_t
#define MEM_TYPE_U64 uint64_t
#define MEM_TYPE_F32 uint32_t
#define MEM_TYPE_F64 uint64_t

#define MEM_TYPE_EXTEND_I32_I8 int32_t
#define MEM_TYPE_EXTEND_I32_U8 uint32_t
#define MEM_TYPE_EXTEND_I32_I16 int32_t
#define MEM_TYPE_EXTEND_I32_U16 uint32_t
#define MEM_TYPE_EXTEND_I32_I32 int32_t
#define MEM_TYPE_EXTEND_I32_U32 uint32_t

#define MEM_TYPE_EXTEND_I64_I8 int64_t
#define MEM_TYPE_EXTEND_I64_U8 uint64_t
#define MEM_TYPE_EXTEND_I64_I16 int64_t
#define MEM_TYPE_EXTEND_I64_U16 uint64_t
#define MEM_TYPE_EXTEND_I64_I32 int64_t
#define MEM_TYPE_EXTEND_I64_U32 uint64_t
#define MEM_TYPE_EXTEND_I64_I64 int64_t
#define MEM_TYPE_EXTEND_I64_U64 uint64_t

#define MEM_TYPE_EXTEND_F32_F32 uint32_t
#define MEM_TYPE_EXTEND_F64_F64 uint64_t

#define BITCAST_I32_TO_SIGNED bitcast_u32_to_i32
#define BITCAST_I64_TO_SIGNED bitcast_u64_to_i64
#define BITCAST_I32_TO_UNSIGNED bitcast_i32_to_u32
#define BITCAST_I64_TO_UNSIGNED bitcast_i64_to_u64

#define BITCAST_TO_F32 bitcast_u32_to_f32
#define BITCAST_TO_F64 bitcast_u64_to_f64
#define BITCAST_FROM_F32 bitcast_f32_to_u32
#define BITCAST_FROM_F64 bitcast_f64_to_u64

#define TYPE_FIELD_NAME_I32 i32
#define TYPE_FIELD_NAME_I64 i64
#define TYPE_FIELD_NAME_F32 f32_bits
#define TYPE_FIELD_NAME_F64 f64_bits

#define TRAP(type) return WASM_INTERPRETER_TRAP_##type

#define CHECK_STACK()              \
  do {                             \
    if (vs_top >= vs_end)          \
      TRAP(VALUE_STACK_EXHAUSTED); \
  } while (0)

#define PUSH(v)        \
  do {                 \
    CHECK_STACK();     \
    (*vs_top++) = (v); \
  } while (0)

#define PUSH_TYPE(type, v)                                       \
  do {                                                           \
    CHECK_STACK();                                               \
    (*vs_top++).TYPE_FIELD_NAME_##type = (VALUE_TYPE_##type)(v); \
  } while (0)

#define PUSH_I32(v) PUSH_TYPE(I32, (v))
#define PUSH_I64(v) PUSH_TYPE(I64, (v))
#define PUSH_F32(v) PUSH_TYPE(F32, (v))
#define PUSH_F64(v) PUSH_TYPE(F64, (v))

#define STACK_VALUE(depth) (*(vs_top - (depth)))
#define TOP() (STACK_VALUE(1))
#define POP() (*--vs_top)
#define POP_I32() (POP().i32)
#define POP_I64() (POP().i64)
#define POP_F32() (POP().f32_bits)
#define POP_F64() (POP().f64_bits)

#define GOTO(offset) pc = &istream[offset]

#define PUSH_CALL(o)              \
  do {                            \
    if (cs_top >= cs_end)         \
      TRAP(CALL_STACK_EXHAUSTED); \
    (*cs_top++) = (pc - istream); \
  } while (0)

#define POP_CALL() (*--cs_top)

#define LOAD(type, mem_type)                                   \
  do {                                                         \
    uint64_t offset = (uint64_t)POP_I32() + read_u32(&pc);     \
    if (offset >= module->memory.byte_size)                    \
      TRAP(MEMORY_ACCESS_OUT_OF_BOUNDS);                       \
    void* src = (void*)((size_t)module->memory.data + offset); \
    MEM_TYPE_##mem_type value;                                 \
    memcpy(&value, src, sizeof(MEM_TYPE_##mem_type));          \
    PUSH_##type((MEM_TYPE_EXTEND_##type##_##mem_type)value);   \
  } while (0)

#define STORE(type, mem_type)                                  \
  do {                                                         \
    VALUE_TYPE_##type value = POP_##type();                    \
    uint64_t offset = (uint64_t)POP_I32() + read_u32(&pc);     \
    if (offset >= module->memory.byte_size)                    \
      TRAP(MEMORY_ACCESS_OUT_OF_BOUNDS);                       \
    void* dst = (void*)((size_t)module->memory.data + offset); \
    MEM_TYPE_##mem_type src = (MEM_TYPE_##mem_type)value;      \
    memcpy(dst, &src, sizeof(MEM_TYPE_##mem_type));            \
    PUSH_##type(value);                                        \
  } while (0)

#define BINOP(type, op)                   \
  do {                                    \
    VALUE_TYPE_##type rhs = POP_##type(); \
    VALUE_TYPE_##type lhs = POP_##type(); \
    PUSH_##type(lhs op rhs);              \
  } while (0)

#define BINOP_SIGNED(type, op)                           \
  do {                                                   \
    VALUE_TYPE_##type rhs = POP_##type();                \
    VALUE_TYPE_##type lhs = POP_##type();                \
    PUSH_##type(BITCAST_##type##_TO_SIGNED(lhs)          \
                    op BITCAST_##type##_TO_SIGNED(rhs)); \
  } while (0)

#define SHIFT_MASK_I32 31
#define SHIFT_MASK_I64 63

#define BINOP_SHIFT(type, op, sign)                                          \
  do {                                                                       \
    VALUE_TYPE_##type rhs = POP_##type();                                    \
    VALUE_TYPE_##type lhs = POP_##type();                                    \
    PUSH_##type(BITCAST_##type##_TO_##sign(lhs) op(rhs& SHIFT_MASK_##type)); \
  } while (0)

#define ROT_LEFT_0_SHIFT_OP <<
#define ROT_LEFT_1_SHIFT_OP >>
#define ROT_RIGHT_0_SHIFT_OP >>
#define ROT_RIGHT_1_SHIFT_OP <<

#define BINOP_ROT(type, dir)                                               \
  do {                                                                     \
    VALUE_TYPE_##type rhs = POP_##type();                                  \
    VALUE_TYPE_##type lhs = POP_##type();                                  \
    uint32_t amount = rhs & SHIFT_MASK_##type;                             \
    if (amount != 0) {                                                     \
      PUSH_##type(                                                         \
          (lhs ROT_##dir##_0_SHIFT_OP amount) |                            \
          (lhs ROT_##dir##_1_SHIFT_OP((SHIFT_MASK_##type + 1) - amount))); \
    } else {                                                               \
      PUSH_##type(lhs);                                                    \
    }                                                                      \
  } while (0)

#define BINOP_TRAP_ZERO(type, op, sign)                  \
  do {                                                   \
    VALUE_TYPE_##type rhs = POP_##type();                \
    VALUE_TYPE_##type lhs = POP_##type();                \
    if (rhs == 0)                                        \
      TRAP(INTEGER_DIVIDE_BY_ZERO);                      \
    PUSH_##type(BITCAST_##type##_TO_##sign(lhs)          \
                    op BITCAST_##type##_TO_##sign(rhs)); \
  } while (0)

#define UNOP_FLOAT(type, func)                                 \
  do {                                                         \
    FLOAT_TYPE_##type value = BITCAST_TO_##type(POP_##type()); \
    PUSH_##type(BITCAST_FROM_##type(func(value)));             \
    break;                                                     \
  } while (0)

#define BINOP_FLOAT(type, op)                                \
  do {                                                       \
    FLOAT_TYPE_##type rhs = BITCAST_TO_##type(POP_##type()); \
    FLOAT_TYPE_##type lhs = BITCAST_TO_##type(POP_##type()); \
    PUSH_##type(BITCAST_FROM_##type(lhs op rhs));            \
  } while (0)

#define MIN_OP <
#define MAX_OP >

#define MINMAX_FLOAT(type, op)                                               \
  do {                                                                       \
    VALUE_TYPE_##type rhs = POP_##type();                                    \
    VALUE_TYPE_##type lhs = POP_##type();                                    \
    VALUE_TYPE_##type result;                                                \
    if (IS_NAN_##type(lhs)) {                                                \
      result = lhs;                                                          \
    } else if (IS_NAN_##type(rhs)) {                                         \
      result = rhs;                                                          \
    } else {                                                                 \
      FLOAT_TYPE_##type float_rhs = BITCAST_TO_##type(rhs);                  \
      FLOAT_TYPE_##type float_lhs = BITCAST_TO_##type(lhs);                  \
      result = BITCAST_FROM_##type(float_lhs op##_OP float_rhs ? float_lhs   \
                                                               : float_rhs); \
    }                                                                        \
    PUSH_##type(result);                                                     \
  } while (0)

static WASM_INLINE uint32_t read_u32(const uint8_t** pc) {
  uint32_t result;
  memcpy(&result, *pc, sizeof(uint32_t));
  *pc += sizeof(uint32_t);
  return result;
}

static WASM_INLINE uint32_t read_u32_at(const uint8_t* pc) {
  uint32_t result;
  memcpy(&result, pc, sizeof(uint32_t));
  return result;
}

static WASM_INLINE uint64_t read_u64(const uint8_t** pc) {
  uint64_t result;
  memcpy(&result, *pc, sizeof(uint64_t));
  *pc += sizeof(uint64_t);
  return result;
}

WasmInterpreterResult wasm_run_interpreter(WasmInterpreterModule* module,
                                           WasmInterpreterThread* thread,
                                           uint32_t num_instructions) {
  WasmInterpreterResult result = WASM_INTERPRETER_OK;
  WasmInterpreterValue* vs_bottom = &thread->value_stack.data[0];
  WasmInterpreterValue* vs_top = vs_bottom + thread->value_stack_top;
  WasmInterpreterValue* vs_end = vs_bottom + thread->value_stack.size;
  uint32_t* cs_bottom = &thread->call_stack.data[0];
  uint32_t* cs_top = cs_bottom + thread->call_stack_top;
  uint32_t* cs_end = cs_bottom + thread->call_stack.size;

  const uint8_t* istream = module->istream.start;
  const uint8_t* pc = &istream[thread->pc];
  uint32_t i;
  for (i = 0; i < num_instructions; ++i) {
    uint8_t opcode = *pc++;
#if LOG
    printf("%" PRIzd ": %s\n", (pc - 1) - istream, s_opcode_name[opcode]);
#endif
    switch (opcode) {
      case WASM_OPCODE_SELECT: {
        VALUE_TYPE_I32 cond = POP_I32();
        WasmInterpreterValue false_ = POP();
        WasmInterpreterValue true_ = POP();
        PUSH(cond ? true_ : false_);
        break;
      }

      case WASM_OPCODE_BR:
        GOTO(read_u32(&pc));
        break;

      case WASM_OPCODE_BR_IF: {
        uint32_t new_pc = read_u32(&pc);
        if (POP_I32())
          GOTO(new_pc);
        break;
      }

      case WASM_OPCODE_BR_TABLE: {
        uint32_t num_targets = read_u32(&pc);
        uint32_t table_offset = read_u32(&pc);
        VALUE_TYPE_I32 key = POP_I32();
        uint32_t key_offset;
        key_offset =
            (key >= num_targets ? num_targets : key) * sizeof(uint32_t);
        uint32_t new_pc = read_u32_at(istream + table_offset + key_offset);
        GOTO(new_pc);
        break;
      }

      case WASM_OPCODE_RETURN:
        if (cs_top == cs_bottom) {
          result = WASM_INTERPRETER_RETURNED;
          goto exit_loop;
        }
        GOTO(POP_CALL());
        break;

      case WASM_OPCODE_UNREACHABLE:
        TRAP(UNREACHABLE);
        break;

      case WASM_OPCODE_I8_CONST:
        break;

      case WASM_OPCODE_I32_CONST:
        PUSH_TYPE(I32, read_u32(&pc));
        break;

      case WASM_OPCODE_I64_CONST:
        PUSH_TYPE(I64, read_u64(&pc));
        break;

      case WASM_OPCODE_F32_CONST:
        PUSH_TYPE(F32, read_u32(&pc));
        break;

      case WASM_OPCODE_F64_CONST:
        PUSH_TYPE(F64, read_u64(&pc));
        break;

      case WASM_OPCODE_GET_LOCAL: {
        WasmInterpreterValue value = STACK_VALUE(read_u32(&pc));
        PUSH(value);
        break;
      }

      case WASM_OPCODE_SET_LOCAL:
        STACK_VALUE(read_u32(&pc)) = TOP();
        break;

      case WASM_OPCODE_CALL_FUNCTION: {
        uint32_t offset = read_u32(&pc);
        PUSH_CALL();
        GOTO(offset);
        break;
      }

      case WASM_OPCODE_CALL_INDIRECT: {
        uint32_t sig_index = read_u32(&pc);
        VALUE_TYPE_I32 entry_index = POP_I32();
        if (entry_index >= module->func_table.size)
          TRAP(UNDEFINED_TABLE_INDEX);
        WasmInterpreterFuncTableEntry* entry =
            &module->func_table.data[entry_index];
        if (entry->sig_index != sig_index)
          TRAP(INDIRECT_CALL_SIGNATURE_MISMATCH);
        PUSH_CALL();
        GOTO(entry->func_offset);
        break;
      }

      case WASM_OPCODE_CALL_IMPORT: {
        uint32_t import_index = read_u32(&pc);
        assert(import_index < module->imports.size);
        WasmInterpreterImport* import = &module->imports.data[import_index];
        uint32_t sig_index = import->sig_index;
        assert(sig_index < module->sigs.size);
        WasmInterpreterFuncSignature* sig = &module->sigs.data[sig_index];
        uint32_t num_args = sig->param_types.size;
        uint32_t i;
        assert(num_args <= thread->import_args.size);
        for (i = num_args; i > 0; --i) {
          WasmInterpreterValue value = POP();
          WasmInterpreterTypedValue* arg = &thread->import_args.data[i - 1];
          arg->type = sig->param_types.data[i - 1];
          arg->value = value;
        }
        assert(import->callback);
        WasmInterpreterTypedValue call_result =
            import->callback(module, import, num_args, thread->import_args.data,
                             import->user_data);
        if (sig->result_type != WASM_TYPE_VOID) {
          if (call_result.type != sig->result_type)
            TRAP(IMPORT_RESULT_TYPE_MISMATCH);
          PUSH(call_result.value);
        }
        break;
      }

      case WASM_OPCODE_I32_LOAD8_S:
        LOAD(I32, I8);
        break;

      case WASM_OPCODE_I32_LOAD8_U:
        LOAD(I32, U8);
        break;

      case WASM_OPCODE_I32_LOAD16_S:
        LOAD(I32, I16);
        break;

      case WASM_OPCODE_I32_LOAD16_U:
        LOAD(I32, U16);
        break;

      case WASM_OPCODE_I64_LOAD8_S:
        LOAD(I64, I8);
        break;

      case WASM_OPCODE_I64_LOAD8_U:
        LOAD(I64, U8);
        break;

      case WASM_OPCODE_I64_LOAD16_S:
        LOAD(I64, I16);
        break;

      case WASM_OPCODE_I64_LOAD16_U:
        LOAD(I64, U16);
        break;

      case WASM_OPCODE_I64_LOAD32_S:
        LOAD(I64, I32);
        break;

      case WASM_OPCODE_I64_LOAD32_U:
        LOAD(I64, U32);
        break;

      case WASM_OPCODE_I32_LOAD:
        LOAD(I32, U32);
        break;

      case WASM_OPCODE_I64_LOAD:
        LOAD(I64, U64);
        break;

      case WASM_OPCODE_F32_LOAD:
        LOAD(F32, F32);
        break;

      case WASM_OPCODE_F64_LOAD:
        LOAD(F64, F64);
        break;

      case WASM_OPCODE_I32_STORE8:
        STORE(I32, U8);
        break;

      case WASM_OPCODE_I32_STORE16:
        STORE(I32, U16);
        break;

      case WASM_OPCODE_I64_STORE8:
        STORE(I64, U8);
        break;

      case WASM_OPCODE_I64_STORE16:
        STORE(I64, U16);
        break;

      case WASM_OPCODE_I64_STORE32:
        STORE(I64, U32);
        break;

      case WASM_OPCODE_I32_STORE:
        STORE(I32, U32);
        break;

      case WASM_OPCODE_I64_STORE:
        STORE(I64, U64);
        break;

      case WASM_OPCODE_F32_STORE:
        STORE(F32, F32);
        break;

      case WASM_OPCODE_F64_STORE:
        STORE(F64, F64);
        break;

      case WASM_OPCODE_MEMORY_SIZE:
        PUSH_TYPE(I32, module->memory.page_size);
        break;

      case WASM_OPCODE_GROW_MEMORY: {
        VALUE_TYPE_I32 new_page_size = POP_I32();
        uint64_t new_byte_size = (uint64_t)new_page_size * WASM_PAGE_SIZE;
        if (new_byte_size > UINT32_MAX)
          TRAP(MEMORY_SIZE_OVERFLOW);
        WasmAllocator* allocator = module->memory.allocator;
        void* new_data = wasm_realloc(allocator, module->memory.data,
                                      new_byte_size, WASM_DEFAULT_ALIGN);
        if (new_data == NULL)
          TRAP(OUT_OF_MEMORY);
        module->memory.data = new_data;
        module->memory.page_size = new_page_size;
        module->memory.byte_size = new_byte_size;
        break;
      }

      case WASM_OPCODE_I32_ADD:
        BINOP(I32, +);
        break;

      case WASM_OPCODE_I32_SUB:
        BINOP(I32, -);
        break;

      case WASM_OPCODE_I32_MUL:
        BINOP(I32, *);
        break;

      case WASM_OPCODE_I32_DIV_S:
        BINOP_TRAP_ZERO(I32, /, SIGNED);
        break;

      case WASM_OPCODE_I32_DIV_U:
        BINOP_TRAP_ZERO(I32, /, UNSIGNED);
        break;

      case WASM_OPCODE_I32_REM_S:
        BINOP_TRAP_ZERO(I32, %, SIGNED);
        break;

      case WASM_OPCODE_I32_REM_U:
        BINOP_TRAP_ZERO(I32, %, UNSIGNED);
        break;

      case WASM_OPCODE_I32_AND:
        BINOP(I32, &);
        break;

      case WASM_OPCODE_I32_OR:
        BINOP(I32, |);
        break;

      case WASM_OPCODE_I32_XOR:
        BINOP(I32, ^);
        break;

      case WASM_OPCODE_I32_SHL:
        BINOP_SHIFT(I32, <<, UNSIGNED);
        break;

      case WASM_OPCODE_I32_SHR_U:
        BINOP_SHIFT(I32, >>, UNSIGNED);
        break;

      case WASM_OPCODE_I32_SHR_S:
        BINOP_SHIFT(I32, >>, SIGNED);
        break;

      case WASM_OPCODE_I32_EQ:
        BINOP(I32, ==);
        break;

      case WASM_OPCODE_I32_NE:
        BINOP(I32, !=);
        break;

      case WASM_OPCODE_I32_LT_S:
        BINOP_SIGNED(I32, <);
        break;

      case WASM_OPCODE_I32_LE_S:
        BINOP_SIGNED(I32, <=);
        break;

      case WASM_OPCODE_I32_LT_U:
        BINOP(I32, <);
        break;

      case WASM_OPCODE_I32_LE_U:
        BINOP(I32, <=);
        break;

      case WASM_OPCODE_I32_GT_S:
        BINOP_SIGNED(I32, >);
        break;

      case WASM_OPCODE_I32_GE_S:
        BINOP_SIGNED(I32, >=);
        break;

      case WASM_OPCODE_I32_GT_U:
        BINOP(I32, >);
        break;

      case WASM_OPCODE_I32_GE_U:
        BINOP(I32, >=);
        break;

      case WASM_OPCODE_I32_CLZ: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I32(value != 0 ? wasm_clz_u32(value) : 32);
        break;
      }

      case WASM_OPCODE_I32_CTZ: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I32(value != 0 ? wasm_ctz_u32(value) : 32);
        break;
      }

      case WASM_OPCODE_I32_POPCNT: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I32(wasm_popcount_u32(value));
        break;
      }

      case WASM_OPCODE_I32_EQZ: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I32(value == 0);
        break;
      }

      case WASM_OPCODE_I64_ADD:
        BINOP(I64, +);
        break;

      case WASM_OPCODE_I64_SUB:
        BINOP(I64, -);
        break;

      case WASM_OPCODE_I64_MUL:
        BINOP(I64, *);
        break;

      case WASM_OPCODE_I64_DIV_S:
        BINOP_TRAP_ZERO(I64, /, SIGNED);
        break;

      case WASM_OPCODE_I64_DIV_U:
        BINOP_TRAP_ZERO(I64, /, UNSIGNED);
        break;

      case WASM_OPCODE_I64_REM_S:
        BINOP_TRAP_ZERO(I64, %, SIGNED);
        break;

      case WASM_OPCODE_I64_REM_U:
        BINOP_TRAP_ZERO(I64, %, UNSIGNED);
        break;

      case WASM_OPCODE_I64_AND:
        BINOP(I64, &);
        break;

      case WASM_OPCODE_I64_OR:
        BINOP(I64, |);
        break;

      case WASM_OPCODE_I64_XOR:
        BINOP(I64, ^);
        break;

      case WASM_OPCODE_I64_SHL:
        BINOP_SHIFT(I64, <<, UNSIGNED);
        break;

      case WASM_OPCODE_I64_SHR_U:
        BINOP_SHIFT(I64, >>, UNSIGNED);
        break;

      case WASM_OPCODE_I64_SHR_S:
        BINOP_SHIFT(I64, >>, SIGNED);
        break;

      case WASM_OPCODE_I64_EQ:
        BINOP(I64, ==);
        break;

      case WASM_OPCODE_I64_NE:
        BINOP(I64, !=);
        break;

      case WASM_OPCODE_I64_LT_S:
        BINOP_SIGNED(I64, <);
        break;

      case WASM_OPCODE_I64_LE_S:
        BINOP_SIGNED(I64, <=);
        break;

      case WASM_OPCODE_I64_LT_U:
        BINOP(I64, <);
        break;

      case WASM_OPCODE_I64_LE_U:
        BINOP(I64, <=);
        break;

      case WASM_OPCODE_I64_GT_S:
        BINOP_SIGNED(I64, >);
        break;

      case WASM_OPCODE_I64_GE_S:
        BINOP_SIGNED(I64, >=);
        break;

      case WASM_OPCODE_I64_GT_U:
        BINOP(I64, >);
        break;

      case WASM_OPCODE_I64_GE_U:
        BINOP(I64, >=);
        break;

      case WASM_OPCODE_I64_CLZ: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I64(value != 0 ? wasm_clz_u64(value) : 64);
        break;
      }

      case WASM_OPCODE_I64_CTZ: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I64(value != 0 ? wasm_ctz_u64(value) : 64);
        break;
      }

      case WASM_OPCODE_I64_POPCNT: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I64(wasm_popcount_u64(value));
        break;
      }

      case WASM_OPCODE_F32_ADD:
        BINOP_FLOAT(F32, +);
        break;

      case WASM_OPCODE_F32_SUB:
        BINOP_FLOAT(F32, -);
        break;

      case WASM_OPCODE_F32_MUL:
        BINOP_FLOAT(F32, *);
        break;

      case WASM_OPCODE_F32_DIV:
        BINOP_FLOAT(F32, /);
        break;

      case WASM_OPCODE_F32_MIN:
        MINMAX_FLOAT(F32, MIN);
        break;

      case WASM_OPCODE_F32_MAX:
        MINMAX_FLOAT(F32, MAX);
        break;

      case WASM_OPCODE_F32_ABS:
        TOP().f32_bits &= ~F32_SIGN_MASK;
        break;

      case WASM_OPCODE_F32_NEG:
        TOP().f32_bits ^= F32_SIGN_MASK;
        break;

      case WASM_OPCODE_F32_COPYSIGN: {
        VALUE_TYPE_F32 rhs = POP_F32();
        VALUE_TYPE_F32 lhs = POP_F32();
        PUSH_F32((lhs & ~F32_SIGN_MASK) | (rhs & F32_SIGN_MASK));
        break;
      }

      case WASM_OPCODE_F32_CEIL:
        UNOP_FLOAT(F32, ceilf);
        break;

      case WASM_OPCODE_F32_FLOOR:
        UNOP_FLOAT(F32, floorf);
        break;

      case WASM_OPCODE_F32_TRUNC:
        UNOP_FLOAT(F32, truncf);
        break;

      case WASM_OPCODE_F32_NEAREST:
        UNOP_FLOAT(F32, nearbyintf);
        break;

      case WASM_OPCODE_F32_SQRT:
        UNOP_FLOAT(F32, sqrtf);
        break;

      case WASM_OPCODE_F32_EQ:
        BINOP_FLOAT(F32, ==);
        break;

      case WASM_OPCODE_F32_NE:
        BINOP_FLOAT(F32, !=);
        break;

      case WASM_OPCODE_F32_LT:
        BINOP_FLOAT(F32, <);
        break;

      case WASM_OPCODE_F32_LE:
        BINOP_FLOAT(F32, <=);
        break;

      case WASM_OPCODE_F32_GT:
        BINOP_FLOAT(F32, >);
        break;

      case WASM_OPCODE_F32_GE:
        BINOP_FLOAT(F32, >=);
        break;

      case WASM_OPCODE_F64_ADD:
        BINOP_FLOAT(F64, +);
        break;

      case WASM_OPCODE_F64_SUB:
        BINOP_FLOAT(F64, -);
        break;

      case WASM_OPCODE_F64_MUL:
        BINOP_FLOAT(F64, *);
        break;

      case WASM_OPCODE_F64_DIV:
        BINOP_FLOAT(F64, /);
        break;

      case WASM_OPCODE_F64_MIN:
        MINMAX_FLOAT(F64, MIN);
        break;

      case WASM_OPCODE_F64_MAX:
        MINMAX_FLOAT(F64, MAX);
        break;

      case WASM_OPCODE_F64_ABS:
        TOP().f64_bits &= ~F64_SIGN_MASK;
        break;

      case WASM_OPCODE_F64_NEG:
        TOP().f64_bits ^= F64_SIGN_MASK;
        break;

      case WASM_OPCODE_F64_COPYSIGN: {
        VALUE_TYPE_F64 rhs = POP_F64();
        VALUE_TYPE_F64 lhs = POP_F64();
        PUSH_F64((lhs & ~F64_SIGN_MASK) | (rhs & F64_SIGN_MASK));
        break;
      }

      case WASM_OPCODE_F64_CEIL:
        UNOP_FLOAT(F64, ceilf);
        break;

      case WASM_OPCODE_F64_FLOOR:
        UNOP_FLOAT(F64, floorf);
        break;

      case WASM_OPCODE_F64_TRUNC:
        UNOP_FLOAT(F64, truncf);
        break;

      case WASM_OPCODE_F64_NEAREST:
        UNOP_FLOAT(F64, nearbyintf);
        break;

      case WASM_OPCODE_F64_SQRT:
        UNOP_FLOAT(F64, sqrtf);
        break;

      case WASM_OPCODE_F64_EQ:
        BINOP_FLOAT(F64, ==);
        break;

      case WASM_OPCODE_F64_NE:
        BINOP_FLOAT(F64, !=);
        break;

      case WASM_OPCODE_F64_LT:
        BINOP_FLOAT(F64, <);
        break;

      case WASM_OPCODE_F64_LE:
        BINOP_FLOAT(F64, <=);
        break;

      case WASM_OPCODE_F64_GT:
        BINOP_FLOAT(F64, >);
        break;

      case WASM_OPCODE_F64_GE:
        BINOP_FLOAT(F64, >=);
        break;

      case WASM_OPCODE_I32_TRUNC_S_F32: {
        VALUE_TYPE_F32 value = POP_F32();
        if (is_nan_f32(value))
          TRAP(INVALID_CONVERSION_TO_INTEGER);
        int exp = get_exp_f32(value);
        if (exp > 31)
          TRAP(INTEGER_OVERFLOW);
        PUSH_I32((int32_t)BITCAST_TO_F32(value));
        break;
      }

      case WASM_OPCODE_I32_TRUNC_S_F64: {
        VALUE_TYPE_F64 value = POP_F64();
        if (is_nan_f64(value))
          TRAP(INVALID_CONVERSION_TO_INTEGER);
        int exp = get_exp_f64(value);
        if (exp >= 31)
          TRAP(INTEGER_OVERFLOW);
        PUSH_I32((int32_t)BITCAST_TO_F64(value));
        break;
      }

      case WASM_OPCODE_I32_TRUNC_U_F32: {
        VALUE_TYPE_F32 value = POP_F32();
        if (is_nan_f32(value))
          TRAP(INVALID_CONVERSION_TO_INTEGER);
        int exp = get_exp_f32(value);
        if (is_signed_f32(value) || exp >= 32)
          TRAP(INTEGER_OVERFLOW);
        PUSH_I32((uint32_t)BITCAST_TO_F32(value));
        break;
      }

      case WASM_OPCODE_I32_TRUNC_U_F64: {
        VALUE_TYPE_F64 value = POP_F64();
        if (is_nan_f64(value))
          TRAP(INVALID_CONVERSION_TO_INTEGER);
        int exp = get_exp_f64(value);
        if (is_signed_f64(value) || exp >= 32)
          TRAP(INTEGER_OVERFLOW);
        PUSH_I32((uint32_t)BITCAST_TO_F64(value));
        break;
      }

      case WASM_OPCODE_I32_WRAP_I64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I32((uint32_t)value);
        break;
      }

      case WASM_OPCODE_I64_TRUNC_S_F32: {
        VALUE_TYPE_F32 value = POP_F32();
        if (is_nan_f32(value))
          TRAP(INVALID_CONVERSION_TO_INTEGER);
        int exp = get_exp_f32(value);
        if (exp >= 63)
          TRAP(INTEGER_OVERFLOW);
        PUSH_I64((int64_t)BITCAST_TO_F32(value));
        break;
      }

      case WASM_OPCODE_I64_TRUNC_S_F64: {
        VALUE_TYPE_F64 value = POP_F64();
        if (is_nan_f64(value))
          TRAP(INVALID_CONVERSION_TO_INTEGER);
        int exp = get_exp_f64(value);
        if (exp >= 63)
          TRAP(INTEGER_OVERFLOW);
        PUSH_I64((int64_t)BITCAST_TO_F64(value));
        break;
      }

      case WASM_OPCODE_I64_TRUNC_U_F32: {
        VALUE_TYPE_F32 value = POP_F32();
        if (is_nan_f32(value))
          TRAP(INVALID_CONVERSION_TO_INTEGER);
        int exp = get_exp_f32(value);
        if (is_signed_f64(value) || exp >= 64)
          TRAP(INTEGER_OVERFLOW);
        PUSH_I64((uint64_t)BITCAST_TO_F32(value));
        break;
      }

      case WASM_OPCODE_I64_TRUNC_U_F64: {
        VALUE_TYPE_F64 value = POP_F64();
        if (is_nan_f64(value))
          TRAP(INVALID_CONVERSION_TO_INTEGER);
        int exp = get_exp_f64(value);
        if (is_signed_f64(value) || exp >= 64)
          TRAP(INTEGER_OVERFLOW);
        PUSH_I64((uint64_t)BITCAST_TO_F64(value));
        break;
      }

      case WASM_OPCODE_I64_EXTEND_S_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I64((int64_t)BITCAST_I32_TO_SIGNED(value));
        break;
      }

      case WASM_OPCODE_I64_EXTEND_U_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I64((uint64_t)value);
        break;
      }

      case WASM_OPCODE_F32_CONVERT_S_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F32(BITCAST_FROM_F32((float)BITCAST_I32_TO_SIGNED(value)));
        break;
      }

      case WASM_OPCODE_F32_CONVERT_U_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F32(BITCAST_FROM_F32((float)value));
        break;
      }

      case WASM_OPCODE_F32_CONVERT_S_I64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F32(BITCAST_FROM_F32((float)BITCAST_I64_TO_SIGNED(value)));
        break;
      }

      case WASM_OPCODE_F32_CONVERT_U_I64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F32(BITCAST_FROM_F32((float)value));
        break;
      }

      case WASM_OPCODE_F32_DEMOTE_F64: {
        VALUE_TYPE_F64 value = POP_F64();
        PUSH_F32(BITCAST_FROM_F32((float)BITCAST_TO_F64(value)));
        break;
      }

      case WASM_OPCODE_F32_REINTERPRET_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F32(value);
        break;
      }

      case WASM_OPCODE_F64_CONVERT_S_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F64(BITCAST_FROM_F64((float)BITCAST_I32_TO_SIGNED(value)));
        break;
      }

      case WASM_OPCODE_F64_CONVERT_U_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F64(BITCAST_FROM_F64((float)value));
        break;
      }

      case WASM_OPCODE_F64_CONVERT_S_I64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F64(BITCAST_FROM_F64((float)BITCAST_I64_TO_SIGNED(value)));
        break;
      }

      case WASM_OPCODE_F64_CONVERT_U_I64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F64(BITCAST_FROM_F64((float)value));
        break;
      }

      case WASM_OPCODE_F64_PROMOTE_F32: {
        VALUE_TYPE_F32 value = POP_F32();
        PUSH_F64(BITCAST_FROM_F64((double)BITCAST_TO_F32(value)));
        break;
      }

      case WASM_OPCODE_F64_REINTERPRET_I64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F64(value);
        break;
      }

      case WASM_OPCODE_I32_REINTERPRET_F32: {
        VALUE_TYPE_F32 value = POP_F32();
        PUSH_I32(value);
        break;
      }

      case WASM_OPCODE_I64_REINTERPRET_F64: {
        VALUE_TYPE_F64 value = POP_F64();
        PUSH_I64(value);
        break;
      }

      case WASM_OPCODE_I32_ROTR:
        BINOP_ROT(I32, RIGHT);
        break;

      case WASM_OPCODE_I32_ROTL:
        BINOP_ROT(I32, LEFT);
        break;

      case WASM_OPCODE_I64_ROTR:
        BINOP_ROT(I64, RIGHT);
        break;

      case WASM_OPCODE_I64_ROTL:
        BINOP_ROT(I64, LEFT);
        break;

      case WASM_OPCODE_I64_EQZ: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I64(value == 0);
        break;
      }

      case WASM_OPCODE_ALLOCA: {
        WasmInterpreterValue* old_vs_top = vs_top;
        vs_top += read_u32(&pc);
        CHECK_STACK();
        memset(old_vs_top, 0, vs_top - old_vs_top);
        break;
      }

      case WASM_OPCODE_DISCARD:
        POP();
        break;

      case WASM_OPCODE_DISCARD_KEEP: {
        uint32_t discard_count = read_u32(&pc);
        uint8_t keep_count = *pc++;
        assert(keep_count <= 1);
        if (keep_count == 1)
          STACK_VALUE(discard_count + 1) = TOP();
        vs_top -= discard_count;
        break;
      }

      default:
        assert(0);
        break;
    }
  }

exit_loop:
  thread->value_stack_top = vs_top - vs_bottom;
  thread->call_stack_top = cs_top - cs_bottom;
  thread->pc = pc - istream;
  return result;
}

#if 0
void wasm_trace_pc(WasmInterpreterModule* module,
                   WasmInterpreterThread* thread) {
  const uint8_t* istream = module->istream.start;
  const uint8_t* pc = &istream[thread->pc];
  uint8_t opcode = *pc++;
  switch (opcode) {
    case WASM_OPCODE_SELECT:
      printf("select %u, %" PRIu64 ", %" PRIu64 "\n", STACK_VALUE(3).u32,
             STACK_VALUE(2).u64, STACK_VALUE(1).u64);
      break;

    case WASM_OPCODE_BR:
      printf("br @%u\n", read_u32_at(pc));
      break;

    case WASM_OPCODE_BR_IF:
      printf("br_if @%u, %u\n", read_u32_at(pc), STACK_VALUE(1).u32);
      break;

    case WASM_OPCODE_BR_TABLE: {
      uint32_t num_targets = read_u32(&pc);
      uint32_t table_offset = read_u32(&pc);
      VALUE_TYPE_I32 key = POP_I32();
      uint32_t key_offset;
      key_offset =
          (key >= num_targets ? num_targets : key) * sizeof(uint32_t);
      uint32_t new_pc = read_u32_at(istream + table_offset + key_offset);
      GOTO(new_pc);
      break;
    }

    case WASM_OPCODE_RETURN:
      if (cs_top > 0) {
        result = WASM_INTERPRETER_RETURNED;
        goto exit_loop;
      }
      GOTO(POP_CALL());
      break;

    case WASM_OPCODE_UNREACHABLE:
      TRAP(UNREACHABLE);
      break;

    case WASM_OPCODE_I8_CONST:
      break;

    case WASM_OPCODE_I32_CONST:
      PUSH_TYPE(I32, read_u32(&pc));
      break;

    case WASM_OPCODE_I64_CONST:
      PUSH_TYPE(I64, read_u64(&pc));
      break;

    case WASM_OPCODE_F32_CONST:
      PUSH_TYPE(F32, read_u32(&pc));
      break;

    case WASM_OPCODE_F64_CONST:
      PUSH_TYPE(F64, read_u64(&pc));
      break;

    case WASM_OPCODE_GET_LOCAL: {
      WasmInterpreterValue value = STACK_VALUE(read_u32(&pc));
      PUSH(value);
      break;
    }

    case WASM_OPCODE_SET_LOCAL:
      STACK_VALUE(read_u32(&pc)) = TOP();
      break;

    case WASM_OPCODE_CALL_FUNCTION: {
      uint32_t offset = read_u32(&pc);
      PUSH_CALL(offset);
      GOTO(offset);
      break;
    }

    case WASM_OPCODE_CALL_INDIRECT: {
      uint32_t sig_index = read_u32(&pc);
      VALUE_TYPE_I32 entry_index = POP_I32();
      if (entry_index >= module->func_table.size)
        TRAP(UNDEFINED_TABLE_INDEX);
      WasmInterpreterFuncTableEntry* entry =
          &module->func_table.data[entry_index];
      if (entry->sig_index != sig_index)
        TRAP(INDIRECT_CALL_SIGNATURE_MISMATCH);
      PUSH_CALL(entry->func_offset);
      GOTO(entry->func_offset);
      break;
    }

    case WASM_OPCODE_CALL_IMPORT: {
      uint32_t import_index = read_u32(&pc);
      assert(import_index < module->imports.size);
      WasmInterpreterImport* import = &module->imports.data[import_index];
      uint32_t sig_index = import->sig_index;
      assert(sig_index < module->sigs.size);
      WasmInterpreterFuncSignature* sig = &module->sigs.data[sig_index];
      uint32_t num_args = sig->param_types.size;
      uint32_t i;
      assert(num_args <= thread->import_args.size);
      for (i = num_args; i > 0; --i) {
        WasmInterpreterValue value = POP();
        WasmInterpreterTypedValue* arg = &thread->import_args.data[i - 1];
        arg->type = sig->param_types.data[i - 1];
        arg->value = value;
      }
      assert(import->callback);
      WasmInterpreterTypedValue call_result =
          import->callback(module, import, num_args, thread->import_args.data,
                           import->user_data);
      if (sig->result_type != WASM_TYPE_VOID) {
        if (call_result.type != sig->result_type)
          TRAP(IMPORT_RESULT_TYPE_MISMATCH);
        PUSH(call_result.value);
      }
      break;
    }

    case WASM_OPCODE_I32_LOAD8_S:
      LOAD(I32, I8);
      break;

    case WASM_OPCODE_I32_LOAD8_U:
      LOAD(I32, U8);
      break;

    case WASM_OPCODE_I32_LOAD16_S:
      LOAD(I32, I16);
      break;

    case WASM_OPCODE_I32_LOAD16_U:
      LOAD(I32, U16);
      break;

    case WASM_OPCODE_I64_LOAD8_S:
      LOAD(I64, I8);
      break;

    case WASM_OPCODE_I64_LOAD8_U:
      LOAD(I64, U8);
      break;

    case WASM_OPCODE_I64_LOAD16_S:
      LOAD(I64, I16);
      break;

    case WASM_OPCODE_I64_LOAD16_U:
      LOAD(I64, U16);
      break;

    case WASM_OPCODE_I64_LOAD32_S:
      LOAD(I64, I32);
      break;

    case WASM_OPCODE_I64_LOAD32_U:
      LOAD(I64, U32);
      break;

    case WASM_OPCODE_I32_LOAD:
      LOAD(I32, U32);
      break;

    case WASM_OPCODE_I64_LOAD:
      LOAD(I64, U64);
      break;

    case WASM_OPCODE_F32_LOAD:
      LOAD(F32, F32);
      break;

    case WASM_OPCODE_F64_LOAD:
      LOAD(F64, F64);
      break;

    case WASM_OPCODE_I32_STORE8:
      STORE(I32, U8);
      break;

    case WASM_OPCODE_I32_STORE16:
      STORE(I32, U16);
      break;

    case WASM_OPCODE_I64_STORE8:
      STORE(I64, U8);
      break;

    case WASM_OPCODE_I64_STORE16:
      STORE(I64, U16);
      break;

    case WASM_OPCODE_I64_STORE32:
      STORE(I64, U32);
      break;

    case WASM_OPCODE_I32_STORE:
      STORE(I32, U32);
      break;

    case WASM_OPCODE_I64_STORE:
      STORE(I64, U64);
      break;

    case WASM_OPCODE_F32_STORE:
      STORE(F32, F32);
      break;

    case WASM_OPCODE_F64_STORE:
      STORE(F64, F64);
      break;

    case WASM_OPCODE_MEMORY_SIZE:
      PUSH_TYPE(I32, module->memory.page_size);
      break;

    case WASM_OPCODE_GROW_MEMORY: {
      VALUE_TYPE_I32 new_page_size = POP_I32();
      uint64_t new_byte_size = (uint64_t)new_page_size * WASM_PAGE_SIZE;
      if (new_byte_size > UINT32_MAX)
        TRAP(MEMORY_SIZE_OVERFLOW);
      WasmAllocator* allocator = module->memory.allocator;
      void* new_data = wasm_realloc(allocator, module->memory.data,
                                    new_byte_size, WASM_DEFAULT_ALIGN);
      if (new_data == NULL)
        TRAP(OUT_OF_MEMORY);
      module->memory.data = new_data;
      module->memory.page_size = new_page_size;
      module->memory.byte_size = new_byte_size;
      break;
    }

    case WASM_OPCODE_I32_ADD:
      BINOP(I32, +);
      break;

    case WASM_OPCODE_I32_SUB:
      BINOP(I32, -);
      break;

    case WASM_OPCODE_I32_MUL:
      BINOP(I32, *);
      break;

    case WASM_OPCODE_I32_DIV_S:
      BINOP_TRAP_ZERO(I32, /, SIGNED);
      break;

    case WASM_OPCODE_I32_DIV_U:
      BINOP_TRAP_ZERO(I32, /, UNSIGNED);
      break;

    case WASM_OPCODE_I32_REM_S:
      BINOP_TRAP_ZERO(I32, %, SIGNED);
      break;

    case WASM_OPCODE_I32_REM_U:
      BINOP_TRAP_ZERO(I32, %, UNSIGNED);
      break;

    case WASM_OPCODE_I32_AND:
      BINOP(I32, &);
      break;

    case WASM_OPCODE_I32_OR:
      BINOP(I32, |);
      break;

    case WASM_OPCODE_I32_XOR:
      BINOP(I32, ^);
      break;

    case WASM_OPCODE_I32_SHL:
      BINOP_SHIFT(I32, <<, UNSIGNED);
      break;

    case WASM_OPCODE_I32_SHR_U:
      BINOP_SHIFT(I32, >>, UNSIGNED);
      break;

    case WASM_OPCODE_I32_SHR_S:
      BINOP_SHIFT(I32, >>, SIGNED);
      break;

    case WASM_OPCODE_I32_EQ:
      BINOP(I32, ==);
      break;

    case WASM_OPCODE_I32_NE:
      BINOP(I32, !=);
      break;

    case WASM_OPCODE_I32_LT_S:
      BINOP_SIGNED(I32, <);
      break;

    case WASM_OPCODE_I32_LE_S:
      BINOP_SIGNED(I32, <=);
      break;

    case WASM_OPCODE_I32_LT_U:
      BINOP(I32, <);
      break;

    case WASM_OPCODE_I32_LE_U:
      BINOP(I32, <=);
      break;

    case WASM_OPCODE_I32_GT_S:
      BINOP_SIGNED(I32, >);
      break;

    case WASM_OPCODE_I32_GE_S:
      BINOP_SIGNED(I32, >=);
      break;

    case WASM_OPCODE_I32_GT_U:
      BINOP(I32, >);
      break;

    case WASM_OPCODE_I32_GE_U:
      BINOP(I32, >=);
      break;

    case WASM_OPCODE_I32_CLZ: {
      VALUE_TYPE_I32 value = POP_I32();
      PUSH_I32(value != 0 ? wasm_clz_u32(value) : 32);
      break;
    }

    case WASM_OPCODE_I32_CTZ: {
      VALUE_TYPE_I32 value = POP_I32();
      PUSH_I32(value != 0 ? wasm_ctz_u32(value) : 32);
      break;
    }

    case WASM_OPCODE_I32_POPCNT: {
      VALUE_TYPE_I32 value = POP_I32();
      PUSH_I32(wasm_popcount_u32(value));
      break;
    }

    case WASM_OPCODE_I32_EQZ: {
      VALUE_TYPE_I32 value = POP_I32();
      PUSH_I32(value == 0);
      break;
    }

    case WASM_OPCODE_I64_ADD:
      BINOP(I64, +);
      break;

    case WASM_OPCODE_I64_SUB:
      BINOP(I64, -);
      break;

    case WASM_OPCODE_I64_MUL:
      BINOP(I64, *);
      break;

    case WASM_OPCODE_I64_DIV_S:
      BINOP_TRAP_ZERO(I64, /, SIGNED);
      break;

    case WASM_OPCODE_I64_DIV_U:
      BINOP_TRAP_ZERO(I64, /, UNSIGNED);
      break;

    case WASM_OPCODE_I64_REM_S:
      BINOP_TRAP_ZERO(I64, %, SIGNED);
      break;

    case WASM_OPCODE_I64_REM_U:
      BINOP_TRAP_ZERO(I64, %, UNSIGNED);
      break;

    case WASM_OPCODE_I64_AND:
      BINOP(I64, &);
      break;

    case WASM_OPCODE_I64_OR:
      BINOP(I64, |);
      break;

    case WASM_OPCODE_I64_XOR:
      BINOP(I64, ^);
      break;

    case WASM_OPCODE_I64_SHL:
      BINOP_SHIFT(I64, <<, UNSIGNED);
      break;

    case WASM_OPCODE_I64_SHR_U:
      BINOP_SHIFT(I64, >>, UNSIGNED);
      break;

    case WASM_OPCODE_I64_SHR_S:
      BINOP_SHIFT(I64, >>, SIGNED);
      break;

    case WASM_OPCODE_I64_EQ:
      BINOP(I64, ==);
      break;

    case WASM_OPCODE_I64_NE:
      BINOP(I64, !=);
      break;

    case WASM_OPCODE_I64_LT_S:
      BINOP_SIGNED(I64, <);
      break;

    case WASM_OPCODE_I64_LE_S:
      BINOP_SIGNED(I64, <=);
      break;

    case WASM_OPCODE_I64_LT_U:
      BINOP(I64, <);
      break;

    case WASM_OPCODE_I64_LE_U:
      BINOP(I64, <=);
      break;

    case WASM_OPCODE_I64_GT_S:
      BINOP_SIGNED(I64, >);
      break;

    case WASM_OPCODE_I64_GE_S:
      BINOP_SIGNED(I64, >=);
      break;

    case WASM_OPCODE_I64_GT_U:
      BINOP(I64, >);
      break;

    case WASM_OPCODE_I64_GE_U:
      BINOP(I64, >=);
      break;

    case WASM_OPCODE_I64_CLZ: {
      VALUE_TYPE_I64 value = POP_I64();
      PUSH_I64(value != 0 ? wasm_clz_u64(value) : 64);
      break;
    }

    case WASM_OPCODE_I64_CTZ: {
      VALUE_TYPE_I64 value = POP_I64();
      PUSH_I64(value != 0 ? wasm_ctz_u64(value) : 64);
      break;
    }

    case WASM_OPCODE_I64_POPCNT: {
      VALUE_TYPE_I64 value = POP_I64();
      PUSH_I64(wasm_popcount_u64(value));
      break;
    }

    case WASM_OPCODE_F32_ADD:
      BINOP_FLOAT(F32, +);
      break;

    case WASM_OPCODE_F32_SUB:
      BINOP_FLOAT(F32, -);
      break;

    case WASM_OPCODE_F32_MUL:
      BINOP_FLOAT(F32, *);
      break;

    case WASM_OPCODE_F32_DIV:
      BINOP_FLOAT(F32, /);
      break;

    case WASM_OPCODE_F32_MIN:
      MINMAX_FLOAT(F32, MIN);
      break;

    case WASM_OPCODE_F32_MAX:
      MINMAX_FLOAT(F32, MAX);
      break;

    case WASM_OPCODE_F32_ABS:
      TOP().f32_bits &= ~F32_SIGN_MASK;
      break;

    case WASM_OPCODE_F32_NEG:
      TOP().f32_bits ^= F32_SIGN_MASK;
      break;

    case WASM_OPCODE_F32_COPYSIGN: {
      VALUE_TYPE_F32 rhs = POP_F32();
      VALUE_TYPE_F32 lhs = POP_F32();
      PUSH_F32((lhs & ~F32_SIGN_MASK) | (rhs & F32_SIGN_MASK));
      break;
    }

    case WASM_OPCODE_F32_CEIL:
      UNOP_FLOAT(F32, ceilf);
      break;

    case WASM_OPCODE_F32_FLOOR:
      UNOP_FLOAT(F32, floorf);
      break;

    case WASM_OPCODE_F32_TRUNC:
      UNOP_FLOAT(F32, truncf);
      break;

    case WASM_OPCODE_F32_NEAREST:
      UNOP_FLOAT(F32, nearbyintf);
      break;

    case WASM_OPCODE_F32_SQRT:
      UNOP_FLOAT(F32, sqrtf);
      break;

    case WASM_OPCODE_F32_EQ:
      BINOP_FLOAT(F32, ==);
      break;

    case WASM_OPCODE_F32_NE:
      BINOP_FLOAT(F32, !=);
      break;

    case WASM_OPCODE_F32_LT:
      BINOP_FLOAT(F32, <);
      break;

    case WASM_OPCODE_F32_LE:
      BINOP_FLOAT(F32, <=);
      break;

    case WASM_OPCODE_F32_GT:
      BINOP_FLOAT(F32, >);
      break;

    case WASM_OPCODE_F32_GE:
      BINOP_FLOAT(F32, >=);
      break;

    case WASM_OPCODE_F64_ADD:
      BINOP_FLOAT(F64, +);
      break;

    case WASM_OPCODE_F64_SUB:
      BINOP_FLOAT(F64, -);
      break;

    case WASM_OPCODE_F64_MUL:
      BINOP_FLOAT(F64, *);
      break;

    case WASM_OPCODE_F64_DIV:
      BINOP_FLOAT(F64, /);
      break;

    case WASM_OPCODE_F64_MIN:
      MINMAX_FLOAT(F64, MIN);
      break;

    case WASM_OPCODE_F64_MAX:
      MINMAX_FLOAT(F64, MAX);
      break;

    case WASM_OPCODE_F64_ABS:
      TOP().f64_bits &= ~F64_SIGN_MASK;
      break;

    case WASM_OPCODE_F64_NEG:
      TOP().f64_bits ^= F64_SIGN_MASK;
      break;

    case WASM_OPCODE_F64_COPYSIGN: {
      VALUE_TYPE_F64 rhs = POP_F64();
      VALUE_TYPE_F64 lhs = POP_F64();
      PUSH_F64((lhs & ~F64_SIGN_MASK) | (rhs & F64_SIGN_MASK));
      break;
    }

    case WASM_OPCODE_F64_CEIL:
      UNOP_FLOAT(F64, ceilf);
      break;

    case WASM_OPCODE_F64_FLOOR:
      UNOP_FLOAT(F64, floorf);
      break;

    case WASM_OPCODE_F64_TRUNC:
      UNOP_FLOAT(F64, truncf);
      break;

    case WASM_OPCODE_F64_NEAREST:
      UNOP_FLOAT(F64, nearbyintf);
      break;

    case WASM_OPCODE_F64_SQRT:
      UNOP_FLOAT(F64, sqrtf);
      break;

    case WASM_OPCODE_F64_EQ:
      BINOP_FLOAT(F64, ==);
      break;

    case WASM_OPCODE_F64_NE:
      BINOP_FLOAT(F64, !=);
      break;

    case WASM_OPCODE_F64_LT:
      BINOP_FLOAT(F64, <);
      break;

    case WASM_OPCODE_F64_LE:
      BINOP_FLOAT(F64, <=);
      break;

    case WASM_OPCODE_F64_GT:
      BINOP_FLOAT(F64, >);
      break;

    case WASM_OPCODE_F64_GE:
      BINOP_FLOAT(F64, >=);
      break;

    case WASM_OPCODE_I32_TRUNC_S_F32: {
      VALUE_TYPE_F32 value = POP_F32();
      if (is_nan_f32(value))
        TRAP(INVALID_CONVERSION_TO_INTEGER);
      int exp = get_exp_f32(value);
      if (exp > 31)
        TRAP(INTEGER_OVERFLOW);
      PUSH_I32((int32_t)BITCAST_TO_F32(value));
      break;
    }

    case WASM_OPCODE_I32_TRUNC_S_F64: {
      VALUE_TYPE_F64 value = POP_F64();
      if (is_nan_f64(value))
        TRAP(INVALID_CONVERSION_TO_INTEGER);
      int exp = get_exp_f64(value);
      if (exp >= 31)
        TRAP(INTEGER_OVERFLOW);
      PUSH_I32((int32_t)BITCAST_TO_F64(value));
      break;
    }

    case WASM_OPCODE_I32_TRUNC_U_F32: {
      VALUE_TYPE_F32 value = POP_F32();
      if (is_nan_f32(value))
        TRAP(INVALID_CONVERSION_TO_INTEGER);
      int exp = get_exp_f32(value);
      if (is_signed_f32(value) || exp >= 32)
        TRAP(INTEGER_OVERFLOW);
      PUSH_I32((uint32_t)BITCAST_TO_F32(value));
      break;
    }

    case WASM_OPCODE_I32_TRUNC_U_F64: {
      VALUE_TYPE_F64 value = POP_F64();
      if (is_nan_f64(value))
        TRAP(INVALID_CONVERSION_TO_INTEGER);
      int exp = get_exp_f64(value);
      if (is_signed_f64(value) || exp >= 32)
        TRAP(INTEGER_OVERFLOW);
      PUSH_I32((uint32_t)BITCAST_TO_F64(value));
      break;
    }

    case WASM_OPCODE_I32_WRAP_I64: {
      VALUE_TYPE_I64 value = POP_I64();
      PUSH_I32((uint32_t)value);
      break;
    }

    case WASM_OPCODE_I64_TRUNC_S_F32: {
      VALUE_TYPE_F32 value = POP_F32();
      if (is_nan_f32(value))
        TRAP(INVALID_CONVERSION_TO_INTEGER);
      int exp = get_exp_f32(value);
      if (exp >= 63)
        TRAP(INTEGER_OVERFLOW);
      PUSH_I64((int64_t)BITCAST_TO_F32(value));
      break;
    }

    case WASM_OPCODE_I64_TRUNC_S_F64: {
      VALUE_TYPE_F64 value = POP_F64();
      if (is_nan_f64(value))
        TRAP(INVALID_CONVERSION_TO_INTEGER);
      int exp = get_exp_f64(value);
      if (exp >= 63)
        TRAP(INTEGER_OVERFLOW);
      PUSH_I64((int64_t)BITCAST_TO_F64(value));
      break;
    }

    case WASM_OPCODE_I64_TRUNC_U_F32: {
      VALUE_TYPE_F32 value = POP_F32();
      if (is_nan_f32(value))
        TRAP(INVALID_CONVERSION_TO_INTEGER);
      int exp = get_exp_f32(value);
      if (is_signed_f64(value) || exp >= 64)
        TRAP(INTEGER_OVERFLOW);
      PUSH_I64((uint64_t)BITCAST_TO_F32(value));
      break;
    }

    case WASM_OPCODE_I64_TRUNC_U_F64: {
      VALUE_TYPE_F64 value = POP_F64();
      if (is_nan_f64(value))
        TRAP(INVALID_CONVERSION_TO_INTEGER);
      int exp = get_exp_f64(value);
      if (is_signed_f64(value) || exp >= 64)
        TRAP(INTEGER_OVERFLOW);
      PUSH_I64((uint64_t)BITCAST_TO_F64(value));
      break;
    }

    case WASM_OPCODE_I64_EXTEND_S_I32: {
      VALUE_TYPE_I32 value = POP_I32();
      PUSH_I64((int64_t)BITCAST_I32_TO_SIGNED(value));
      break;
    }

    case WASM_OPCODE_I64_EXTEND_U_I32: {
      VALUE_TYPE_I32 value = POP_I32();
      PUSH_I64((uint64_t)value);
      break;
    }

    case WASM_OPCODE_F32_CONVERT_S_I32: {
      VALUE_TYPE_I32 value = POP_I32();
      PUSH_F32(BITCAST_FROM_F32((float)BITCAST_I32_TO_SIGNED(value)));
      break;
    }

    case WASM_OPCODE_F32_CONVERT_U_I32: {
      VALUE_TYPE_I32 value = POP_I32();
      PUSH_F32(BITCAST_FROM_F32((float)value));
      break;
    }

    case WASM_OPCODE_F32_CONVERT_S_I64: {
      VALUE_TYPE_I64 value = POP_I64();
      PUSH_F32(BITCAST_FROM_F32((float)BITCAST_I64_TO_SIGNED(value)));
      break;
    }

    case WASM_OPCODE_F32_CONVERT_U_I64: {
      VALUE_TYPE_I64 value = POP_I64();
      PUSH_F32(BITCAST_FROM_F32((float)value));
      break;
    }

    case WASM_OPCODE_F32_DEMOTE_F64: {
      VALUE_TYPE_F64 value = POP_F64();
      PUSH_F32(BITCAST_FROM_F32((float)BITCAST_TO_F64(value)));
      break;
    }

    case WASM_OPCODE_F32_REINTERPRET_I32: {
      VALUE_TYPE_I32 value = POP_I32();
      PUSH_F32(value);
      break;
    }

    case WASM_OPCODE_F64_CONVERT_S_I32: {
      VALUE_TYPE_I32 value = POP_I32();
      PUSH_F64(BITCAST_FROM_F64((float)BITCAST_I32_TO_SIGNED(value)));
      break;
    }

    case WASM_OPCODE_F64_CONVERT_U_I32: {
      VALUE_TYPE_I32 value = POP_I32();
      PUSH_F64(BITCAST_FROM_F64((float)value));
      break;
    }

    case WASM_OPCODE_F64_CONVERT_S_I64: {
      VALUE_TYPE_I64 value = POP_I64();
      PUSH_F64(BITCAST_FROM_F64((float)BITCAST_I64_TO_SIGNED(value)));
      break;
    }

    case WASM_OPCODE_F64_CONVERT_U_I64: {
      VALUE_TYPE_I64 value = POP_I64();
      PUSH_F64(BITCAST_FROM_F64((float)value));
      break;
    }

    case WASM_OPCODE_F64_PROMOTE_F32: {
      VALUE_TYPE_F32 value = POP_F32();
      PUSH_F64(BITCAST_FROM_F64((double)BITCAST_TO_F32(value)));
      break;
    }

    case WASM_OPCODE_F64_REINTERPRET_I64: {
      VALUE_TYPE_I64 value = POP_I64();
      PUSH_F64(value);
      break;
    }

    case WASM_OPCODE_I32_REINTERPRET_F32: {
      VALUE_TYPE_F32 value = POP_F32();
      PUSH_I32(value);
      break;
    }

    case WASM_OPCODE_I64_REINTERPRET_F64: {
      VALUE_TYPE_F64 value = POP_F64();
      PUSH_I64(value);
      break;
    }

    case WASM_OPCODE_I32_ROTR:
      BINOP_ROT(I32, RIGHT);
      break;

    case WASM_OPCODE_I32_ROTL:
      BINOP_ROT(I32, LEFT);
      break;

    case WASM_OPCODE_I64_ROTR:
      BINOP_ROT(I64, RIGHT);
      break;

    case WASM_OPCODE_I64_ROTL:
      BINOP_ROT(I64, LEFT);
      break;

    case WASM_OPCODE_I64_EQZ: {
      VALUE_TYPE_I64 value = POP_I64();
      PUSH_I64(value == 0);
      break;
    }

    case WASM_OPCODE_ALLOCA: {
      WasmInterpreterValue* old_vs_top = vs_top;
      vs_top += read_u32(&pc);
      CHECK_STACK();
      memset(old_vs_top, 0, vs_top - old_vs_top);
      break;
    }

    case WASM_OPCODE_DISCARD:
      POP();
      break;

    case WASM_OPCODE_DISCARD_KEEP: {
      uint32_t discard_count = read_u32(&pc);
      uint8_t keep_count = *pc++;
      assert(keep_count <= 1);
      if (keep_count == 1)
        STACK_VALUE(discard_count + 1) = TOP();
      vs_top -= discard_count;
      break;
    }

    default:
      assert(0);
      break;
  }
}
#endif

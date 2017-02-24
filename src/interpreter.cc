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

#include "interpreter.h"

#include <assert.h>
#include <inttypes.h>
#include <math.h>

#include "stream.h"

#define INITIAL_ISTREAM_CAPACITY (64 * 1024)

static const char* s_interpreter_opcode_name[256];

#define CHECK_RESULT(expr) \
  do {                     \
    if (WABT_FAILED(expr)) \
      return WABT_ERROR;   \
  } while (0)

/* TODO(binji): It's annoying to have to have an initializer function, but it
 * seems to be necessary as g++ doesn't allow non-trival designated
 * initializers (e.g. [314] = "blah") */
static void init_interpreter_opcode_table(void) {
  static bool s_initialized = false;
  if (!s_initialized) {
#define V(rtype, type1, type2, mem_size, code, NAME, text) \
  s_interpreter_opcode_name[code] = text;

    WABT_FOREACH_OPCODE(V)
    s_interpreter_opcode_name[WABT_OPCODE_ALLOCA] = "alloca";
    s_interpreter_opcode_name[WABT_OPCODE_BR_UNLESS] = "br_unless";
    s_interpreter_opcode_name[WABT_OPCODE_CALL_HOST] = "call_host";
    s_interpreter_opcode_name[WABT_OPCODE_DATA] = "data";
    s_interpreter_opcode_name[WABT_OPCODE_DROP_KEEP] = "drop_keep";

#undef V
  }
}

static const char* wabt_get_interpreter_opcode_name(uint8_t opcode) {
  init_interpreter_opcode_table();
  return s_interpreter_opcode_name[opcode];
}

void wabt_init_interpreter_environment(WabtInterpreterEnvironment* env) {
  WABT_ZERO_MEMORY(*env);
  wabt_init_output_buffer(&env->istream, INITIAL_ISTREAM_CAPACITY);
}

static void wabt_destroy_interpreter_func_signature(
    WabtInterpreterFuncSignature* sig) {
  wabt_destroy_type_vector(&sig->param_types);
  wabt_destroy_type_vector(&sig->result_types);
}

static void wabt_destroy_interpreter_func(WabtInterpreterFunc* func) {
  if (!func->is_host)
    wabt_destroy_type_vector(&func->defined.param_and_local_types);
}

static void wabt_destroy_interpreter_memory(WabtInterpreterMemory* memory) {
  wabt_free(memory->data);
}

static void wabt_destroy_interpreter_table(WabtInterpreterTable* table) {
  wabt_destroy_uint32_array(&table->func_indexes);
}

static void wabt_destroy_interpreter_import(WabtInterpreterImport* import) {
  wabt_destroy_string_slice(&import->module_name);
  wabt_destroy_string_slice(&import->field_name);
}

static void wabt_destroy_interpreter_module(WabtInterpreterModule* module) {
  wabt_destroy_interpreter_export_vector(&module->exports);
  wabt_destroy_binding_hash(&module->export_bindings);
  wabt_destroy_string_slice(&module->name);
  if (!module->is_host) {
    WABT_DESTROY_ARRAY_AND_ELEMENTS(module->defined.imports,
                                    interpreter_import);
  }
}

void wabt_destroy_interpreter_environment(WabtInterpreterEnvironment* env) {
  WABT_DESTROY_VECTOR_AND_ELEMENTS(env->modules, interpreter_module);
  WABT_DESTROY_VECTOR_AND_ELEMENTS(env->sigs, interpreter_func_signature);
  WABT_DESTROY_VECTOR_AND_ELEMENTS(env->funcs, interpreter_func);
  WABT_DESTROY_VECTOR_AND_ELEMENTS(env->memories, interpreter_memory);
  WABT_DESTROY_VECTOR_AND_ELEMENTS(env->tables, interpreter_table);
  wabt_destroy_interpreter_global_vector(&env->globals);
  wabt_destroy_output_buffer(&env->istream);
  wabt_destroy_binding_hash(&env->module_bindings);
  wabt_destroy_binding_hash(&env->registered_module_bindings);
}

WabtInterpreterEnvironmentMark wabt_mark_interpreter_environment(
    WabtInterpreterEnvironment* env) {
  WabtInterpreterEnvironmentMark mark;
  WABT_ZERO_MEMORY(mark);
  mark.modules_size = env->modules.size;
  mark.sigs_size = env->sigs.size;
  mark.funcs_size = env->funcs.size;
  mark.memories_size = env->memories.size;
  mark.tables_size = env->tables.size;
  mark.globals_size = env->globals.size;
  mark.istream_size = env->istream.size;
  return mark;
}

void wabt_reset_interpreter_environment_to_mark(
    WabtInterpreterEnvironment* env,
    WabtInterpreterEnvironmentMark mark) {
  size_t i;

#define DESTROY_PAST_MARK(destroy_name, names)                      \
  do {                                                              \
    assert(mark.names##_size <= env->names.size);                   \
    for (i = mark.names##_size; i < env->names.size; ++i)           \
      wabt_destroy_interpreter_##destroy_name(&env->names.data[i]); \
    env->names.size = mark.names##_size;                            \
  } while (0)

  /* Destroy entries in the binding hash. */
  for (i = mark.modules_size; i < env->modules.size; ++i) {
    const WabtStringSlice* name = &env->modules.data[i].name;
    if (!wabt_string_slice_is_empty(name))
      wabt_remove_binding(&env->module_bindings, name);
  }

  /* registered_module_bindings maps from an arbitrary name to a module index,
   * so we have to iterate through the entire table to find entries to remove.
   */
  for (i = 0; i < env->registered_module_bindings.entries.capacity; ++i) {
    WabtBindingHashEntry* entry =
        &env->registered_module_bindings.entries.data[i];
    if (!wabt_hash_entry_is_free(entry) &&
        entry->binding.index >= (int)mark.modules_size) {
      wabt_remove_binding(&env->registered_module_bindings,
                          &entry->binding.name);
    }
  }

  DESTROY_PAST_MARK(module, modules);
  DESTROY_PAST_MARK(func_signature, sigs);
  DESTROY_PAST_MARK(func, funcs);
  DESTROY_PAST_MARK(memory, memories);
  DESTROY_PAST_MARK(table, tables);
  env->globals.size = mark.globals_size;
  env->istream.size = mark.istream_size;

#undef DESTROY_PAST_MARK
}

WabtInterpreterModule* wabt_append_host_module(WabtInterpreterEnvironment* env,
                                               WabtStringSlice name) {
  WabtInterpreterModule* module = wabt_append_interpreter_module(&env->modules);
  module->name = wabt_dup_string_slice(name);
  module->memory_index = WABT_INVALID_INDEX;
  module->table_index = WABT_INVALID_INDEX;
  module->is_host = true;

  WabtStringSlice dup_name = wabt_dup_string_slice(name);
  WabtBinding* binding =
      wabt_insert_binding(&env->registered_module_bindings, &dup_name);
  binding->index = env->modules.size - 1;
  return module;
}

void wabt_init_interpreter_thread(WabtInterpreterEnvironment* env,
                                  WabtInterpreterThread* thread,
                                  WabtInterpreterThreadOptions* options) {
  WABT_ZERO_MEMORY(*thread);
  wabt_new_interpreter_value_array(&thread->value_stack,
                                   options->value_stack_size);
  wabt_new_uint32_array(&thread->call_stack, options->call_stack_size);
  thread->env = env;
  thread->value_stack_top = thread->value_stack.data;
  thread->value_stack_end = thread->value_stack.data + thread->value_stack.size;
  thread->call_stack_top = thread->call_stack.data;
  thread->call_stack_end = thread->call_stack.data + thread->call_stack.size;
  thread->pc = options->pc;
}

WabtInterpreterResult wabt_push_thread_value(WabtInterpreterThread* thread,
                                             WabtInterpreterValue value) {
  if (thread->value_stack_top >= thread->value_stack_end)
    return WABT_INTERPRETER_TRAP_VALUE_STACK_EXHAUSTED;
  *thread->value_stack_top++ = value;
  return WABT_INTERPRETER_OK;
}

WabtInterpreterExport* wabt_get_interpreter_export_by_name(
    WabtInterpreterModule* module,
    const WabtStringSlice* name) {
  int field_index =
      wabt_find_binding_index_by_name(&module->export_bindings, name);
  if (field_index < 0)
    return NULL;
  assert((size_t)field_index < module->exports.size);
  return &module->exports.data[field_index];
}

void wabt_destroy_interpreter_thread(WabtInterpreterThread* thread) {
  wabt_destroy_interpreter_value_array(&thread->value_stack);
  wabt_destroy_uint32_array(&thread->call_stack);
  wabt_destroy_interpreter_typed_value_vector(&thread->host_args);
}

/* 3 32222222 222...00
 * 1 09876543 210...10
 * -------------------
 * 0 00000000 000...00 => 0x00000000 => 0
 * 0 10011101 111...11 => 0x4effffff => 2147483520                  (~INT32_MAX)
 * 0 10011110 000...00 => 0x4f000000 => 2147483648
 * 0 10011110 111...11 => 0x4f7fffff => 4294967040                 (~UINT32_MAX)
 * 0 10111110 111...11 => 0x5effffff => 9223371487098961920         (~INT64_MAX)
 * 0 10111110 000...00 => 0x5f000000 => 9223372036854775808
 * 0 10111111 111...11 => 0x5f7fffff => 18446742974197923840       (~UINT64_MAX)
 * 0 10111111 000...00 => 0x5f800000 => 18446744073709551616
 * 0 11111111 000...00 => 0x7f800000 => inf
 * 0 11111111 000...01 => 0x7f800001 => nan(0x1)
 * 0 11111111 111...11 => 0x7fffffff => nan(0x7fffff)
 * 1 00000000 000...00 => 0x80000000 => -0
 * 1 01111110 111...11 => 0xbf7fffff => -1 + ulp      (~UINT32_MIN, ~UINT64_MIN)
 * 1 01111111 000...00 => 0xbf800000 => -1
 * 1 10011110 000...00 => 0xcf000000 => -2147483648                  (INT32_MIN)
 * 1 10111110 000...00 => 0xdf000000 => -9223372036854775808         (INT64_MIN)
 * 1 11111111 000...00 => 0xff800000 => -inf
 * 1 11111111 000...01 => 0xff800001 => -nan(0x1)
 * 1 11111111 111...11 => 0xffffffff => -nan(0x7fffff)
 */

#define F32_MAX 0x7f7fffffU
#define F32_INF 0x7f800000U
#define F32_NEG_MAX 0xff7fffffU
#define F32_NEG_INF 0xff800000U
#define F32_NEG_ONE 0xbf800000U
#define F32_NEG_ZERO 0x80000000U
#define F32_QUIET_NAN 0x7fc00000U
#define F32_QUIET_NAN_BIT 0x00400000U
#define F32_SIG_BITS 23
#define F32_SIG_MASK 0x7fffff
#define F32_SIGN_MASK 0x80000000U

bool wabt_is_nan_f32(uint32_t f32_bits) {
  return (f32_bits > F32_INF && f32_bits < F32_NEG_ZERO) ||
         (f32_bits > F32_NEG_INF);
}

static WABT_INLINE bool is_zero_f32(uint32_t f32_bits) {
  return f32_bits == 0 || f32_bits == F32_NEG_ZERO;
}

static WABT_INLINE bool is_in_range_i32_trunc_s_f32(uint32_t f32_bits) {
  return (f32_bits < 0x4f000000U) ||
         (f32_bits >= F32_NEG_ZERO && f32_bits <= 0xcf000000U);
}

static WABT_INLINE bool is_in_range_i64_trunc_s_f32(uint32_t f32_bits) {
  return (f32_bits < 0x5f000000U) ||
         (f32_bits >= F32_NEG_ZERO && f32_bits <= 0xdf000000U);
}

static WABT_INLINE bool is_in_range_i32_trunc_u_f32(uint32_t f32_bits) {
  return (f32_bits < 0x4f800000U) ||
         (f32_bits >= F32_NEG_ZERO && f32_bits < F32_NEG_ONE);
}

static WABT_INLINE bool is_in_range_i64_trunc_u_f32(uint32_t f32_bits) {
  return (f32_bits < 0x5f800000U) ||
         (f32_bits >= F32_NEG_ZERO && f32_bits < F32_NEG_ONE);
}

/*
 * 6 66655555555 5544..2..222221...000
 * 3 21098765432 1098..9..432109...210
 * -----------------------------------
 * 0 00000000000 0000..0..000000...000 0x0000000000000000 => 0
 * 0 10000011101 1111..1..111000...000 0x41dfffffffc00000 => 2147483647               (INT32_MAX)
 * 0 10000011110 1111..1..111100...000 0x41efffffffe00000 => 4294967295              (UINT32_MAX)
 * 0 10000111101 1111..1..111111...111 0x43dfffffffffffff => 9223372036854774784     (~INT64_MAX)
 * 0 10000111110 0000..0..000000...000 0x43e0000000000000 => 9223372036854775808
 * 0 10000111110 1111..1..111111...111 0x43efffffffffffff => 18446744073709549568   (~UINT64_MAX)
 * 0 10000111111 0000..0..000000...000 0x43f0000000000000 => 18446744073709551616
 * 0 10001111110 1111..1..000000...000 0x47efffffe0000000 => 3.402823e+38               (FLT_MAX)
 * 0 11111111111 0000..0..000000...000 0x7ff0000000000000 => inf
 * 0 11111111111 0000..0..000000...001 0x7ff0000000000001 => nan(0x1)
 * 0 11111111111 1111..1..111111...111 0x7fffffffffffffff => nan(0xfff...)
 * 1 00000000000 0000..0..000000...000 0x8000000000000000 => -0
 * 1 01111111110 1111..1..111111...111 0xbfefffffffffffff => -1 + ulp  (~UINT32_MIN, ~UINT64_MIN)
 * 1 01111111111 0000..0..000000...000 0xbff0000000000000 => -1
 * 1 10000011110 0000..0..000000...000 0xc1e0000000000000 => -2147483648              (INT32_MIN)
 * 1 10000111110 0000..0..000000...000 0xc3e0000000000000 => -9223372036854775808     (INT64_MIN)
 * 1 10001111110 1111..1..000000...000 0xc7efffffe0000000 => -3.402823e+38             (-FLT_MAX)
 * 1 11111111111 0000..0..000000...000 0xfff0000000000000 => -inf
 * 1 11111111111 0000..0..000000...001 0xfff0000000000001 => -nan(0x1)
 * 1 11111111111 1111..1..111111...111 0xffffffffffffffff => -nan(0xfff...)
 */

#define F64_INF 0x7ff0000000000000ULL
#define F64_NEG_INF 0xfff0000000000000ULL
#define F64_NEG_ONE 0xbff0000000000000ULL
#define F64_NEG_ZERO 0x8000000000000000ULL
#define F64_QUIET_NAN 0x7ff8000000000000ULL
#define F64_QUIET_NAN_BIT 0x0008000000000000ULL
#define F64_SIG_BITS 52
#define F64_SIG_MASK 0xfffffffffffffULL
#define F64_SIGN_MASK 0x8000000000000000ULL

bool wabt_is_nan_f64(uint64_t f64_bits) {
  return (f64_bits > F64_INF && f64_bits < F64_NEG_ZERO) ||
         (f64_bits > F64_NEG_INF);
}

static WABT_INLINE bool is_zero_f64(uint64_t f64_bits) {
  return f64_bits == 0 || f64_bits == F64_NEG_ZERO;
}

static WABT_INLINE bool is_in_range_i32_trunc_s_f64(uint64_t f64_bits) {
  return (f64_bits <= 0x41dfffffffc00000ULL) ||
         (f64_bits >= F64_NEG_ZERO && f64_bits <= 0xc1e0000000000000ULL);
}

static WABT_INLINE bool is_in_range_i32_trunc_u_f64(uint64_t f64_bits) {
  return (f64_bits <= 0x41efffffffe00000ULL) ||
         (f64_bits >= F64_NEG_ZERO && f64_bits < F64_NEG_ONE);
}

static WABT_INLINE bool is_in_range_i64_trunc_s_f64(uint64_t f64_bits) {
  return (f64_bits < 0x43e0000000000000ULL) ||
         (f64_bits >= F64_NEG_ZERO && f64_bits <= 0xc3e0000000000000ULL);
}

static WABT_INLINE bool is_in_range_i64_trunc_u_f64(uint64_t f64_bits) {
  return (f64_bits < 0x43f0000000000000ULL) ||
         (f64_bits >= F64_NEG_ZERO && f64_bits < F64_NEG_ONE);
}

static WABT_INLINE bool is_in_range_f64_demote_f32(uint64_t f64_bits) {
  return (f64_bits <= 0x47efffffe0000000ULL) ||
         (f64_bits >= F64_NEG_ZERO && f64_bits <= 0xc7efffffe0000000ULL);
}

/* The WebAssembly rounding mode means that these values (which are > F32_MAX)
 * should be rounded to F32_MAX and not set to infinity. Unfortunately, UBSAN
 * complains that the value is not representable as a float, so we'll special
 * case them. */
static WABT_INLINE bool is_in_range_f64_demote_f32_round_to_f32_max(
    uint64_t f64_bits) {
  return f64_bits > 0x47efffffe0000000ULL && f64_bits < 0x47effffff0000000ULL;
}

static WABT_INLINE bool is_in_range_f64_demote_f32_round_to_neg_f32_max(
    uint64_t f64_bits) {
  return f64_bits > 0xc7efffffe0000000ULL && f64_bits < 0xc7effffff0000000ULL;
}

#define IS_NAN_F32 wabt_is_nan_f32
#define IS_NAN_F64 wabt_is_nan_f64
#define IS_ZERO_F32 is_zero_f32
#define IS_ZERO_F64 is_zero_f64

#define DEFINE_BITCAST(name, src, dst) \
  static WABT_INLINE dst name(src x) { \
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

#define VALUE_TYPE_SIGNED_MAX_I32 (uint32_t)(0x80000000U)
#define VALUE_TYPE_UNSIGNED_MAX_I32 (uint32_t)(0xFFFFFFFFU)
#define VALUE_TYPE_SIGNED_MAX_I64 (uint64_t)(0x8000000000000000ULL)
#define VALUE_TYPE_UNSIGNED_MAX_I64 (uint64_t)(0xFFFFFFFFFFFFFFFFULL)

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

#define TRAP(type) return WABT_INTERPRETER_TRAP_##type
#define TRAP_UNLESS(cond, type) TRAP_IF(!(cond), type)
#define TRAP_IF(cond, type)                \
  do {                                     \
    if (WABT_UNLIKELY(cond))               \
      return WABT_INTERPRETER_TRAP_##type; \
  } while (0)

#define CHECK_STACK()                                         \
  TRAP_IF(thread->value_stack_top >= thread->value_stack_end, \
          VALUE_STACK_EXHAUSTED)

#define PUSH_NEG_1_AND_BREAK_IF(cond) \
  if (WABT_UNLIKELY(cond)) {          \
    PUSH_I32(-1);                     \
    break;                            \
  }

#define PUSH(v)        \
  do {                 \
    CHECK_STACK();     \
    (*thread->value_stack_top++) = (v); \
  } while (0)

#define PUSH_TYPE(type, v)                                \
  do {                                                    \
    CHECK_STACK();                                        \
    (*thread->value_stack_top++).TYPE_FIELD_NAME_##type = \
        (VALUE_TYPE_##type)(v);                           \
  } while (0)

#define PUSH_I32(v) PUSH_TYPE(I32, (v))
#define PUSH_I64(v) PUSH_TYPE(I64, (v))
#define PUSH_F32(v) PUSH_TYPE(F32, (v))
#define PUSH_F64(v) PUSH_TYPE(F64, (v))

#define PICK(depth) (*(thread->value_stack_top - (depth)))
#define TOP() (PICK(1))
#define POP() (*--thread->value_stack_top)
#define POP_I32() (POP().i32)
#define POP_I64() (POP().i64)
#define POP_F32() (POP().f32_bits)
#define POP_F64() (POP().f64_bits)
#define DROP_KEEP(drop, keep)          \
  do {                                 \
    assert((keep) <= 1);               \
    if ((keep) == 1)                   \
      PICK((drop) + 1) = TOP();        \
    thread->value_stack_top -= (drop); \
  } while (0)

#define GOTO(offset) pc = &istream[offset]

#define PUSH_CALL()                                           \
  do {                                                        \
    TRAP_IF(thread->call_stack_top >= thread->call_stack_end, \
            CALL_STACK_EXHAUSTED);                            \
    (*thread->call_stack_top++) = (pc - istream);             \
  } while (0)

#define POP_CALL() (*--thread->call_stack_top)

#define GET_MEMORY(var)                      \
  uint32_t memory_index = read_u32(&pc);     \
  assert(memory_index < env->memories.size); \
  WabtInterpreterMemory* var = &env->memories.data[memory_index]

#define LOAD(type, mem_type)                                        \
  do {                                                              \
    GET_MEMORY(memory);                                             \
    uint64_t offset = (uint64_t)POP_I32() + read_u32(&pc);          \
    MEM_TYPE_##mem_type value;                                      \
    TRAP_IF(offset + sizeof(value) > memory->byte_size,             \
            MEMORY_ACCESS_OUT_OF_BOUNDS);                           \
    void* src = (void*)((intptr_t)memory->data + (uint32_t)offset); \
    memcpy(&value, src, sizeof(MEM_TYPE_##mem_type));               \
    PUSH_##type((MEM_TYPE_EXTEND_##type##_##mem_type)value);        \
  } while (0)

#define STORE(type, mem_type)                                       \
  do {                                                              \
    GET_MEMORY(memory);                                             \
    VALUE_TYPE_##type value = POP_##type();                         \
    uint64_t offset = (uint64_t)POP_I32() + read_u32(&pc);          \
    MEM_TYPE_##mem_type src = (MEM_TYPE_##mem_type)value;           \
    TRAP_IF(offset + sizeof(src) > memory->byte_size,               \
            MEMORY_ACCESS_OUT_OF_BOUNDS);                           \
    void* dst = (void*)((intptr_t)memory->data + (uint32_t)offset); \
    memcpy(dst, &src, sizeof(MEM_TYPE_##mem_type));                 \
  } while (0)

#define BINOP(rtype, type, op)            \
  do {                                    \
    VALUE_TYPE_##type rhs = POP_##type(); \
    VALUE_TYPE_##type lhs = POP_##type(); \
    PUSH_##rtype(lhs op rhs);             \
  } while (0)

#define BINOP_SIGNED(rtype, type, op)                     \
  do {                                                    \
    VALUE_TYPE_##type rhs = POP_##type();                 \
    VALUE_TYPE_##type lhs = POP_##type();                 \
    PUSH_##rtype(BITCAST_##type##_TO_SIGNED(lhs)          \
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
    if (WABT_LIKELY(amount != 0)) {                                        \
      PUSH_##type(                                                         \
          (lhs ROT_##dir##_0_SHIFT_OP amount) |                            \
          (lhs ROT_##dir##_1_SHIFT_OP((SHIFT_MASK_##type + 1) - amount))); \
    } else {                                                               \
      PUSH_##type(lhs);                                                    \
    }                                                                      \
  } while (0)

#define BINOP_DIV_REM_U(type, op)                          \
  do {                                                     \
    VALUE_TYPE_##type rhs = POP_##type();                  \
    VALUE_TYPE_##type lhs = POP_##type();                  \
    TRAP_IF(rhs == 0, INTEGER_DIVIDE_BY_ZERO);             \
    PUSH_##type(BITCAST_##type##_TO_UNSIGNED(lhs)          \
                    op BITCAST_##type##_TO_UNSIGNED(rhs)); \
  } while (0)

/* {i32,i64}.{div,rem}_s are special-cased because they trap when dividing the
 * max signed value by -1. The modulo operation on x86 uses the same
 * instruction to generate the quotient and the remainder. */
#define BINOP_DIV_S(type)                              \
  do {                                                 \
    VALUE_TYPE_##type rhs = POP_##type();              \
    VALUE_TYPE_##type lhs = POP_##type();              \
    TRAP_IF(rhs == 0, INTEGER_DIVIDE_BY_ZERO);         \
    TRAP_IF(lhs == VALUE_TYPE_SIGNED_MAX_##type &&     \
                rhs == VALUE_TYPE_UNSIGNED_MAX_##type, \
            INTEGER_OVERFLOW);                         \
    PUSH_##type(BITCAST_##type##_TO_SIGNED(lhs) /      \
                BITCAST_##type##_TO_SIGNED(rhs));      \
  } while (0)

#define BINOP_REM_S(type)                                       \
  do {                                                          \
    VALUE_TYPE_##type rhs = POP_##type();                       \
    VALUE_TYPE_##type lhs = POP_##type();                       \
    TRAP_IF(rhs == 0, INTEGER_DIVIDE_BY_ZERO);                  \
    if (WABT_UNLIKELY(lhs == VALUE_TYPE_SIGNED_MAX_##type &&    \
                      rhs == VALUE_TYPE_UNSIGNED_MAX_##type)) { \
      PUSH_##type(0);                                           \
    } else {                                                    \
      PUSH_##type(BITCAST_##type##_TO_SIGNED(lhs) %             \
                  BITCAST_##type##_TO_SIGNED(rhs));             \
    }                                                           \
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

#define BINOP_FLOAT_DIV(type)                                    \
  do {                                                           \
    VALUE_TYPE_##type rhs = POP_##type();                        \
    VALUE_TYPE_##type lhs = POP_##type();                        \
    if (WABT_UNLIKELY(IS_ZERO_##type(rhs))) {                    \
      if (IS_NAN_##type(lhs)) {                                  \
        PUSH_##type(lhs | type##_QUIET_NAN);                     \
      } else if (IS_ZERO_##type(lhs)) {                          \
        PUSH_##type(type##_QUIET_NAN);                           \
      } else {                                                   \
        VALUE_TYPE_##type sign =                                 \
            (lhs & type##_SIGN_MASK) ^ (rhs & type##_SIGN_MASK); \
        PUSH_##type(sign | type##_INF);                          \
      }                                                          \
    } else {                                                     \
      PUSH_##type(BITCAST_FROM_##type(BITCAST_TO_##type(lhs) /   \
                                      BITCAST_TO_##type(rhs)));  \
    }                                                            \
  } while (0)

#define BINOP_FLOAT_COMPARE(type, op)                        \
  do {                                                       \
    FLOAT_TYPE_##type rhs = BITCAST_TO_##type(POP_##type()); \
    FLOAT_TYPE_##type lhs = BITCAST_TO_##type(POP_##type()); \
    PUSH_I32(lhs op rhs);                                    \
  } while (0)

#define MIN_OP <
#define MAX_OP >

#define MINMAX_FLOAT(type, op)                                                 \
  do {                                                                         \
    VALUE_TYPE_##type rhs = POP_##type();                                      \
    VALUE_TYPE_##type lhs = POP_##type();                                      \
    VALUE_TYPE_##type result;                                                  \
    if (WABT_UNLIKELY(IS_NAN_##type(lhs))) {                                   \
      result = lhs | type##_QUIET_NAN_BIT;                                     \
    } else if (WABT_UNLIKELY(IS_NAN_##type(rhs))) {                            \
      result = rhs | type##_QUIET_NAN_BIT;                                     \
    } else if ((lhs ^ rhs) & type##_SIGN_MASK) {                               \
      /* min(-0.0, 0.0) => -0.0; since we know the sign bits are different, we \
       * can just use the inverse integer comparison (because the sign bit is  \
       * set when the value is negative) */                                    \
      result = !(lhs op##_OP rhs) ? lhs : rhs;                                 \
    } else {                                                                   \
      FLOAT_TYPE_##type float_rhs = BITCAST_TO_##type(rhs);                    \
      FLOAT_TYPE_##type float_lhs = BITCAST_TO_##type(lhs);                    \
      result = BITCAST_FROM_##type(float_lhs op##_OP float_rhs ? float_lhs     \
                                                               : float_rhs);   \
    }                                                                          \
    PUSH_##type(result);                                                       \
  } while (0)

static WABT_INLINE uint32_t read_u32_at(const uint8_t* pc) {
  uint32_t result;
  memcpy(&result, pc, sizeof(uint32_t));
  return result;
}

static WABT_INLINE uint32_t read_u32(const uint8_t** pc) {
  uint32_t result = read_u32_at(*pc);
  *pc += sizeof(uint32_t);
  return result;
}

static WABT_INLINE uint64_t read_u64_at(const uint8_t* pc) {
  uint64_t result;
  memcpy(&result, pc, sizeof(uint64_t));
  return result;
}

static WABT_INLINE uint64_t read_u64(const uint8_t** pc) {
  uint64_t result = read_u64_at(*pc);
  *pc += sizeof(uint64_t);
  return result;
}

static WABT_INLINE void read_table_entry_at(const uint8_t* pc,
                                            uint32_t* out_offset,
                                            uint32_t* out_drop,
                                            uint8_t* out_keep) {
  *out_offset = read_u32_at(pc + WABT_TABLE_ENTRY_OFFSET_OFFSET);
  *out_drop = read_u32_at(pc + WABT_TABLE_ENTRY_DROP_OFFSET);
  *out_keep = *(pc + WABT_TABLE_ENTRY_KEEP_OFFSET);
}

bool wabt_func_signatures_are_equal(WabtInterpreterEnvironment* env,
                                    uint32_t sig_index_0,
                                    uint32_t sig_index_1) {
  if (sig_index_0 == sig_index_1)
    return true;
  WabtInterpreterFuncSignature* sig_0 = &env->sigs.data[sig_index_0];
  WabtInterpreterFuncSignature* sig_1 = &env->sigs.data[sig_index_1];
  return wabt_type_vectors_are_equal(&sig_0->param_types,
                                     &sig_1->param_types) &&
         wabt_type_vectors_are_equal(&sig_0->result_types,
                                     &sig_1->result_types);
}

WabtInterpreterResult wabt_call_host(WabtInterpreterThread* thread,
                                     WabtInterpreterFunc* func) {
  assert(func->is_host);
  assert(func->sig_index < thread->env->sigs.size);
  WabtInterpreterFuncSignature* sig = &thread->env->sigs.data[func->sig_index];

  uint32_t num_args = sig->param_types.size;
  if (thread->host_args.size < num_args) {
    wabt_resize_interpreter_typed_value_vector(&thread->host_args, num_args);
  }

  uint32_t i;
  for (i = num_args; i > 0; --i) {
    WabtInterpreterValue value = POP();
    WabtInterpreterTypedValue* arg = &thread->host_args.data[i - 1];
    arg->type = sig->param_types.data[i - 1];
    arg->value = value;
  }

  uint32_t num_results = sig->result_types.size;
  WabtInterpreterTypedValue* call_result_values =
      (WabtInterpreterTypedValue*)alloca(sizeof(WabtInterpreterTypedValue) *
                                         num_results);

  WabtResult call_result = func->host.callback(
      func, sig, num_args, thread->host_args.data, num_results,
      call_result_values, func->host.user_data);
  TRAP_IF(call_result != WABT_OK, HOST_TRAPPED);

  for (i = 0; i < num_results; ++i) {
    TRAP_IF(call_result_values[i].type != sig->result_types.data[i],
            HOST_RESULT_TYPE_MISMATCH);
    PUSH(call_result_values[i].value);
  }

  return WABT_INTERPRETER_OK;
}

WabtInterpreterResult wabt_run_interpreter(WabtInterpreterThread* thread,
                                           uint32_t num_instructions,
                                           uint32_t* call_stack_return_top) {
  WabtInterpreterResult result = WABT_INTERPRETER_OK;
  assert(call_stack_return_top < thread->call_stack_end);

  WabtInterpreterEnvironment* env = thread->env;

  const uint8_t* istream = (const uint8_t*)env->istream.start;
  const uint8_t* pc = &istream[thread->pc];
  uint32_t i;
  for (i = 0; i < num_instructions; ++i) {
    uint8_t opcode = *pc++;
    switch (opcode) {
      case WABT_OPCODE_SELECT: {
        VALUE_TYPE_I32 cond = POP_I32();
        WabtInterpreterValue false_ = POP();
        WabtInterpreterValue true_ = POP();
        PUSH(cond ? true_ : false_);
        break;
      }

      case WABT_OPCODE_BR:
        GOTO(read_u32(&pc));
        break;

      case WABT_OPCODE_BR_IF: {
        uint32_t new_pc = read_u32(&pc);
        if (POP_I32())
          GOTO(new_pc);
        break;
      }

      case WABT_OPCODE_BR_TABLE: {
        uint32_t num_targets = read_u32(&pc);
        uint32_t table_offset = read_u32(&pc);
        VALUE_TYPE_I32 key = POP_I32();
        uint32_t key_offset =
            (key >= num_targets ? num_targets : key) * WABT_TABLE_ENTRY_SIZE;
        const uint8_t* entry = istream + table_offset + key_offset;
        uint32_t new_pc;
        uint32_t drop_count;
        uint8_t keep_count;
        read_table_entry_at(entry, &new_pc, &drop_count, &keep_count);
        DROP_KEEP(drop_count, keep_count);
        GOTO(new_pc);
        break;
      }

      case WABT_OPCODE_RETURN:
        if (thread->call_stack_top == call_stack_return_top) {
          result = WABT_INTERPRETER_RETURNED;
          goto exit_loop;
        }
        GOTO(POP_CALL());
        break;

      case WABT_OPCODE_UNREACHABLE:
        TRAP(UNREACHABLE);
        break;

      case WABT_OPCODE_I32_CONST:
        PUSH_I32(read_u32(&pc));
        break;

      case WABT_OPCODE_I64_CONST:
        PUSH_I64(read_u64(&pc));
        break;

      case WABT_OPCODE_F32_CONST:
        PUSH_F32(read_u32(&pc));
        break;

      case WABT_OPCODE_F64_CONST:
        PUSH_F64(read_u64(&pc));
        break;

      case WABT_OPCODE_GET_GLOBAL: {
        uint32_t index = read_u32(&pc);
        assert(index < env->globals.size);
        PUSH(env->globals.data[index].typed_value.value);
        break;
      }

      case WABT_OPCODE_SET_GLOBAL: {
        uint32_t index = read_u32(&pc);
        assert(index < env->globals.size);
        env->globals.data[index].typed_value.value = POP();
        break;
      }

      case WABT_OPCODE_GET_LOCAL: {
        WabtInterpreterValue value = PICK(read_u32(&pc));
        PUSH(value);
        break;
      }

      case WABT_OPCODE_SET_LOCAL: {
        WabtInterpreterValue value = POP();
        PICK(read_u32(&pc)) = value;
        break;
      }

      case WABT_OPCODE_TEE_LOCAL:
        PICK(read_u32(&pc)) = TOP();
        break;

      case WABT_OPCODE_CALL: {
        uint32_t offset = read_u32(&pc);
        PUSH_CALL();
        GOTO(offset);
        break;
      }

      case WABT_OPCODE_CALL_INDIRECT: {
        uint32_t table_index = read_u32(&pc);
        assert(table_index < env->tables.size);
        WabtInterpreterTable* table = &env->tables.data[table_index];
        uint32_t sig_index = read_u32(&pc);
        assert(sig_index < env->sigs.size);
        VALUE_TYPE_I32 entry_index = POP_I32();
        TRAP_IF(entry_index >= table->func_indexes.size, UNDEFINED_TABLE_INDEX);
        uint32_t func_index = table->func_indexes.data[entry_index];
        TRAP_IF(func_index == WABT_INVALID_INDEX, UNINITIALIZED_TABLE_ELEMENT);
        WabtInterpreterFunc* func = &env->funcs.data[func_index];
        TRAP_UNLESS(
            wabt_func_signatures_are_equal(env, func->sig_index, sig_index),
            INDIRECT_CALL_SIGNATURE_MISMATCH);
        if (func->is_host) {
          wabt_call_host(thread, func);
        } else {
          PUSH_CALL();
          GOTO(func->defined.offset);
        }
        break;
      }

      case WABT_OPCODE_CALL_HOST: {
        uint32_t func_index = read_u32(&pc);
        assert(func_index < env->funcs.size);
        WabtInterpreterFunc* func = &env->funcs.data[func_index];
        wabt_call_host(thread, func);
        break;
      }

      case WABT_OPCODE_I32_LOAD8_S:
        LOAD(I32, I8);
        break;

      case WABT_OPCODE_I32_LOAD8_U:
        LOAD(I32, U8);
        break;

      case WABT_OPCODE_I32_LOAD16_S:
        LOAD(I32, I16);
        break;

      case WABT_OPCODE_I32_LOAD16_U:
        LOAD(I32, U16);
        break;

      case WABT_OPCODE_I64_LOAD8_S:
        LOAD(I64, I8);
        break;

      case WABT_OPCODE_I64_LOAD8_U:
        LOAD(I64, U8);
        break;

      case WABT_OPCODE_I64_LOAD16_S:
        LOAD(I64, I16);
        break;

      case WABT_OPCODE_I64_LOAD16_U:
        LOAD(I64, U16);
        break;

      case WABT_OPCODE_I64_LOAD32_S:
        LOAD(I64, I32);
        break;

      case WABT_OPCODE_I64_LOAD32_U:
        LOAD(I64, U32);
        break;

      case WABT_OPCODE_I32_LOAD:
        LOAD(I32, U32);
        break;

      case WABT_OPCODE_I64_LOAD:
        LOAD(I64, U64);
        break;

      case WABT_OPCODE_F32_LOAD:
        LOAD(F32, F32);
        break;

      case WABT_OPCODE_F64_LOAD:
        LOAD(F64, F64);
        break;

      case WABT_OPCODE_I32_STORE8:
        STORE(I32, U8);
        break;

      case WABT_OPCODE_I32_STORE16:
        STORE(I32, U16);
        break;

      case WABT_OPCODE_I64_STORE8:
        STORE(I64, U8);
        break;

      case WABT_OPCODE_I64_STORE16:
        STORE(I64, U16);
        break;

      case WABT_OPCODE_I64_STORE32:
        STORE(I64, U32);
        break;

      case WABT_OPCODE_I32_STORE:
        STORE(I32, U32);
        break;

      case WABT_OPCODE_I64_STORE:
        STORE(I64, U64);
        break;

      case WABT_OPCODE_F32_STORE:
        STORE(F32, F32);
        break;

      case WABT_OPCODE_F64_STORE:
        STORE(F64, F64);
        break;

      case WABT_OPCODE_CURRENT_MEMORY: {
        GET_MEMORY(memory);
        PUSH_I32(memory->page_limits.initial);
        break;
      }

      case WABT_OPCODE_GROW_MEMORY: {
        GET_MEMORY(memory);
        uint32_t old_page_size = memory->page_limits.initial;
        uint32_t old_byte_size = memory->byte_size;
        VALUE_TYPE_I32 grow_pages = POP_I32();
        uint32_t new_page_size = old_page_size + grow_pages;
        uint32_t max_page_size = memory->page_limits.has_max
                                     ? memory->page_limits.max
                                     : WABT_MAX_PAGES;
        PUSH_NEG_1_AND_BREAK_IF(new_page_size > max_page_size);
        PUSH_NEG_1_AND_BREAK_IF((uint64_t)new_page_size * WABT_PAGE_SIZE >
                                UINT32_MAX);
        uint32_t new_byte_size = new_page_size * WABT_PAGE_SIZE;
        void* new_data = wabt_realloc(memory->data, new_byte_size);
        PUSH_NEG_1_AND_BREAK_IF(new_data == NULL);
        memset((void*)((intptr_t)new_data + old_byte_size), 0,
               new_byte_size - old_byte_size);
        memory->data = new_data;
        memory->page_limits.initial = new_page_size;
        memory->byte_size = new_byte_size;
        PUSH_I32(old_page_size);
        break;
      }

      case WABT_OPCODE_I32_ADD:
        BINOP(I32, I32, +);
        break;

      case WABT_OPCODE_I32_SUB:
        BINOP(I32, I32, -);
        break;

      case WABT_OPCODE_I32_MUL:
        BINOP(I32, I32, *);
        break;

      case WABT_OPCODE_I32_DIV_S:
        BINOP_DIV_S(I32);
        break;

      case WABT_OPCODE_I32_DIV_U:
        BINOP_DIV_REM_U(I32, / );
        break;

      case WABT_OPCODE_I32_REM_S:
        BINOP_REM_S(I32);
        break;

      case WABT_OPCODE_I32_REM_U:
        BINOP_DIV_REM_U(I32, % );
        break;

      case WABT_OPCODE_I32_AND:
        BINOP(I32, I32, &);
        break;

      case WABT_OPCODE_I32_OR:
        BINOP(I32, I32, | );
        break;

      case WABT_OPCODE_I32_XOR:
        BINOP(I32, I32, ^);
        break;

      case WABT_OPCODE_I32_SHL:
        BINOP_SHIFT(I32, <<, UNSIGNED);
        break;

      case WABT_OPCODE_I32_SHR_U:
        BINOP_SHIFT(I32, >>, UNSIGNED);
        break;

      case WABT_OPCODE_I32_SHR_S:
        BINOP_SHIFT(I32, >>, SIGNED);
        break;

      case WABT_OPCODE_I32_EQ:
        BINOP(I32, I32, == );
        break;

      case WABT_OPCODE_I32_NE:
        BINOP(I32, I32, != );
        break;

      case WABT_OPCODE_I32_LT_S:
        BINOP_SIGNED(I32, I32, < );
        break;

      case WABT_OPCODE_I32_LE_S:
        BINOP_SIGNED(I32, I32, <= );
        break;

      case WABT_OPCODE_I32_LT_U:
        BINOP(I32, I32, < );
        break;

      case WABT_OPCODE_I32_LE_U:
        BINOP(I32, I32, <= );
        break;

      case WABT_OPCODE_I32_GT_S:
        BINOP_SIGNED(I32, I32, > );
        break;

      case WABT_OPCODE_I32_GE_S:
        BINOP_SIGNED(I32, I32, >= );
        break;

      case WABT_OPCODE_I32_GT_U:
        BINOP(I32, I32, > );
        break;

      case WABT_OPCODE_I32_GE_U:
        BINOP(I32, I32, >= );
        break;

      case WABT_OPCODE_I32_CLZ: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I32(value != 0 ? wabt_clz_u32(value) : 32);
        break;
      }

      case WABT_OPCODE_I32_CTZ: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I32(value != 0 ? wabt_ctz_u32(value) : 32);
        break;
      }

      case WABT_OPCODE_I32_POPCNT: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I32(wabt_popcount_u32(value));
        break;
      }

      case WABT_OPCODE_I32_EQZ: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I32(value == 0);
        break;
      }

      case WABT_OPCODE_I64_ADD:
        BINOP(I64, I64, +);
        break;

      case WABT_OPCODE_I64_SUB:
        BINOP(I64, I64, -);
        break;

      case WABT_OPCODE_I64_MUL:
        BINOP(I64, I64, *);
        break;

      case WABT_OPCODE_I64_DIV_S:
        BINOP_DIV_S(I64);
        break;

      case WABT_OPCODE_I64_DIV_U:
        BINOP_DIV_REM_U(I64, / );
        break;

      case WABT_OPCODE_I64_REM_S:
        BINOP_REM_S(I64);
        break;

      case WABT_OPCODE_I64_REM_U:
        BINOP_DIV_REM_U(I64, % );
        break;

      case WABT_OPCODE_I64_AND:
        BINOP(I64, I64, &);
        break;

      case WABT_OPCODE_I64_OR:
        BINOP(I64, I64, | );
        break;

      case WABT_OPCODE_I64_XOR:
        BINOP(I64, I64, ^);
        break;

      case WABT_OPCODE_I64_SHL:
        BINOP_SHIFT(I64, <<, UNSIGNED);
        break;

      case WABT_OPCODE_I64_SHR_U:
        BINOP_SHIFT(I64, >>, UNSIGNED);
        break;

      case WABT_OPCODE_I64_SHR_S:
        BINOP_SHIFT(I64, >>, SIGNED);
        break;

      case WABT_OPCODE_I64_EQ:
        BINOP(I32, I64, == );
        break;

      case WABT_OPCODE_I64_NE:
        BINOP(I32, I64, != );
        break;

      case WABT_OPCODE_I64_LT_S:
        BINOP_SIGNED(I32, I64, < );
        break;

      case WABT_OPCODE_I64_LE_S:
        BINOP_SIGNED(I32, I64, <= );
        break;

      case WABT_OPCODE_I64_LT_U:
        BINOP(I32, I64, < );
        break;

      case WABT_OPCODE_I64_LE_U:
        BINOP(I32, I64, <= );
        break;

      case WABT_OPCODE_I64_GT_S:
        BINOP_SIGNED(I32, I64, > );
        break;

      case WABT_OPCODE_I64_GE_S:
        BINOP_SIGNED(I32, I64, >= );
        break;

      case WABT_OPCODE_I64_GT_U:
        BINOP(I32, I64, > );
        break;

      case WABT_OPCODE_I64_GE_U:
        BINOP(I32, I64, >= );
        break;

      case WABT_OPCODE_I64_CLZ: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I64(value != 0 ? wabt_clz_u64(value) : 64);
        break;
      }

      case WABT_OPCODE_I64_CTZ: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I64(value != 0 ? wabt_ctz_u64(value) : 64);
        break;
      }

      case WABT_OPCODE_I64_POPCNT: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I64(wabt_popcount_u64(value));
        break;
      }

      case WABT_OPCODE_F32_ADD:
        BINOP_FLOAT(F32, +);
        break;

      case WABT_OPCODE_F32_SUB:
        BINOP_FLOAT(F32, -);
        break;

      case WABT_OPCODE_F32_MUL:
        BINOP_FLOAT(F32, *);
        break;

      case WABT_OPCODE_F32_DIV:
        BINOP_FLOAT_DIV(F32);
        break;

      case WABT_OPCODE_F32_MIN:
        MINMAX_FLOAT(F32, MIN);
        break;

      case WABT_OPCODE_F32_MAX:
        MINMAX_FLOAT(F32, MAX);
        break;

      case WABT_OPCODE_F32_ABS:
        TOP().f32_bits &= ~F32_SIGN_MASK;
        break;

      case WABT_OPCODE_F32_NEG:
        TOP().f32_bits ^= F32_SIGN_MASK;
        break;

      case WABT_OPCODE_F32_COPYSIGN: {
        VALUE_TYPE_F32 rhs = POP_F32();
        VALUE_TYPE_F32 lhs = POP_F32();
        PUSH_F32((lhs & ~F32_SIGN_MASK) | (rhs & F32_SIGN_MASK));
        break;
      }

      case WABT_OPCODE_F32_CEIL:
        UNOP_FLOAT(F32, ceilf);
        break;

      case WABT_OPCODE_F32_FLOOR:
        UNOP_FLOAT(F32, floorf);
        break;

      case WABT_OPCODE_F32_TRUNC:
        UNOP_FLOAT(F32, truncf);
        break;

      case WABT_OPCODE_F32_NEAREST:
        UNOP_FLOAT(F32, nearbyintf);
        break;

      case WABT_OPCODE_F32_SQRT:
        UNOP_FLOAT(F32, sqrtf);
        break;

      case WABT_OPCODE_F32_EQ:
        BINOP_FLOAT_COMPARE(F32, == );
        break;

      case WABT_OPCODE_F32_NE:
        BINOP_FLOAT_COMPARE(F32, != );
        break;

      case WABT_OPCODE_F32_LT:
        BINOP_FLOAT_COMPARE(F32, < );
        break;

      case WABT_OPCODE_F32_LE:
        BINOP_FLOAT_COMPARE(F32, <= );
        break;

      case WABT_OPCODE_F32_GT:
        BINOP_FLOAT_COMPARE(F32, > );
        break;

      case WABT_OPCODE_F32_GE:
        BINOP_FLOAT_COMPARE(F32, >= );
        break;

      case WABT_OPCODE_F64_ADD:
        BINOP_FLOAT(F64, +);
        break;

      case WABT_OPCODE_F64_SUB:
        BINOP_FLOAT(F64, -);
        break;

      case WABT_OPCODE_F64_MUL:
        BINOP_FLOAT(F64, *);
        break;

      case WABT_OPCODE_F64_DIV:
        BINOP_FLOAT_DIV(F64);
        break;

      case WABT_OPCODE_F64_MIN:
        MINMAX_FLOAT(F64, MIN);
        break;

      case WABT_OPCODE_F64_MAX:
        MINMAX_FLOAT(F64, MAX);
        break;

      case WABT_OPCODE_F64_ABS:
        TOP().f64_bits &= ~F64_SIGN_MASK;
        break;

      case WABT_OPCODE_F64_NEG:
        TOP().f64_bits ^= F64_SIGN_MASK;
        break;

      case WABT_OPCODE_F64_COPYSIGN: {
        VALUE_TYPE_F64 rhs = POP_F64();
        VALUE_TYPE_F64 lhs = POP_F64();
        PUSH_F64((lhs & ~F64_SIGN_MASK) | (rhs & F64_SIGN_MASK));
        break;
      }

      case WABT_OPCODE_F64_CEIL:
        UNOP_FLOAT(F64, ceil);
        break;

      case WABT_OPCODE_F64_FLOOR:
        UNOP_FLOAT(F64, floor);
        break;

      case WABT_OPCODE_F64_TRUNC:
        UNOP_FLOAT(F64, trunc);
        break;

      case WABT_OPCODE_F64_NEAREST:
        UNOP_FLOAT(F64, nearbyint);
        break;

      case WABT_OPCODE_F64_SQRT:
        UNOP_FLOAT(F64, sqrt);
        break;

      case WABT_OPCODE_F64_EQ:
        BINOP_FLOAT_COMPARE(F64, == );
        break;

      case WABT_OPCODE_F64_NE:
        BINOP_FLOAT_COMPARE(F64, != );
        break;

      case WABT_OPCODE_F64_LT:
        BINOP_FLOAT_COMPARE(F64, < );
        break;

      case WABT_OPCODE_F64_LE:
        BINOP_FLOAT_COMPARE(F64, <= );
        break;

      case WABT_OPCODE_F64_GT:
        BINOP_FLOAT_COMPARE(F64, > );
        break;

      case WABT_OPCODE_F64_GE:
        BINOP_FLOAT_COMPARE(F64, >= );
        break;

      case WABT_OPCODE_I32_TRUNC_S_F32: {
        VALUE_TYPE_F32 value = POP_F32();
        TRAP_IF(wabt_is_nan_f32(value), INVALID_CONVERSION_TO_INTEGER);
        TRAP_UNLESS(is_in_range_i32_trunc_s_f32(value), INTEGER_OVERFLOW);
        PUSH_I32((int32_t)BITCAST_TO_F32(value));
        break;
      }

      case WABT_OPCODE_I32_TRUNC_S_F64: {
        VALUE_TYPE_F64 value = POP_F64();
        TRAP_IF(wabt_is_nan_f64(value), INVALID_CONVERSION_TO_INTEGER);
        TRAP_UNLESS(is_in_range_i32_trunc_s_f64(value), INTEGER_OVERFLOW);
        PUSH_I32((int32_t)BITCAST_TO_F64(value));
        break;
      }

      case WABT_OPCODE_I32_TRUNC_U_F32: {
        VALUE_TYPE_F32 value = POP_F32();
        TRAP_IF(wabt_is_nan_f32(value), INVALID_CONVERSION_TO_INTEGER);
        TRAP_UNLESS(is_in_range_i32_trunc_u_f32(value), INTEGER_OVERFLOW);
        PUSH_I32((uint32_t)BITCAST_TO_F32(value));
        break;
      }

      case WABT_OPCODE_I32_TRUNC_U_F64: {
        VALUE_TYPE_F64 value = POP_F64();
        TRAP_IF(wabt_is_nan_f64(value), INVALID_CONVERSION_TO_INTEGER);
        TRAP_UNLESS(is_in_range_i32_trunc_u_f64(value), INTEGER_OVERFLOW);
        PUSH_I32((uint32_t)BITCAST_TO_F64(value));
        break;
      }

      case WABT_OPCODE_I32_WRAP_I64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I32((uint32_t)value);
        break;
      }

      case WABT_OPCODE_I64_TRUNC_S_F32: {
        VALUE_TYPE_F32 value = POP_F32();
        TRAP_IF(wabt_is_nan_f32(value), INVALID_CONVERSION_TO_INTEGER);
        TRAP_UNLESS(is_in_range_i64_trunc_s_f32(value), INTEGER_OVERFLOW);
        PUSH_I64((int64_t)BITCAST_TO_F32(value));
        break;
      }

      case WABT_OPCODE_I64_TRUNC_S_F64: {
        VALUE_TYPE_F64 value = POP_F64();
        TRAP_IF(wabt_is_nan_f64(value), INVALID_CONVERSION_TO_INTEGER);
        TRAP_UNLESS(is_in_range_i64_trunc_s_f64(value), INTEGER_OVERFLOW);
        PUSH_I64((int64_t)BITCAST_TO_F64(value));
        break;
      }

      case WABT_OPCODE_I64_TRUNC_U_F32: {
        VALUE_TYPE_F32 value = POP_F32();
        TRAP_IF(wabt_is_nan_f32(value), INVALID_CONVERSION_TO_INTEGER);
        TRAP_UNLESS(is_in_range_i64_trunc_u_f32(value), INTEGER_OVERFLOW);
        PUSH_I64((uint64_t)BITCAST_TO_F32(value));
        break;
      }

      case WABT_OPCODE_I64_TRUNC_U_F64: {
        VALUE_TYPE_F64 value = POP_F64();
        TRAP_IF(wabt_is_nan_f64(value), INVALID_CONVERSION_TO_INTEGER);
        TRAP_UNLESS(is_in_range_i64_trunc_u_f64(value), INTEGER_OVERFLOW);
        PUSH_I64((uint64_t)BITCAST_TO_F64(value));
        break;
      }

      case WABT_OPCODE_I64_EXTEND_S_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I64((int64_t)BITCAST_I32_TO_SIGNED(value));
        break;
      }

      case WABT_OPCODE_I64_EXTEND_U_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I64((uint64_t)value);
        break;
      }

      case WABT_OPCODE_F32_CONVERT_S_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F32(BITCAST_FROM_F32((float)BITCAST_I32_TO_SIGNED(value)));
        break;
      }

      case WABT_OPCODE_F32_CONVERT_U_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F32(BITCAST_FROM_F32((float)value));
        break;
      }

      case WABT_OPCODE_F32_CONVERT_S_I64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F32(BITCAST_FROM_F32((float)BITCAST_I64_TO_SIGNED(value)));
        break;
      }

      case WABT_OPCODE_F32_CONVERT_U_I64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F32(BITCAST_FROM_F32((float)value));
        break;
      }

      case WABT_OPCODE_F32_DEMOTE_F64: {
        VALUE_TYPE_F64 value = POP_F64();
        if (WABT_LIKELY(is_in_range_f64_demote_f32(value))) {
          PUSH_F32(BITCAST_FROM_F32((float)BITCAST_TO_F64(value)));
        } else if (is_in_range_f64_demote_f32_round_to_f32_max(value)) {
          PUSH_F32(F32_MAX);
        } else if (is_in_range_f64_demote_f32_round_to_neg_f32_max(value)) {
          PUSH_F32(F32_NEG_MAX);
        } else {
          uint32_t sign = (value >> 32) & F32_SIGN_MASK;
          uint32_t tag = 0;
          if (IS_NAN_F64(value)) {
            tag = F32_QUIET_NAN_BIT |
                  ((value >> (F64_SIG_BITS - F32_SIG_BITS)) & F32_SIG_MASK);
          }
          PUSH_F32(sign | F32_INF | tag);
        }
        break;
      }

      case WABT_OPCODE_F32_REINTERPRET_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F32(value);
        break;
      }

      case WABT_OPCODE_F64_CONVERT_S_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F64(BITCAST_FROM_F64((double)BITCAST_I32_TO_SIGNED(value)));
        break;
      }

      case WABT_OPCODE_F64_CONVERT_U_I32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F64(BITCAST_FROM_F64((double)value));
        break;
      }

      case WABT_OPCODE_F64_CONVERT_S_I64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F64(BITCAST_FROM_F64((double)BITCAST_I64_TO_SIGNED(value)));
        break;
      }

      case WABT_OPCODE_F64_CONVERT_U_I64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F64(BITCAST_FROM_F64((double)value));
        break;
      }

      case WABT_OPCODE_F64_PROMOTE_F32: {
        VALUE_TYPE_F32 value = POP_F32();
        PUSH_F64(BITCAST_FROM_F64((double)BITCAST_TO_F32(value)));
        break;
      }

      case WABT_OPCODE_F64_REINTERPRET_I64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F64(value);
        break;
      }

      case WABT_OPCODE_I32_REINTERPRET_F32: {
        VALUE_TYPE_F32 value = POP_F32();
        PUSH_I32(value);
        break;
      }

      case WABT_OPCODE_I64_REINTERPRET_F64: {
        VALUE_TYPE_F64 value = POP_F64();
        PUSH_I64(value);
        break;
      }

      case WABT_OPCODE_I32_ROTR:
        BINOP_ROT(I32, RIGHT);
        break;

      case WABT_OPCODE_I32_ROTL:
        BINOP_ROT(I32, LEFT);
        break;

      case WABT_OPCODE_I64_ROTR:
        BINOP_ROT(I64, RIGHT);
        break;

      case WABT_OPCODE_I64_ROTL:
        BINOP_ROT(I64, LEFT);
        break;

      case WABT_OPCODE_I64_EQZ: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I64(value == 0);
        break;
      }

      case WABT_OPCODE_ALLOCA: {
        WabtInterpreterValue* old_value_stack_top = thread->value_stack_top;
        thread->value_stack_top += read_u32(&pc);
        CHECK_STACK();
        memset(old_value_stack_top, 0,
               (thread->value_stack_top - old_value_stack_top) *
                   sizeof(WabtInterpreterValue));
        break;
      }

      case WABT_OPCODE_BR_UNLESS: {
        uint32_t new_pc = read_u32(&pc);
        if (!POP_I32())
          GOTO(new_pc);
        break;
      }

      case WABT_OPCODE_DROP:
        (void)POP();
        break;

      case WABT_OPCODE_DROP_KEEP: {
        uint32_t drop_count = read_u32(&pc);
        uint8_t keep_count = *pc++;
        DROP_KEEP(drop_count, keep_count);
        break;
      }

      case WABT_OPCODE_DATA:
        /* shouldn't ever execute this */
        assert(0);
        break;

      case WABT_OPCODE_NOP:
        break;

      default:
        assert(0);
        break;
    }
  }

exit_loop:
  thread->pc = pc - istream;
  return result;
}

void wabt_trace_pc(WabtInterpreterThread* thread, WabtStream* stream) {
  const uint8_t* istream = (const uint8_t*)thread->env->istream.start;
  const uint8_t* pc = &istream[thread->pc];
  size_t value_stack_depth = thread->value_stack_top - thread->value_stack.data;
  size_t call_stack_depth = thread->call_stack_top - thread->call_stack.data;

  wabt_writef(stream, "#%" PRIzd ". %4" PRIzd ": V:%-3" PRIzd "| ",
              call_stack_depth, pc - (uint8_t*)thread->env->istream.start,
              value_stack_depth);

  uint8_t opcode = *pc++;
  switch (opcode) {
    case WABT_OPCODE_SELECT:
      wabt_writef(stream, "%s %u, %" PRIu64 ", %" PRIu64 "\n",
                  wabt_get_interpreter_opcode_name(opcode), PICK(3).i32,
                  PICK(2).i64, PICK(1).i64);
      break;

    case WABT_OPCODE_BR:
      wabt_writef(stream, "%s @%u\n", wabt_get_interpreter_opcode_name(opcode),
                  read_u32_at(pc));
      break;

    case WABT_OPCODE_BR_IF:
      wabt_writef(stream, "%s @%u, %u\n",
                  wabt_get_interpreter_opcode_name(opcode), read_u32_at(pc),
                  TOP().i32);
      break;

    case WABT_OPCODE_BR_TABLE: {
      uint32_t num_targets = read_u32_at(pc);
      uint32_t table_offset = read_u32_at(pc + 4);
      VALUE_TYPE_I32 key = TOP().i32;
      wabt_writef(stream, "%s %u, $#%u, table:$%u\n",
                  wabt_get_interpreter_opcode_name(opcode), key, num_targets,
                  table_offset);
      break;
    }

    case WABT_OPCODE_NOP:
    case WABT_OPCODE_RETURN:
    case WABT_OPCODE_UNREACHABLE:
    case WABT_OPCODE_DROP:
      wabt_writef(stream, "%s\n", wabt_get_interpreter_opcode_name(opcode));
      break;

    case WABT_OPCODE_CURRENT_MEMORY: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u\n", wabt_get_interpreter_opcode_name(opcode),
                  memory_index);
      break;
    }

    case WABT_OPCODE_I32_CONST:
      wabt_writef(stream, "%s $%u\n", wabt_get_interpreter_opcode_name(opcode),
                  read_u32_at(pc));
      break;

    case WABT_OPCODE_I64_CONST:
      wabt_writef(stream, "%s $%" PRIu64 "\n",
                  wabt_get_interpreter_opcode_name(opcode), read_u64_at(pc));
      break;

    case WABT_OPCODE_F32_CONST:
      wabt_writef(stream, "%s $%g\n", wabt_get_interpreter_opcode_name(opcode),
                  bitcast_u32_to_f32(read_u32_at(pc)));
      break;

    case WABT_OPCODE_F64_CONST:
      wabt_writef(stream, "%s $%g\n", wabt_get_interpreter_opcode_name(opcode),
                  bitcast_u64_to_f64(read_u64_at(pc)));
      break;

    case WABT_OPCODE_GET_LOCAL:
    case WABT_OPCODE_GET_GLOBAL:
      wabt_writef(stream, "%s $%u\n", wabt_get_interpreter_opcode_name(opcode),
                  read_u32_at(pc));
      break;

    case WABT_OPCODE_SET_LOCAL:
    case WABT_OPCODE_SET_GLOBAL:
    case WABT_OPCODE_TEE_LOCAL:
      wabt_writef(stream, "%s $%u, %u\n",
                  wabt_get_interpreter_opcode_name(opcode), read_u32_at(pc),
                  TOP().i32);
      break;

    case WABT_OPCODE_CALL:
      wabt_writef(stream, "%s @%u\n", wabt_get_interpreter_opcode_name(opcode),
                  read_u32_at(pc));
      break;

    case WABT_OPCODE_CALL_INDIRECT:
      wabt_writef(stream, "%s $%u, %u\n",
                  wabt_get_interpreter_opcode_name(opcode), read_u32_at(pc),
                  TOP().i32);
      break;

    case WABT_OPCODE_CALL_HOST:
      wabt_writef(stream, "%s $%u\n", wabt_get_interpreter_opcode_name(opcode),
                  read_u32_at(pc));
      break;

    case WABT_OPCODE_I32_LOAD8_S:
    case WABT_OPCODE_I32_LOAD8_U:
    case WABT_OPCODE_I32_LOAD16_S:
    case WABT_OPCODE_I32_LOAD16_U:
    case WABT_OPCODE_I64_LOAD8_S:
    case WABT_OPCODE_I64_LOAD8_U:
    case WABT_OPCODE_I64_LOAD16_S:
    case WABT_OPCODE_I64_LOAD16_U:
    case WABT_OPCODE_I64_LOAD32_S:
    case WABT_OPCODE_I64_LOAD32_U:
    case WABT_OPCODE_I32_LOAD:
    case WABT_OPCODE_I64_LOAD:
    case WABT_OPCODE_F32_LOAD:
    case WABT_OPCODE_F64_LOAD: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u:%u+$%u\n",
                  wabt_get_interpreter_opcode_name(opcode), memory_index,
                  TOP().i32, read_u32_at(pc));
      break;
    }

    case WABT_OPCODE_I32_STORE8:
    case WABT_OPCODE_I32_STORE16:
    case WABT_OPCODE_I32_STORE: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u:%u+$%u, %u\n",
                  wabt_get_interpreter_opcode_name(opcode), memory_index,
                  PICK(2).i32, read_u32_at(pc), PICK(1).i32);
      break;
    }

    case WABT_OPCODE_I64_STORE8:
    case WABT_OPCODE_I64_STORE16:
    case WABT_OPCODE_I64_STORE32:
    case WABT_OPCODE_I64_STORE: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u:%u+$%u, %" PRIu64 "\n",
                  wabt_get_interpreter_opcode_name(opcode), memory_index,
                  PICK(2).i32, read_u32_at(pc), PICK(1).i64);
      break;
    }

    case WABT_OPCODE_F32_STORE: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u:%u+$%u, %g\n",
                  wabt_get_interpreter_opcode_name(opcode), memory_index,
                  PICK(2).i32, read_u32_at(pc),
                  bitcast_u32_to_f32(PICK(1).f32_bits));
      break;
    }

    case WABT_OPCODE_F64_STORE: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u:%u+$%u, %g\n",
                  wabt_get_interpreter_opcode_name(opcode), memory_index,
                  PICK(2).i32, read_u32_at(pc),
                  bitcast_u64_to_f64(PICK(1).f64_bits));
      break;
    }

    case WABT_OPCODE_GROW_MEMORY: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u:%u\n",
                  wabt_get_interpreter_opcode_name(opcode), memory_index,
                  TOP().i32);
      break;
    }

    case WABT_OPCODE_I32_ADD:
    case WABT_OPCODE_I32_SUB:
    case WABT_OPCODE_I32_MUL:
    case WABT_OPCODE_I32_DIV_S:
    case WABT_OPCODE_I32_DIV_U:
    case WABT_OPCODE_I32_REM_S:
    case WABT_OPCODE_I32_REM_U:
    case WABT_OPCODE_I32_AND:
    case WABT_OPCODE_I32_OR:
    case WABT_OPCODE_I32_XOR:
    case WABT_OPCODE_I32_SHL:
    case WABT_OPCODE_I32_SHR_U:
    case WABT_OPCODE_I32_SHR_S:
    case WABT_OPCODE_I32_EQ:
    case WABT_OPCODE_I32_NE:
    case WABT_OPCODE_I32_LT_S:
    case WABT_OPCODE_I32_LE_S:
    case WABT_OPCODE_I32_LT_U:
    case WABT_OPCODE_I32_LE_U:
    case WABT_OPCODE_I32_GT_S:
    case WABT_OPCODE_I32_GE_S:
    case WABT_OPCODE_I32_GT_U:
    case WABT_OPCODE_I32_GE_U:
    case WABT_OPCODE_I32_ROTR:
    case WABT_OPCODE_I32_ROTL:
      wabt_writef(stream, "%s %u, %u\n",
                  wabt_get_interpreter_opcode_name(opcode), PICK(2).i32,
                  PICK(1).i32);
      break;

    case WABT_OPCODE_I32_CLZ:
    case WABT_OPCODE_I32_CTZ:
    case WABT_OPCODE_I32_POPCNT:
    case WABT_OPCODE_I32_EQZ:
      wabt_writef(stream, "%s %u\n", wabt_get_interpreter_opcode_name(opcode),
                  TOP().i32);
      break;

    case WABT_OPCODE_I64_ADD:
    case WABT_OPCODE_I64_SUB:
    case WABT_OPCODE_I64_MUL:
    case WABT_OPCODE_I64_DIV_S:
    case WABT_OPCODE_I64_DIV_U:
    case WABT_OPCODE_I64_REM_S:
    case WABT_OPCODE_I64_REM_U:
    case WABT_OPCODE_I64_AND:
    case WABT_OPCODE_I64_OR:
    case WABT_OPCODE_I64_XOR:
    case WABT_OPCODE_I64_SHL:
    case WABT_OPCODE_I64_SHR_U:
    case WABT_OPCODE_I64_SHR_S:
    case WABT_OPCODE_I64_EQ:
    case WABT_OPCODE_I64_NE:
    case WABT_OPCODE_I64_LT_S:
    case WABT_OPCODE_I64_LE_S:
    case WABT_OPCODE_I64_LT_U:
    case WABT_OPCODE_I64_LE_U:
    case WABT_OPCODE_I64_GT_S:
    case WABT_OPCODE_I64_GE_S:
    case WABT_OPCODE_I64_GT_U:
    case WABT_OPCODE_I64_GE_U:
    case WABT_OPCODE_I64_ROTR:
    case WABT_OPCODE_I64_ROTL:
      wabt_writef(stream, "%s %" PRIu64 ", %" PRIu64 "\n",
                  wabt_get_interpreter_opcode_name(opcode), PICK(2).i64,
                  PICK(1).i64);
      break;

    case WABT_OPCODE_I64_CLZ:
    case WABT_OPCODE_I64_CTZ:
    case WABT_OPCODE_I64_POPCNT:
    case WABT_OPCODE_I64_EQZ:
      wabt_writef(stream, "%s %" PRIu64 "\n",
                  wabt_get_interpreter_opcode_name(opcode), TOP().i64);
      break;

    case WABT_OPCODE_F32_ADD:
    case WABT_OPCODE_F32_SUB:
    case WABT_OPCODE_F32_MUL:
    case WABT_OPCODE_F32_DIV:
    case WABT_OPCODE_F32_MIN:
    case WABT_OPCODE_F32_MAX:
    case WABT_OPCODE_F32_COPYSIGN:
    case WABT_OPCODE_F32_EQ:
    case WABT_OPCODE_F32_NE:
    case WABT_OPCODE_F32_LT:
    case WABT_OPCODE_F32_LE:
    case WABT_OPCODE_F32_GT:
    case WABT_OPCODE_F32_GE:
      wabt_writef(
          stream, "%s %g, %g\n", wabt_get_interpreter_opcode_name(opcode),
          bitcast_u32_to_f32(PICK(2).i32), bitcast_u32_to_f32(PICK(1).i32));
      break;

    case WABT_OPCODE_F32_ABS:
    case WABT_OPCODE_F32_NEG:
    case WABT_OPCODE_F32_CEIL:
    case WABT_OPCODE_F32_FLOOR:
    case WABT_OPCODE_F32_TRUNC:
    case WABT_OPCODE_F32_NEAREST:
    case WABT_OPCODE_F32_SQRT:
      wabt_writef(stream, "%s %g\n", wabt_get_interpreter_opcode_name(opcode),
                  bitcast_u32_to_f32(TOP().i32));
      break;

    case WABT_OPCODE_F64_ADD:
    case WABT_OPCODE_F64_SUB:
    case WABT_OPCODE_F64_MUL:
    case WABT_OPCODE_F64_DIV:
    case WABT_OPCODE_F64_MIN:
    case WABT_OPCODE_F64_MAX:
    case WABT_OPCODE_F64_COPYSIGN:
    case WABT_OPCODE_F64_EQ:
    case WABT_OPCODE_F64_NE:
    case WABT_OPCODE_F64_LT:
    case WABT_OPCODE_F64_LE:
    case WABT_OPCODE_F64_GT:
    case WABT_OPCODE_F64_GE:
      wabt_writef(
          stream, "%s %g, %g\n", wabt_get_interpreter_opcode_name(opcode),
          bitcast_u64_to_f64(PICK(2).i64), bitcast_u64_to_f64(PICK(1).i64));
      break;

    case WABT_OPCODE_F64_ABS:
    case WABT_OPCODE_F64_NEG:
    case WABT_OPCODE_F64_CEIL:
    case WABT_OPCODE_F64_FLOOR:
    case WABT_OPCODE_F64_TRUNC:
    case WABT_OPCODE_F64_NEAREST:
    case WABT_OPCODE_F64_SQRT:
      wabt_writef(stream, "%s %g\n", wabt_get_interpreter_opcode_name(opcode),
                  bitcast_u64_to_f64(TOP().i64));
      break;

    case WABT_OPCODE_I32_TRUNC_S_F32:
    case WABT_OPCODE_I32_TRUNC_U_F32:
    case WABT_OPCODE_I64_TRUNC_S_F32:
    case WABT_OPCODE_I64_TRUNC_U_F32:
    case WABT_OPCODE_F64_PROMOTE_F32:
    case WABT_OPCODE_I32_REINTERPRET_F32:
      wabt_writef(stream, "%s %g\n", wabt_get_interpreter_opcode_name(opcode),
                  bitcast_u32_to_f32(TOP().i32));
      break;

    case WABT_OPCODE_I32_TRUNC_S_F64:
    case WABT_OPCODE_I32_TRUNC_U_F64:
    case WABT_OPCODE_I64_TRUNC_S_F64:
    case WABT_OPCODE_I64_TRUNC_U_F64:
    case WABT_OPCODE_F32_DEMOTE_F64:
    case WABT_OPCODE_I64_REINTERPRET_F64:
      wabt_writef(stream, "%s %g\n", wabt_get_interpreter_opcode_name(opcode),
                  bitcast_u64_to_f64(TOP().i64));
      break;

    case WABT_OPCODE_I32_WRAP_I64:
    case WABT_OPCODE_F32_CONVERT_S_I64:
    case WABT_OPCODE_F32_CONVERT_U_I64:
    case WABT_OPCODE_F64_CONVERT_S_I64:
    case WABT_OPCODE_F64_CONVERT_U_I64:
    case WABT_OPCODE_F64_REINTERPRET_I64:
      wabt_writef(stream, "%s %" PRIu64 "\n",
                  wabt_get_interpreter_opcode_name(opcode), TOP().i64);
      break;

    case WABT_OPCODE_I64_EXTEND_S_I32:
    case WABT_OPCODE_I64_EXTEND_U_I32:
    case WABT_OPCODE_F32_CONVERT_S_I32:
    case WABT_OPCODE_F32_CONVERT_U_I32:
    case WABT_OPCODE_F32_REINTERPRET_I32:
    case WABT_OPCODE_F64_CONVERT_S_I32:
    case WABT_OPCODE_F64_CONVERT_U_I32:
      wabt_writef(stream, "%s %u\n", wabt_get_interpreter_opcode_name(opcode),
                  TOP().i32);
      break;

    case WABT_OPCODE_ALLOCA:
      wabt_writef(stream, "%s $%u\n", wabt_get_interpreter_opcode_name(opcode),
                  read_u32_at(pc));
      break;

    case WABT_OPCODE_BR_UNLESS:
      wabt_writef(stream, "%s @%u, %u\n",
                  wabt_get_interpreter_opcode_name(opcode), read_u32_at(pc),
                  TOP().i32);
      break;

    case WABT_OPCODE_DROP_KEEP:
      wabt_writef(stream, "%s $%u $%u\n",
                  wabt_get_interpreter_opcode_name(opcode), read_u32_at(pc),
                  *(pc + 4));
      break;

    case WABT_OPCODE_DATA:
      /* shouldn't ever execute this */
      assert(0);
      break;

    default:
      assert(0);
      break;
  }
}

void wabt_disassemble(WabtInterpreterEnvironment* env,
                      WabtStream* stream,
                      uint32_t from,
                      uint32_t to) {
  /* TODO(binji): mark function entries */
  /* TODO(binji): track value stack size */
  if (from >= env->istream.size)
    return;
  if (to > env->istream.size)
    to = env->istream.size;
  const uint8_t* istream = (const uint8_t*)env->istream.start;
  const uint8_t* pc = &istream[from];

  while ((uint32_t)(pc - istream) < to) {
    wabt_writef(stream, "%4" PRIzd "| ", pc - istream);

    uint8_t opcode = *pc++;
    switch (opcode) {
      case WABT_OPCODE_SELECT:
        wabt_writef(stream, "%s %%[-3], %%[-2], %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode));
        break;

      case WABT_OPCODE_BR:
        wabt_writef(stream, "%s @%u\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WABT_OPCODE_BR_IF:
        wabt_writef(stream, "%s @%u, %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WABT_OPCODE_BR_TABLE: {
        uint32_t num_targets = read_u32(&pc);
        uint32_t table_offset = read_u32(&pc);
        wabt_writef(stream, "%s %%[-1], $#%u, table:$%u\n",
                    wabt_get_interpreter_opcode_name(opcode), num_targets,
                    table_offset);
        break;
      }

      case WABT_OPCODE_NOP:
      case WABT_OPCODE_RETURN:
      case WABT_OPCODE_UNREACHABLE:
      case WABT_OPCODE_DROP:
        wabt_writef(stream, "%s\n", wabt_get_interpreter_opcode_name(opcode));
        break;

      case WABT_OPCODE_CURRENT_MEMORY: {
        uint32_t memory_index = read_u32(&pc);
        wabt_writef(stream, "%s $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), memory_index);
        break;
      }

      case WABT_OPCODE_I32_CONST:
        wabt_writef(stream, "%s $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WABT_OPCODE_I64_CONST:
        wabt_writef(stream, "%s $%" PRIu64 "\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u64(&pc));
        break;

      case WABT_OPCODE_F32_CONST:
        wabt_writef(stream, "%s $%g\n",
                    wabt_get_interpreter_opcode_name(opcode),
                    bitcast_u32_to_f32(read_u32(&pc)));
        break;

      case WABT_OPCODE_F64_CONST:
        wabt_writef(stream, "%s $%g\n",
                    wabt_get_interpreter_opcode_name(opcode),
                    bitcast_u64_to_f64(read_u64(&pc)));
        break;

      case WABT_OPCODE_GET_LOCAL:
      case WABT_OPCODE_GET_GLOBAL:
        wabt_writef(stream, "%s $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WABT_OPCODE_SET_LOCAL:
      case WABT_OPCODE_SET_GLOBAL:
      case WABT_OPCODE_TEE_LOCAL:
        wabt_writef(stream, "%s $%u, %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WABT_OPCODE_CALL:
        wabt_writef(stream, "%s @%u\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WABT_OPCODE_CALL_INDIRECT: {
        uint32_t table_index = read_u32(&pc);
        wabt_writef(stream, "%s $%u:%u, %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode), table_index,
                    read_u32(&pc));
        break;
      }

      case WABT_OPCODE_CALL_HOST:
        wabt_writef(stream, "%s $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WABT_OPCODE_I32_LOAD8_S:
      case WABT_OPCODE_I32_LOAD8_U:
      case WABT_OPCODE_I32_LOAD16_S:
      case WABT_OPCODE_I32_LOAD16_U:
      case WABT_OPCODE_I64_LOAD8_S:
      case WABT_OPCODE_I64_LOAD8_U:
      case WABT_OPCODE_I64_LOAD16_S:
      case WABT_OPCODE_I64_LOAD16_U:
      case WABT_OPCODE_I64_LOAD32_S:
      case WABT_OPCODE_I64_LOAD32_U:
      case WABT_OPCODE_I32_LOAD:
      case WABT_OPCODE_I64_LOAD:
      case WABT_OPCODE_F32_LOAD:
      case WABT_OPCODE_F64_LOAD: {
        uint32_t memory_index = read_u32(&pc);
        wabt_writef(stream, "%s $%u:%%[-1]+$%u\n",
                    wabt_get_interpreter_opcode_name(opcode), memory_index,
                    read_u32(&pc));
        break;
      }

      case WABT_OPCODE_I32_STORE8:
      case WABT_OPCODE_I32_STORE16:
      case WABT_OPCODE_I32_STORE:
      case WABT_OPCODE_I64_STORE8:
      case WABT_OPCODE_I64_STORE16:
      case WABT_OPCODE_I64_STORE32:
      case WABT_OPCODE_I64_STORE:
      case WABT_OPCODE_F32_STORE:
      case WABT_OPCODE_F64_STORE: {
        uint32_t memory_index = read_u32(&pc);
        wabt_writef(stream, "%s %%[-2]+$%u, $%u:%%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode), memory_index,
                    read_u32(&pc));
        break;
      }

      case WABT_OPCODE_I32_ADD:
      case WABT_OPCODE_I32_SUB:
      case WABT_OPCODE_I32_MUL:
      case WABT_OPCODE_I32_DIV_S:
      case WABT_OPCODE_I32_DIV_U:
      case WABT_OPCODE_I32_REM_S:
      case WABT_OPCODE_I32_REM_U:
      case WABT_OPCODE_I32_AND:
      case WABT_OPCODE_I32_OR:
      case WABT_OPCODE_I32_XOR:
      case WABT_OPCODE_I32_SHL:
      case WABT_OPCODE_I32_SHR_U:
      case WABT_OPCODE_I32_SHR_S:
      case WABT_OPCODE_I32_EQ:
      case WABT_OPCODE_I32_NE:
      case WABT_OPCODE_I32_LT_S:
      case WABT_OPCODE_I32_LE_S:
      case WABT_OPCODE_I32_LT_U:
      case WABT_OPCODE_I32_LE_U:
      case WABT_OPCODE_I32_GT_S:
      case WABT_OPCODE_I32_GE_S:
      case WABT_OPCODE_I32_GT_U:
      case WABT_OPCODE_I32_GE_U:
      case WABT_OPCODE_I32_ROTR:
      case WABT_OPCODE_I32_ROTL:
      case WABT_OPCODE_F32_ADD:
      case WABT_OPCODE_F32_SUB:
      case WABT_OPCODE_F32_MUL:
      case WABT_OPCODE_F32_DIV:
      case WABT_OPCODE_F32_MIN:
      case WABT_OPCODE_F32_MAX:
      case WABT_OPCODE_F32_COPYSIGN:
      case WABT_OPCODE_F32_EQ:
      case WABT_OPCODE_F32_NE:
      case WABT_OPCODE_F32_LT:
      case WABT_OPCODE_F32_LE:
      case WABT_OPCODE_F32_GT:
      case WABT_OPCODE_F32_GE:
      case WABT_OPCODE_I64_ADD:
      case WABT_OPCODE_I64_SUB:
      case WABT_OPCODE_I64_MUL:
      case WABT_OPCODE_I64_DIV_S:
      case WABT_OPCODE_I64_DIV_U:
      case WABT_OPCODE_I64_REM_S:
      case WABT_OPCODE_I64_REM_U:
      case WABT_OPCODE_I64_AND:
      case WABT_OPCODE_I64_OR:
      case WABT_OPCODE_I64_XOR:
      case WABT_OPCODE_I64_SHL:
      case WABT_OPCODE_I64_SHR_U:
      case WABT_OPCODE_I64_SHR_S:
      case WABT_OPCODE_I64_EQ:
      case WABT_OPCODE_I64_NE:
      case WABT_OPCODE_I64_LT_S:
      case WABT_OPCODE_I64_LE_S:
      case WABT_OPCODE_I64_LT_U:
      case WABT_OPCODE_I64_LE_U:
      case WABT_OPCODE_I64_GT_S:
      case WABT_OPCODE_I64_GE_S:
      case WABT_OPCODE_I64_GT_U:
      case WABT_OPCODE_I64_GE_U:
      case WABT_OPCODE_I64_ROTR:
      case WABT_OPCODE_I64_ROTL:
      case WABT_OPCODE_F64_ADD:
      case WABT_OPCODE_F64_SUB:
      case WABT_OPCODE_F64_MUL:
      case WABT_OPCODE_F64_DIV:
      case WABT_OPCODE_F64_MIN:
      case WABT_OPCODE_F64_MAX:
      case WABT_OPCODE_F64_COPYSIGN:
      case WABT_OPCODE_F64_EQ:
      case WABT_OPCODE_F64_NE:
      case WABT_OPCODE_F64_LT:
      case WABT_OPCODE_F64_LE:
      case WABT_OPCODE_F64_GT:
      case WABT_OPCODE_F64_GE:
        wabt_writef(stream, "%s %%[-2], %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode));
        break;

      case WABT_OPCODE_I32_CLZ:
      case WABT_OPCODE_I32_CTZ:
      case WABT_OPCODE_I32_POPCNT:
      case WABT_OPCODE_I32_EQZ:
      case WABT_OPCODE_I64_CLZ:
      case WABT_OPCODE_I64_CTZ:
      case WABT_OPCODE_I64_POPCNT:
      case WABT_OPCODE_I64_EQZ:
      case WABT_OPCODE_F32_ABS:
      case WABT_OPCODE_F32_NEG:
      case WABT_OPCODE_F32_CEIL:
      case WABT_OPCODE_F32_FLOOR:
      case WABT_OPCODE_F32_TRUNC:
      case WABT_OPCODE_F32_NEAREST:
      case WABT_OPCODE_F32_SQRT:
      case WABT_OPCODE_F64_ABS:
      case WABT_OPCODE_F64_NEG:
      case WABT_OPCODE_F64_CEIL:
      case WABT_OPCODE_F64_FLOOR:
      case WABT_OPCODE_F64_TRUNC:
      case WABT_OPCODE_F64_NEAREST:
      case WABT_OPCODE_F64_SQRT:
      case WABT_OPCODE_I32_TRUNC_S_F32:
      case WABT_OPCODE_I32_TRUNC_U_F32:
      case WABT_OPCODE_I64_TRUNC_S_F32:
      case WABT_OPCODE_I64_TRUNC_U_F32:
      case WABT_OPCODE_F64_PROMOTE_F32:
      case WABT_OPCODE_I32_REINTERPRET_F32:
      case WABT_OPCODE_I32_TRUNC_S_F64:
      case WABT_OPCODE_I32_TRUNC_U_F64:
      case WABT_OPCODE_I64_TRUNC_S_F64:
      case WABT_OPCODE_I64_TRUNC_U_F64:
      case WABT_OPCODE_F32_DEMOTE_F64:
      case WABT_OPCODE_I64_REINTERPRET_F64:
      case WABT_OPCODE_I32_WRAP_I64:
      case WABT_OPCODE_F32_CONVERT_S_I64:
      case WABT_OPCODE_F32_CONVERT_U_I64:
      case WABT_OPCODE_F64_CONVERT_S_I64:
      case WABT_OPCODE_F64_CONVERT_U_I64:
      case WABT_OPCODE_F64_REINTERPRET_I64:
      case WABT_OPCODE_I64_EXTEND_S_I32:
      case WABT_OPCODE_I64_EXTEND_U_I32:
      case WABT_OPCODE_F32_CONVERT_S_I32:
      case WABT_OPCODE_F32_CONVERT_U_I32:
      case WABT_OPCODE_F32_REINTERPRET_I32:
      case WABT_OPCODE_F64_CONVERT_S_I32:
      case WABT_OPCODE_F64_CONVERT_U_I32:
        wabt_writef(stream, "%s %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode));
        break;

      case WABT_OPCODE_GROW_MEMORY: {
        uint32_t memory_index = read_u32(&pc);
        wabt_writef(stream, "%s $%u:%%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode), memory_index);
        break;
      }

      case WABT_OPCODE_ALLOCA:
        wabt_writef(stream, "%s $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WABT_OPCODE_BR_UNLESS:
        wabt_writef(stream, "%s @%u, %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WABT_OPCODE_DROP_KEEP: {
        uint32_t drop = read_u32(&pc);
        uint32_t keep = *pc++;
        wabt_writef(stream, "%s $%u $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), drop, keep);
        break;
      }

      case WABT_OPCODE_DATA: {
        uint32_t num_bytes = read_u32(&pc);
        wabt_writef(stream, "%s $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), num_bytes);
        /* for now, the only reason this is emitted is for br_table, so display
         * it as a list of table entries */
        if (num_bytes % WABT_TABLE_ENTRY_SIZE == 0) {
          uint32_t num_entries = num_bytes / WABT_TABLE_ENTRY_SIZE;
          uint32_t i;
          for (i = 0; i < num_entries; ++i) {
            wabt_writef(stream, "%4" PRIzd "| ", pc - istream);
            uint32_t offset;
            uint32_t drop;
            uint8_t keep;
            read_table_entry_at(pc, &offset, &drop, &keep);
            wabt_writef(stream, "  entry %d: offset: %u drop: %u keep: %u\n", i,
                        offset, drop, keep);
            pc += WABT_TABLE_ENTRY_SIZE;
          }
        } else {
          /* just skip those data bytes */
          pc += num_bytes;
        }

        break;
      }

      default:
        assert(0);
        break;
    }
  }
}

void wabt_disassemble_module(WabtInterpreterEnvironment* env,
                             WabtStream* stream,
                             WabtInterpreterModule* module) {
  assert(!module->is_host);
  wabt_disassemble(env, stream, module->defined.istream_start,
                   module->defined.istream_end);
}


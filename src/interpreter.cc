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
      return WabtResult::Error;   \
  } while (0)

/* TODO(binji): It's annoying to have to have an initializer function, but it
 * seems to be necessary as g++ doesn't allow non-trival designated
 * initializers (e.g. [314] = "blah") */
static void init_interpreter_opcode_table(void) {
  static bool s_initialized = false;
  if (!s_initialized) {
#define V(rtype, type1, type2, mem_size, code, NAME, text) \
  s_interpreter_opcode_name[code] = text;

    WABT_FOREACH_INTERPRETER_OPCODE(V)

#undef V
  }
}

static const char* wabt_get_interpreter_opcode_name(
    WabtInterpreterOpcode opcode) {
  init_interpreter_opcode_table();
  return s_interpreter_opcode_name[static_cast<int>(opcode)];
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
        entry->binding.index >= static_cast<int>(mark.modules_size)) {
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
    return WabtInterpreterResult::TrapValueStackExhausted;
  *thread->value_stack_top++ = value;
  return WabtInterpreterResult::Ok;
}

WabtInterpreterExport* wabt_get_interpreter_export_by_name(
    WabtInterpreterModule* module,
    const WabtStringSlice* name) {
  int field_index =
      wabt_find_binding_index_by_name(&module->export_bindings, name);
  if (field_index < 0)
    return nullptr;
  assert(static_cast<size_t>(field_index) < module->exports.size);
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

#define bitcast_i32_to_u32(x) (static_cast<uint32_t>(x))
#define bitcast_i64_to_u64(x) (static_cast<uint64_t>(x))

#define VALUE_TYPE_I32 uint32_t
#define VALUE_TYPE_I64 uint64_t
#define VALUE_TYPE_F32 uint32_t
#define VALUE_TYPE_F64 uint64_t

#define VALUE_TYPE_SIGNED_MAX_I32 (0x80000000U)
#define VALUE_TYPE_UNSIGNED_MAX_I32 (0xFFFFFFFFU)
#define VALUE_TYPE_SIGNED_MAX_I64 static_cast<uint64_t>(0x8000000000000000ULL)
#define VALUE_TYPE_UNSIGNED_MAX_I64 static_cast<uint64_t>(0xFFFFFFFFFFFFFFFFULL)

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

#define TRAP(type) return WabtInterpreterResult::Trap##type
#define TRAP_UNLESS(cond, type) TRAP_IF(!(cond), type)
#define TRAP_IF(cond, type)  \
  do {                       \
    if (WABT_UNLIKELY(cond)) \
      TRAP(type);            \
  } while (0)

#define CHECK_STACK()                                         \
  TRAP_IF(thread->value_stack_top >= thread->value_stack_end, \
          ValueStackExhausted)

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
        static_cast<VALUE_TYPE_##type>(v);                \
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
            CallStackExhausted);                              \
    (*thread->call_stack_top++) = (pc - istream);             \
  } while (0)

#define POP_CALL() (*--thread->call_stack_top)

#define GET_MEMORY(var)                      \
  uint32_t memory_index = read_u32(&pc);     \
  assert(memory_index < env->memories.size); \
  WabtInterpreterMemory* var = &env->memories.data[memory_index]

#define LOAD(type, mem_type)                                               \
  do {                                                                     \
    GET_MEMORY(memory);                                                    \
    uint64_t offset = static_cast<uint64_t>(POP_I32()) + read_u32(&pc);    \
    MEM_TYPE_##mem_type value;                                             \
    TRAP_IF(offset + sizeof(value) > memory->byte_size,                    \
            MemoryAccessOutOfBounds);                                  \
    void* src =                                                            \
        reinterpret_cast<void*>(reinterpret_cast<intptr_t>(memory->data) + \
                                static_cast<uint32_t>(offset));            \
    memcpy(&value, src, sizeof(MEM_TYPE_##mem_type));                      \
    PUSH_##type(static_cast<MEM_TYPE_EXTEND_##type##_##mem_type>(value));  \
  } while (0)

#define STORE(type, mem_type)                                              \
  do {                                                                     \
    GET_MEMORY(memory);                                                    \
    VALUE_TYPE_##type value = POP_##type();                                \
    uint64_t offset = static_cast<uint64_t>(POP_I32()) + read_u32(&pc);    \
    MEM_TYPE_##mem_type src = static_cast<MEM_TYPE_##mem_type>(value);     \
    TRAP_IF(offset + sizeof(src) > memory->byte_size,                      \
            MemoryAccessOutOfBounds);                                  \
    void* dst =                                                            \
        reinterpret_cast<void*>(reinterpret_cast<intptr_t>(memory->data) + \
                                static_cast<uint32_t>(offset));            \
    memcpy(dst, &src, sizeof(MEM_TYPE_##mem_type));                        \
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
    TRAP_IF(rhs == 0, IntegerDivideByZero);            \
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
    TRAP_IF(rhs == 0, IntegerDivideByZero);            \
    TRAP_IF(lhs == VALUE_TYPE_SIGNED_MAX_##type &&     \
                rhs == VALUE_TYPE_UNSIGNED_MAX_##type, \
            IntegerOverflow);                          \
    PUSH_##type(BITCAST_##type##_TO_SIGNED(lhs) /      \
                BITCAST_##type##_TO_SIGNED(rhs));      \
  } while (0)

#define BINOP_REM_S(type)                                       \
  do {                                                          \
    VALUE_TYPE_##type rhs = POP_##type();                       \
    VALUE_TYPE_##type lhs = POP_##type();                       \
    TRAP_IF(rhs == 0, IntegerDivideByZero);                 \
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
      static_cast<WabtInterpreterTypedValue*>(
          alloca(sizeof(WabtInterpreterTypedValue) * num_results));

  WabtResult call_result = func->host.callback(
      func, sig, num_args, thread->host_args.data, num_results,
      call_result_values, func->host.user_data);
  TRAP_IF(call_result != WabtResult::Ok, HostTrapped);

  for (i = 0; i < num_results; ++i) {
    TRAP_IF(call_result_values[i].type != sig->result_types.data[i],
            HostResultTypeMismatch);
    PUSH(call_result_values[i].value);
  }

  return WabtInterpreterResult::Ok;
}

WabtInterpreterResult wabt_run_interpreter(WabtInterpreterThread* thread,
                                           uint32_t num_instructions,
                                           uint32_t* call_stack_return_top) {
  WabtInterpreterResult result = WabtInterpreterResult::Ok;
  assert(call_stack_return_top < thread->call_stack_end);

  WabtInterpreterEnvironment* env = thread->env;

  const uint8_t* istream = static_cast<const uint8_t*>(env->istream.start);
  const uint8_t* pc = &istream[thread->pc];
  uint32_t i;
  for (i = 0; i < num_instructions; ++i) {
    WabtInterpreterOpcode opcode = static_cast<WabtInterpreterOpcode>(*pc++);
    switch (opcode) {
      case WabtInterpreterOpcode::Select: {
        VALUE_TYPE_I32 cond = POP_I32();
        WabtInterpreterValue false_ = POP();
        WabtInterpreterValue true_ = POP();
        PUSH(cond ? true_ : false_);
        break;
      }

      case WabtInterpreterOpcode::Br:
        GOTO(read_u32(&pc));
        break;

      case WabtInterpreterOpcode::BrIf: {
        uint32_t new_pc = read_u32(&pc);
        if (POP_I32())
          GOTO(new_pc);
        break;
      }

      case WabtInterpreterOpcode::BrTable: {
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

      case WabtInterpreterOpcode::Return:
        if (thread->call_stack_top == call_stack_return_top) {
          result = WabtInterpreterResult::Returned;
          goto exit_loop;
        }
        GOTO(POP_CALL());
        break;

      case WabtInterpreterOpcode::Unreachable:
        TRAP(Unreachable);
        break;

      case WabtInterpreterOpcode::I32Const:
        PUSH_I32(read_u32(&pc));
        break;

      case WabtInterpreterOpcode::I64Const:
        PUSH_I64(read_u64(&pc));
        break;

      case WabtInterpreterOpcode::F32Const:
        PUSH_F32(read_u32(&pc));
        break;

      case WabtInterpreterOpcode::F64Const:
        PUSH_F64(read_u64(&pc));
        break;

      case WabtInterpreterOpcode::GetGlobal: {
        uint32_t index = read_u32(&pc);
        assert(index < env->globals.size);
        PUSH(env->globals.data[index].typed_value.value);
        break;
      }

      case WabtInterpreterOpcode::SetGlobal: {
        uint32_t index = read_u32(&pc);
        assert(index < env->globals.size);
        env->globals.data[index].typed_value.value = POP();
        break;
      }

      case WabtInterpreterOpcode::GetLocal: {
        WabtInterpreterValue value = PICK(read_u32(&pc));
        PUSH(value);
        break;
      }

      case WabtInterpreterOpcode::SetLocal: {
        WabtInterpreterValue value = POP();
        PICK(read_u32(&pc)) = value;
        break;
      }

      case WabtInterpreterOpcode::TeeLocal:
        PICK(read_u32(&pc)) = TOP();
        break;

      case WabtInterpreterOpcode::Call: {
        uint32_t offset = read_u32(&pc);
        PUSH_CALL();
        GOTO(offset);
        break;
      }

      case WabtInterpreterOpcode::CallIndirect: {
        uint32_t table_index = read_u32(&pc);
        assert(table_index < env->tables.size);
        WabtInterpreterTable* table = &env->tables.data[table_index];
        uint32_t sig_index = read_u32(&pc);
        assert(sig_index < env->sigs.size);
        VALUE_TYPE_I32 entry_index = POP_I32();
        TRAP_IF(entry_index >= table->func_indexes.size, UndefinedTableIndex);
        uint32_t func_index = table->func_indexes.data[entry_index];
        TRAP_IF(func_index == WABT_INVALID_INDEX, UninitializedTableElement);
        WabtInterpreterFunc* func = &env->funcs.data[func_index];
        TRAP_UNLESS(
            wabt_func_signatures_are_equal(env, func->sig_index, sig_index),
            IndirectCallSignatureMismatch);
        if (func->is_host) {
          wabt_call_host(thread, func);
        } else {
          PUSH_CALL();
          GOTO(func->defined.offset);
        }
        break;
      }

      case WabtInterpreterOpcode::CallHost: {
        uint32_t func_index = read_u32(&pc);
        assert(func_index < env->funcs.size);
        WabtInterpreterFunc* func = &env->funcs.data[func_index];
        wabt_call_host(thread, func);
        break;
      }

      case WabtInterpreterOpcode::I32Load8S:
        LOAD(I32, I8);
        break;

      case WabtInterpreterOpcode::I32Load8U:
        LOAD(I32, U8);
        break;

      case WabtInterpreterOpcode::I32Load16S:
        LOAD(I32, I16);
        break;

      case WabtInterpreterOpcode::I32Load16U:
        LOAD(I32, U16);
        break;

      case WabtInterpreterOpcode::I64Load8S:
        LOAD(I64, I8);
        break;

      case WabtInterpreterOpcode::I64Load8U:
        LOAD(I64, U8);
        break;

      case WabtInterpreterOpcode::I64Load16S:
        LOAD(I64, I16);
        break;

      case WabtInterpreterOpcode::I64Load16U:
        LOAD(I64, U16);
        break;

      case WabtInterpreterOpcode::I64Load32S:
        LOAD(I64, I32);
        break;

      case WabtInterpreterOpcode::I64Load32U:
        LOAD(I64, U32);
        break;

      case WabtInterpreterOpcode::I32Load:
        LOAD(I32, U32);
        break;

      case WabtInterpreterOpcode::I64Load:
        LOAD(I64, U64);
        break;

      case WabtInterpreterOpcode::F32Load:
        LOAD(F32, F32);
        break;

      case WabtInterpreterOpcode::F64Load:
        LOAD(F64, F64);
        break;

      case WabtInterpreterOpcode::I32Store8:
        STORE(I32, U8);
        break;

      case WabtInterpreterOpcode::I32Store16:
        STORE(I32, U16);
        break;

      case WabtInterpreterOpcode::I64Store8:
        STORE(I64, U8);
        break;

      case WabtInterpreterOpcode::I64Store16:
        STORE(I64, U16);
        break;

      case WabtInterpreterOpcode::I64Store32:
        STORE(I64, U32);
        break;

      case WabtInterpreterOpcode::I32Store:
        STORE(I32, U32);
        break;

      case WabtInterpreterOpcode::I64Store:
        STORE(I64, U64);
        break;

      case WabtInterpreterOpcode::F32Store:
        STORE(F32, F32);
        break;

      case WabtInterpreterOpcode::F64Store:
        STORE(F64, F64);
        break;

      case WabtInterpreterOpcode::CurrentMemory: {
        GET_MEMORY(memory);
        PUSH_I32(memory->page_limits.initial);
        break;
      }

      case WabtInterpreterOpcode::GrowMemory: {
        GET_MEMORY(memory);
        uint32_t old_page_size = memory->page_limits.initial;
        uint32_t old_byte_size = memory->byte_size;
        VALUE_TYPE_I32 grow_pages = POP_I32();
        uint32_t new_page_size = old_page_size + grow_pages;
        uint32_t max_page_size = memory->page_limits.has_max
                                     ? memory->page_limits.max
                                     : WABT_MAX_PAGES;
        PUSH_NEG_1_AND_BREAK_IF(new_page_size > max_page_size);
        PUSH_NEG_1_AND_BREAK_IF(
            static_cast<uint64_t>(new_page_size) * WABT_PAGE_SIZE > UINT32_MAX);
        uint32_t new_byte_size = new_page_size * WABT_PAGE_SIZE;
        void* new_data = wabt_realloc(memory->data, new_byte_size);
        PUSH_NEG_1_AND_BREAK_IF(!new_data);
        memset(reinterpret_cast<void*>(reinterpret_cast<intptr_t>(new_data) +
                                       old_byte_size),
               0, new_byte_size - old_byte_size);
        memory->data = new_data;
        memory->page_limits.initial = new_page_size;
        memory->byte_size = new_byte_size;
        PUSH_I32(old_page_size);
        break;
      }

      case WabtInterpreterOpcode::I32Add:
        BINOP(I32, I32, +);
        break;

      case WabtInterpreterOpcode::I32Sub:
        BINOP(I32, I32, -);
        break;

      case WabtInterpreterOpcode::I32Mul:
        BINOP(I32, I32, *);
        break;

      case WabtInterpreterOpcode::I32DivS:
        BINOP_DIV_S(I32);
        break;

      case WabtInterpreterOpcode::I32DivU:
        BINOP_DIV_REM_U(I32, / );
        break;

      case WabtInterpreterOpcode::I32RemS:
        BINOP_REM_S(I32);
        break;

      case WabtInterpreterOpcode::I32RemU:
        BINOP_DIV_REM_U(I32, % );
        break;

      case WabtInterpreterOpcode::I32And:
        BINOP(I32, I32, &);
        break;

      case WabtInterpreterOpcode::I32Or:
        BINOP(I32, I32, | );
        break;

      case WabtInterpreterOpcode::I32Xor:
        BINOP(I32, I32, ^);
        break;

      case WabtInterpreterOpcode::I32Shl:
        BINOP_SHIFT(I32, <<, UNSIGNED);
        break;

      case WabtInterpreterOpcode::I32ShrU:
        BINOP_SHIFT(I32, >>, UNSIGNED);
        break;

      case WabtInterpreterOpcode::I32ShrS:
        BINOP_SHIFT(I32, >>, SIGNED);
        break;

      case WabtInterpreterOpcode::I32Eq:
        BINOP(I32, I32, == );
        break;

      case WabtInterpreterOpcode::I32Ne:
        BINOP(I32, I32, != );
        break;

      case WabtInterpreterOpcode::I32LtS:
        BINOP_SIGNED(I32, I32, < );
        break;

      case WabtInterpreterOpcode::I32LeS:
        BINOP_SIGNED(I32, I32, <= );
        break;

      case WabtInterpreterOpcode::I32LtU:
        BINOP(I32, I32, < );
        break;

      case WabtInterpreterOpcode::I32LeU:
        BINOP(I32, I32, <= );
        break;

      case WabtInterpreterOpcode::I32GtS:
        BINOP_SIGNED(I32, I32, > );
        break;

      case WabtInterpreterOpcode::I32GeS:
        BINOP_SIGNED(I32, I32, >= );
        break;

      case WabtInterpreterOpcode::I32GtU:
        BINOP(I32, I32, > );
        break;

      case WabtInterpreterOpcode::I32GeU:
        BINOP(I32, I32, >= );
        break;

      case WabtInterpreterOpcode::I32Clz: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I32(value != 0 ? wabt_clz_u32(value) : 32);
        break;
      }

      case WabtInterpreterOpcode::I32Ctz: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I32(value != 0 ? wabt_ctz_u32(value) : 32);
        break;
      }

      case WabtInterpreterOpcode::I32Popcnt: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I32(wabt_popcount_u32(value));
        break;
      }

      case WabtInterpreterOpcode::I32Eqz: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I32(value == 0);
        break;
      }

      case WabtInterpreterOpcode::I64Add:
        BINOP(I64, I64, +);
        break;

      case WabtInterpreterOpcode::I64Sub:
        BINOP(I64, I64, -);
        break;

      case WabtInterpreterOpcode::I64Mul:
        BINOP(I64, I64, *);
        break;

      case WabtInterpreterOpcode::I64DivS:
        BINOP_DIV_S(I64);
        break;

      case WabtInterpreterOpcode::I64DivU:
        BINOP_DIV_REM_U(I64, / );
        break;

      case WabtInterpreterOpcode::I64RemS:
        BINOP_REM_S(I64);
        break;

      case WabtInterpreterOpcode::I64RemU:
        BINOP_DIV_REM_U(I64, % );
        break;

      case WabtInterpreterOpcode::I64And:
        BINOP(I64, I64, &);
        break;

      case WabtInterpreterOpcode::I64Or:
        BINOP(I64, I64, | );
        break;

      case WabtInterpreterOpcode::I64Xor:
        BINOP(I64, I64, ^);
        break;

      case WabtInterpreterOpcode::I64Shl:
        BINOP_SHIFT(I64, <<, UNSIGNED);
        break;

      case WabtInterpreterOpcode::I64ShrU:
        BINOP_SHIFT(I64, >>, UNSIGNED);
        break;

      case WabtInterpreterOpcode::I64ShrS:
        BINOP_SHIFT(I64, >>, SIGNED);
        break;

      case WabtInterpreterOpcode::I64Eq:
        BINOP(I32, I64, == );
        break;

      case WabtInterpreterOpcode::I64Ne:
        BINOP(I32, I64, != );
        break;

      case WabtInterpreterOpcode::I64LtS:
        BINOP_SIGNED(I32, I64, < );
        break;

      case WabtInterpreterOpcode::I64LeS:
        BINOP_SIGNED(I32, I64, <= );
        break;

      case WabtInterpreterOpcode::I64LtU:
        BINOP(I32, I64, < );
        break;

      case WabtInterpreterOpcode::I64LeU:
        BINOP(I32, I64, <= );
        break;

      case WabtInterpreterOpcode::I64GtS:
        BINOP_SIGNED(I32, I64, > );
        break;

      case WabtInterpreterOpcode::I64GeS:
        BINOP_SIGNED(I32, I64, >= );
        break;

      case WabtInterpreterOpcode::I64GtU:
        BINOP(I32, I64, > );
        break;

      case WabtInterpreterOpcode::I64GeU:
        BINOP(I32, I64, >= );
        break;

      case WabtInterpreterOpcode::I64Clz: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I64(value != 0 ? wabt_clz_u64(value) : 64);
        break;
      }

      case WabtInterpreterOpcode::I64Ctz: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I64(value != 0 ? wabt_ctz_u64(value) : 64);
        break;
      }

      case WabtInterpreterOpcode::I64Popcnt: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I64(wabt_popcount_u64(value));
        break;
      }

      case WabtInterpreterOpcode::F32Add:
        BINOP_FLOAT(F32, +);
        break;

      case WabtInterpreterOpcode::F32Sub:
        BINOP_FLOAT(F32, -);
        break;

      case WabtInterpreterOpcode::F32Mul:
        BINOP_FLOAT(F32, *);
        break;

      case WabtInterpreterOpcode::F32Div:
        BINOP_FLOAT_DIV(F32);
        break;

      case WabtInterpreterOpcode::F32Min:
        MINMAX_FLOAT(F32, MIN);
        break;

      case WabtInterpreterOpcode::F32Max:
        MINMAX_FLOAT(F32, MAX);
        break;

      case WabtInterpreterOpcode::F32Abs:
        TOP().f32_bits &= ~F32_SIGN_MASK;
        break;

      case WabtInterpreterOpcode::F32Neg:
        TOP().f32_bits ^= F32_SIGN_MASK;
        break;

      case WabtInterpreterOpcode::F32Copysign: {
        VALUE_TYPE_F32 rhs = POP_F32();
        VALUE_TYPE_F32 lhs = POP_F32();
        PUSH_F32((lhs & ~F32_SIGN_MASK) | (rhs & F32_SIGN_MASK));
        break;
      }

      case WabtInterpreterOpcode::F32Ceil:
        UNOP_FLOAT(F32, ceilf);
        break;

      case WabtInterpreterOpcode::F32Floor:
        UNOP_FLOAT(F32, floorf);
        break;

      case WabtInterpreterOpcode::F32Trunc:
        UNOP_FLOAT(F32, truncf);
        break;

      case WabtInterpreterOpcode::F32Nearest:
        UNOP_FLOAT(F32, nearbyintf);
        break;

      case WabtInterpreterOpcode::F32Sqrt:
        UNOP_FLOAT(F32, sqrtf);
        break;

      case WabtInterpreterOpcode::F32Eq:
        BINOP_FLOAT_COMPARE(F32, == );
        break;

      case WabtInterpreterOpcode::F32Ne:
        BINOP_FLOAT_COMPARE(F32, != );
        break;

      case WabtInterpreterOpcode::F32Lt:
        BINOP_FLOAT_COMPARE(F32, < );
        break;

      case WabtInterpreterOpcode::F32Le:
        BINOP_FLOAT_COMPARE(F32, <= );
        break;

      case WabtInterpreterOpcode::F32Gt:
        BINOP_FLOAT_COMPARE(F32, > );
        break;

      case WabtInterpreterOpcode::F32Ge:
        BINOP_FLOAT_COMPARE(F32, >= );
        break;

      case WabtInterpreterOpcode::F64Add:
        BINOP_FLOAT(F64, +);
        break;

      case WabtInterpreterOpcode::F64Sub:
        BINOP_FLOAT(F64, -);
        break;

      case WabtInterpreterOpcode::F64Mul:
        BINOP_FLOAT(F64, *);
        break;

      case WabtInterpreterOpcode::F64Div:
        BINOP_FLOAT_DIV(F64);
        break;

      case WabtInterpreterOpcode::F64Min:
        MINMAX_FLOAT(F64, MIN);
        break;

      case WabtInterpreterOpcode::F64Max:
        MINMAX_FLOAT(F64, MAX);
        break;

      case WabtInterpreterOpcode::F64Abs:
        TOP().f64_bits &= ~F64_SIGN_MASK;
        break;

      case WabtInterpreterOpcode::F64Neg:
        TOP().f64_bits ^= F64_SIGN_MASK;
        break;

      case WabtInterpreterOpcode::F64Copysign: {
        VALUE_TYPE_F64 rhs = POP_F64();
        VALUE_TYPE_F64 lhs = POP_F64();
        PUSH_F64((lhs & ~F64_SIGN_MASK) | (rhs & F64_SIGN_MASK));
        break;
      }

      case WabtInterpreterOpcode::F64Ceil:
        UNOP_FLOAT(F64, ceil);
        break;

      case WabtInterpreterOpcode::F64Floor:
        UNOP_FLOAT(F64, floor);
        break;

      case WabtInterpreterOpcode::F64Trunc:
        UNOP_FLOAT(F64, trunc);
        break;

      case WabtInterpreterOpcode::F64Nearest:
        UNOP_FLOAT(F64, nearbyint);
        break;

      case WabtInterpreterOpcode::F64Sqrt:
        UNOP_FLOAT(F64, sqrt);
        break;

      case WabtInterpreterOpcode::F64Eq:
        BINOP_FLOAT_COMPARE(F64, == );
        break;

      case WabtInterpreterOpcode::F64Ne:
        BINOP_FLOAT_COMPARE(F64, != );
        break;

      case WabtInterpreterOpcode::F64Lt:
        BINOP_FLOAT_COMPARE(F64, < );
        break;

      case WabtInterpreterOpcode::F64Le:
        BINOP_FLOAT_COMPARE(F64, <= );
        break;

      case WabtInterpreterOpcode::F64Gt:
        BINOP_FLOAT_COMPARE(F64, > );
        break;

      case WabtInterpreterOpcode::F64Ge:
        BINOP_FLOAT_COMPARE(F64, >= );
        break;

      case WabtInterpreterOpcode::I32TruncSF32: {
        VALUE_TYPE_F32 value = POP_F32();
        TRAP_IF(wabt_is_nan_f32(value), InvalidConversionToInteger);
        TRAP_UNLESS(is_in_range_i32_trunc_s_f32(value), IntegerOverflow);
        PUSH_I32(static_cast<int32_t>(BITCAST_TO_F32(value)));
        break;
      }

      case WabtInterpreterOpcode::I32TruncSF64: {
        VALUE_TYPE_F64 value = POP_F64();
        TRAP_IF(wabt_is_nan_f64(value), InvalidConversionToInteger);
        TRAP_UNLESS(is_in_range_i32_trunc_s_f64(value), IntegerOverflow);
        PUSH_I32(static_cast<int32_t>(BITCAST_TO_F64(value)));
        break;
      }

      case WabtInterpreterOpcode::I32TruncUF32: {
        VALUE_TYPE_F32 value = POP_F32();
        TRAP_IF(wabt_is_nan_f32(value), InvalidConversionToInteger);
        TRAP_UNLESS(is_in_range_i32_trunc_u_f32(value), IntegerOverflow);
        PUSH_I32(static_cast<uint32_t>(BITCAST_TO_F32(value)));
        break;
      }

      case WabtInterpreterOpcode::I32TruncUF64: {
        VALUE_TYPE_F64 value = POP_F64();
        TRAP_IF(wabt_is_nan_f64(value), InvalidConversionToInteger);
        TRAP_UNLESS(is_in_range_i32_trunc_u_f64(value), IntegerOverflow);
        PUSH_I32(static_cast<uint32_t>(BITCAST_TO_F64(value)));
        break;
      }

      case WabtInterpreterOpcode::I32WrapI64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I32(static_cast<uint32_t>(value));
        break;
      }

      case WabtInterpreterOpcode::I64TruncSF32: {
        VALUE_TYPE_F32 value = POP_F32();
        TRAP_IF(wabt_is_nan_f32(value), InvalidConversionToInteger);
        TRAP_UNLESS(is_in_range_i64_trunc_s_f32(value), IntegerOverflow);
        PUSH_I64(static_cast<int64_t>(BITCAST_TO_F32(value)));
        break;
      }

      case WabtInterpreterOpcode::I64TruncSF64: {
        VALUE_TYPE_F64 value = POP_F64();
        TRAP_IF(wabt_is_nan_f64(value), InvalidConversionToInteger);
        TRAP_UNLESS(is_in_range_i64_trunc_s_f64(value), IntegerOverflow);
        PUSH_I64(static_cast<int64_t>(BITCAST_TO_F64(value)));
        break;
      }

      case WabtInterpreterOpcode::I64TruncUF32: {
        VALUE_TYPE_F32 value = POP_F32();
        TRAP_IF(wabt_is_nan_f32(value), InvalidConversionToInteger);
        TRAP_UNLESS(is_in_range_i64_trunc_u_f32(value), IntegerOverflow);
        PUSH_I64(static_cast<uint64_t>(BITCAST_TO_F32(value)));
        break;
      }

      case WabtInterpreterOpcode::I64TruncUF64: {
        VALUE_TYPE_F64 value = POP_F64();
        TRAP_IF(wabt_is_nan_f64(value), InvalidConversionToInteger);
        TRAP_UNLESS(is_in_range_i64_trunc_u_f64(value), IntegerOverflow);
        PUSH_I64(static_cast<uint64_t>(BITCAST_TO_F64(value)));
        break;
      }

      case WabtInterpreterOpcode::I64ExtendSI32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I64(static_cast<int64_t>(BITCAST_I32_TO_SIGNED(value)));
        break;
      }

      case WabtInterpreterOpcode::I64ExtendUI32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_I64(static_cast<uint64_t>(value));
        break;
      }

      case WabtInterpreterOpcode::F32ConvertSI32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F32(
            BITCAST_FROM_F32(static_cast<float>(BITCAST_I32_TO_SIGNED(value))));
        break;
      }

      case WabtInterpreterOpcode::F32ConvertUI32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F32(BITCAST_FROM_F32(static_cast<float>(value)));
        break;
      }

      case WabtInterpreterOpcode::F32ConvertSI64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F32(
            BITCAST_FROM_F32(static_cast<float>(BITCAST_I64_TO_SIGNED(value))));
        break;
      }

      case WabtInterpreterOpcode::F32ConvertUI64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F32(BITCAST_FROM_F32(static_cast<float>(value)));
        break;
      }

      case WabtInterpreterOpcode::F32DemoteF64: {
        VALUE_TYPE_F64 value = POP_F64();
        if (WABT_LIKELY(is_in_range_f64_demote_f32(value))) {
          PUSH_F32(BITCAST_FROM_F32(static_cast<float>(BITCAST_TO_F64(value))));
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

      case WabtInterpreterOpcode::F32ReinterpretI32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F32(value);
        break;
      }

      case WabtInterpreterOpcode::F64ConvertSI32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F64(BITCAST_FROM_F64(
            static_cast<double>(BITCAST_I32_TO_SIGNED(value))));
        break;
      }

      case WabtInterpreterOpcode::F64ConvertUI32: {
        VALUE_TYPE_I32 value = POP_I32();
        PUSH_F64(BITCAST_FROM_F64(static_cast<double>(value)));
        break;
      }

      case WabtInterpreterOpcode::F64ConvertSI64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F64(BITCAST_FROM_F64(
            static_cast<double>(BITCAST_I64_TO_SIGNED(value))));
        break;
      }

      case WabtInterpreterOpcode::F64ConvertUI64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F64(BITCAST_FROM_F64(static_cast<double>(value)));
        break;
      }

      case WabtInterpreterOpcode::F64PromoteF32: {
        VALUE_TYPE_F32 value = POP_F32();
        PUSH_F64(BITCAST_FROM_F64(static_cast<double>(BITCAST_TO_F32(value))));
        break;
      }

      case WabtInterpreterOpcode::F64ReinterpretI64: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_F64(value);
        break;
      }

      case WabtInterpreterOpcode::I32ReinterpretF32: {
        VALUE_TYPE_F32 value = POP_F32();
        PUSH_I32(value);
        break;
      }

      case WabtInterpreterOpcode::I64ReinterpretF64: {
        VALUE_TYPE_F64 value = POP_F64();
        PUSH_I64(value);
        break;
      }

      case WabtInterpreterOpcode::I32Rotr:
        BINOP_ROT(I32, RIGHT);
        break;

      case WabtInterpreterOpcode::I32Rotl:
        BINOP_ROT(I32, LEFT);
        break;

      case WabtInterpreterOpcode::I64Rotr:
        BINOP_ROT(I64, RIGHT);
        break;

      case WabtInterpreterOpcode::I64Rotl:
        BINOP_ROT(I64, LEFT);
        break;

      case WabtInterpreterOpcode::I64Eqz: {
        VALUE_TYPE_I64 value = POP_I64();
        PUSH_I64(value == 0);
        break;
      }

      case WabtInterpreterOpcode::Alloca: {
        WabtInterpreterValue* old_value_stack_top = thread->value_stack_top;
        thread->value_stack_top += read_u32(&pc);
        CHECK_STACK();
        memset(old_value_stack_top, 0,
               (thread->value_stack_top - old_value_stack_top) *
                   sizeof(WabtInterpreterValue));
        break;
      }

      case WabtInterpreterOpcode::BrUnless: {
        uint32_t new_pc = read_u32(&pc);
        if (!POP_I32())
          GOTO(new_pc);
        break;
      }

      case WabtInterpreterOpcode::Drop:
        (void)POP();
        break;

      case WabtInterpreterOpcode::DropKeep: {
        uint32_t drop_count = read_u32(&pc);
        uint8_t keep_count = *pc++;
        DROP_KEEP(drop_count, keep_count);
        break;
      }

      case WabtInterpreterOpcode::Data:
        /* shouldn't ever execute this */
        assert(0);
        break;

      case WabtInterpreterOpcode::Nop:
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
  const uint8_t* istream =
      static_cast<const uint8_t*>(thread->env->istream.start);
  const uint8_t* pc = &istream[thread->pc];
  size_t value_stack_depth = thread->value_stack_top - thread->value_stack.data;
  size_t call_stack_depth = thread->call_stack_top - thread->call_stack.data;

  wabt_writef(stream, "#%" PRIzd ". %4" PRIzd ": V:%-3" PRIzd "| ",
              call_stack_depth,
              pc - static_cast<uint8_t*>(thread->env->istream.start),
              value_stack_depth);

  WabtInterpreterOpcode opcode = static_cast<WabtInterpreterOpcode>(*pc++);
  switch (opcode) {
    case WabtInterpreterOpcode::Select:
      wabt_writef(stream, "%s %u, %" PRIu64 ", %" PRIu64 "\n",
                  wabt_get_interpreter_opcode_name(opcode), PICK(3).i32,
                  PICK(2).i64, PICK(1).i64);
      break;

    case WabtInterpreterOpcode::Br:
      wabt_writef(stream, "%s @%u\n", wabt_get_interpreter_opcode_name(opcode),
                  read_u32_at(pc));
      break;

    case WabtInterpreterOpcode::BrIf:
      wabt_writef(stream, "%s @%u, %u\n",
                  wabt_get_interpreter_opcode_name(opcode), read_u32_at(pc),
                  TOP().i32);
      break;

    case WabtInterpreterOpcode::BrTable: {
      uint32_t num_targets = read_u32_at(pc);
      uint32_t table_offset = read_u32_at(pc + 4);
      VALUE_TYPE_I32 key = TOP().i32;
      wabt_writef(stream, "%s %u, $#%u, table:$%u\n",
                  wabt_get_interpreter_opcode_name(opcode), key, num_targets,
                  table_offset);
      break;
    }

    case WabtInterpreterOpcode::Nop:
    case WabtInterpreterOpcode::Return:
    case WabtInterpreterOpcode::Unreachable:
    case WabtInterpreterOpcode::Drop:
      wabt_writef(stream, "%s\n", wabt_get_interpreter_opcode_name(opcode));
      break;

    case WabtInterpreterOpcode::CurrentMemory: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u\n", wabt_get_interpreter_opcode_name(opcode),
                  memory_index);
      break;
    }

    case WabtInterpreterOpcode::I32Const:
      wabt_writef(stream, "%s $%u\n", wabt_get_interpreter_opcode_name(opcode),
                  read_u32_at(pc));
      break;

    case WabtInterpreterOpcode::I64Const:
      wabt_writef(stream, "%s $%" PRIu64 "\n",
                  wabt_get_interpreter_opcode_name(opcode), read_u64_at(pc));
      break;

    case WabtInterpreterOpcode::F32Const:
      wabt_writef(stream, "%s $%g\n", wabt_get_interpreter_opcode_name(opcode),
                  bitcast_u32_to_f32(read_u32_at(pc)));
      break;

    case WabtInterpreterOpcode::F64Const:
      wabt_writef(stream, "%s $%g\n", wabt_get_interpreter_opcode_name(opcode),
                  bitcast_u64_to_f64(read_u64_at(pc)));
      break;

    case WabtInterpreterOpcode::GetLocal:
    case WabtInterpreterOpcode::GetGlobal:
      wabt_writef(stream, "%s $%u\n", wabt_get_interpreter_opcode_name(opcode),
                  read_u32_at(pc));
      break;

    case WabtInterpreterOpcode::SetLocal:
    case WabtInterpreterOpcode::SetGlobal:
    case WabtInterpreterOpcode::TeeLocal:
      wabt_writef(stream, "%s $%u, %u\n",
                  wabt_get_interpreter_opcode_name(opcode), read_u32_at(pc),
                  TOP().i32);
      break;

    case WabtInterpreterOpcode::Call:
      wabt_writef(stream, "%s @%u\n", wabt_get_interpreter_opcode_name(opcode),
                  read_u32_at(pc));
      break;

    case WabtInterpreterOpcode::CallIndirect:
      wabt_writef(stream, "%s $%u, %u\n",
                  wabt_get_interpreter_opcode_name(opcode), read_u32_at(pc),
                  TOP().i32);
      break;

    case WabtInterpreterOpcode::CallHost:
      wabt_writef(stream, "%s $%u\n", wabt_get_interpreter_opcode_name(opcode),
                  read_u32_at(pc));
      break;

    case WabtInterpreterOpcode::I32Load8S:
    case WabtInterpreterOpcode::I32Load8U:
    case WabtInterpreterOpcode::I32Load16S:
    case WabtInterpreterOpcode::I32Load16U:
    case WabtInterpreterOpcode::I64Load8S:
    case WabtInterpreterOpcode::I64Load8U:
    case WabtInterpreterOpcode::I64Load16S:
    case WabtInterpreterOpcode::I64Load16U:
    case WabtInterpreterOpcode::I64Load32S:
    case WabtInterpreterOpcode::I64Load32U:
    case WabtInterpreterOpcode::I32Load:
    case WabtInterpreterOpcode::I64Load:
    case WabtInterpreterOpcode::F32Load:
    case WabtInterpreterOpcode::F64Load: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u:%u+$%u\n",
                  wabt_get_interpreter_opcode_name(opcode), memory_index,
                  TOP().i32, read_u32_at(pc));
      break;
    }

    case WabtInterpreterOpcode::I32Store8:
    case WabtInterpreterOpcode::I32Store16:
    case WabtInterpreterOpcode::I32Store: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u:%u+$%u, %u\n",
                  wabt_get_interpreter_opcode_name(opcode), memory_index,
                  PICK(2).i32, read_u32_at(pc), PICK(1).i32);
      break;
    }

    case WabtInterpreterOpcode::I64Store8:
    case WabtInterpreterOpcode::I64Store16:
    case WabtInterpreterOpcode::I64Store32:
    case WabtInterpreterOpcode::I64Store: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u:%u+$%u, %" PRIu64 "\n",
                  wabt_get_interpreter_opcode_name(opcode), memory_index,
                  PICK(2).i32, read_u32_at(pc), PICK(1).i64);
      break;
    }

    case WabtInterpreterOpcode::F32Store: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u:%u+$%u, %g\n",
                  wabt_get_interpreter_opcode_name(opcode), memory_index,
                  PICK(2).i32, read_u32_at(pc),
                  bitcast_u32_to_f32(PICK(1).f32_bits));
      break;
    }

    case WabtInterpreterOpcode::F64Store: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u:%u+$%u, %g\n",
                  wabt_get_interpreter_opcode_name(opcode), memory_index,
                  PICK(2).i32, read_u32_at(pc),
                  bitcast_u64_to_f64(PICK(1).f64_bits));
      break;
    }

    case WabtInterpreterOpcode::GrowMemory: {
      uint32_t memory_index = read_u32(&pc);
      wabt_writef(stream, "%s $%u:%u\n",
                  wabt_get_interpreter_opcode_name(opcode), memory_index,
                  TOP().i32);
      break;
    }

    case WabtInterpreterOpcode::I32Add:
    case WabtInterpreterOpcode::I32Sub:
    case WabtInterpreterOpcode::I32Mul:
    case WabtInterpreterOpcode::I32DivS:
    case WabtInterpreterOpcode::I32DivU:
    case WabtInterpreterOpcode::I32RemS:
    case WabtInterpreterOpcode::I32RemU:
    case WabtInterpreterOpcode::I32And:
    case WabtInterpreterOpcode::I32Or:
    case WabtInterpreterOpcode::I32Xor:
    case WabtInterpreterOpcode::I32Shl:
    case WabtInterpreterOpcode::I32ShrU:
    case WabtInterpreterOpcode::I32ShrS:
    case WabtInterpreterOpcode::I32Eq:
    case WabtInterpreterOpcode::I32Ne:
    case WabtInterpreterOpcode::I32LtS:
    case WabtInterpreterOpcode::I32LeS:
    case WabtInterpreterOpcode::I32LtU:
    case WabtInterpreterOpcode::I32LeU:
    case WabtInterpreterOpcode::I32GtS:
    case WabtInterpreterOpcode::I32GeS:
    case WabtInterpreterOpcode::I32GtU:
    case WabtInterpreterOpcode::I32GeU:
    case WabtInterpreterOpcode::I32Rotr:
    case WabtInterpreterOpcode::I32Rotl:
      wabt_writef(stream, "%s %u, %u\n",
                  wabt_get_interpreter_opcode_name(opcode), PICK(2).i32,
                  PICK(1).i32);
      break;

    case WabtInterpreterOpcode::I32Clz:
    case WabtInterpreterOpcode::I32Ctz:
    case WabtInterpreterOpcode::I32Popcnt:
    case WabtInterpreterOpcode::I32Eqz:
      wabt_writef(stream, "%s %u\n", wabt_get_interpreter_opcode_name(opcode),
                  TOP().i32);
      break;

    case WabtInterpreterOpcode::I64Add:
    case WabtInterpreterOpcode::I64Sub:
    case WabtInterpreterOpcode::I64Mul:
    case WabtInterpreterOpcode::I64DivS:
    case WabtInterpreterOpcode::I64DivU:
    case WabtInterpreterOpcode::I64RemS:
    case WabtInterpreterOpcode::I64RemU:
    case WabtInterpreterOpcode::I64And:
    case WabtInterpreterOpcode::I64Or:
    case WabtInterpreterOpcode::I64Xor:
    case WabtInterpreterOpcode::I64Shl:
    case WabtInterpreterOpcode::I64ShrU:
    case WabtInterpreterOpcode::I64ShrS:
    case WabtInterpreterOpcode::I64Eq:
    case WabtInterpreterOpcode::I64Ne:
    case WabtInterpreterOpcode::I64LtS:
    case WabtInterpreterOpcode::I64LeS:
    case WabtInterpreterOpcode::I64LtU:
    case WabtInterpreterOpcode::I64LeU:
    case WabtInterpreterOpcode::I64GtS:
    case WabtInterpreterOpcode::I64GeS:
    case WabtInterpreterOpcode::I64GtU:
    case WabtInterpreterOpcode::I64GeU:
    case WabtInterpreterOpcode::I64Rotr:
    case WabtInterpreterOpcode::I64Rotl:
      wabt_writef(stream, "%s %" PRIu64 ", %" PRIu64 "\n",
                  wabt_get_interpreter_opcode_name(opcode), PICK(2).i64,
                  PICK(1).i64);
      break;

    case WabtInterpreterOpcode::I64Clz:
    case WabtInterpreterOpcode::I64Ctz:
    case WabtInterpreterOpcode::I64Popcnt:
    case WabtInterpreterOpcode::I64Eqz:
      wabt_writef(stream, "%s %" PRIu64 "\n",
                  wabt_get_interpreter_opcode_name(opcode), TOP().i64);
      break;

    case WabtInterpreterOpcode::F32Add:
    case WabtInterpreterOpcode::F32Sub:
    case WabtInterpreterOpcode::F32Mul:
    case WabtInterpreterOpcode::F32Div:
    case WabtInterpreterOpcode::F32Min:
    case WabtInterpreterOpcode::F32Max:
    case WabtInterpreterOpcode::F32Copysign:
    case WabtInterpreterOpcode::F32Eq:
    case WabtInterpreterOpcode::F32Ne:
    case WabtInterpreterOpcode::F32Lt:
    case WabtInterpreterOpcode::F32Le:
    case WabtInterpreterOpcode::F32Gt:
    case WabtInterpreterOpcode::F32Ge:
      wabt_writef(
          stream, "%s %g, %g\n", wabt_get_interpreter_opcode_name(opcode),
          bitcast_u32_to_f32(PICK(2).i32), bitcast_u32_to_f32(PICK(1).i32));
      break;

    case WabtInterpreterOpcode::F32Abs:
    case WabtInterpreterOpcode::F32Neg:
    case WabtInterpreterOpcode::F32Ceil:
    case WabtInterpreterOpcode::F32Floor:
    case WabtInterpreterOpcode::F32Trunc:
    case WabtInterpreterOpcode::F32Nearest:
    case WabtInterpreterOpcode::F32Sqrt:
      wabt_writef(stream, "%s %g\n", wabt_get_interpreter_opcode_name(opcode),
                  bitcast_u32_to_f32(TOP().i32));
      break;

    case WabtInterpreterOpcode::F64Add:
    case WabtInterpreterOpcode::F64Sub:
    case WabtInterpreterOpcode::F64Mul:
    case WabtInterpreterOpcode::F64Div:
    case WabtInterpreterOpcode::F64Min:
    case WabtInterpreterOpcode::F64Max:
    case WabtInterpreterOpcode::F64Copysign:
    case WabtInterpreterOpcode::F64Eq:
    case WabtInterpreterOpcode::F64Ne:
    case WabtInterpreterOpcode::F64Lt:
    case WabtInterpreterOpcode::F64Le:
    case WabtInterpreterOpcode::F64Gt:
    case WabtInterpreterOpcode::F64Ge:
      wabt_writef(
          stream, "%s %g, %g\n", wabt_get_interpreter_opcode_name(opcode),
          bitcast_u64_to_f64(PICK(2).i64), bitcast_u64_to_f64(PICK(1).i64));
      break;

    case WabtInterpreterOpcode::F64Abs:
    case WabtInterpreterOpcode::F64Neg:
    case WabtInterpreterOpcode::F64Ceil:
    case WabtInterpreterOpcode::F64Floor:
    case WabtInterpreterOpcode::F64Trunc:
    case WabtInterpreterOpcode::F64Nearest:
    case WabtInterpreterOpcode::F64Sqrt:
      wabt_writef(stream, "%s %g\n", wabt_get_interpreter_opcode_name(opcode),
                  bitcast_u64_to_f64(TOP().i64));
      break;

    case WabtInterpreterOpcode::I32TruncSF32:
    case WabtInterpreterOpcode::I32TruncUF32:
    case WabtInterpreterOpcode::I64TruncSF32:
    case WabtInterpreterOpcode::I64TruncUF32:
    case WabtInterpreterOpcode::F64PromoteF32:
    case WabtInterpreterOpcode::I32ReinterpretF32:
      wabt_writef(stream, "%s %g\n", wabt_get_interpreter_opcode_name(opcode),
                  bitcast_u32_to_f32(TOP().i32));
      break;

    case WabtInterpreterOpcode::I32TruncSF64:
    case WabtInterpreterOpcode::I32TruncUF64:
    case WabtInterpreterOpcode::I64TruncSF64:
    case WabtInterpreterOpcode::I64TruncUF64:
    case WabtInterpreterOpcode::F32DemoteF64:
    case WabtInterpreterOpcode::I64ReinterpretF64:
      wabt_writef(stream, "%s %g\n", wabt_get_interpreter_opcode_name(opcode),
                  bitcast_u64_to_f64(TOP().i64));
      break;

    case WabtInterpreterOpcode::I32WrapI64:
    case WabtInterpreterOpcode::F32ConvertSI64:
    case WabtInterpreterOpcode::F32ConvertUI64:
    case WabtInterpreterOpcode::F64ConvertSI64:
    case WabtInterpreterOpcode::F64ConvertUI64:
    case WabtInterpreterOpcode::F64ReinterpretI64:
      wabt_writef(stream, "%s %" PRIu64 "\n",
                  wabt_get_interpreter_opcode_name(opcode), TOP().i64);
      break;

    case WabtInterpreterOpcode::I64ExtendSI32:
    case WabtInterpreterOpcode::I64ExtendUI32:
    case WabtInterpreterOpcode::F32ConvertSI32:
    case WabtInterpreterOpcode::F32ConvertUI32:
    case WabtInterpreterOpcode::F32ReinterpretI32:
    case WabtInterpreterOpcode::F64ConvertSI32:
    case WabtInterpreterOpcode::F64ConvertUI32:
      wabt_writef(stream, "%s %u\n", wabt_get_interpreter_opcode_name(opcode),
                  TOP().i32);
      break;

    case WabtInterpreterOpcode::Alloca:
      wabt_writef(stream, "%s $%u\n", wabt_get_interpreter_opcode_name(opcode),
                  read_u32_at(pc));
      break;

    case WabtInterpreterOpcode::BrUnless:
      wabt_writef(stream, "%s @%u, %u\n",
                  wabt_get_interpreter_opcode_name(opcode), read_u32_at(pc),
                  TOP().i32);
      break;

    case WabtInterpreterOpcode::DropKeep:
      wabt_writef(stream, "%s $%u $%u\n",
                  wabt_get_interpreter_opcode_name(opcode), read_u32_at(pc),
                  *(pc + 4));
      break;

    case WabtInterpreterOpcode::Data:
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
  const uint8_t* istream = static_cast<const uint8_t*>(env->istream.start);
  const uint8_t* pc = &istream[from];

  while (static_cast<uint32_t>(pc - istream) < to) {
    wabt_writef(stream, "%4" PRIzd "| ", pc - istream);

    WabtInterpreterOpcode opcode = static_cast<WabtInterpreterOpcode>(*pc++);
    switch (opcode) {
      case WabtInterpreterOpcode::Select:
        wabt_writef(stream, "%s %%[-3], %%[-2], %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode));
        break;

      case WabtInterpreterOpcode::Br:
        wabt_writef(stream, "%s @%u\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WabtInterpreterOpcode::BrIf:
        wabt_writef(stream, "%s @%u, %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WabtInterpreterOpcode::BrTable: {
        uint32_t num_targets = read_u32(&pc);
        uint32_t table_offset = read_u32(&pc);
        wabt_writef(stream, "%s %%[-1], $#%u, table:$%u\n",
                    wabt_get_interpreter_opcode_name(opcode), num_targets,
                    table_offset);
        break;
      }

      case WabtInterpreterOpcode::Nop:
      case WabtInterpreterOpcode::Return:
      case WabtInterpreterOpcode::Unreachable:
      case WabtInterpreterOpcode::Drop:
        wabt_writef(stream, "%s\n", wabt_get_interpreter_opcode_name(opcode));
        break;

      case WabtInterpreterOpcode::CurrentMemory: {
        uint32_t memory_index = read_u32(&pc);
        wabt_writef(stream, "%s $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), memory_index);
        break;
      }

      case WabtInterpreterOpcode::I32Const:
        wabt_writef(stream, "%s $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WabtInterpreterOpcode::I64Const:
        wabt_writef(stream, "%s $%" PRIu64 "\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u64(&pc));
        break;

      case WabtInterpreterOpcode::F32Const:
        wabt_writef(stream, "%s $%g\n",
                    wabt_get_interpreter_opcode_name(opcode),
                    bitcast_u32_to_f32(read_u32(&pc)));
        break;

      case WabtInterpreterOpcode::F64Const:
        wabt_writef(stream, "%s $%g\n",
                    wabt_get_interpreter_opcode_name(opcode),
                    bitcast_u64_to_f64(read_u64(&pc)));
        break;

      case WabtInterpreterOpcode::GetLocal:
      case WabtInterpreterOpcode::GetGlobal:
        wabt_writef(stream, "%s $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WabtInterpreterOpcode::SetLocal:
      case WabtInterpreterOpcode::SetGlobal:
      case WabtInterpreterOpcode::TeeLocal:
        wabt_writef(stream, "%s $%u, %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WabtInterpreterOpcode::Call:
        wabt_writef(stream, "%s @%u\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WabtInterpreterOpcode::CallIndirect: {
        uint32_t table_index = read_u32(&pc);
        wabt_writef(stream, "%s $%u:%u, %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode), table_index,
                    read_u32(&pc));
        break;
      }

      case WabtInterpreterOpcode::CallHost:
        wabt_writef(stream, "%s $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WabtInterpreterOpcode::I32Load8S:
      case WabtInterpreterOpcode::I32Load8U:
      case WabtInterpreterOpcode::I32Load16S:
      case WabtInterpreterOpcode::I32Load16U:
      case WabtInterpreterOpcode::I64Load8S:
      case WabtInterpreterOpcode::I64Load8U:
      case WabtInterpreterOpcode::I64Load16S:
      case WabtInterpreterOpcode::I64Load16U:
      case WabtInterpreterOpcode::I64Load32S:
      case WabtInterpreterOpcode::I64Load32U:
      case WabtInterpreterOpcode::I32Load:
      case WabtInterpreterOpcode::I64Load:
      case WabtInterpreterOpcode::F32Load:
      case WabtInterpreterOpcode::F64Load: {
        uint32_t memory_index = read_u32(&pc);
        wabt_writef(stream, "%s $%u:%%[-1]+$%u\n",
                    wabt_get_interpreter_opcode_name(opcode), memory_index,
                    read_u32(&pc));
        break;
      }

      case WabtInterpreterOpcode::I32Store8:
      case WabtInterpreterOpcode::I32Store16:
      case WabtInterpreterOpcode::I32Store:
      case WabtInterpreterOpcode::I64Store8:
      case WabtInterpreterOpcode::I64Store16:
      case WabtInterpreterOpcode::I64Store32:
      case WabtInterpreterOpcode::I64Store:
      case WabtInterpreterOpcode::F32Store:
      case WabtInterpreterOpcode::F64Store: {
        uint32_t memory_index = read_u32(&pc);
        wabt_writef(stream, "%s %%[-2]+$%u, $%u:%%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode), memory_index,
                    read_u32(&pc));
        break;
      }

      case WabtInterpreterOpcode::I32Add:
      case WabtInterpreterOpcode::I32Sub:
      case WabtInterpreterOpcode::I32Mul:
      case WabtInterpreterOpcode::I32DivS:
      case WabtInterpreterOpcode::I32DivU:
      case WabtInterpreterOpcode::I32RemS:
      case WabtInterpreterOpcode::I32RemU:
      case WabtInterpreterOpcode::I32And:
      case WabtInterpreterOpcode::I32Or:
      case WabtInterpreterOpcode::I32Xor:
      case WabtInterpreterOpcode::I32Shl:
      case WabtInterpreterOpcode::I32ShrU:
      case WabtInterpreterOpcode::I32ShrS:
      case WabtInterpreterOpcode::I32Eq:
      case WabtInterpreterOpcode::I32Ne:
      case WabtInterpreterOpcode::I32LtS:
      case WabtInterpreterOpcode::I32LeS:
      case WabtInterpreterOpcode::I32LtU:
      case WabtInterpreterOpcode::I32LeU:
      case WabtInterpreterOpcode::I32GtS:
      case WabtInterpreterOpcode::I32GeS:
      case WabtInterpreterOpcode::I32GtU:
      case WabtInterpreterOpcode::I32GeU:
      case WabtInterpreterOpcode::I32Rotr:
      case WabtInterpreterOpcode::I32Rotl:
      case WabtInterpreterOpcode::F32Add:
      case WabtInterpreterOpcode::F32Sub:
      case WabtInterpreterOpcode::F32Mul:
      case WabtInterpreterOpcode::F32Div:
      case WabtInterpreterOpcode::F32Min:
      case WabtInterpreterOpcode::F32Max:
      case WabtInterpreterOpcode::F32Copysign:
      case WabtInterpreterOpcode::F32Eq:
      case WabtInterpreterOpcode::F32Ne:
      case WabtInterpreterOpcode::F32Lt:
      case WabtInterpreterOpcode::F32Le:
      case WabtInterpreterOpcode::F32Gt:
      case WabtInterpreterOpcode::F32Ge:
      case WabtInterpreterOpcode::I64Add:
      case WabtInterpreterOpcode::I64Sub:
      case WabtInterpreterOpcode::I64Mul:
      case WabtInterpreterOpcode::I64DivS:
      case WabtInterpreterOpcode::I64DivU:
      case WabtInterpreterOpcode::I64RemS:
      case WabtInterpreterOpcode::I64RemU:
      case WabtInterpreterOpcode::I64And:
      case WabtInterpreterOpcode::I64Or:
      case WabtInterpreterOpcode::I64Xor:
      case WabtInterpreterOpcode::I64Shl:
      case WabtInterpreterOpcode::I64ShrU:
      case WabtInterpreterOpcode::I64ShrS:
      case WabtInterpreterOpcode::I64Eq:
      case WabtInterpreterOpcode::I64Ne:
      case WabtInterpreterOpcode::I64LtS:
      case WabtInterpreterOpcode::I64LeS:
      case WabtInterpreterOpcode::I64LtU:
      case WabtInterpreterOpcode::I64LeU:
      case WabtInterpreterOpcode::I64GtS:
      case WabtInterpreterOpcode::I64GeS:
      case WabtInterpreterOpcode::I64GtU:
      case WabtInterpreterOpcode::I64GeU:
      case WabtInterpreterOpcode::I64Rotr:
      case WabtInterpreterOpcode::I64Rotl:
      case WabtInterpreterOpcode::F64Add:
      case WabtInterpreterOpcode::F64Sub:
      case WabtInterpreterOpcode::F64Mul:
      case WabtInterpreterOpcode::F64Div:
      case WabtInterpreterOpcode::F64Min:
      case WabtInterpreterOpcode::F64Max:
      case WabtInterpreterOpcode::F64Copysign:
      case WabtInterpreterOpcode::F64Eq:
      case WabtInterpreterOpcode::F64Ne:
      case WabtInterpreterOpcode::F64Lt:
      case WabtInterpreterOpcode::F64Le:
      case WabtInterpreterOpcode::F64Gt:
      case WabtInterpreterOpcode::F64Ge:
        wabt_writef(stream, "%s %%[-2], %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode));
        break;

      case WabtInterpreterOpcode::I32Clz:
      case WabtInterpreterOpcode::I32Ctz:
      case WabtInterpreterOpcode::I32Popcnt:
      case WabtInterpreterOpcode::I32Eqz:
      case WabtInterpreterOpcode::I64Clz:
      case WabtInterpreterOpcode::I64Ctz:
      case WabtInterpreterOpcode::I64Popcnt:
      case WabtInterpreterOpcode::I64Eqz:
      case WabtInterpreterOpcode::F32Abs:
      case WabtInterpreterOpcode::F32Neg:
      case WabtInterpreterOpcode::F32Ceil:
      case WabtInterpreterOpcode::F32Floor:
      case WabtInterpreterOpcode::F32Trunc:
      case WabtInterpreterOpcode::F32Nearest:
      case WabtInterpreterOpcode::F32Sqrt:
      case WabtInterpreterOpcode::F64Abs:
      case WabtInterpreterOpcode::F64Neg:
      case WabtInterpreterOpcode::F64Ceil:
      case WabtInterpreterOpcode::F64Floor:
      case WabtInterpreterOpcode::F64Trunc:
      case WabtInterpreterOpcode::F64Nearest:
      case WabtInterpreterOpcode::F64Sqrt:
      case WabtInterpreterOpcode::I32TruncSF32:
      case WabtInterpreterOpcode::I32TruncUF32:
      case WabtInterpreterOpcode::I64TruncSF32:
      case WabtInterpreterOpcode::I64TruncUF32:
      case WabtInterpreterOpcode::F64PromoteF32:
      case WabtInterpreterOpcode::I32ReinterpretF32:
      case WabtInterpreterOpcode::I32TruncSF64:
      case WabtInterpreterOpcode::I32TruncUF64:
      case WabtInterpreterOpcode::I64TruncSF64:
      case WabtInterpreterOpcode::I64TruncUF64:
      case WabtInterpreterOpcode::F32DemoteF64:
      case WabtInterpreterOpcode::I64ReinterpretF64:
      case WabtInterpreterOpcode::I32WrapI64:
      case WabtInterpreterOpcode::F32ConvertSI64:
      case WabtInterpreterOpcode::F32ConvertUI64:
      case WabtInterpreterOpcode::F64ConvertSI64:
      case WabtInterpreterOpcode::F64ConvertUI64:
      case WabtInterpreterOpcode::F64ReinterpretI64:
      case WabtInterpreterOpcode::I64ExtendSI32:
      case WabtInterpreterOpcode::I64ExtendUI32:
      case WabtInterpreterOpcode::F32ConvertSI32:
      case WabtInterpreterOpcode::F32ConvertUI32:
      case WabtInterpreterOpcode::F32ReinterpretI32:
      case WabtInterpreterOpcode::F64ConvertSI32:
      case WabtInterpreterOpcode::F64ConvertUI32:
        wabt_writef(stream, "%s %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode));
        break;

      case WabtInterpreterOpcode::GrowMemory: {
        uint32_t memory_index = read_u32(&pc);
        wabt_writef(stream, "%s $%u:%%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode), memory_index);
        break;
      }

      case WabtInterpreterOpcode::Alloca:
        wabt_writef(stream, "%s $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WabtInterpreterOpcode::BrUnless:
        wabt_writef(stream, "%s @%u, %%[-1]\n",
                    wabt_get_interpreter_opcode_name(opcode), read_u32(&pc));
        break;

      case WabtInterpreterOpcode::DropKeep: {
        uint32_t drop = read_u32(&pc);
        uint32_t keep = *pc++;
        wabt_writef(stream, "%s $%u $%u\n",
                    wabt_get_interpreter_opcode_name(opcode), drop, keep);
        break;
      }

      case WabtInterpreterOpcode::Data: {
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


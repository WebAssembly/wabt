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

#ifndef WABT_INTERPRETER_H_
#define WABT_INTERPRETER_H_

#include <stdint.h>

#include "array.h"
#include "binding-hash.h"
#include "type-vector.h"
#include "vector.h"
#include "writer.h"

struct WabtStream;

#define FOREACH_INTERPRETER_RESULT(V)                                          \
  V(OK, "ok")                                                                  \
  /* returned from the top-most function */                                    \
  V(RETURNED, "returned")                                                      \
  /* memory access is out of bounds */                                         \
  V(TRAP_MEMORY_ACCESS_OUT_OF_BOUNDS, "out of bounds memory access")           \
  /* converting from float -> int would overflow int */                        \
  V(TRAP_INTEGER_OVERFLOW, "integer overflow")                                 \
  /* dividend is zero in integer divide */                                     \
  V(TRAP_INTEGER_DIVIDE_BY_ZERO, "integer divide by zero")                     \
  /* converting from float -> int where float is nan */                        \
  V(TRAP_INVALID_CONVERSION_TO_INTEGER, "invalid conversion to integer")       \
  /* function table index is out of bounds */                                  \
  V(TRAP_UNDEFINED_TABLE_INDEX, "undefined table index")                       \
  /* function table element is uninitialized */                                \
  V(TRAP_UNINITIALIZED_TABLE_ELEMENT, "uninitialized table element")           \
  /* unreachable instruction executed */                                       \
  V(TRAP_UNREACHABLE, "unreachable executed")                                  \
  /* call indirect signature doesn't match function table signature */         \
  V(TRAP_INDIRECT_CALL_SIGNATURE_MISMATCH, "indirect call signature mismatch") \
  /* ran out of call stack frames (probably infinite recursion) */             \
  V(TRAP_CALL_STACK_EXHAUSTED, "call stack exhausted")                         \
  /* ran out of value stack space */                                           \
  V(TRAP_VALUE_STACK_EXHAUSTED, "value stack exhausted")                       \
  /* we called a host function, but the return value didn't match the */       \
  /* expected type */                                                          \
  V(TRAP_HOST_RESULT_TYPE_MISMATCH, "host result type mismatch")               \
  /* we called an import function, but it didn't complete succesfully */       \
  V(TRAP_HOST_TRAPPED, "host function trapped")                                \
  /* we attempted to call a function with the an argument list that doesn't    \
   * match the function signature */                                           \
  V(ARGUMENT_TYPE_MISMATCH, "argument type mismatch")                          \
  /* we tried to get an export by name that doesn't exist */                   \
  V(UNKNOWN_EXPORT, "unknown export")                                          \
  /* the expected export kind doesn't match. */                                \
  V(EXPORT_KIND_MISMATCH, "export kind mismatch")

enum WabtInterpreterResult {
#define V(name, str) WABT_INTERPRETER_##name,
  FOREACH_INTERPRETER_RESULT(V)
#undef V
};

#define WABT_INVALID_INDEX ((uint32_t)~0)
#define WABT_INVALID_OFFSET ((uint32_t)~0)
#define WABT_TABLE_ENTRY_SIZE (sizeof(uint32_t) * 2 + sizeof(uint8_t))
#define WABT_TABLE_ENTRY_OFFSET_OFFSET 0
#define WABT_TABLE_ENTRY_DROP_OFFSET sizeof(uint32_t)
#define WABT_TABLE_ENTRY_KEEP_OFFSET (sizeof(uint32_t) * 2)

enum {
  /* push space on the value stack for N entries */
  WABT_OPCODE_ALLOCA = WABT_NUM_OPCODES,
  WABT_OPCODE_BR_UNLESS,
  WABT_OPCODE_CALL_HOST,
  WABT_OPCODE_DATA,
  WABT_OPCODE_DROP_KEEP,
  WABT_NUM_INTERPRETER_OPCODES,
};
WABT_STATIC_ASSERT(WABT_NUM_INTERPRETER_OPCODES <= 256);

typedef uint32_t WabtUint32;
WABT_DEFINE_ARRAY(uint32, WabtUint32);

/* TODO(binji): identical to WabtFuncSignature. Share? */
struct WabtInterpreterFuncSignature {
  WabtTypeVector param_types;
  WabtTypeVector result_types;
};
WABT_DEFINE_VECTOR(interpreter_func_signature, WabtInterpreterFuncSignature);

struct WabtInterpreterTable {
  WabtLimits limits;
  WabtUint32Array func_indexes;
};
WABT_DEFINE_VECTOR(interpreter_table, WabtInterpreterTable);

struct WabtInterpreterMemory {
  void* data;
  WabtLimits page_limits;
  uint32_t byte_size; /* Cached from page_limits. */
};
WABT_DEFINE_VECTOR(interpreter_memory, WabtInterpreterMemory);

union WabtInterpreterValue {
  uint32_t i32;
  uint64_t i64;
  uint32_t f32_bits;
  uint64_t f64_bits;
};
WABT_DEFINE_ARRAY(interpreter_value, WabtInterpreterValue);

struct WabtInterpreterTypedValue {
  WabtType type;
  WabtInterpreterValue value;
};
WABT_DEFINE_VECTOR(interpreter_typed_value, WabtInterpreterTypedValue);

struct WabtInterpreterGlobal {
  WabtInterpreterTypedValue typed_value;
  bool mutable_;
  uint32_t import_index; /* or INVALID_INDEX if not imported */
};
WABT_DEFINE_VECTOR(interpreter_global, WabtInterpreterGlobal);

struct WabtInterpreterImport {
  WabtStringSlice module_name;
  WabtStringSlice field_name;
  WabtExternalKind kind;
  union {
    struct {
      uint32_t sig_index;
    } func;
    struct {
      WabtLimits limits;
    } table, memory;
    struct {
      WabtType type;
      bool mutable_;
    } global;
  };
};
WABT_DEFINE_ARRAY(interpreter_import, WabtInterpreterImport);

struct WabtInterpreterFunc;

typedef WabtResult (*WabtInterpreterHostFuncCallback)(
    const struct WabtInterpreterFunc* func,
    const WabtInterpreterFuncSignature* sig,
    uint32_t num_args,
    WabtInterpreterTypedValue* args,
    uint32_t num_results,
    WabtInterpreterTypedValue* out_results,
    void* user_data);

struct WabtInterpreterFunc {
  uint32_t sig_index;
  bool is_host;
  union {
    struct {
      uint32_t offset;
      uint32_t local_decl_count;
      uint32_t local_count;
      WabtTypeVector param_and_local_types;
    } defined;
    struct {
      WabtStringSlice module_name;
      WabtStringSlice field_name;
      WabtInterpreterHostFuncCallback callback;
      void* user_data;
    } host;
  };
};
WABT_DEFINE_VECTOR(interpreter_func, WabtInterpreterFunc);

struct WabtInterpreterExport {
  WabtStringSlice name; /* Owned by the export_bindings hash */
  WabtExternalKind kind;
  uint32_t index;
};
WABT_DEFINE_VECTOR(interpreter_export, WabtInterpreterExport);

struct WabtPrintErrorCallback {
  void* user_data;
  void (*print_error)(const char* msg, void* user_data);
};

struct WabtInterpreterHostImportDelegate {
  void *user_data;
  WabtResult (*import_func)(WabtInterpreterImport*,
                            WabtInterpreterFunc*,
                            WabtInterpreterFuncSignature*,
                            WabtPrintErrorCallback,
                            void* user_data);
  WabtResult (*import_table)(WabtInterpreterImport*,
                             WabtInterpreterTable*,
                             WabtPrintErrorCallback,
                             void* user_data);
  WabtResult (*import_memory)(WabtInterpreterImport*,
                              WabtInterpreterMemory*,
                              WabtPrintErrorCallback,
                              void* user_data);
  WabtResult (*import_global)(WabtInterpreterImport*,
                              WabtInterpreterGlobal*,
                              WabtPrintErrorCallback,
                              void* user_data);
};

struct WabtInterpreterModule {
  WabtStringSlice name;
  WabtInterpreterExportVector exports;
  WabtBindingHash export_bindings;
  uint32_t memory_index; /* INVALID_INDEX if not defined */
  uint32_t table_index;  /* INVALID_INDEX if not defined */
  bool is_host;
  union {
    struct {
      WabtInterpreterImportArray imports;
      uint32_t start_func_index; /* INVALID_INDEX if not defined */
      size_t istream_start;
      size_t istream_end;
    } defined;
    struct {
      WabtInterpreterHostImportDelegate import_delegate;
    } host;
  };
};
WABT_DEFINE_VECTOR(interpreter_module, WabtInterpreterModule);

/* Used to track and reset the state of the environment. */
struct WabtInterpreterEnvironmentMark {
  size_t modules_size;
  size_t sigs_size;
  size_t funcs_size;
  size_t memories_size;
  size_t tables_size;
  size_t globals_size;
  size_t istream_size;
};

struct WabtInterpreterEnvironment {
  WabtInterpreterModuleVector modules;
  WabtInterpreterFuncSignatureVector sigs;
  WabtInterpreterFuncVector funcs;
  WabtInterpreterMemoryVector memories;
  WabtInterpreterTableVector tables;
  WabtInterpreterGlobalVector globals;
  WabtOutputBuffer istream;
  WabtBindingHash module_bindings;
  WabtBindingHash registered_module_bindings;
};

struct WabtInterpreterThread {
  WabtInterpreterEnvironment* env;
  WabtInterpreterValueArray value_stack;
  WabtUint32Array call_stack;
  WabtInterpreterValue* value_stack_top;
  WabtInterpreterValue* value_stack_end;
  uint32_t* call_stack_top;
  uint32_t* call_stack_end;
  uint32_t pc;

  /* a temporary buffer that is for passing args to host functions */
  WabtInterpreterTypedValueVector host_args;
};

#define WABT_INTERPRETER_THREAD_OPTIONS_DEFAULT \
  { 512 * 1024 / sizeof(WabtInterpreterValue), 64 * 1024, WABT_INVALID_OFFSET }

struct WabtInterpreterThreadOptions {
  uint32_t value_stack_size;
  uint32_t call_stack_size;
  uint32_t pc;
};

WABT_EXTERN_C_BEGIN
bool wabt_is_nan_f32(uint32_t f32_bits);
bool wabt_is_nan_f64(uint64_t f64_bits);
bool wabt_func_signatures_are_equal(WabtInterpreterEnvironment* env,
                                    uint32_t sig_index_0,
                                    uint32_t sig_index_1);

void wabt_init_interpreter_environment(WabtInterpreterEnvironment* env);
void wabt_destroy_interpreter_environment(WabtInterpreterEnvironment* env);
WabtInterpreterEnvironmentMark wabt_mark_interpreter_environment(
    WabtInterpreterEnvironment* env);
void wabt_reset_interpreter_environment_to_mark(
    WabtInterpreterEnvironment* env,
    WabtInterpreterEnvironmentMark mark);
WabtInterpreterModule* wabt_append_host_module(WabtInterpreterEnvironment* env,
                                               WabtStringSlice name);
void wabt_init_interpreter_thread(WabtInterpreterEnvironment* env,
                                  WabtInterpreterThread* thread,
                                  WabtInterpreterThreadOptions* options);
WabtInterpreterResult wabt_push_thread_value(WabtInterpreterThread* thread,
                                             WabtInterpreterValue value);
void wabt_destroy_interpreter_thread(WabtInterpreterThread* thread);
WabtInterpreterResult wabt_call_host(WabtInterpreterThread* thread,
                                     WabtInterpreterFunc* func);
WabtInterpreterResult wabt_run_interpreter(WabtInterpreterThread* thread,
                                           uint32_t num_instructions,
                                           uint32_t* call_stack_return_top);
void wabt_trace_pc(WabtInterpreterThread* thread, struct WabtStream* stream);
void wabt_disassemble(WabtInterpreterEnvironment* env,
                      struct WabtStream* stream,
                      uint32_t from,
                      uint32_t to);
void wabt_disassemble_module(WabtInterpreterEnvironment* env,
                             struct WabtStream* stream,
                             WabtInterpreterModule* module);

WabtInterpreterExport* wabt_get_interpreter_export_by_name(
    WabtInterpreterModule* module,
    const WabtStringSlice* name);
WABT_EXTERN_C_END

#endif /* WABT_INTERPRETER_H_ */

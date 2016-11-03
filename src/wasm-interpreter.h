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

#ifndef WASM_INTERPRETER_H_
#define WASM_INTERPRETER_H_

#include <stdint.h>

#include "wasm-array.h"
#include "wasm-binding-hash.h"
#include "wasm-type-vector.h"
#include "wasm-vector.h"
#include "wasm-writer.h"

struct WasmStream;

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

typedef enum WasmInterpreterResult {
#define V(name, str) WASM_INTERPRETER_##name,
  FOREACH_INTERPRETER_RESULT(V)
#undef V
} WasmInterpreterResult;

#define WASM_INVALID_INDEX ((uint32_t)~0)
#define WASM_INVALID_OFFSET ((uint32_t)~0)
#define WASM_TABLE_ENTRY_SIZE (sizeof(uint32_t) * 2 + sizeof(uint8_t))
#define WASM_TABLE_ENTRY_OFFSET_OFFSET 0
#define WASM_TABLE_ENTRY_DROP_OFFSET sizeof(uint32_t)
#define WASM_TABLE_ENTRY_KEEP_OFFSET (sizeof(uint32_t) * 2)

enum {
  /* push space on the value stack for N entries */
  WASM_OPCODE_ALLOCA = WASM_NUM_OPCODES,
  WASM_OPCODE_BR_UNLESS,
  WASM_OPCODE_CALL_HOST,
  WASM_OPCODE_DATA,
  WASM_OPCODE_DROP_KEEP,
  WASM_NUM_INTERPRETER_OPCODES,
};
WASM_STATIC_ASSERT(WASM_NUM_INTERPRETER_OPCODES <= 256);

typedef uint32_t WasmUint32;
WASM_DEFINE_ARRAY(uint32, WasmUint32);

/* TODO(binji): identical to WasmFuncSignature. Share? */
typedef struct WasmInterpreterFuncSignature {
  WasmTypeVector param_types;
  WasmTypeVector result_types;
} WasmInterpreterFuncSignature;
WASM_DEFINE_VECTOR(interpreter_func_signature, WasmInterpreterFuncSignature);

typedef struct WasmInterpreterTable {
  WasmLimits limits;
  WasmUint32Array func_indexes;
} WasmInterpreterTable;
WASM_DEFINE_VECTOR(interpreter_table, WasmInterpreterTable);

typedef struct WasmInterpreterMemory {
  WasmAllocator* allocator;
  void* data;
  uint32_t page_size;
  uint32_t byte_size;
  uint32_t max_page_size;
} WasmInterpreterMemory;
WASM_DEFINE_VECTOR(interpreter_memory, WasmInterpreterMemory);

typedef union WasmInterpreterValue {
  uint32_t i32;
  uint64_t i64;
  uint32_t f32_bits;
  uint64_t f64_bits;
} WasmInterpreterValue;
WASM_DEFINE_ARRAY(interpreter_value, WasmInterpreterValue);

typedef struct WasmInterpreterTypedValue {
  WasmType type;
  WasmInterpreterValue value;
} WasmInterpreterTypedValue;
WASM_DEFINE_ARRAY(interpreter_typed_value, WasmInterpreterTypedValue);
WASM_DEFINE_VECTOR(interpreter_typed_value, WasmInterpreterTypedValue);

typedef struct WasmInterpreterGlobal {
  WasmInterpreterTypedValue typed_value;
  WasmBool mutable_;
  uint32_t import_index; /* or INVALID_INDEX if not imported */
} WasmInterpreterGlobal;
WASM_DEFINE_VECTOR(interpreter_global, WasmInterpreterGlobal);

typedef struct WasmInterpreterImport {
  WasmStringSlice module_name;
  WasmStringSlice field_name;
  WasmExternalKind kind;
  union {
    struct {
      uint32_t sig_index;
    } func;
    struct {
      WasmLimits limits;
    } table, memory;
    struct {
      WasmType type;
      WasmBool mutable_;
    } global;
  };
} WasmInterpreterImport;
WASM_DEFINE_ARRAY(interpreter_import, WasmInterpreterImport);

struct WasmInterpreterFunc;

typedef WasmResult (*WasmInterpreterHostFuncCallback)(
    const struct WasmInterpreterFunc* func,
    const WasmInterpreterFuncSignature* sig,
    uint32_t num_args,
    WasmInterpreterTypedValue* args,
    uint32_t num_results,
    WasmInterpreterTypedValue* out_results,
    void* user_data);

typedef struct WasmInterpreterFunc {
  uint32_t sig_index;
  WasmBool is_host;
  union {
    struct {
      uint32_t offset;
      uint32_t local_decl_count;
      uint32_t local_count;
      WasmTypeVector param_and_local_types;
    } defined;
    struct {
      WasmStringSlice module_name;
      WasmStringSlice field_name;
      WasmInterpreterHostFuncCallback callback;
      void* user_data;
    } host;
  };
} WasmInterpreterFunc;
WASM_DEFINE_VECTOR(interpreter_func, WasmInterpreterFunc);

typedef struct WasmInterpreterExport {
  WasmStringSlice name; /* Owned by the export_bindings hash */
  WasmExternalKind kind;
  uint32_t index;
} WasmInterpreterExport;
WASM_DEFINE_VECTOR(interpreter_export, WasmInterpreterExport);

typedef struct WasmInterpreterHostImportDelegate {
  void *user_data;
  WasmResult (*import_func)(WasmInterpreterImport*,
                            WasmInterpreterFunc*,
                            WasmInterpreterFuncSignature*,
                            void* user_data);
  WasmResult (*import_table)(WasmInterpreterImport*,
                             WasmInterpreterTable*,
                             void* user_data);
  WasmResult (*import_memory)(WasmInterpreterImport*,
                              WasmInterpreterMemory*,
                              void* user_data);
  WasmResult (*import_global)(WasmInterpreterImport*,
                              WasmInterpreterGlobal*,
                              void* user_data);
} WasmInterpreterHostImportDelegate;

typedef struct WasmInterpreterModule {
  WasmStringSlice name;
  WasmInterpreterExportVector exports;
  WasmBindingHash export_bindings;
  uint32_t memory_index; /* INVALID_INDEX if not defined */
  uint32_t table_index;  /* INVALID_INDEX if not defined */
  WasmBool is_host;
  union {
    struct {
      WasmInterpreterImportArray imports;
      uint32_t start_func_index; /* INVALID_INDEX if not defined */
      size_t istream_start;
      size_t istream_end;
    } defined;
    struct {
      WasmInterpreterHostImportDelegate import_delegate;
    } host;
  };
} WasmInterpreterModule;
WASM_DEFINE_VECTOR(interpreter_module, WasmInterpreterModule);

typedef struct WasmInterpreterEnvironment {
  WasmInterpreterModuleVector modules;
  WasmInterpreterFuncSignatureVector sigs;
  WasmInterpreterFuncVector funcs;
  WasmInterpreterMemoryVector memories;
  WasmInterpreterTableVector tables;
  WasmInterpreterGlobalVector globals;
  WasmOutputBuffer istream;
  WasmBindingHash module_bindings;
} WasmInterpreterEnvironment;

typedef struct WasmInterpreterThread {
  WasmInterpreterEnvironment* env;
  WasmInterpreterModule* module;
  WasmInterpreterTable* table;
  WasmInterpreterMemory* memory;
  WasmInterpreterValueArray value_stack;
  WasmUint32Array call_stack;
  WasmInterpreterValue* value_stack_top;
  WasmInterpreterValue* value_stack_end;
  uint32_t* call_stack_top;
  uint32_t* call_stack_end;
  uint32_t pc;

  /* a temporary buffer that is for passing args to host functions */
  WasmInterpreterTypedValueArray host_args;
} WasmInterpreterThread;

#define WASM_INTERPRETER_THREAD_OPTIONS_DEFAULT \
  { 512 * 1024 / sizeof(WasmInterpreterValue), 64 * 1024, WASM_INVALID_OFFSET }

typedef struct WasmInterpreterThreadOptions {
  uint32_t value_stack_size;
  uint32_t call_stack_size;
  uint32_t pc;
} WasmInterpreterThreadOptions;

WASM_EXTERN_C_BEGIN
WasmBool is_nan_f32(uint32_t f32_bits);
WasmBool is_nan_f64(uint64_t f64_bits);

void wasm_init_interpreter_environment(WasmAllocator* allocator,
                                       WasmInterpreterEnvironment* env);
void wasm_destroy_interpreter_environment(WasmAllocator* allocator,
                                          WasmInterpreterEnvironment* env);
WasmInterpreterModule* wasm_append_host_module(WasmAllocator* allocator,
                                               WasmInterpreterEnvironment* env,
                                               WasmStringSlice name);
WasmResult wasm_init_interpreter_thread(WasmAllocator* allocator,
                                        WasmInterpreterEnvironment* env,
                                        WasmInterpreterModule* module,
                                        WasmInterpreterThread* thread,
                                        WasmInterpreterThreadOptions* options);
WasmInterpreterResult wasm_push_thread_value(WasmInterpreterThread* thread,
                                             WasmInterpreterValue value);
void wasm_destroy_interpreter_thread(WasmAllocator* allocator,
                                     WasmInterpreterThread* thread);
WasmInterpreterResult wasm_call_host(WasmInterpreterThread* thread,
                                     WasmInterpreterFunc* func);
WasmInterpreterResult wasm_run_interpreter(WasmInterpreterThread* thread,
                                           uint32_t num_instructions,
                                           uint32_t* call_stack_return_top);
void wasm_trace_pc(WasmInterpreterThread* thread, struct WasmStream* stream);
void wasm_disassemble(WasmInterpreterEnvironment* env,
                      struct WasmStream* stream,
                      uint32_t from,
                      uint32_t to);
void wasm_disassemble_module(WasmInterpreterEnvironment* env,
                             struct WasmStream* stream,
                             WasmInterpreterModule* module);

WasmInterpreterExport* wasm_get_interpreter_export_by_name(
    WasmInterpreterModule* module,
    const WasmStringSlice* name);
WASM_EXTERN_C_END

#endif /* WASM_INTERPRETER_H_ */

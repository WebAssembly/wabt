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
  /* we called an import function, but the return value didn't match the */    \
  /* expected type */                                                          \
  V(TRAP_IMPORT_RESULT_TYPE_MISMATCH, "import result type mismatch")           \
  /* we called an import function, but it didn't complete succesfully */       \
  V(TRAP_IMPORT_TRAPPED, "import function trapped")

typedef enum WasmInterpreterResult {
#define V(name, str) WASM_INTERPRETER_##name,
  FOREACH_INTERPRETER_RESULT(V)
#undef V
} WasmInterpreterResult;

#define WASM_INVALID_OFFSET ((uint32_t)~0)
#define WASM_TABLE_ENTRY_SIZE (sizeof(uint32_t) * 2 + sizeof(uint8_t))
#define WASM_TABLE_ENTRY_OFFSET_OFFSET 0
#define WASM_TABLE_ENTRY_DROP_OFFSET sizeof(uint32_t)
#define WASM_TABLE_ENTRY_KEEP_OFFSET (sizeof(uint32_t) * 2)

enum {
  /* push space on the value stack for N entries */
  WASM_OPCODE_ALLOCA = WASM_NUM_OPCODES,
  WASM_OPCODE_BR_UNLESS,
  WASM_OPCODE_CALL_IMPORT,
  WASM_OPCODE_DATA,
  WASM_OPCODE_DROP_KEEP,
  WASM_NUM_INTERPRETER_OPCODES,
};
WASM_STATIC_ASSERT(WASM_NUM_INTERPRETER_OPCODES <= 256);

/* TODO(binji): identical to WasmFuncSignature. Share? */
typedef struct WasmInterpreterFuncSignature {
  WasmTypeVector param_types;
  WasmTypeVector result_types;
} WasmInterpreterFuncSignature;
WASM_DEFINE_ARRAY(interpreter_func_signature, WasmInterpreterFuncSignature);

typedef struct WasmInterpreterMemory {
  WasmAllocator* allocator;
  void* data;
  uint32_t page_size;
  uint32_t byte_size;
  uint32_t max_page_size;
} WasmInterpreterMemory;

typedef struct WasmInterpreterFuncTableEntry {
  uint32_t sig_index;
  uint32_t func_index;
} WasmInterpreterFuncTableEntry;
WASM_DEFINE_ARRAY(interpreter_func_table_entry, WasmInterpreterFuncTableEntry);

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
} WasmInterpreterGlobal;
WASM_DEFINE_ARRAY(interpreter_global, WasmInterpreterGlobal);

struct WasmInterpreterModule;
struct WasmInterpreterImport;

typedef WasmResult (*WasmInterpreterImportCallback)(
    struct WasmInterpreterModule* module,
    struct WasmInterpreterImport* import,
    uint32_t num_args,
    WasmInterpreterTypedValue* args,
    uint32_t num_results,
    WasmInterpreterTypedValue* out_results,
    void* user_data);

typedef struct WasmInterpreterImport {
  WasmStringSlice module_name;
  WasmStringSlice field_name;
  WasmExternalKind kind;
  union {
    struct {
      uint32_t sig_index;
      WasmInterpreterImportCallback callback;
      void* user_data;
    } func;
  };
} WasmInterpreterImport;
typedef WasmInterpreterImport* WasmInterpreterImportPtr;
WASM_DEFINE_ARRAY(interpreter_import, WasmInterpreterImport);
WASM_DEFINE_ARRAY(interpreter_import_ptr, WasmInterpreterImportPtr);

typedef struct WasmInterpreterFunc {
  uint32_t sig_index;
  uint32_t offset;
  uint32_t local_decl_count;
  uint32_t local_count;
  WasmTypeVector param_and_local_types;
} WasmInterpreterFunc;
WASM_DEFINE_ARRAY(interpreter_func, WasmInterpreterFunc);

typedef struct WasmInterpreterExport {
  WasmStringSlice name;
  WasmExternalKind kind;
  union {
    struct {
      uint32_t index;
    } func;
  };
} WasmInterpreterExport;
WASM_DEFINE_ARRAY(interpreter_export, WasmInterpreterExport);

typedef uint32_t WasmUint32;
WASM_DEFINE_ARRAY(uint32, WasmUint32);

typedef struct WasmInterpreterModule {
  WasmInterpreterMemory memory;
  WasmInterpreterFuncSignatureArray sigs;
  WasmInterpreterFuncArray funcs;
  WasmUint32Array func_table;
  WasmInterpreterImportArray imports;
  WasmInterpreterImportPtrArray func_imports;
  WasmInterpreterExportArray exports;
  WasmInterpreterGlobalArray globals;
  WasmOutputBuffer istream;
  uint32_t start_func_offset; /* == WASM_INVALID_OFFSET if not defined */
} WasmInterpreterModule;

typedef struct WasmInterpreterThread {
  WasmInterpreterValueArray value_stack;
  WasmUint32Array call_stack;
  WasmInterpreterValue* value_stack_top;
  WasmInterpreterValue* value_stack_end;
  uint32_t* call_stack_top;
  uint32_t* call_stack_end;
  uint32_t pc;

  /* a temporary buffer that is for passing args to import functions */
  WasmInterpreterTypedValueArray import_args;
} WasmInterpreterThread;

#define WASM_INTERPRETER_THREAD_OPTIONS_DEFAULT \
  { 512 * 1024 / sizeof(WasmInterpreterValue), 64 * 1024, WASM_INVALID_OFFSET }

typedef struct WasmInterpreterThreadOptions {
  uint32_t value_stack_size;
  uint32_t call_stack_size;
  uint32_t pc;
} WasmInterpreterThreadOptions;

WASM_EXTERN_C_BEGIN
WasmResult wasm_init_interpreter_thread(WasmAllocator* allocator,
                                        WasmInterpreterModule* module,
                                        WasmInterpreterThread* thread,
                                        WasmInterpreterThreadOptions* options);
WasmInterpreterResult wasm_push_thread_value(WasmInterpreterThread* thread,
                                             WasmInterpreterValue value);
void wasm_destroy_interpreter_thread(WasmAllocator* allocator,
                                     WasmInterpreterThread* thread);
WasmInterpreterResult wasm_run_interpreter(WasmInterpreterModule* module,
                                           WasmInterpreterThread* thread,
                                           uint32_t num_instructions,
                                           uint32_t* call_stack_return_top);
void wasm_trace_pc(WasmInterpreterModule* module,
                   WasmInterpreterThread* thread,
                   struct WasmStream* stream);
void wasm_disassemble_module(WasmInterpreterModule* module,
                             struct WasmStream* stream,
                             uint32_t from,
                             uint32_t to);
void wasm_destroy_interpreter_module(WasmAllocator* allocator,
                                     WasmInterpreterModule* module);

WasmInterpreterExport* wasm_get_interpreter_export_by_name(
    WasmInterpreterModule* module,
    WasmStringSlice* name);
WASM_EXTERN_C_END

#endif /* WASM_INTERPRETER_H_ */

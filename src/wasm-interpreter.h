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
  /* growing the memory would overflow the memory size type */                 \
  V(TRAP_MEMORY_SIZE_OVERFLOW, "memory size overflow")                         \
  /* out of memory */                                                          \
  V(TRAP_OUT_OF_MEMORY, "memory size exceeds implementation limit")            \
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
#define WASM_TABLE_ENTRY_DISCARD_OFFSET sizeof(uint32_t)
#define WASM_TABLE_ENTRY_KEEP_OFFSET (sizeof(uint32_t) * 2)

enum {
  /* push space on the value stack for N entries */
  WASM_OPCODE_ALLOCA = WASM_LAST_OPCODE,
  WASM_OPCODE_BR_UNLESS,
  WASM_OPCODE_DATA,
  WASM_OPCODE_DISCARD,
  WASM_OPCODE_DISCARD_KEEP,
  WASM_LAST_INTERPRETER_OPCODE,
};
WASM_STATIC_ASSERT(WASM_LAST_INTERPRETER_OPCODE <= 256);

typedef uint8_t WasmUint8;
WASM_DEFINE_VECTOR(uint8, WasmUint8);

/* TODO(binji): identical to WasmFuncSignature. Share? */
typedef struct WasmInterpreterFuncSignature {
  WasmType result_type;
  WasmTypeVector param_types;
} WasmInterpreterFuncSignature;
WASM_DEFINE_ARRAY(interpreter_func_signature, WasmInterpreterFuncSignature);

typedef struct WasmInterpreterMemory {
  WasmAllocator* allocator;
  void* data;
  uint32_t page_size;
  uint32_t byte_size;
} WasmInterpreterMemory;

typedef struct WasmInterpreterFuncTableEntry {
  uint32_t sig_index;
  uint32_t func_offset;
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

struct WasmInterpreterModule;
struct WasmInterpreterImport;

typedef WasmResult (*WasmInterpreterImportCallback)(
    struct WasmInterpreterModule* module,
    struct WasmInterpreterImport* import,
    uint32_t num_args,
    WasmInterpreterTypedValue* args,
    WasmInterpreterTypedValue* out_result,
    void* user_data);

typedef struct WasmInterpreterImport {
  WasmStringSlice module_name;
  WasmStringSlice func_name;
  uint32_t sig_index;
  WasmInterpreterImportCallback callback;
  void* user_data;
} WasmInterpreterImport;
WASM_DEFINE_ARRAY(interpreter_import, WasmInterpreterImport);

typedef struct WasmInterpreterExport {
  uint32_t func_index;
  WasmStringSlice name;
  uint32_t func_offset;
  uint32_t sig_index;
} WasmInterpreterExport;
WASM_DEFINE_ARRAY(interpreter_export, WasmInterpreterExport);

typedef struct WasmInterpreterModule {
  WasmInterpreterMemory memory;
  WasmInterpreterFuncSignatureArray sigs;
  WasmInterpreterFuncTableEntryArray func_table;
  WasmInterpreterImportArray imports;
  WasmInterpreterExportArray exports;
  WasmOutputBuffer istream;
  uint32_t start_func_offset; /* == WASM_INVALID_OFFSET if not defined */
} WasmInterpreterModule;

typedef uint32_t WasmUint32;
WASM_DEFINE_ARRAY(uint32, WasmUint32);

typedef struct WasmInterpreterThread {
  WasmInterpreterValueArray value_stack;
  WasmUint32Array call_stack;
  uint32_t value_stack_top;
  uint32_t call_stack_top;
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
                                           uint32_t call_stack_return_top);
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
WasmInterpreterImport* wasm_get_interpreter_import_by_name(
    WasmInterpreterModule* module,
    WasmStringSlice* module_name,
    WasmStringSlice* func_name);
WASM_EXTERN_C_END

#endif /* WASM_INTERPRETER_H_ */

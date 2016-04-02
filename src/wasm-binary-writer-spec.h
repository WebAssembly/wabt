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

#ifndef WASM_BINARY_WRITER_SPEC_H_
#define WASM_BINARY_WRITER_SPEC_H_

#include "wasm-ast.h"
#include "wasm-common.h"

struct WasmAllocator;
struct WasmWriteBinaryOptions;
struct WasmWriter;

#define WASM_WRITE_BINARY_SPEC_OPTIONS_DEFAULT \
  { 0 }

typedef struct WasmWriteBinarySpecOptions {
  /* callbacks for writing multiple modules */
  void (*on_script_begin)(void* user_data);
  void (*on_module_begin)(uint32_t index,
                          void* user_data);
  void (*on_command)(uint32_t index,
                     WasmCommandType type,
                     WasmStringSlice* name,
                     WasmLocation* loc,
                     void* user_data);
  void (*on_module_before_write)(uint32_t index,
                                 struct WasmWriter** out_writer,
                                 void* user_data);
  void (*on_module_end)(uint32_t index, WasmResult result, void* user_data);
  void (*on_script_end)(void* user_data);

  void* user_data;
} WasmWriteBinarySpecOptions;

WASM_EXTERN_C_BEGIN
WasmResult wasm_write_binary_spec_script(struct WasmAllocator*,
                                         struct WasmScript*,
                                         struct WasmWriteBinaryOptions*,
                                         WasmWriteBinarySpecOptions*);
WASM_EXTERN_C_END

#endif /* WASM_BINARY_WRITER_SPEC_H_ */

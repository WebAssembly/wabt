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

#ifndef WASM_BINARY_WRITER_H_
#define WASM_BINARY_WRITER_H_

#include "wasm-common.h"

struct WasmAllocator;
struct WasmModule;
struct WasmScript;
struct WasmWriter;

#define WASM_WRITE_BINARY_OPTIONS_DEFAULT \
  { 0, 1, 1, 0 }

typedef struct WasmWriteBinaryOptions {
  int log_writes;
  int canonicalize_lebs;
  int remap_locals;
  int write_debug_names;
} WasmWriteBinaryOptions;

WASM_EXTERN_C_BEGIN
WasmResult wasm_write_binary_module(struct WasmAllocator*,
                                    struct WasmWriter*,
                                    struct WasmModule*,
                                    WasmWriteBinaryOptions*);

WasmResult wasm_write_binary_script(struct WasmAllocator*,
                                    struct WasmWriter*,
                                    struct WasmScript*,
                                    WasmWriteBinaryOptions*);
WASM_EXTERN_C_END

#endif /* WASM_BINARY_WRITER_H_ */

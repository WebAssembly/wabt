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

#ifndef WASM_BINARY_READER_INTERPRETER_H_
#define WASM_BINARY_READER_INTERPRETER_H_

#include "common.h"

struct WasmAllocator;
struct WasmInterpreterEnvironment;
struct WasmInterpreterModule;
struct WasmReadBinaryOptions;

WASM_EXTERN_C_BEGIN
WasmResult wasm_read_binary_interpreter(
    struct WasmAllocator* allocator,
    struct WasmAllocator* memory_allocator,
    struct WasmInterpreterEnvironment* env,
    const void* data,
    size_t size,
    const struct WasmReadBinaryOptions* options,
    WasmBinaryErrorHandler*,
    struct WasmInterpreterModule** out_module);
WASM_EXTERN_C_END

#endif /* WASM_BINARY_READER_INTERPRETER_H_ */

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

#ifndef WASM_BINARY_H_
#define WASM_BINARY_H_

#include <stdint.h>

#define WASM_BINARY_MAGIC 0x6d736100
#define WASM_BINARY_VERSION 0x0a

#define WASM_BINARY_SECTION_DATA_SEGMENTS "data_segments"
#define WASM_BINARY_SECTION_EXPORTS "export_table"
#define WASM_BINARY_SECTION_FUNCTION_BODIES "function_bodies"
#define WASM_BINARY_SECTION_FUNCTION_SIGNATURES "function_signatures"
#define WASM_BINARY_SECTION_FUNCTION_TABLE "function_table"
#define WASM_BINARY_SECTION_IMPORTS "import_table"
#define WASM_BINARY_SECTION_MEMORY "memory"
#define WASM_BINARY_SECTION_NAMES "names"
#define WASM_BINARY_SECTION_SIGNATURES "signatures"
#define WASM_BINARY_SECTION_START "start_function"

#endif /* WASM_BINARY_H_ */

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

#define WASM_BINARY_MAGIC 0x6d736100
#define WASM_BINARY_VERSION 0x0a

#define WASM_SECTION_NAME_SIGNATURES "signatures"
#define WASM_SECTION_NAME_IMPORT_TABLE "import_table"
#define WASM_SECTION_NAME_FUNCTION_SIGNATURES "function_signatures"
#define WASM_SECTION_NAME_FUNCTION_TABLE "function_table"
#define WASM_SECTION_NAME_MEMORY "memory"
#define WASM_SECTION_NAME_EXPORT_TABLE "export_table"
#define WASM_SECTION_NAME_START_FUNCTION "start_function"
#define WASM_SECTION_NAME_FUNCTION_BODIES "function_bodies"
#define WASM_SECTION_NAME_DATA_SEGMENTS "data_segments"
#define WASM_SECTION_NAME_NAMES "names"

#define WASM_FOREACH_SECTION(V) \
  V(SIGNATURES)                 \
  V(IMPORT_TABLE)               \
  V(FUNCTION_SIGNATURES)        \
  V(FUNCTION_TABLE)             \
  V(MEMORY)                     \
  V(EXPORT_TABLE)               \
  V(START_FUNCTION)             \
  V(FUNCTION_BODIES)            \
  V(DATA_SEGMENTS)              \
  V(NAMES)

#endif /* WASM_BINARY_H_ */

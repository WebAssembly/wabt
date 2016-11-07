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
#define WASM_BINARY_VERSION 0x0d
#define WASM_BINARY_LIMITS_HAS_MAX_FLAG 0x1

#define WASM_BINARY_SECTION_NAME "name"

#define WASM_FOREACH_BINARY_SECTION(V) \
  V(UNKNOWN, 0)                        \
  V(TYPE, 1)                           \
  V(IMPORT, 2)                         \
  V(FUNCTION, 3)                       \
  V(TABLE, 4)                          \
  V(MEMORY, 5)                         \
  V(GLOBAL, 6)                         \
  V(EXPORT, 7)                         \
  V(START, 8)                          \
  V(ELEM, 9)                           \
  V(CODE, 10)                          \
  V(DATA, 11)

/* clang-format off */
typedef enum WasmBinarySection {
#define V(NAME, code) WASM_BINARY_SECTION_##NAME = code,
  WASM_FOREACH_BINARY_SECTION(V)
#undef V
  WASM_NUM_BINARY_SECTIONS
} WasmBinarySection;
/* clang-format on */

#endif /* WASM_BINARY_H_ */

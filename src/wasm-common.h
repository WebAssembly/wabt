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

#ifndef WASM_COMMON_H_
#define WASM_COMMON_H_

#include <stddef.h>

#ifdef __cplusplus
#define EXTERN_C extern "C"
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

#define WARN_UNUSED __attribute__ ((warn_unused_result))

typedef enum WasmResult {
  WASM_OK,
  WASM_ERROR,
} WasmResult;

typedef struct WasmStringSlice {
  const char* start;
  size_t length;
} WasmStringSlice;

typedef struct WasmLocation {
  const char* filename;
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} WasmLocation;

#endif /* WASM_COMMON_H_ */

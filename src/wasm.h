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

#ifndef WASM_H_
#define WASM_H_

#include <stdint.h>
#include <stddef.h>

#include "wasm-ast.h"
#include "wasm-common.h"
#include "wasm-vector.h"

struct WasmWriter;

typedef void* WasmScanner;

typedef struct WasmParser {
  struct WasmAllocator* allocator;
  WasmScript script;
  int errors;
} WasmParser;

typedef struct WasmWriteBinaryOptions {
  int spec;
  int spec_verbose;
  int log_writes;
} WasmWriteBinaryOptions;

EXTERN_C_BEGIN
WasmScanner wasm_new_scanner(struct WasmAllocator*, const char* filename);
void wasm_destroy_scanner(WasmScanner);
int wasm_parse(WasmScanner scanner, struct WasmParser* parser);
WasmResult wasm_check_script(struct WasmScript*);
WasmResult wasm_write_binary(struct WasmAllocator*,
                             struct WasmWriter*,
                             struct WasmScript*,
                             WasmWriteBinaryOptions*);
void wasm_destroy_script(struct WasmScript*);
EXTERN_C_END

#endif /* WASM_H_ */

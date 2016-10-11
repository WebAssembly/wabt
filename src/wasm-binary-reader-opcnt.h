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

#ifndef WASM_BINARY_READER_OPCNT_H_
#define WASM_BINARY_READER_OPCNT_H_

#include "wasm-common.h"
#include "wasm-vector.h"

struct WasmAllocator;
struct WasmModule;
struct WasmReadBinaryOptions;

WASM_EXTERN_C_BEGIN

typedef struct WasmIntCounter {
  intmax_t value;
  size_t count;
} WasmIntCounter;

WASM_DEFINE_VECTOR(int_counter, WasmIntCounter)

typedef struct WasmIntPairCounter {
  intmax_t first;
  intmax_t second;
  size_t count;
} WasmIntPairCounter;

WASM_DEFINE_VECTOR(int_pair_counter, WasmIntPairCounter);

typedef struct WasmOpcntData {
  struct WasmAllocator* allocator;
  WasmIntCounterVector opcode_vec;
  WasmIntCounterVector i32_const_vec;
  WasmIntCounterVector get_local_vec;
  WasmIntCounterVector set_local_vec;
  WasmIntCounterVector tee_local_vec;
  WasmIntPairCounterVector i32_load_vec;
  WasmIntPairCounterVector i32_store_vec;
  WasmBool collect_opcode_dist;
  WasmBool collect_get_local_dist;
  WasmBool collect_i32_const_dist;
  WasmBool collect_i32_load_dist;
  WasmBool collect_i32_store_dist;
  WasmBool collect_set_local_dist;
  WasmBool collect_tee_local_dist;
} WasmOpcntData;

void wasm_init_opcnt_data(struct WasmAllocator* allocator, WasmOpcntData* data);
void wasm_destroy_opcnt_data(struct WasmAllocator* allocator,
                             WasmOpcntData* data);

void wasm_opcnt_data_set_flags(WasmOpcntData* data);
void wasm_opcnt_data_clear_flags(WasmOpcntData* data);

WasmResult wasm_read_binary_opcnt(struct WasmAllocator* allocator,
                                  const void* data,
                                  size_t size,
                                  const struct WasmReadBinaryOptions* options,
                                  WasmBinaryErrorHandler*,
                                  WasmOpcntData* opcnt_data);

WASM_EXTERN_C_END

#endif /* WASM_BINARY_READER_OPCNT_H_ */

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

#ifndef WABT_BINARY_READER_OPCNT_H_
#define WABT_BINARY_READER_OPCNT_H_

#include "common.h"
#include "vector.h"

struct WabtModule;
struct WabtReadBinaryOptions;

WABT_EXTERN_C_BEGIN

typedef struct WabtIntCounter {
  intmax_t value;
  size_t count;
} WabtIntCounter;

WABT_DEFINE_VECTOR(int_counter, WabtIntCounter)

typedef struct WabtIntPairCounter {
  intmax_t first;
  intmax_t second;
  size_t count;
} WabtIntPairCounter;

WABT_DEFINE_VECTOR(int_pair_counter, WabtIntPairCounter);

typedef struct WabtOpcntData {
  WabtIntCounterVector opcode_vec;
  WabtIntCounterVector i32_const_vec;
  WabtIntCounterVector get_local_vec;
  WabtIntCounterVector set_local_vec;
  WabtIntCounterVector tee_local_vec;
  WabtIntPairCounterVector i32_load_vec;
  WabtIntPairCounterVector i32_store_vec;
} WabtOpcntData;

void wabt_init_opcnt_data(WabtOpcntData* data);
void wabt_destroy_opcnt_data(WabtOpcntData* data);

WabtResult wabt_read_binary_opcnt(const void* data,
                                  size_t size,
                                  const struct WabtReadBinaryOptions* options,
                                  WabtOpcntData* opcnt_data);

WABT_EXTERN_C_END

#endif /* WABT_BINARY_READER_OPCNT_H_ */

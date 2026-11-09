/*
 * Copyright 2018 WebAssembly Community Group participants
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

#ifndef WABT_WASM2C_WASM_RT_UVWASI_ADAPTER_H_
#define WABT_WASM2C_WASM_RT_UVWASI_ADAPTER_H_

#include "wasm-rt.h"

#include "uvwasi.h"

#ifdef __cplusplus
extern "C" {
#endif

struct w2c_wasi__snapshot__preview1 {
  wasm_rt_memory_t* memory;
  uvwasi_t* uvwasi;
};

void wasm2c_uvwasi_link(struct w2c_wasi__snapshot__preview1* wasi,
                        wasm_rt_memory_t* memory,
                        uvwasi_t* uvwasi);

#ifdef __cplusplus
}
#endif

#endif

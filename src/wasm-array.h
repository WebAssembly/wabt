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

#ifndef WASM_ARRAY_H_
#define WASM_ARRAY_H_

#include <stddef.h>

#include "wasm-allocator.h"
#include "wasm-common.h"

#define WASM_DEFINE_ARRAY(name, type)                                        \
  typedef struct type##Array {                                               \
    type* data;                                                              \
    size_t size;                                                             \
  } type##Array;                                                             \
                                                                             \
  WASM_EXTERN_C_BEGIN                                                        \
  static WASM_INLINE void wasm_destroy_##name##_array(                       \
      struct WasmAllocator* allocator, type##Array* array) WASM_UNUSED;      \
  static WASM_INLINE WasmResult wasm_new_##name##_array(                     \
      struct WasmAllocator* allocator, type##Array* array, size_t size)      \
      WASM_UNUSED;                                                           \
  WASM_EXTERN_C_END                                                          \
                                                                             \
  void wasm_destroy_##name##_array(struct WasmAllocator* allocator,          \
                                   type##Array* array) {                     \
    wasm_free(allocator, array->data);                                       \
  }                                                                          \
  WasmResult wasm_new_##name##_array(struct WasmAllocator* allocator,        \
                                     type##Array* array, size_t size) {      \
    array->size = size;                                                      \
    array->data =                                                            \
        wasm_alloc_zero(allocator, size * sizeof(type), WASM_DEFAULT_ALIGN); \
    return array->data ? WASM_OK : WASM_ERROR;                               \
  }

#define WASM_DESTROY_ARRAY_AND_ELEMENTS(allocator, v, name) \
  {                                                         \
    int i;                                                  \
    for (i = 0; i < (v).size; ++i)                          \
      wasm_destroy_##name(allocator, &((v).data[i]));       \
    wasm_destroy_##name##_array(allocator, &(v));           \
  }

#endif /* WASM_ARRAY_H_ */

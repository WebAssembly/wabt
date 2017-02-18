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

#ifndef WABT_ARRAY_H_
#define WABT_ARRAY_H_

#include <stddef.h>

#include "common.h"

#define WABT_DEFINE_ARRAY(name, type)                                       \
  typedef struct type##Array {                                              \
    type* data;                                                             \
    size_t size;                                                            \
  } type##Array;                                                            \
                                                                            \
  WABT_EXTERN_C_BEGIN                                                       \
  static WABT_INLINE void wabt_destroy_##name##_array(type##Array* array)   \
      WABT_UNUSED;                                                          \
  static WABT_INLINE void wabt_new_##name##_array(type##Array* array,       \
                                                  size_t size) WABT_UNUSED; \
  WABT_EXTERN_C_END                                                         \
                                                                            \
  void wabt_destroy_##name##_array(type##Array* array) {                    \
    wabt_free(array->data);                                                 \
  }                                                                         \
  void wabt_new_##name##_array(type##Array* array, size_t size) {           \
    array->size = size;                                                     \
    array->data = wabt_alloc_zero(size * sizeof(type));                     \
  }

#define WABT_DESTROY_ARRAY_AND_ELEMENTS(v, name) \
  {                                              \
    size_t i;                                    \
    for (i = 0; i < (v).size; ++i)               \
      wabt_destroy_##name(&((v).data[i]));       \
    wabt_destroy_##name##_array(&(v));           \
  }

#endif /* WABT_ARRAY_H_ */

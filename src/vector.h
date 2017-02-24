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

#ifndef WABT_VECTOR_H_
#define WABT_VECTOR_H_

#include <stddef.h>

#include "common.h"
#include "config.h"

/*
 * WABT_DEFINE_VECTOR(widget, WabtWidget) defines struct and functions like the
 * following:
 *
 * typedef struct WabtWidgetVector {
 *   WabtWidget* data;
 *   size_t size;
 *   size_t capacity;
 * } WabtWidgetVector;
 *
 * void wabt_destroy_widget_vector(WabtWidgetVector* vec);
 * WabtWidget* wabt_append_widget(WabtWidgetVector* vec);
 * void wabt_resize_widget_vector(WabtWidgetVector* vec, size_t size);
 * void wabt_reserve_widgets(WabtWidgetVector* vec, size_t desired);
 * void wabt_append_widget_value(WabtWidgetVector* vec,
 *                               const WabtWidget* value);
 * void wabt_extend_widgets(WabtWidgetVector* dst, const WabtWidgetVector* src);
 */

#define WABT_DEFINE_VECTOR(name, type)                                        \
  typedef struct type##Vector {                                               \
    type* data;                                                               \
    size_t size;                                                              \
    size_t capacity;                                                          \
  } type##Vector;                                                             \
                                                                              \
  WABT_EXTERN_C_BEGIN                                                         \
  static WABT_INLINE void wabt_destroy_##name##_vector(type##Vector* vec)     \
      WABT_UNUSED;                                                            \
  static WABT_INLINE void wabt_resize_##name##_vector(                        \
      type##Vector* vec, size_t desired) WABT_UNUSED;                         \
  static WABT_INLINE void wabt_reserve_##name##s(type##Vector* vec,           \
                                                 size_t desired) WABT_UNUSED; \
  static WABT_INLINE type* wabt_append_##name(type##Vector* vec) WABT_UNUSED; \
  static WABT_INLINE void wabt_append_##name##_value(                         \
      type##Vector* vec, const type* value) WABT_UNUSED;                      \
  static WABT_INLINE void wabt_extend_##name##s(                              \
      type##Vector* dst, const type##Vector* src) WABT_UNUSED;                \
  WABT_EXTERN_C_END                                                           \
                                                                              \
  void wabt_destroy_##name##_vector(type##Vector* vec) {                      \
    wabt_free(vec->data);                                                     \
    vec->data = nullptr;                                                      \
    vec->size = 0;                                                            \
    vec->capacity = 0;                                                        \
  }                                                                           \
  void wabt_resize_##name##_vector(type##Vector* vec, size_t size) {          \
    wabt_resize_vector((void**)&vec->data, &vec->size, &vec->capacity, size,  \
                       sizeof(type));                                         \
  }                                                                           \
  void wabt_reserve_##name##s(type##Vector* vec, size_t desired) {            \
    wabt_ensure_capacity((void**)&vec->data, &vec->capacity, desired,         \
                         sizeof(type));                                       \
  }                                                                           \
  type* wabt_append_##name(type##Vector* vec) {                               \
    return (type*)wabt_append_element((void**)&vec->data, &vec->size,         \
                                      &vec->capacity, sizeof(type));          \
  }                                                                           \
  void wabt_append_##name##_value(type##Vector* vec, const type* value) {     \
    type* slot = wabt_append_##name(vec);                                     \
    *slot = *value;                                                           \
  }                                                                           \
  void wabt_extend_##name##s(type##Vector* dst, const type##Vector* src) {    \
    wabt_extend_elements((void**)&dst->data, &dst->size, &dst->capacity,      \
                         (const void**)&src->data, src->size, sizeof(type));  \
  }

#define WABT_DESTROY_VECTOR_AND_ELEMENTS(v, name) \
  {                                               \
    size_t i;                                     \
    for (i = 0; i < (v).size; ++i)                \
      wabt_destroy_##name(&((v).data[i]));        \
    wabt_destroy_##name##_vector(&(v));           \
  }

WABT_EXTERN_C_BEGIN
void wabt_ensure_capacity(void** data,
                          size_t* capacity,
                          size_t desired_size,
                          size_t elt_byte_size);

void wabt_resize_vector(void** data,
                        size_t* size,
                        size_t* capacity,
                        size_t desired_size,
                        size_t elt_byte_size);

void* wabt_append_element(void** data,
                          size_t* size,
                          size_t* capacity,
                          size_t elt_byte_size);

void wabt_extend_elements(void** dst,
                          size_t* dst_size,
                          size_t* dst_capacity,
                          const void** src,
                          size_t src_size,
                          size_t elt_byte_size);
WABT_EXTERN_C_END

#endif /* WABT_VECTOR_H_ */

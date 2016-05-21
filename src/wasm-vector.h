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

#ifndef WASM_VECTOR_H_
#define WASM_VECTOR_H_

#include <stddef.h>

#include "wasm-allocator.h"
#include "wasm-common.h"
#include "wasm-config.h"

/*
 * WASM_DEFINE_VECTOR(widget, WasmWidget) defines struct and functions like the
 * following:
 *
 * typedef struct WasmWidgetVector {
 *   WasmWidget* data;
 *   size_t size;
 *   size_t capacity;
 * } WasmWidgetVector;
 *
 * void wasm_destroy_widget_vector(WasmAllocator*, WasmWidgetVector* vec);
 * WasmWidget* wasm_append_widget(WasmAllocator*, WasmWidgetVector* vec);
 * void wasm_resize_widget_vector(WasmAllocator*, WasmWidgetVector* vec,
 *                                size_t size);
 * void wasm_reserve_widgets(WasmAllocator*, WasmWidgetVector* vec,
 *                           size_t desired);
 * void wasm_append_widget_value(WasmAllocator*, WasmWidgetVector* vec,
 *                               const WasmWidget* value);
 * void wasm_extend_widgets(WasmAllocator*, WasmWidgetVector* dst,
 *                          const WasmWidgetVector* src);
 */

#define WASM_DEFINE_VECTOR(name, type)                                         \
  typedef struct type##Vector {                                                \
    type* data;                                                                \
    size_t size;                                                               \
    size_t capacity;                                                           \
  } type##Vector;                                                              \
                                                                               \
  WASM_EXTERN_C_BEGIN                                                          \
  static WASM_INLINE void wasm_destroy_##name##_vector(                        \
      struct WasmAllocator* allocator, type##Vector* vec) WASM_UNUSED;         \
  static WASM_INLINE void wasm_resize_##name##_vector(                         \
      struct WasmAllocator* allocator, type##Vector* vec, size_t desired)      \
      WASM_UNUSED;                                                             \
  static WASM_INLINE void wasm_reserve_##name##s(                              \
      struct WasmAllocator* allocator, type##Vector* vec, size_t desired)      \
      WASM_UNUSED;                                                             \
  static WASM_INLINE type* wasm_append_##name(struct WasmAllocator* allocator, \
                                              type##Vector* vec) WASM_UNUSED;  \
  static WASM_INLINE void wasm_append_##name##_value(                          \
      struct WasmAllocator* allocator, type##Vector* vec, const type* value)   \
      WASM_UNUSED;                                                             \
  static WASM_INLINE void wasm_extend_##name##s(                               \
      struct WasmAllocator* allocator, type##Vector* dst,                      \
      const type##Vector* src) WASM_UNUSED;                                    \
  WASM_EXTERN_C_END                                                            \
                                                                               \
  void wasm_destroy_##name##_vector(struct WasmAllocator* allocator,           \
                                    type##Vector* vec) {                       \
    wasm_free(allocator, vec->data);                                           \
    vec->data = NULL;                                                          \
    vec->size = 0;                                                             \
    vec->capacity = 0;                                                         \
  }                                                                            \
  void wasm_resize_##name##_vector(struct WasmAllocator* allocator,            \
                                   type##Vector* vec, size_t size) {           \
    wasm_resize_vector(allocator, (void**) & vec->data, &vec->size,            \
                       &vec->capacity, size, sizeof(type));                    \
  }                                                                            \
  void wasm_reserve_##name##s(struct WasmAllocator* allocator,                 \
                              type##Vector* vec, size_t desired) {             \
    wasm_ensure_capacity(allocator, (void**) & vec->data, &vec->capacity,      \
                         desired, sizeof(type));                               \
  }                                                                            \
  type* wasm_append_##name(struct WasmAllocator* allocator,                    \
                           type##Vector* vec) {                                \
    return wasm_append_element(allocator, (void**)&vec->data, &vec->size,      \
                               &vec->capacity, sizeof(type));                  \
  }                                                                            \
  void wasm_append_##name##_value(struct WasmAllocator* allocator,             \
                                  type##Vector* vec, const type* value) {      \
    type* slot = wasm_append_##name(allocator, vec);                           \
    *slot = *value;                                                            \
  }                                                                            \
  void wasm_extend_##name##s(struct WasmAllocator* allocator,                  \
                             type##Vector* dst, const type##Vector* src) {     \
    wasm_extend_elements(allocator, (void**) & dst->data, &dst->size,          \
                         &dst->capacity, (const void**)&src->data, src->size,  \
                         sizeof(type));                                        \
  }

#define WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, v, name) \
  {                                                          \
    size_t i;                                                \
    for (i = 0; i < (v).size; ++i)                           \
      wasm_destroy_##name(allocator, &((v).data[i]));        \
    wasm_destroy_##name##_vector(allocator, &(v));           \
  }

WASM_EXTERN_C_BEGIN
void wasm_ensure_capacity(struct WasmAllocator*,
                          void** data,
                          size_t* capacity,
                          size_t desired_size,
                          size_t elt_byte_size);

void wasm_resize_vector(struct WasmAllocator*,
                        void** data,
                        size_t* size,
                        size_t* capacity,
                        size_t desired_size,
                        size_t elt_byte_size);

void* wasm_append_element(struct WasmAllocator*,
                          void** data,
                          size_t* size,
                          size_t* capacity,
                          size_t elt_byte_size);

void wasm_extend_elements(struct WasmAllocator*,
                          void** dst,
                          size_t* dst_size,
                          size_t* dst_capacity,
                          const void** src,
                          size_t src_size,
                          size_t elt_byte_size);
WASM_EXTERN_C_END

#endif /* WASM_VECTOR_H_ */

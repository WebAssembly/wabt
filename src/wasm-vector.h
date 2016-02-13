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

#include <stdlib.h>

#include "wasm-common.h"

/*
 * DECLARE_VECTOR(widget, WasmWidget) defines struct and functions like the
 * following:
 *
 * typedef struct WasmWidgetVector {
 *   WasmWidget* data;
 *   size_t size;
 *   size_t capacity;
 * } WasmWidgetVector;
 *
 * void wasm_destroy_widget_vector(WasmWidgetVector* vec);
 * type* wasm_append_widget(WasmWidgetVector* vec);
 * WasmResult wasm_reserve_widgets(WasmWidgetVector* vec, size_t desired);
 * WasmResult wasm_append_widget_value(WasmWidgetVector* vec, type* value);
 * WasmResult wasm_extend_widgets(WasmWidgetVector* dst, WasmWidgetVector* src);
 */

#define DECLARE_VECTOR(name, type)                                         \
  typedef struct type##Vector {                                            \
    type* data;                                                            \
    size_t size;                                                           \
    size_t capacity;                                                       \
  } type##Vector;                                                          \
  EXTERN_C void wasm_destroy_##name##_vector(type##Vector* vec);           \
  EXTERN_C type* wasm_append_##name(type##Vector* vec) WARN_UNUSED;        \
  EXTERN_C WasmResult wasm_reserve_##name##s(type##Vector* vec,            \
                                             size_t desired);              \
  EXTERN_C WasmResult wasm_append_##name##_value(type##Vector* vec,        \
                                                 type* value) WARN_UNUSED; \
  EXTERN_C WasmResult wasm_extend_##name##s(type##Vector* dst,             \
                                            type##Vector* src) WARN_UNUSED;

#define DEFINE_VECTOR(name, type)                                              \
  void wasm_destroy_##name##_vector(type##Vector* vec) { free(vec->data); }    \
  WasmResult wasm_reserve_##name##s(type##Vector* vec, size_t desired) {       \
    return wasm_ensure_capacity((void**)&vec->data, &vec->capacity, desired,   \
                                sizeof(type));                                 \
  }                                                                            \
  type* wasm_append_##name(type##Vector* vec) {                                \
    return wasm_append_element((void**)&vec->data, &vec->size, &vec->capacity, \
                               sizeof(type));                                  \
  }                                                                            \
  WasmResult wasm_append_##name##_value(type##Vector* vec, type* value) {      \
    type* slot = wasm_append_##name(vec);                                      \
    if (!slot)                                                                 \
      return WASM_ERROR;                                                       \
    *slot = *value;                                                            \
    return WASM_OK;                                                            \
  }                                                                            \
  WasmResult wasm_extend_##name##s(type##Vector* dst, type##Vector* src) {     \
    return wasm_extend_elements((void**)&dst->data, &dst->size,                \
                                &dst->capacity, (const void**)&src->data,      \
                                src->size, sizeof(type));                      \
  }

#define DESTROY_VECTOR_AND_ELEMENTS(V, name) \
  {                                          \
    int i;                                   \
    for (i = 0; i < (V).size; ++i)           \
      wasm_destroy_##name(&((V).data[i]));   \
    wasm_destroy_##name##_vector(&(V));      \
  }

WasmResult wasm_ensure_capacity(void** data,
                                size_t* capacity,
                                size_t desired_size,
                                size_t elt_byte_size);

void* wasm_append_element(void** data,
                          size_t* size,
                          size_t* capacity,
                          size_t elt_byte_size) WARN_UNUSED;

WasmResult wasm_extend_elements(void** dst,
                                size_t* dst_size,
                                size_t* dst_capacity,
                                const void** src,
                                size_t src_size,
                                size_t elt_byte_size) WARN_UNUSED;

#endif /* WASM_VECTOR_H_ */

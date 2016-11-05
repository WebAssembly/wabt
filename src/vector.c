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

#include "vector.h"

#include "allocator.h"

#define INITIAL_VECTOR_CAPACITY 8

void wabt_ensure_capacity(WabtAllocator* allocator,
                                void** data,
                                size_t* capacity,
                                size_t desired_size,
                                size_t elt_byte_size) {
  if (desired_size > *capacity) {
    size_t new_capacity = *capacity ? *capacity * 2 : INITIAL_VECTOR_CAPACITY;
    while (new_capacity < desired_size)
      new_capacity *= 2;
    size_t new_byte_size = new_capacity * elt_byte_size;
    *data = wabt_realloc(allocator, *data, new_byte_size, WABT_DEFAULT_ALIGN);
    *capacity = new_capacity;
  }
}

void wabt_resize_vector(struct WabtAllocator* allocator,
                              void** data,
                              size_t* size,
                              size_t* capacity,
                              size_t desired_size,
                              size_t elt_byte_size) {
  size_t old_size = *size;
  wabt_ensure_capacity(allocator, data, capacity, desired_size, elt_byte_size);
  if (desired_size > old_size) {
    memset((void*)((size_t)*data + old_size * elt_byte_size), 0,
           (desired_size - old_size) * elt_byte_size);
  }
  *size = desired_size;
}

void* wabt_append_element(WabtAllocator* allocator,
                          void** data,
                          size_t* size,
                          size_t* capacity,
                          size_t elt_byte_size) {
  wabt_ensure_capacity(allocator, data, capacity, *size + 1, elt_byte_size);
  void* p = (void*)((size_t)*data + (*size)++ * elt_byte_size);
  memset(p, 0, elt_byte_size);
  return p;
}

void wabt_extend_elements(WabtAllocator* allocator,
                                void** dst,
                                size_t* dst_size,
                                size_t* dst_capacity,
                                const void** src,
                                size_t src_size,
                                size_t elt_byte_size) {
  wabt_ensure_capacity(allocator, dst, dst_capacity, *dst_size + src_size,
                       elt_byte_size);
  memcpy((void*)((size_t)*dst + (*dst_size * elt_byte_size)), *src,
         src_size * elt_byte_size);
  *dst_size += src_size;
}

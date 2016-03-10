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

#include "wasm-allocator.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct MemInfo {
  void* real_pointer; /* pointer as returned from malloc */
  size_t size;
} MemInfo;

#ifndef NDEBUG
static int is_power_of_two(size_t x) {
  return x && ((x & (x - 1)) == 0);
}

static int is_aligned(void* p, size_t align) {
  return ((intptr_t)p & (align - 1)) == 0;
}
#endif /* NDEBUG */

static void* align_up(void* p, size_t align) {
  return (void*)(((intptr_t)p + align - 1) & ~(align - 1));
}

static void* libc_alloc(WasmAllocator* allocator, size_t size, size_t align) {
  assert(is_power_of_two(align));
  if (align < sizeof(void*))
    align = sizeof(void*);

  void* p = malloc(size + sizeof(MemInfo) + align - 1);
  if (!p)
    return NULL;

  void* aligned = align_up((void*)((size_t)p + sizeof(MemInfo)), align);
  MemInfo* mem_info = (MemInfo*)aligned - 1;
  assert(is_aligned(mem_info, sizeof(void*)));
  mem_info->real_pointer = p;
  mem_info->size = size;
  return aligned;
}

static void libc_free(WasmAllocator* allocator, void* p) {
  if (!p)
    return;

  MemInfo* mem_info = (MemInfo*)p - 1;
  free(mem_info->real_pointer);
}

static void* libc_realloc(WasmAllocator* allocator,
                          void* p,
                          size_t size,
                          size_t align) {
  if (!p)
    return libc_alloc(allocator, size, align);

  MemInfo* mem_info = (MemInfo*)p - 1;
  void* new_p = libc_alloc(allocator, size, align);
  if (!new_p)
    return NULL;

  size_t copy_size = size > mem_info->size ? mem_info->size : size;
  memcpy(new_p, p, copy_size);
  free(mem_info->real_pointer);
  return new_p;
}

WasmAllocator g_wasm_libc_allocator = {libc_alloc, libc_realloc, libc_free};

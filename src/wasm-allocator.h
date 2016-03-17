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

#ifndef WASM_ALLOCATOR_H_
#define WASM_ALLOCATOR_H_

#include <stdlib.h>
#include <string.h>

#include "wasm-common.h"

#define WASM_DEFAULT_ALIGN sizeof(void*)

typedef struct WasmAllocator {
  void* (*alloc)(struct WasmAllocator*, size_t size, size_t align);
  void* (*realloc)(struct WasmAllocator*, void* p, size_t size, size_t align);
  void (*free)(struct WasmAllocator*, void* p);
} WasmAllocator;

extern WasmAllocator g_wasm_libc_allocator;

WASM_EXTERN_C_BEGIN
static WASM_INLINE void* wasm_alloc(WasmAllocator* allocator,
                                    size_t size,
                                    size_t align) {
  return allocator->alloc(allocator, size, align);
}

static WASM_INLINE void* wasm_alloc_zero(WasmAllocator* allocator,
                                         size_t size,
                                         size_t align) {
  void* result = allocator->alloc(allocator, size, align);
  if (!result)
    return NULL;
  memset(result, 0, size);
  return result;
}

static WASM_INLINE void* wasm_realloc(WasmAllocator* allocator,
                                      void* p,
                                      size_t size,
                                      size_t align) {
  return allocator->realloc(allocator, p, size, align);
}

static WASM_INLINE void wasm_free(WasmAllocator* allocator, void* p) {
  allocator->free(allocator, p);
}

static WASM_INLINE char* wasm_strndup(WasmAllocator* allocator,
                                      const char* s,
                                      size_t len) {
  size_t real_len = 0;
  const char* p = s;
  while (*p && real_len < len) {
    p++;
    real_len++;
  }

  char* new_s = allocator->alloc(allocator, real_len + 1, 1);
  if (!new_s)
    return NULL;

  memcpy(new_s, s, real_len);
  new_s[real_len] = 0;
  return new_s;
}
WASM_EXTERN_C_END

#endif /* WASM_ALLOCATOR_H_ */

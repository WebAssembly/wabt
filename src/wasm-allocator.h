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

typedef void* WasmAllocatorMark;

typedef struct WasmAllocator {
  void* (*alloc)(struct WasmAllocator*,
                 size_t size,
                 size_t align,
                 const char* file,
                 int line);
  void* (*realloc)(struct WasmAllocator*,
                   void* p,
                   size_t size,
                   size_t align,
                   const char* file,
                   int line);
  void (*free)(struct WasmAllocator*, void* p, const char* file, int line);
  /* destroy the allocator */
  void (*destroy)(struct WasmAllocator*);
  /* mark/reset_to_mark are only supported by the stack allocator */
  WasmAllocatorMark (*mark)(struct WasmAllocator*);
  void (*reset_to_mark)(struct WasmAllocator*, WasmAllocatorMark);
  void (*print_stats)(struct WasmAllocator*);
} WasmAllocator;

extern WasmAllocator g_wasm_libc_allocator;

#define wasm_alloc(allocator, size, align) \
  (allocator)->alloc((allocator), (size), (align), __FILE__, __LINE__)

#define wasm_alloc_zero(allocator, size, align) \
  wasm_alloc_zero_((allocator), (size), (align), __FILE__, __LINE__)

#define wasm_realloc(allocator, p, size, align) \
  (allocator)->realloc((allocator), (p), (size), (align), __FILE__, __LINE__)

#define wasm_free(allocator, p) \
  (allocator)->free((allocator), (p), __FILE__, __LINE__)

#define wasm_destroy_allocator(allocator) (allocator)->destroy(allocator)
#define wasm_mark(allocator) (allocator)->mark(allocator)

#define wasm_reset_to_mark(allocator, mark) \
  (allocator)->reset_to_mark(allocator, mark)

#define wasm_print_allocator_stats(allocator) \
  (allocator)->print_stats(allocator)

#define wasm_strndup(allocator, s, len) \
  wasm_strndup_(allocator, s, len, __FILE__, __LINE__)

#define wasm_dup_string_slice(allocator, str) \
  wasm_dup_string_slice_(allocator, str, __FILE__, __LINE__)

WASM_EXTERN_C_BEGIN

WasmAllocator* wasm_get_libc_allocator(void);

static WASM_INLINE void* wasm_alloc_zero_(WasmAllocator* allocator,
                                          size_t size,
                                          size_t align,
                                          const char* file,
                                          int line) {
  void* result = allocator->alloc(allocator, size, align, file, line);
  if (!result)
    return NULL;
  memset(result, 0, size);
  return result;
}

static WASM_INLINE char* wasm_strndup_(WasmAllocator* allocator,
                                       const char* s,
                                       size_t len,
                                       const char* file,
                                       int line) {
  size_t real_len = 0;
  const char* p = s;
  while (real_len < len && *p) {
    p++;
    real_len++;
  }

  char* new_s = allocator->alloc(allocator, real_len + 1, 1, file, line);
  if (!new_s)
    return NULL;

  memcpy(new_s, s, real_len);
  new_s[real_len] = 0;
  return new_s;
}

static WASM_INLINE WasmStringSlice
wasm_dup_string_slice_(WasmAllocator* allocator,
                       WasmStringSlice str,
                       const char* file,
                       int line) {
  WasmStringSlice result;
  result.start = wasm_strndup_(allocator, str.start, str.length, file, line);
  result.length = str.length;
  return result;
}

WASM_EXTERN_C_END

#endif /* WASM_ALLOCATOR_H_ */

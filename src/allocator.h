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

#ifndef WABT_ALLOCATOR_H_
#define WABT_ALLOCATOR_H_

#include <stdlib.h>
#include <string.h>

#include "common.h"

#define WABT_DEFAULT_ALIGN sizeof(void*)

typedef void* WabtAllocatorMark;

typedef struct WabtAllocator {
  void* (*alloc)(struct WabtAllocator*,
                 size_t size,
                 size_t align,
                 const char* file,
                 int line);
  void* (*realloc)(struct WabtAllocator*,
                   void* p,
                   size_t size,
                   size_t align,
                   const char* file,
                   int line);
  void (*free)(struct WabtAllocator*, void* p, const char* file, int line);
  /* destroy the allocator */
  void (*destroy)(struct WabtAllocator*);
  /* mark/reset_to_mark are only supported by the stack allocator */
  WabtAllocatorMark (*mark)(struct WabtAllocator*);
  void (*reset_to_mark)(struct WabtAllocator*, WabtAllocatorMark);
  void (*print_stats)(struct WabtAllocator*);
  /* set the location to longjmp to if allocation fails. the return value is 0
   * for normal execution, and 1 if an allocation failed. */
  int (*setjmp_handler)(struct WabtAllocator*);
} WabtAllocator;

extern WabtAllocator g_wabt_libc_allocator;

#define wabt_alloc(allocator, size, align) \
  (allocator)->alloc((allocator), (size), (align), __FILE__, __LINE__)

#define wabt_alloc_zero(allocator, size, align) \
  wabt_alloc_zero_((allocator), (size), (align), __FILE__, __LINE__)

#define wabt_realloc(allocator, p, size, align) \
  (allocator)->realloc((allocator), (p), (size), (align), __FILE__, __LINE__)

#define wabt_free(allocator, p) \
  (allocator)->free((allocator), (p), __FILE__, __LINE__)

#define wabt_destroy_allocator(allocator) (allocator)->destroy(allocator)
#define wabt_mark(allocator) (allocator)->mark(allocator)

#define wabt_reset_to_mark(allocator, mark) \
  (allocator)->reset_to_mark(allocator, mark)

#define wabt_print_allocator_stats(allocator) \
  (allocator)->print_stats(allocator)

#define wabt_strndup(allocator, s, len) \
  wabt_strndup_(allocator, s, len, __FILE__, __LINE__)

#define wabt_dup_string_slice(allocator, str) \
  wabt_dup_string_slice_(allocator, str, __FILE__, __LINE__)

WABT_EXTERN_C_BEGIN

WabtAllocator* wabt_get_libc_allocator(void);

static WABT_INLINE void* wabt_alloc_zero_(WabtAllocator* allocator,
                                          size_t size,
                                          size_t align,
                                          const char* file,
                                          int line) {
  void* result = allocator->alloc(allocator, size, align, file, line);
  memset(result, 0, size);
  return result;
}

static WABT_INLINE char* wabt_strndup_(WabtAllocator* allocator,
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

  char* new_s = (char*)allocator->alloc(allocator, real_len + 1, 1, file, line);
  memcpy(new_s, s, real_len);
  new_s[real_len] = 0;
  return new_s;
}

static WABT_INLINE WabtStringSlice
wabt_dup_string_slice_(WabtAllocator* allocator,
                       WabtStringSlice str,
                       const char* file,
                       int line) {
  WabtStringSlice result;
  result.start = wabt_strndup_(allocator, str.start, str.length, file, line);
  result.length = str.length;
  return result;
}

WABT_EXTERN_C_END

#endif /* WABT_ALLOCATOR_H_ */

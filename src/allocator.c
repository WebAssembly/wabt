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

#include "allocator.h"

#include <assert.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct MemInfo {
  void* real_pointer; /* pointer as returned from malloc */
  size_t size;
} MemInfo;

static WabtBool s_has_jmpbuf;
static jmp_buf s_jmpbuf;

#ifndef NDEBUG
static WabtBool is_power_of_two(size_t x) {
  return x && ((x & (x - 1)) == 0);
}

static WabtBool is_aligned(void* p, size_t align) {
  return ((intptr_t)p & (align - 1)) == 0;
}
#endif /* NDEBUG */

static void* align_up(void* p, size_t align) {
  return (void*)(((intptr_t)p + align - 1) & ~(align - 1));
}

static void* libc_alloc(WabtAllocator* allocator,
                        size_t size,
                        size_t align,
                        const char* file,
                        int line) {
  assert(is_power_of_two(align));
  if (align < sizeof(void*))
    align = sizeof(void*);

  void* p = malloc(size + sizeof(MemInfo) + align - 1);
  if (!p) {
    if (s_has_jmpbuf)
      longjmp(s_jmpbuf, 1);
    WABT_FATAL("%s:%d: memory allocation failed\n", file, line);
  }

  void* aligned = align_up((void*)((size_t)p + sizeof(MemInfo)), align);
  MemInfo* mem_info = (MemInfo*)aligned - 1;
  assert(is_aligned(mem_info, sizeof(void*)));
  mem_info->real_pointer = p;
  mem_info->size = size;
  return aligned;
}

static void libc_free(WabtAllocator* allocator,
                      void* p,
                      const char* file,
                      int line) {
  if (!p)
    return;

  MemInfo* mem_info = (MemInfo*)p - 1;
  free(mem_info->real_pointer);
}

/* nothing to destroy */
static void libc_destroy(WabtAllocator* allocator) {}

static void* libc_realloc(WabtAllocator* allocator,
                          void* p,
                          size_t size,
                          size_t align,
                          const char* file,
                          int line) {
  if (!p)
    return libc_alloc(allocator, size, align, NULL, 0);

  MemInfo* mem_info = (MemInfo*)p - 1;
  void* new_p = libc_alloc(allocator, size, align, NULL, 0);
  size_t copy_size = size > mem_info->size ? mem_info->size : size;
  memcpy(new_p, p, copy_size);
  free(mem_info->real_pointer);
  return new_p;
}

/* mark/reset_to_mark are not supported by the libc allocator */
static WabtAllocatorMark libc_mark(WabtAllocator* allocator) {
  return NULL;
}

static void libc_reset_to_mark(WabtAllocator* allocator,
                               WabtAllocatorMark mark) {}

static void libc_print_stats(WabtAllocator* allocator) {
  /* TODO(binji): nothing for now, implement later */
}

static int libc_setjmp_handler(WabtAllocator* allocator) {
  s_has_jmpbuf = WABT_TRUE;
  return setjmp(s_jmpbuf);
}

WabtAllocator g_wabt_libc_allocator = {
    libc_alloc, libc_realloc,       libc_free,        libc_destroy,
    libc_mark,  libc_reset_to_mark, libc_print_stats, libc_setjmp_handler};

WabtAllocator* wabt_get_libc_allocator(void) {
  return &g_wabt_libc_allocator;
}

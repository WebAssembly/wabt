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

#ifndef WABT_STACK_ALLOCATOR_H_
#define WABT_STACK_ALLOCATOR_H_

#include "allocator.h"
#include "common.h"

#include <setjmp.h>

#define WABT_STACK_ALLOCATOR_STATS 0

typedef struct WabtStackAllocatorChunk {
  void* start;
  void* current;
  void* end;
  union {
    struct WabtStackAllocatorChunk* prev;
    struct WabtStackAllocatorChunk* next_free;
  };
} WabtStackAllocatorChunk;

typedef struct WabtStackAllocator {
  WabtAllocator allocator;
  WabtStackAllocatorChunk* first;
  WabtStackAllocatorChunk* last;
  WabtAllocator* fallback;
  void* last_allocation;
  WabtStackAllocatorChunk* next_free;
  WabtBool has_jmpbuf;
  jmp_buf jmpbuf;

#if WABT_STACK_ALLOCATOR_STATS
  /* some random stats */
  size_t chunk_alloc_count;
  size_t alloc_count;
  size_t realloc_count;
  size_t free_count;
  size_t total_chunk_bytes;
  size_t total_alloc_bytes;
  size_t total_realloc_bytes;
#endif /* WABT_STACK_ALLOCATOR_STATS */
} WabtStackAllocator;

WABT_EXTERN_C_BEGIN
WabtResult wabt_init_stack_allocator(WabtStackAllocator*,
                                     WabtAllocator* fallback);
void wabt_destroy_stack_allocator(WabtStackAllocator*);
WABT_EXTERN_C_END

#endif /* WABT_STACK_ALLOCATOR_H_ */

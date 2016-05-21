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

#ifndef WASM_STACK_ALLOCATOR_H_
#define WASM_STACK_ALLOCATOR_H_

#include "wasm-allocator.h"
#include "wasm-common.h"

#include <setjmp.h>

#define WASM_STACK_ALLOCATOR_STATS 0

typedef struct WasmStackAllocatorChunk {
  void* start;
  void* current;
  void* end;
  union {
    struct WasmStackAllocatorChunk* prev;
    struct WasmStackAllocatorChunk* next_free;
  };
} WasmStackAllocatorChunk;

typedef struct WasmStackAllocator {
  WasmAllocator allocator;
  WasmStackAllocatorChunk* first;
  WasmStackAllocatorChunk* last;
  WasmAllocator* fallback;
  void* last_allocation;
  WasmStackAllocatorChunk* next_free;
  WasmBool has_jmpbuf;
  jmp_buf jmpbuf;

#if WASM_STACK_ALLOCATOR_STATS
  /* some random stats */
  size_t chunk_alloc_count;
  size_t alloc_count;
  size_t realloc_count;
  size_t free_count;
  size_t total_chunk_bytes;
  size_t total_alloc_bytes;
  size_t total_realloc_bytes;
#endif /* WASM_STACK_ALLOCATOR_STATS */
} WasmStackAllocator;

WASM_EXTERN_C_BEGIN
WasmResult wasm_init_stack_allocator(WasmStackAllocator*,
                                     WasmAllocator* fallback);
void wasm_destroy_stack_allocator(WasmStackAllocator*);
WASM_EXTERN_C_END

#endif /* WASM_STACK_ALLOCATOR_H_ */

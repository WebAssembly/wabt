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

#include "wasm-stack-allocator.h"

#include <assert.h>
#include <string.h>

#include "wasm-internal.h"

#define CHUNK_ALIGN 8
#define CHUNK_SIZE (1024 * 1024)
#define CHUNK_MAX_AVAIL (CHUNK_SIZE - sizeof(WasmStackAllocatorChunk))

static int is_power_of_two(uint32_t x) {
  return x && ((x & (x - 1)) == 0);
}

static void* align_up(void* p, size_t align) {
  return (void*)(((intptr_t)p + align - 1) & ~(align - 1));
}

static int allocation_in_chunk(WasmStackAllocatorChunk* chunk, void* p) {
  return p >= (void*)chunk && p < chunk->end;
}

static WasmStackAllocatorChunk* allocate_chunk(
    WasmStackAllocator* stack_allocator,
    size_t max_avail) {
  size_t real_size = max_avail + sizeof(WasmStackAllocatorChunk);
  WasmStackAllocatorChunk* chunk = stack_allocator->fallback->alloc(
      stack_allocator->fallback, real_size, CHUNK_ALIGN);
  if (!chunk)
    return NULL;
  void* start = (void*)(chunk + sizeof(WasmStackAllocatorChunk));
  chunk->current = start;
  chunk->end = (void*)((size_t)start + max_avail);
  return chunk;
}

static void* stack_alloc(WasmAllocator* allocator, size_t size, size_t align) {
  WasmStackAllocator* stack_allocator = (WasmStackAllocator*)allocator;
  assert(is_power_of_two(align));
  WasmStackAllocatorChunk* chunk = stack_allocator->last;
  size_t alloc_size = size + align - 1;
  if (alloc_size >= CHUNK_MAX_AVAIL) {
    chunk = allocate_chunk(stack_allocator, alloc_size);
    if (!chunk)
      return NULL;
    void* p = align_up(chunk->current, align);
    assert((void*)((size_t)p + size) <= chunk->end);
    chunk->current = chunk->end;
    /* thread this chunk before first. There's no available space, so it's not
     worth considering. */
    stack_allocator->first->prev = chunk;
    stack_allocator->first = chunk;
    return p;
  }
  assert(chunk);

  chunk->current = align_up(chunk->current, align);
  void* result;
  void* new_current = (void*)((size_t)chunk->current + size);
  if (new_current < chunk->end) {
    result = chunk->current;
    chunk->current = new_current;
  } else {
    chunk = allocate_chunk(stack_allocator, CHUNK_MAX_AVAIL);
    if (!chunk)
      return NULL;
    chunk->prev = stack_allocator->last;
    stack_allocator->last = chunk;
    chunk->current = align_up(chunk->current, align);
    result = chunk->current;
    chunk->current = (void*)((size_t)chunk->current + size);
    assert(chunk->current <= chunk->end);
  }

  stack_allocator->last_allocation = result;
  return result;
}

static void* stack_realloc(WasmAllocator* allocator,
                           void* p,
                           size_t size,
                           size_t align) {
  if (!p)
    return stack_alloc(allocator, size, align);

  WasmStackAllocator* stack_allocator = (WasmStackAllocator*)allocator;

  /* TODO(binji): optimize */
  WasmStackAllocatorChunk* chunk = stack_allocator->last;
  while (chunk) {
    if (allocation_in_chunk(chunk, p))
      break;
    chunk = chunk->prev;
  }

  assert(chunk);
  void* result = stack_alloc(allocator, size, align);
  if (!result)
    return NULL;

  /* We know that the previously allocated data was at most extending from |p|
   to the end of the chunk. So we can copy at most that many bytes, or the
   new size, whichever is less. Use memmove because the regions may be
   overlapping. */
  size_t old_max_size = (size_t)chunk->end - (size_t)p;
  size_t copy_size = size > old_max_size ? old_max_size : size;
  memmove(result, p, copy_size);
  return result;
}

static void stack_free(WasmAllocator* allocator, void* p) {
  if (!p)
    return;

  WasmStackAllocator* stack_allocator = (WasmStackAllocator*)allocator;
  if (p != stack_allocator->last_allocation)
    return;

  WasmStackAllocatorChunk* chunk = stack_allocator->last;
  assert(allocation_in_chunk(chunk, p));
  chunk->current = p;
}

WasmResult wasm_init_stack_allocator(WasmStackAllocator* stack_allocator,
                                     WasmAllocator* fallback) {
  memset(stack_allocator, 0, sizeof(WasmStackAllocator));
  stack_allocator->allocator.alloc = stack_alloc;
  stack_allocator->allocator.realloc = stack_realloc;
  stack_allocator->allocator.free = stack_free;
  stack_allocator->fallback = fallback;

  WasmStackAllocatorChunk* chunk =
      allocate_chunk(stack_allocator, CHUNK_MAX_AVAIL);
  if (!chunk)
    return WASM_ERROR;
  chunk->prev = NULL;
  stack_allocator->first = stack_allocator->last = chunk;
  return WASM_OK;
}

void wasm_destroy_stack_allocator(WasmStackAllocator* stack_allocator) {
  wasm_reset_stack_allocator(stack_allocator);
  /* also destroy the first block, it is left around for reuse by
   stack_reset. */
  stack_allocator->fallback->free(stack_allocator->fallback,
                                  stack_allocator->first);
}

void wasm_reset_stack_allocator(WasmStackAllocator* stack_allocator) {
  WasmStackAllocatorChunk* chunk = stack_allocator->last;
  while (chunk != stack_allocator->first) {
    WasmStackAllocatorChunk* prev = chunk->prev;
    stack_allocator->fallback->free(stack_allocator->fallback, chunk);
    chunk = prev;
  }
  stack_allocator->first->current =
      (void*)(stack_allocator->first + sizeof(WasmStackAllocatorChunk));
  stack_allocator->first->prev = NULL;
  stack_allocator->last = stack_allocator->first;
  stack_allocator->last_allocation = NULL;
}

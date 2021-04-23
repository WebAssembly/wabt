/*
 * Copyright 2018 WebAssembly Community Group participants
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

#include "wasm-rt.h"
#include "wasm-rt-os.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if (defined(__linux__) || defined(__unix__) || defined(__APPLE__)) && \
    defined(__WORDSIZE) && __WORDSIZE == 64
    #define WASM_GUARDPAGE_MODEL
#endif

#define PAGE_SIZE 65536

void wasm_rt_trap(wasm_rt_trap_t code) {
  assert(code != WASM_RT_TRAP_NONE);
  abort();
}

static bool func_types_are_equal(wasm_func_type_t* a, wasm_func_type_t* b) {
  if (a->param_count != b->param_count || a->result_count != b->result_count)
    return 0;
  uint32_t i;
  for (i = 0; i < a->param_count; ++i)
    if (a->params[i] != b->params[i])
      return 0;
  for (i = 0; i < a->result_count; ++i)
    if (a->results[i] != b->results[i])
      return 0;
  return 1;
}

uint32_t wasm_rt_register_func_type(wasm_func_type_t** p_func_type_structs,
                                    uint32_t* p_func_type_count,
                                    uint32_t param_count,
                                    uint32_t result_count,
                                    wasm_rt_type_t* types) {
  wasm_func_type_t func_type;
  func_type.param_count = param_count;
  func_type.params = malloc(param_count * sizeof(wasm_rt_type_t));
  func_type.result_count = result_count;
  func_type.results = malloc(result_count * sizeof(wasm_rt_type_t));

  uint32_t i;
  for (i = 0; i < param_count; ++i)
    func_type.params[i] = types[i];
  for (i = 0; i < result_count; ++i)
    func_type.results[i] = types[(uint64_t)(param_count) + i];

  for (i = 0; i < *p_func_type_count; ++i) {
    wasm_func_type_t* func_types = *p_func_type_structs;
    if (func_types_are_equal(&func_types[i], &func_type)) {
      free(func_type.params);
      free(func_type.results);
      return i + 1;
    }
  }

  uint32_t idx = (*p_func_type_count)++;
  // realloc works fine even if *p_func_type_structs is null
  *p_func_type_structs = realloc(*p_func_type_structs, *p_func_type_count * sizeof(wasm_func_type_t));
  (*p_func_type_structs)[idx] = func_type;
  return idx + 1;
}

static void* mmap_aligned(void *addr, size_t requested_length, int prot, int flags, size_t alignment, size_t alignment_offset)
{
    size_t padded_length = requested_length + alignment + alignment_offset;
    uintptr_t unaligned = (uintptr_t) os_mmap(addr, padded_length, prot, flags);

    if (!unaligned) {
        return (void*) unaligned;
    }

    // Round up the next address that has addr % alignment = 0
    uintptr_t aligned_nonoffset = (unaligned + (alignment - 1)) & ~(alignment - 1);

    // Currently offset 0 is aligned according to alignment
    // Alignment needs to be enforced at the given offset
    uintptr_t aligned = 0;
    if ((aligned_nonoffset - alignment_offset) >= unaligned) {
        aligned = aligned_nonoffset - alignment_offset;
    } else {
        aligned = aligned_nonoffset - alignment_offset + alignment;
    }

    //Sanity check
    if (aligned < unaligned
        || (aligned + (requested_length - 1)) > (unaligned + (padded_length - 1))
        || (aligned + alignment_offset) % alignment != 0)
    {
        os_munmap((void*) unaligned, padded_length);
        return NULL;
    }

    {
        size_t unused_front = aligned - unaligned;
        if (unused_front != 0) {
            os_munmap((void*) unaligned, unused_front);
        }
    }

    {
        size_t unused_back = (unaligned + (padded_length - 1)) - (aligned + (requested_length - 1));
        if (unused_back != 0) {
            os_munmap((void*) (aligned + requested_length), unused_back);
        }
    }

    return (void*) aligned;
}

void wasm_rt_allocate_memory(wasm_rt_memory_t* memory,
                             uint32_t initial_pages,
                             uint32_t max_pages) {
  uint32_t byte_length = initial_pages * PAGE_SIZE;
#if defined(WASM_GUARDPAGE_MODEL)
  /* Reserve 8GiB. */
  const uint64_t heap_alignment = 0x100000000ul;
  void* addr = mmap_aligned(NULL, 0x200000000ul, MMAP_PROT_NONE, MMAP_MAP_NONE, heap_alignment, 0 /* alignment_offset */);
  if (addr == (void*)-1) {
    perror("mmap failed");
    abort();
  }
  os_mprotect(addr, byte_length, MMAP_PROT_READ | MMAP_PROT_WRITE);
  memory->data = addr;
#else
  memory->data = calloc(byte_length, 1);
#endif
  memory->size = byte_length;
  memory->pages = initial_pages;
  memory->max_pages = max_pages;
}

uint32_t wasm_rt_grow_memory(wasm_rt_memory_t* memory, uint32_t delta) {
  uint32_t old_pages = memory->pages;
  uint32_t new_pages = memory->pages + delta;
  if (new_pages == 0) {
    return 0;
  }
  if (new_pages < old_pages || new_pages > memory->max_pages) {
    return (uint32_t)-1;
  }
  uint32_t old_size = old_pages * PAGE_SIZE;
  uint32_t new_size = new_pages * PAGE_SIZE;
  uint32_t delta_size = delta * PAGE_SIZE;

#if defined(WASM_GUARDPAGE_MODEL)
  uint8_t* new_data = memory->data;
  os_mprotect(new_data + old_size, delta_size, MMAP_PROT_READ | MMAP_PROT_WRITE);
#else
  uint8_t* new_data = realloc(memory->data, new_size);
  if (new_data == NULL) {
    return (uint32_t)-1;
  }
#if !WABT_BIG_ENDIAN
  memset(new_data + old_size, 0, delta_size);
#endif
#endif
#if WABT_BIG_ENDIAN
  memmove(new_data + new_size - old_size, new_data, old_size);
  memset(new_data, 0, delta_size);
#endif
  memory->pages = new_pages;
  memory->size = new_size;
  memory->data = new_data;
  return old_pages;
}

void wasm_rt_allocate_table(wasm_rt_table_t* table,
                            uint32_t elements,
                            uint32_t max_elements) {
  assert(max_elements >= elements);
  table->size = elements;
  table->max_size = max_elements;
  table->data = calloc(table->max_size, sizeof(wasm_rt_elem_t));
}

#undef WASM_GUARDPAGE_MODEL
#undef PAGE_SIZE

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

#include "wasm-common.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "wasm-allocator.h"

#define DUMP_OCTETS_PER_LINE 16
#define DUMP_OCTETS_PER_GROUP 2

#define V(rtype, type1, type2, mem_size, code, NAME, text) [code] = mem_size,
static uint8_t s_opcode_mem_size[] = {WASM_FOREACH_OPCODE(V)};
#undef V

int wasm_is_naturally_aligned(WasmOpcode opcode, uint32_t alignment) {
  assert(opcode < WASM_ARRAY_SIZE(s_opcode_mem_size));
  uint32_t opcode_align = s_opcode_mem_size[opcode];
  return alignment == WASM_USE_NATURAL_ALIGNMENT ||
         alignment == opcode_align;
}

uint32_t wasm_get_opcode_alignment(WasmOpcode opcode, uint32_t alignment) {
  assert(opcode < WASM_ARRAY_SIZE(s_opcode_mem_size));
  if (alignment == WASM_USE_NATURAL_ALIGNMENT)
    return s_opcode_mem_size[opcode];
  return alignment;
}

int wasm_string_slices_are_equal(const WasmStringSlice* a,
                                 const WasmStringSlice* b) {
  return a->start && b->start && a->length == b->length &&
         memcmp(a->start, b->start, a->length) == 0;
}

void wasm_destroy_string_slice(WasmAllocator* allocator, WasmStringSlice* str) {
  wasm_free(allocator, (void*)str->start);
}

void wasm_print_memory(const void* start,
                       size_t size,
                       size_t offset,
                       int print_chars,
                       const char* desc) {
  /* mimic xxd output */
  const uint8_t* p = start;
  const uint8_t* end = p + size;
  while (p < end) {
    const uint8_t* line = p;
    const uint8_t* line_end = p + DUMP_OCTETS_PER_LINE;
    printf("%07x: ", (int)((size_t)p - (size_t)start + offset));
    while (p < line_end) {
      int i;
      for (i = 0; i < DUMP_OCTETS_PER_GROUP; ++i, ++p) {
        if (p < end) {
          printf("%02x", *p);
        } else {
          putchar(' ');
          putchar(' ');
        }
      }
      putchar(' ');
    }

    putchar(' ');
    p = line;
    int i;
    for (i = 0; i < DUMP_OCTETS_PER_LINE && p < end; ++i, ++p)
      if (print_chars)
        printf("%c", isprint(*p) ? *p : '.');
    /* if there are multiple lines, only print the desc on the last one */
    if (p >= end && desc)
      printf("  ; %s", desc);
    putchar('\n');
  }
}

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
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "wasm-allocator.h"

#define V(rtype, type1, type2, mem_size, code, NAME, text) [code] = mem_size,
static uint8_t s_opcode_mem_size[] = {WASM_FOREACH_OPCODE(V)};
#undef V

WasmBool wasm_is_naturally_aligned(WasmOpcode opcode, uint32_t alignment) {
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

WasmBool wasm_string_slices_are_equal(const WasmStringSlice* a,
                                      const WasmStringSlice* b) {
  return a->start && b->start && a->length == b->length &&
         memcmp(a->start, b->start, a->length) == 0;
}

void wasm_destroy_string_slice(WasmAllocator* allocator, WasmStringSlice* str) {
  wasm_free(allocator, (void*)str->start);
}

WasmResult wasm_read_file(WasmAllocator* allocator,
                          const char* filename,
                          void** out_data,
                          size_t* out_size) {
  FILE* infile = fopen(filename, "rb");
  if (!infile) {
    fprintf(stderr, "unable to read %s\n", filename);
    return WASM_ERROR;
  }

  if (fseek(infile, 0, SEEK_END) < 0) {
    fprintf(stderr, "fseek to end failed.\n");
    return WASM_ERROR;
  }

  long size = ftell(infile);
  if (size < 0) {
    fprintf(stderr, "ftell failed.\n");
    return WASM_ERROR;
  }

  if (fseek(infile, 0, SEEK_SET) < 0) {
    fprintf(stderr, "fseek to beginning failed.\n");
    return WASM_ERROR;
  }

  void* data = wasm_alloc(allocator, size, WASM_DEFAULT_ALIGN);
  if (fread(data, size, 1, infile) != 1) {
    fprintf(stderr, "fread failed.\n");
    return WASM_ERROR;
  }

  *out_data = data;
  *out_size = size;
  fclose(infile);
  return WASM_OK;
}

static void print_carets(FILE* out,
                         size_t num_spaces,
                         size_t num_carets,
                         size_t max_line) {
  /* print the caret */
  char* carets = alloca(max_line);
  memset(carets, '^', max_line);
  if (num_carets > max_line - num_spaces)
    num_carets = max_line - num_spaces;
  fprintf(out, "%*s%.*s\n", (int)num_spaces, "", (int)num_carets, carets);
}

static void print_source_error(FILE* out,
                               const WasmLocation* loc,
                               const char* error,
                               const char* source_line,
                               size_t source_line_length,
                               size_t source_line_column_offset) {
  fprintf(out, "%s:%d:%d: %s\n", loc->filename, loc->line, loc->first_column,
          error);
  if (source_line && source_line_length > 0) {
    fprintf(out, "%s\n", source_line);
    size_t num_spaces = (loc->first_column - 1) - source_line_column_offset;
    size_t num_carets = loc->last_column - loc->first_column;
    print_carets(out, num_spaces, num_carets, source_line_length);
  }
}

void wasm_default_source_error_callback(const WasmLocation* loc,
                                        const char* error,
                                        const char* source_line,
                                        size_t source_line_length,
                                        size_t source_line_column_offset,
                                        void* user_data) {
  print_source_error(stderr, loc, error, source_line, source_line_length,
                     source_line_column_offset);
}

void wasm_default_assert_invalid_source_error_callback(
    const WasmLocation* loc,
    const char* error,
    const char* source_line,
    size_t source_line_length,
    size_t source_line_column_offset,
    void* user_data) {
  fprintf(stdout, "assert_invalid error:\n  ");
  print_source_error(stdout, loc, error, source_line, source_line_length,
                     source_line_column_offset);
}

void wasm_default_binary_error_callback(uint32_t offset,
                                        const char* error,
                                        void* user_data) {
  if (offset == WASM_UNKNOWN_OFFSET)
    fprintf(stderr, "error: %s\n", error);
  else
    fprintf(stderr, "error: @0x%08x: %s\n", offset, error);
}

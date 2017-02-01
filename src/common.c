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

#include "common.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if COMPILER_IS_MSVC
#include <fcntl.h>
#include <io.h>
#endif

#include "allocator.h"

#define V(rtype, type1, type2, mem_size, code, NAME, text)                 \
  [code] = {text, WABT_TYPE_##rtype, WABT_TYPE_##type1, WABT_TYPE_##type2, \
            mem_size},
WabtOpcodeInfo g_wabt_opcode_info[] = {WABT_FOREACH_OPCODE(V)};
#undef V

const char* g_wabt_kind_name[] = {"func", "table", "memory", "global"};
WABT_STATIC_ASSERT(WABT_ARRAY_SIZE(g_wabt_kind_name) ==
                   WABT_NUM_EXTERNAL_KINDS);

const char* g_wabt_reloc_type_name[] = {
    "R_FUNC_INDEX_LEB", "R_TABLE_INDEX_SLEB", "R_TABLE_INDEX_I32",
    "R_GLOBAL_INDEX_LEB", "R_DATA"};
WABT_STATIC_ASSERT(WABT_ARRAY_SIZE(g_wabt_reloc_type_name) ==
                   WABT_NUM_RELOC_TYPES);

WabtBool wabt_is_naturally_aligned(WabtOpcode opcode, uint32_t alignment) {
  uint32_t opcode_align = wabt_get_opcode_memory_size(opcode);
  return alignment == WABT_USE_NATURAL_ALIGNMENT || alignment == opcode_align;
}

uint32_t wabt_get_opcode_alignment(WabtOpcode opcode, uint32_t alignment) {
  if (alignment == WABT_USE_NATURAL_ALIGNMENT)
    return wabt_get_opcode_memory_size(opcode);
  return alignment;
}

WabtStringSlice wabt_empty_string_slice(void) {
  WabtStringSlice result;
  result.start = "";
  result.length = 0;
  return result;
}

WabtBool wabt_string_slice_eq_cstr(const WabtStringSlice* s1, const char* s2) {
  size_t s2_len = strlen(s2);
  if (s2_len != s1->length)
    return WABT_FALSE;

  return strncmp(s1->start, s2, s2_len) == 0 ? WABT_TRUE : WABT_FALSE;
}

WabtBool wabt_string_slice_startswith(const WabtStringSlice* s1,
                                      const char* s2) {
  size_t s2_len = strlen(s2);
  if (s2_len > s1->length)
    return WABT_FALSE;

  return strncmp(s1->start, s2, s2_len) == 0 ? WABT_TRUE : WABT_FALSE;
}

WabtStringSlice wabt_string_slice_from_cstr(const char* string) {
  WabtStringSlice result;
  result.start = string;
  result.length = strlen(string);
  return result;
}

WabtBool wabt_string_slice_is_empty(const WabtStringSlice* str) {
  assert(str);
  return str->start == NULL || str->length == 0;
}

WabtBool wabt_string_slices_are_equal(const WabtStringSlice* a,
                                      const WabtStringSlice* b) {
  assert(a && b);
  return a->start && b->start && a->length == b->length &&
         memcmp(a->start, b->start, a->length) == 0;
}

void wabt_destroy_string_slice(WabtAllocator* allocator, WabtStringSlice* str) {
  assert(str);
  wabt_free(allocator, (void*)str->start);
}

WabtResult wabt_read_file(WabtAllocator* allocator,
                          const char* filename,
                          void** out_data,
                          size_t* out_size) {
  FILE* infile = fopen(filename, "rb");
  if (!infile) {
    fprintf(stderr, "unable to read file: %s\n", filename);
    return WABT_ERROR;
  }

  if (fseek(infile, 0, SEEK_END) < 0) {
    fprintf(stderr, "fseek to end failed.\n");
    return WABT_ERROR;
  }

  long size = ftell(infile);
  if (size < 0) {
    fprintf(stderr, "ftell failed.\n");
    return WABT_ERROR;
  }

  if (fseek(infile, 0, SEEK_SET) < 0) {
    fprintf(stderr, "fseek to beginning failed.\n");
    return WABT_ERROR;
  }

  void* data = wabt_alloc(allocator, size, WABT_DEFAULT_ALIGN);
  if (size != 0 && fread(data, size, 1, infile) != 1) {
    fprintf(stderr, "fread failed.\n");
    return WABT_ERROR;
  }

  *out_data = data;
  *out_size = size;
  fclose(infile);
  return WABT_OK;
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
  /* always print at least one caret */
  if (num_carets == 0)
    num_carets = 1;
  fprintf(out, "%*s%.*s\n", (int)num_spaces, "", (int)num_carets, carets);
}

static void print_source_error(FILE* out,
                               const WabtLocation* loc,
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

static void print_error_header(FILE* out, WabtDefaultErrorHandlerInfo* info) {
  if (info && info->header) {
    switch (info->print_header) {
      case WABT_PRINT_ERROR_HEADER_NEVER:
        break;

      case WABT_PRINT_ERROR_HEADER_ONCE:
        info->print_header = WABT_PRINT_ERROR_HEADER_NEVER;
        /* Fallthrough. */

      case WABT_PRINT_ERROR_HEADER_ALWAYS:
        fprintf(out, "%s:\n", info->header);
        break;
    }
    /* If there's a header, indent the following message. */
    fprintf(out, "  ");
  }
}

static FILE* get_default_error_handler_info_output_file(
    WabtDefaultErrorHandlerInfo* info) {
  return info && info->out_file ? info->out_file : stderr;
}

void wabt_default_source_error_callback(const WabtLocation* loc,
                                        const char* error,
                                        const char* source_line,
                                        size_t source_line_length,
                                        size_t source_line_column_offset,
                                        void* user_data) {
  WabtDefaultErrorHandlerInfo* info = user_data;
  FILE* out = get_default_error_handler_info_output_file(info);
  print_error_header(out, info);
  print_source_error(out, loc, error, source_line, source_line_length,
                     source_line_column_offset);
}

void wabt_default_binary_error_callback(uint32_t offset,
                                        const char* error,
                                        void* user_data) {
  WabtDefaultErrorHandlerInfo* info = user_data;
  FILE* out = get_default_error_handler_info_output_file(info);
  print_error_header(out, info);
  if (offset == WABT_UNKNOWN_OFFSET)
    fprintf(out, "error: %s\n", error);
  else
    fprintf(out, "error: @0x%08x: %s\n", offset, error);
  fflush(out);
}

void wabt_init_stdio() {
#if COMPILER_IS_MSVC
  int result = _setmode(_fileno(stdout), _O_BINARY);
  if (result == -1)
    perror("Cannot set mode binary to stdout");
  result = _setmode(_fileno(stderr), _O_BINARY);
  if (result == -1)
    perror("Cannot set mode binary to stderr");
#endif
}

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

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "wasm-allocator.h"
#include "wasm-internal.h"

#define DUMP_OCTETS_PER_LINE 16
#define DUMP_OCTETS_PER_GROUP 2

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

void wasm_vfprint_error(FILE* error_file,
                        WasmLocation* loc,
                        WasmLexer lexer,
                        const char* fmt,
                        va_list args) {
  if (loc)
    fprintf(error_file, "%s:%d:%d: ", loc->filename, loc->line,
            loc->first_column);

  vfprintf(error_file, fmt, args);
  fprintf(error_file, "\n");

  if (loc) {
    /* print the line and a cute little caret, like clang */
    size_t line_offset = wasm_lexer_get_file_offset_from_line(lexer, loc->line);
    FILE* lexer_file = wasm_lexer_get_file(lexer);
    long old_offset = ftell(lexer_file);
    if (old_offset != -1) {
      size_t next_line_offset =
          wasm_lexer_get_file_offset_from_line(lexer, loc->line + 1);
      if (next_line_offset == INVALID_LINE_OFFSET) {
        /* we haven't gotten to the next line yet. read the file to find it. - 1
         because columns are 1-based */
        size_t offset = line_offset + (loc->last_column - 1);
        next_line_offset = offset;
        if (fseek(lexer_file, offset, SEEK_SET) != -1) {
          char buffer[256];
          while (1) {
            size_t bytes_read = fread(buffer, 1, sizeof(buffer), lexer_file);
            if (bytes_read <= 0)
              break;

            char* newline = memchr(buffer, '\n', bytes_read);
            if (newline) {
              next_line_offset += (newline - buffer);
              break;
            } else {
              next_line_offset += bytes_read;
            }
          }
        }
      } else {
        /* don't include the newline */
        next_line_offset--;
      }

#define max_line 80
      size_t line_length = next_line_offset - line_offset;
      size_t column_range = loc->last_column - loc->first_column;
      size_t start_offset = line_offset;
      if (line_length > max_line) {
        line_length = max_line;
        size_t center_on;
        if (column_range > max_line) {
          /* the column range doesn't fit, just center on first_column */
          center_on = loc->first_column - 1;
        } else {
          /* the entire range fits, display it all in the center */
          center_on = (loc->first_column + loc->last_column) / 2 - 1;
        }
        if (center_on > max_line / 2)
          start_offset = line_offset + center_on - max_line / 2;
        if (start_offset > next_line_offset - max_line)
          start_offset = next_line_offset - max_line;
      }

      const char ellipsis[] = "...";
      size_t ellipsis_length = sizeof(ellipsis) - 1;
      const char* line_prefix = "";
      const char* line_suffix = "";
      if (start_offset + line_length != next_line_offset) {
        line_suffix = ellipsis;
        line_length -= ellipsis_length;
      }

      if (start_offset != line_offset) {
        start_offset += ellipsis_length;
        line_length -= ellipsis_length;
        line_prefix = ellipsis;
      }

      if (fseek(lexer_file, start_offset, SEEK_SET) != -1) {
        char buffer[max_line];
        size_t bytes_read = fread(buffer, 1, line_length, lexer_file);
        if (bytes_read > 0) {
          fprintf(error_file, "%s%.*s%s\n", line_prefix, (int)bytes_read,
                  buffer, line_suffix);

          /* print the caret */
          char carets[max_line];
          memset(carets, '^', sizeof(carets));
          size_t num_spaces = (loc->first_column - 1) -
                              (start_offset - line_offset) +
                              strlen(line_prefix);
          size_t num_carets = column_range;
          if (num_carets > max_line - num_spaces)
            num_carets = max_line - num_spaces;
          fprintf(error_file, "%*s%.*s\n", (int)num_spaces, "", (int)num_carets,
                  carets);
        }
      }

      if (fseek(lexer_file, old_offset, SEEK_SET) == -1) {
        /* we're screwed now, blow up. */
        FATAL("failed to seek.");
      }
    }
  }
}

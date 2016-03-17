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

#include "wasm-parser-lexer-shared.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void wasm_vfprint_error(FILE* error_file,
                        WasmLocation* loc,
                        WasmLexer lexer,
                        const char* fmt,
                        va_list args) {
  if (loc) {
    fprintf(error_file, "%s:%d:%d: ", loc->filename, loc->line,
            loc->first_column);
  }

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

#define MAX_LINE 80
      size_t line_length = next_line_offset - line_offset;
      size_t column_range = loc->last_column - loc->first_column;
      size_t start_offset = line_offset;
      if (line_length > MAX_LINE) {
        line_length = MAX_LINE;
        size_t center_on;
        if (column_range > MAX_LINE) {
          /* the column range doesn't fit, just center on first_column */
          center_on = loc->first_column - 1;
        } else {
          /* the entire range fits, display it all in the center */
          center_on = (loc->first_column + loc->last_column) / 2 - 1;
        }
        if (center_on > MAX_LINE / 2)
          start_offset = line_offset + center_on - MAX_LINE / 2;
        if (start_offset > next_line_offset - MAX_LINE)
          start_offset = next_line_offset - MAX_LINE;
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
        char buffer[MAX_LINE];
        size_t bytes_read = fread(buffer, 1, line_length, lexer_file);
        if (bytes_read > 0) {
          fprintf(error_file, "%s%.*s%s\n", line_prefix, (int)bytes_read,
                  buffer, line_suffix);

          /* print the caret */
          char carets[MAX_LINE];
          memset(carets, '^', sizeof(carets));
          size_t num_spaces = (loc->first_column - 1) -
                              (start_offset - line_offset) +
                              strlen(line_prefix);
          size_t num_carets = column_range;
          if (num_carets > MAX_LINE - num_spaces)
            num_carets = MAX_LINE - num_spaces;
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


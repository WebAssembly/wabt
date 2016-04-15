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

void wasm_parser_error(WasmLocation* loc,
                       WasmLexer lexer,
                       WasmParser* parser,
                       const char* format,
                       ...) {
  parser->errors++;
  va_list args;
  va_start(args, format);
  wasm_format_error(parser->error_handler, loc, lexer, format, args);
  va_end(args);
}

void wasm_format_error(WasmSourceErrorHandler* error_handler,
                       const struct WasmLocation* loc,
                       WasmLexer lexer,
                       const char* format,
                       va_list args) {
  va_list args_copy;
  va_copy(args_copy, args);
  char fixed_buf[WASM_DEFAULT_SNPRINTF_ALLOCA_BUFSIZE];
  char* buffer = fixed_buf;
  size_t len = wasm_vsnprintf(fixed_buf, sizeof(fixed_buf), format, args);
  if (len + 1 > sizeof(fixed_buf)) {
    buffer = alloca(len + 1);
    len = wasm_vsnprintf(buffer, len + 1, format, args_copy);
  }

  char* source_line = NULL;
  size_t source_line_length = 0;
  size_t source_line_column_offset = 0;
  size_t source_line_max_length = error_handler->source_line_max_length;
  if (loc) {
    source_line = alloca(source_line_max_length + 1);

    size_t line_offset = wasm_lexer_get_file_offset_from_line(lexer, loc->line);
    FILE* lexer_file = wasm_lexer_get_file(lexer);
    long old_offset = ftell(lexer_file);
    if (old_offset != -1) {
      size_t next_line_offset =
          wasm_lexer_get_file_offset_from_line(lexer, loc->line + 1);
      if (next_line_offset == WASM_INVALID_LINE_OFFSET) {
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

      source_line_length = next_line_offset - line_offset;
      size_t column_range = loc->last_column - loc->first_column;
      size_t start_offset = line_offset;
      if (source_line_length > source_line_max_length) {
        source_line_length = source_line_max_length;
        size_t center_on;
        if (column_range > source_line_max_length) {
          /* the column range doesn't fit, just center on first_column */
          center_on = loc->first_column - 1;
        } else {
          /* the entire range fits, display it all in the center */
          center_on = (loc->first_column + loc->last_column) / 2 - 1;
        }
        if (center_on > source_line_max_length / 2)
          start_offset = line_offset + center_on - source_line_max_length / 2;
        if (start_offset > next_line_offset - source_line_max_length)
          start_offset = next_line_offset - source_line_max_length;
      }

      source_line_column_offset = start_offset - line_offset;

      char* p = source_line;
      size_t read_start = start_offset;
      size_t read_length = source_line_length;
      WasmBool has_start_ellipsis = start_offset != line_offset;
      WasmBool has_end_ellipsis =
          start_offset + source_line_length != next_line_offset;

      if (has_start_ellipsis) {
        memcpy(p, "...", 3);
        p += 3;
        read_start += 3;
        read_length -= 3;
      }

      if (has_end_ellipsis) {
        read_length -= 3;
      }

      if (fseek(lexer_file, read_start, SEEK_SET) != -1) {
        size_t bytes_read = fread(p, 1, read_length, lexer_file);
        if (bytes_read > 0)
          p += bytes_read;
      }

      if (has_end_ellipsis) {
        memcpy(p, "...", 3);
        p += 3;
      }

      source_line_length = p - source_line;

      *p = '\0';

      if (fseek(lexer_file, old_offset, SEEK_SET) == -1) {
        /* we're screwed now, blow up. */
        WASM_FATAL("failed to seek.");
      }
    }
  }

  if (error_handler->on_error) {
    error_handler->on_error(loc, buffer, source_line, source_line_length,
                            source_line_column_offset, error_handler->user_data);
  }
  va_end(args_copy);
}

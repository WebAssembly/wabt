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

#include "wasm-ast-parser-lexer-shared.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void wasm_ast_parser_error(WasmLocation* loc,
                           WasmAstLexer* lexer,
                           WasmAstParser* parser,
                           const char* format,
                           ...) {
  parser->errors++;
  va_list args;
  va_start(args, format);
  wasm_ast_format_error(parser->error_handler, loc, lexer, format, args);
  va_end(args);
}

void wasm_ast_format_error(WasmSourceErrorHandler* error_handler,
                           const struct WasmLocation* loc,
                           WasmAstLexer* lexer,
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
  int source_line_column_offset = 0;
  size_t source_line_max_length = error_handler->source_line_max_length;
  if (loc && lexer) {
    source_line = alloca(source_line_max_length + 1);
    WasmResult result = wasm_ast_lexer_get_source_line(
        lexer, loc, source_line_max_length, source_line, &source_line_length,
        &source_line_column_offset);
    if (WASM_FAILED(result)) {
      /* if this fails, it means that we've probably screwed up the lexer. blow
       * up. */
      WASM_FATAL("error getting the source line.\n");
    }
  }

  if (error_handler->on_error) {
    error_handler->on_error(loc, buffer, source_line, source_line_length,
                            source_line_column_offset,
                            error_handler->user_data);
  }
  va_end(args_copy);
}

void wasm_destroy_exported_func(WasmAllocator* allocator,
                                WasmExportedFunc* exported_func) {
  wasm_destroy_export(allocator, &exported_func->export_);
  wasm_free(allocator, exported_func->func);
}

void wasm_destroy_text_list(WasmAllocator* allocator, WasmTextList* text_list) {
  WasmTextListNode* node = text_list->first;
  while (node) {
    WasmTextListNode* next = node->next;
    wasm_destroy_string_slice(allocator, &node->text);
    wasm_free(allocator, node);
    node = next;
  }
}

void wasm_destroy_func_fields(struct WasmAllocator* allocator,
                              WasmFuncField* func_field) {
  /* destroy the entire linked-list */
  while (func_field) {
    WasmFuncField* next_func_field = func_field->next;

    switch (func_field->type) {
      case WASM_FUNC_FIELD_TYPE_EXPRS:
        wasm_destroy_expr_list(allocator, func_field->first_expr);
        break;

      case WASM_FUNC_FIELD_TYPE_PARAM_TYPES:
      case WASM_FUNC_FIELD_TYPE_LOCAL_TYPES:
        wasm_destroy_type_vector(allocator, &func_field->types);
        break;

      case WASM_FUNC_FIELD_TYPE_BOUND_PARAM:
      case WASM_FUNC_FIELD_TYPE_BOUND_LOCAL:
        wasm_destroy_string_slice(allocator, &func_field->bound_type.name);
        break;

      case WASM_FUNC_FIELD_TYPE_RESULT_TYPE:
        /* nothing to free */
        break;
    }

    wasm_free(allocator, func_field);
    func_field = next_func_field;
  }
}

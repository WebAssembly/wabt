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

#include "ast-parser-lexer-shared.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void wabt_ast_parser_error(WabtLocation* loc,
                           WabtAstLexer* lexer,
                           WabtAstParser* parser,
                           const char* format,
                           ...) {
  parser->errors++;
  va_list args;
  va_start(args, format);
  wabt_ast_format_error(parser->error_handler, loc, lexer, format, args);
  va_end(args);
}

void wabt_ast_format_error(WabtSourceErrorHandler* error_handler,
                           const struct WabtLocation* loc,
                           WabtAstLexer* lexer,
                           const char* format,
                           va_list args) {
  va_list args_copy;
  va_copy(args_copy, args);
  char fixed_buf[WABT_DEFAULT_SNPRINTF_ALLOCA_BUFSIZE];
  char* buffer = fixed_buf;
  size_t len = wabt_vsnprintf(fixed_buf, sizeof(fixed_buf), format, args);
  if (len + 1 > sizeof(fixed_buf)) {
    buffer = alloca(len + 1);
    len = wabt_vsnprintf(buffer, len + 1, format, args_copy);
  }

  char* source_line = NULL;
  size_t source_line_length = 0;
  int source_line_column_offset = 0;
  size_t source_line_max_length = error_handler->source_line_max_length;
  if (loc && lexer) {
    source_line = alloca(source_line_max_length + 1);
    WabtResult result = wabt_ast_lexer_get_source_line(
        lexer, loc, source_line_max_length, source_line, &source_line_length,
        &source_line_column_offset);
    if (WABT_FAILED(result)) {
      /* if this fails, it means that we've probably screwed up the lexer. blow
       * up. */
      WABT_FATAL("error getting the source line.\n");
    }
  }

  if (error_handler->on_error) {
    error_handler->on_error(loc, buffer, source_line, source_line_length,
                            source_line_column_offset,
                            error_handler->user_data);
  }
  va_end(args_copy);
}

void wabt_destroy_optional_export(WabtOptionalExport* export_) {
  if (export_->has_export)
    wabt_destroy_export(&export_->export_);
}

void wabt_destroy_exported_func(WabtExportedFunc* exported_func) {
  wabt_destroy_optional_export(&exported_func->export_);
  wabt_destroy_func(exported_func->func);
  wabt_free(exported_func->func);
}

void wabt_destroy_text_list(WabtTextList* text_list) {
  WabtTextListNode* node = text_list->first;
  while (node) {
    WabtTextListNode* next = node->next;
    wabt_destroy_string_slice(&node->text);
    wabt_free(node);
    node = next;
  }
}

void wabt_destroy_func_fields(WabtFuncField* func_field) {
  /* destroy the entire linked-list */
  while (func_field) {
    WabtFuncField* next_func_field = func_field->next;

    switch (func_field->type) {
      case WABT_FUNC_FIELD_TYPE_EXPRS:
        wabt_destroy_expr_list(func_field->first_expr);
        break;

      case WABT_FUNC_FIELD_TYPE_PARAM_TYPES:
      case WABT_FUNC_FIELD_TYPE_LOCAL_TYPES:
      case WABT_FUNC_FIELD_TYPE_RESULT_TYPES:
        wabt_destroy_type_vector(&func_field->types);
        break;

      case WABT_FUNC_FIELD_TYPE_BOUND_PARAM:
      case WABT_FUNC_FIELD_TYPE_BOUND_LOCAL:
        wabt_destroy_string_slice(&func_field->bound_type.name);
        break;
    }

    wabt_free(func_field);
    func_field = next_func_field;
  }
}

void wabt_destroy_exported_memory(WabtExportedMemory* memory) {
  wabt_destroy_memory(&memory->memory);
  wabt_destroy_optional_export(&memory->export_);
  if (memory->has_data_segment)
    wabt_destroy_data_segment(&memory->data_segment);
}

void wabt_destroy_exported_table(WabtExportedTable* table) {
  wabt_destroy_table(&table->table);
  wabt_destroy_optional_export(&table->export_);
  if (table->has_elem_segment)
    wabt_destroy_elem_segment(&table->elem_segment);
}

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

#ifndef WABT_AST_PARSER_LEXER_SHARED_H_
#define WABT_AST_PARSER_LEXER_SHARED_H_

#include <stdarg.h>

#include "ast.h"
#include "ast-lexer.h"
#include "common.h"

#define WABT_AST_PARSER_STYPE WabtToken
#define WABT_AST_PARSER_LTYPE WabtLocation
#define YYSTYPE WABT_AST_PARSER_STYPE
#define YYLTYPE WABT_AST_PARSER_LTYPE

#define WABT_INVALID_LINE_OFFSET ((size_t)~0)

typedef struct WabtExprList {
  WabtExpr* first;
  WabtExpr* last;
  size_t size;
} WabtExprList;

typedef struct WabtTextListNode {
  WabtStringSlice text;
  struct WabtTextListNode* next;
} WabtTextListNode;

typedef struct WabtTextList {
  WabtTextListNode* first;
  WabtTextListNode* last;
} WabtTextList;

typedef struct WabtOptionalExport {
  WabtExport export_;
  bool has_export;
} WabtOptionalExport;

typedef struct WabtExportedFunc {
  WabtFunc* func;
  WabtOptionalExport export_;
} WabtExportedFunc;

typedef struct WabtExportedGlobal {
  WabtGlobal global;
  WabtOptionalExport export_;
} WabtExportedGlobal;

typedef struct WabtExportedTable {
  WabtTable table;
  WabtElemSegment elem_segment;
  WabtOptionalExport export_;
  bool has_elem_segment;
} WabtExportedTable;

typedef struct WabtExportedMemory {
  WabtMemory memory;
  WabtDataSegment data_segment;
  WabtOptionalExport export_;
  bool has_data_segment;
} WabtExportedMemory;

typedef enum WabtFuncFieldType {
  WABT_FUNC_FIELD_TYPE_EXPRS,
  WABT_FUNC_FIELD_TYPE_PARAM_TYPES,
  WABT_FUNC_FIELD_TYPE_BOUND_PARAM,
  WABT_FUNC_FIELD_TYPE_RESULT_TYPES,
  WABT_FUNC_FIELD_TYPE_LOCAL_TYPES,
  WABT_FUNC_FIELD_TYPE_BOUND_LOCAL,
} WabtFuncFieldType;

typedef struct WabtBoundType {
  WabtLocation loc;
  WabtStringSlice name;
  WabtType type;
} WabtBoundType;

typedef struct WabtFuncField {
  WabtFuncFieldType type;
  union {
    WabtExpr* first_expr;     /* WABT_FUNC_FIELD_TYPE_EXPRS */
    WabtTypeVector types;     /* WABT_FUNC_FIELD_TYPE_*_TYPES */
    WabtBoundType bound_type; /* WABT_FUNC_FIELD_TYPE_BOUND_{LOCAL, PARAM} */
  };
  struct WabtFuncField* next;
} WabtFuncField;

typedef union WabtToken {
  /* terminals */
  WabtStringSlice text;
  WabtType type;
  WabtOpcode opcode;
  WabtLiteral literal;

  /* non-terminals */
  /* some of these use pointers to keep the size of WabtToken down; copying the
   tokens is a hotspot when parsing large files. */
  WabtAction action;
  WabtBlock block;
  WabtCommand* command;
  WabtCommandVector commands;
  WabtConst const_;
  WabtConstVector consts;
  WabtDataSegment data_segment;
  WabtElemSegment elem_segment;
  WabtExport export_;
  WabtExportedFunc exported_func;
  WabtExportedGlobal exported_global;
  WabtExportedMemory exported_memory;
  WabtExportedTable exported_table;
  WabtExpr* expr;
  WabtExprList expr_list;
  WabtFuncField* func_fields;
  WabtFunc* func;
  WabtFuncSignature func_sig;
  WabtFuncType func_type;
  WabtGlobal global;
  WabtImport* import;
  WabtLimits limits;
  WabtOptionalExport optional_export;
  WabtMemory memory;
  WabtModule* module;
  WabtRawModule raw_module;
  WabtScript script;
  WabtTable table;
  WabtTextList text_list;
  WabtTypeVector types;
  uint32_t u32;
  uint64_t u64;
  WabtVar var;
  WabtVarVector vars;
} WabtToken;

typedef struct WabtAstParser {
  WabtScript script;
  WabtSourceErrorHandler* error_handler;
  int errors;
  /* Cached pointers to reallocated parser buffers, so they don't leak. */
  int16_t* yyssa;
  YYSTYPE* yyvsa;
  YYLTYPE* yylsa;
} WabtAstParser;

WABT_EXTERN_C_BEGIN
int wabt_ast_lexer_lex(union WabtToken*,
                       struct WabtLocation*,
                       WabtAstLexer*,
                       struct WabtAstParser*);
WabtResult wabt_ast_lexer_get_source_line(WabtAstLexer*,
                                          const struct WabtLocation*,
                                          size_t line_max_length,
                                          char* line,
                                          size_t* out_line_length,
                                          int* out_column_offset);
void WABT_PRINTF_FORMAT(4, 5) wabt_ast_parser_error(struct WabtLocation*,
                                                    WabtAstLexer*,
                                                    struct WabtAstParser*,
                                                    const char*,
                                                    ...);
void wabt_ast_format_error(WabtSourceErrorHandler*,
                           const struct WabtLocation*,
                           WabtAstLexer*,
                           const char* format,
                           va_list);
void wabt_destroy_optional_export(WabtOptionalExport*);
void wabt_destroy_exported_func(WabtExportedFunc*);
void wabt_destroy_exported_global(WabtExportedFunc*);
void wabt_destroy_exported_memory(WabtExportedMemory*);
void wabt_destroy_exported_table(WabtExportedTable*);
void wabt_destroy_func_fields(WabtFuncField*);
void wabt_destroy_text_list(WabtTextList*);
WABT_EXTERN_C_END

#endif /* WABT_AST_PARSER_LEXER_SHARED_H_ */

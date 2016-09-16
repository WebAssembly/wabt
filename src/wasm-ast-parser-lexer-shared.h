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

#ifndef WASM_AST_PARSER_LEXER_SHARED_H_
#define WASM_AST_PARSER_LEXER_SHARED_H_

#include <stdarg.h>

#include "wasm-ast.h"
#include "wasm-ast-lexer.h"
#include "wasm-common.h"

#define WASM_AST_PARSER_STYPE WasmToken
#define WASM_AST_PARSER_LTYPE WasmLocation
#define YYSTYPE WASM_AST_PARSER_STYPE
#define YYLTYPE WASM_AST_PARSER_LTYPE

#define WASM_INVALID_LINE_OFFSET ((size_t)~0)

struct WasmAllocator;

typedef struct WasmExprList {
  WasmExpr* first;
  WasmExpr* last;
  size_t size;
} WasmExprList;

typedef struct WasmTextListNode {
  WasmStringSlice text;
  struct WasmTextListNode* next;
} WasmTextListNode;

typedef struct WasmTextList {
  WasmTextListNode* first;
  WasmTextListNode* last;
} WasmTextList;

typedef struct WasmOptionalExport {
  WasmExport export_;
  WasmBool has_export;
} WasmOptionalExport;

typedef struct WasmExportedFunc {
  WasmFunc* func;
  WasmOptionalExport export_;
} WasmExportedFunc;

typedef struct WasmExportedGlobal {
  WasmGlobal global;
  WasmOptionalExport export_;
} WasmExportedGlobal;

typedef struct WasmExportedTable {
  WasmTable table;
  WasmElemSegment elem_segment;
  WasmOptionalExport export_;
  WasmBool has_elem_segment;
} WasmExportedTable;

typedef struct WasmExportedMemory {
  WasmMemory memory;
  WasmDataSegment data_segment;
  WasmOptionalExport export_;
  WasmBool has_data_segment;
} WasmExportedMemory;

typedef enum WasmFuncFieldType {
  WASM_FUNC_FIELD_TYPE_EXPRS,
  WASM_FUNC_FIELD_TYPE_PARAM_TYPES,
  WASM_FUNC_FIELD_TYPE_BOUND_PARAM,
  WASM_FUNC_FIELD_TYPE_RESULT_TYPES,
  WASM_FUNC_FIELD_TYPE_LOCAL_TYPES,
  WASM_FUNC_FIELD_TYPE_BOUND_LOCAL,
} WasmFuncFieldType;

typedef struct WasmBoundType {
  WasmLocation loc;
  WasmStringSlice name;
  WasmType type;
} WasmBoundType;

typedef struct WasmFuncField {
  WasmFuncFieldType type;
  union {
    WasmExpr* first_expr;     /* WASM_FUNC_FIELD_TYPE_EXPRS */
    WasmTypeVector types;     /* WASM_FUNC_FIELD_TYPE_*_TYPES */
    WasmBoundType bound_type; /* WASM_FUNC_FIELD_TYPE_BOUND_{LOCAL, PARAM} */
  };
  struct WasmFuncField* next;
} WasmFuncField;

typedef union WasmToken {
  /* terminals */
  WasmStringSlice text;
  WasmType type;
  WasmOpcode opcode;
  WasmLiteral literal;

  /* non-terminals */
  /* some of these use pointers to keep the size of WasmToken down; copying the
   tokens is a hotspot when parsing large files. */
  WasmAction action;
  WasmBlock block;
  WasmCommand* command;
  WasmCommandVector commands;
  WasmConst const_;
  WasmConstVector consts;
  WasmDataSegment data_segment;
  WasmElemSegment elem_segment;
  WasmExport export_;
  WasmExportedFunc exported_func;
  WasmExportedGlobal exported_global;
  WasmExportedMemory exported_memory;
  WasmExportedTable exported_table;
  WasmExpr* expr;
  WasmExprList expr_list;
  WasmFuncField* func_fields;
  WasmFunc* func;
  WasmFuncSignature func_sig;
  WasmFuncType func_type;
  WasmGlobal global;
  WasmImport* import;
  WasmLimits limits;
  WasmOptionalExport optional_export;
  WasmMemory memory;
  WasmModule* module;
  WasmRawModule raw_module;
  WasmScript script;
  WasmTable table;
  WasmTextList text_list;
  WasmTypeVector types;
  uint32_t u32;
  uint64_t u64;
  WasmVar var;
  WasmVarVector vars;
} WasmToken;

typedef struct WasmAstParser {
  struct WasmAllocator* allocator;
  WasmScript script;
  WasmSourceErrorHandler* error_handler;
  int errors;
} WasmAstParser;

WASM_EXTERN_C_BEGIN
struct WasmAllocator* wasm_ast_lexer_get_allocator(WasmAstLexer* lexer);
int wasm_ast_lexer_lex(union WasmToken*,
                       struct WasmLocation*,
                       WasmAstLexer*,
                       struct WasmAstParser*);
WasmResult wasm_ast_lexer_get_source_line(WasmAstLexer*,
                                          const struct WasmLocation*,
                                          size_t line_max_length,
                                          char* line,
                                          size_t* out_line_length,
                                          int* out_column_offset);
void WASM_PRINTF_FORMAT(4, 5) wasm_ast_parser_error(struct WasmLocation*,
                                                    WasmAstLexer*,
                                                    struct WasmAstParser*,
                                                    const char*,
                                                    ...);
void wasm_ast_format_error(WasmSourceErrorHandler*,
                           const struct WasmLocation*,
                           WasmAstLexer*,
                           const char* format,
                           va_list);
void wasm_destroy_optional_export(WasmAllocator*, WasmOptionalExport*);
void wasm_destroy_exported_func(WasmAllocator*, WasmExportedFunc*);
void wasm_destroy_exported_global(WasmAllocator*, WasmExportedFunc*);
void wasm_destroy_exported_memory(WasmAllocator*, WasmExportedMemory*);
void wasm_destroy_exported_table(WasmAllocator*, WasmExportedTable*);
void wasm_destroy_func_fields(WasmAllocator*, WasmFuncField*);
void wasm_destroy_text_list(WasmAllocator*, WasmTextList*);
WASM_EXTERN_C_END

#endif /* WASM_AST_PARSER_LEXER_SHARED_H_ */

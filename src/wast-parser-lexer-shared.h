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

#ifndef WABT_WAST_PARSER_LEXER_SHARED_H_
#define WABT_WAST_PARSER_LEXER_SHARED_H_

#include <cstdarg>
#include <memory>

#include "common.h"
#include "error-handler.h"
#include "ir.h"
#include "literal.h"
#include "wast-parser.h"

#define WABT_WAST_PARSER_STYPE Token
#define WABT_WAST_PARSER_LTYPE Location
#define YYSTYPE WABT_WAST_PARSER_STYPE
#define YYLTYPE WABT_WAST_PARSER_LTYPE

namespace wabt {

// Terminal types are C-style structs so they don't need to be allocated. Any
// string memory used by terminals is shared with the lexer and does not need
// to be dellocated.

struct StringTerminal {
  const char* data;
  size_t size;

  // Helper functions.
  std::string to_string() const { return std::string(data, size); }
  string_view to_string_view() const { return string_view(data, size); }
};

struct LiteralTerminal {
  LiteralType type;
  StringTerminal text;
};

struct Literal {
  explicit Literal(LiteralTerminal terminal)
      : type(terminal.type), text(terminal.text.to_string()) {}

  LiteralType type;
  std::string text;
};

typedef std::vector<std::string> TextVector;

union Token {
  // Terminals
  StringTerminal t_text;
  Type t_type;
  Opcode t_opcode;
  LiteralTerminal t_literal;

  Token() {}

  // Non-terminals
  // Some of these use pointers to keep the size of Token down; copying the
  // tokens is a hotspot when parsing large files.
  Action* action;
  Block* block;
  Catch* catch_;
  Command* command;
  CommandPtrVector* commands;
  Const const_;
  ConstVector* consts;
  DataSegment* data_segment;
  ElemSegment* elem_segment;
  Exception* exception;
  Export* export_;
  Expr* expr;
  ExprList* expr_list;
  Func* func;
  FuncSignature* func_sig;
  FuncType* func_type;
  Global* global;
  Import* import;
  Limits limits;
  Literal* literal;
  Memory* memory;
  Module* module;
  ModuleField* module_field;
  ModuleFieldList* module_fields;
  ScriptModule* script_module;
  Script* script;
  std::string* string;
  Table* table;
  TextVector* texts;
  TryExpr* try_expr;
  TypeVector* types;
  uint32_t u32;
  uint64_t u64;
  Var* var;
  VarVector* vars;
};

struct WastParser {
  Script* script;
  ErrorHandler* error_handler;
  int errors;
  /* Cached pointers to reallocated parser buffers, so they don't leak. */
  int16_t* yyssa;
  YYSTYPE* yyvsa;
  YYLTYPE* yylsa;
  WastParseOptions* options;
};

int WastLexerLex(union Token*,
                 struct Location*,
                 WastLexer*,
                 struct WastParser*);
void WABT_PRINTF_FORMAT(4, 5) WastParserError(struct Location*,
                                              WastLexer*,
                                              struct WastParser*,
                                              const char*,
                                              ...);
void WastFormatError(ErrorHandler*,
                     const struct Location*,
                     WastLexer*,
                     const char* format,
                     va_list);

}  // namespace wabt

#endif /* WABT_WAST_PARSER_LEXER_SHARED_H_ */

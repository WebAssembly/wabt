%{
#include <stdio.h>
#include <stdlib.h>
#include "wasm-tokens.h"

#define YYSTYPE WasmToken
#define YYLTYPE WasmLocation

int yylex(WasmToken*, WasmLocation*, void*);
void yyerror(WasmLocation*, void*, const char*);
%}

%define api.pure true
%define api.value.type {WasmToken}
%define api.token.prefix {WASM_TOKEN_TYPE_}
%lex-param {void* scanner}
%parse-param {void* scanner}
%locations

%token INT FLOAT TEXT VAR VALUE_TYPE LPAR RPAR
%token NOP BLOCK IF IF_ELSE LOOP LABEL BR BR_IF TABLESWITCH CASE
%token CALL CALL_IMPORT CALL_INDIRECT RETURN
%token GET_LOCAL SET_LOCAL LOAD STORE LOAD_EXTEND STORE_WRAP OFFSET ALIGN
%token CONST UNARY BINARY COMPARE CONVERT CAST SELECT
%token FUNC TYPE PARAM RESULT LOCAL
%token MODULE MEMORY SEGMENT IMPORT EXPORT TABLE
%token UNREACHABLE MEMORY_SIZE GROW_MEMORY HAS_FEATURE
%token ASSERT_INVALID ASSERT_RETURN ASSERT_RETURN_NAN ASSERT_TRAP INVOKE
%token EOF

%%

start:

%%

void yyerror(WasmLocation* loc, void* scanner, const char* msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

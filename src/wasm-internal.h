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

#ifndef WASM_INTERNAL_H
#define WASM_INTERNAL_H

#include <stdio.h>

#include "wasm-ast.h"
#include "wasm-lexer.h"

#define FATAL(...) fprintf(stderr, __VA_ARGS__), exit(1)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define ZERO_MEMORY(var) memset((void*)&(var), 0, sizeof(var))
#define STATIC_ASSERT__(x, c) typedef char static_assert_##c[x ? 1 : -1]
#define STATIC_ASSERT_(x, c) STATIC_ASSERT__(x, c)
#define STATIC_ASSERT(x) STATIC_ASSERT_(x, __COUNTER__)

struct WasmAllocator;

typedef union WasmToken {
  /* terminals */
  WasmStringSlice text;
  WasmType type;
  WasmUnaryOp unary;
  WasmBinaryOp binary;
  WasmCompareOp compare;
  WasmConvertOp convert;
  WasmMemOp mem;

  /* non-terminals */
  /* some of these use pointers to keep the size of WasmToken down; copying the
   tokens is a hotspot when parsing large files. */
  uint32_t u32;
  uint64_t u64;
  WasmTypeVector types;
  WasmVar var;
  WasmVarVector vars;
  WasmExprPtr expr;
  WasmExprPtrVector exprs;
  WasmCase case_;
  WasmCaseVector cases;
  WasmTypeBindings type_bindings;
  WasmFunc* func;
  WasmSegment segment;
  WasmSegmentVector segments;
  WasmMemory memory;
  WasmFuncSignature func_sig;
  WasmFuncType func_type;
  WasmImport* import;
  WasmExport export_;
  WasmExportMemory export_memory;
  WasmModuleFieldVector module_fields;
  WasmModule* module;
  WasmConst const_;
  WasmConstVector consts;
  WasmCommand* command;
  WasmCommandVector commands;
  WasmScript script;
} WasmToken;

typedef struct WasmParser {
  struct WasmAllocator* allocator;
  WasmScript script;
  int errors;
} WasmParser;

#define WASM_PARSER_STYPE WasmToken
#define WASM_PARSER_LTYPE WasmLocation
#define YYSTYPE WASM_PARSER_STYPE
#define YYLTYPE WASM_PARSER_LTYPE

#define INVALID_LINE_OFFSET ((size_t)~0)

struct WasmAllocator* wasm_lexer_get_allocator(WasmLexer lexer);
FILE* wasm_lexer_get_file(WasmLexer);
size_t wasm_lexer_get_file_offset_from_line(WasmLexer, int line);
int wasm_lexer_lex(WasmToken*, WasmLocation*, WasmLexer, WasmParser*);
void wasm_parser_error(WasmLocation*, WasmLexer, WasmParser*, const char*, ...);
void wasm_vfprint_error(FILE*, WasmLocation*, WasmLexer, const char*, va_list);
void wasm_print_memory(const void* start,
                       size_t size,
                       size_t offset,
                       int print_chars,
                       const char* desc);

int wasm_string_slices_are_equal(const WasmStringSlice*,
                                 const WasmStringSlice*);
void wasm_destroy_string_slice(struct WasmAllocator*, WasmStringSlice*);

#endif /* WASM_INTERNAL_H */

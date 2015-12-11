#ifndef WASM_INTERNAL_H
#define WASM_INTERNAL_H

#include "wasm.h"

#define FATAL(...) fprintf(stderr, __VA_ARGS__), exit(1)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define STATIC_ASSERT__(x, c) typedef char static_assert_##c[x ? 1 : -1]
#define STATIC_ASSERT_(x, c) STATIC_ASSERT__(x, c)
#define STATIC_ASSERT(x) STATIC_ASSERT_(x, __COUNTER__)

typedef union WasmToken {
  /* terminals */
  WasmStringSlice text;
  WasmType type;
  WasmUnaryOp unary;
  WasmBinaryOp binary;
  WasmCompareOp compare;
  WasmConvertOp convert;
  WasmCastOp cast;
  WasmMemOp mem;

  /* non-terminals */
  uint32_t u32;
  uint64_t u64;
  WasmTypeVector types;
  WasmVar var;
  WasmVarVector vars;
  WasmExprPtr expr;
  WasmExprPtrVector exprs;
  WasmTarget target;
  WasmTargetVector targets;
  WasmCase case_;
  WasmCaseVector cases;
  WasmTypeBindings type_bindings;
  WasmFunc func;
  WasmSegment segment;
  WasmSegmentVector segments;
  WasmMemory memory;
  WasmFuncSignature func_sig;
  WasmFuncType func_type;
  WasmImport import;
  WasmExport export;
  WasmModuleFieldVector module_fields;
  WasmModule module;
  WasmConst const_;
  WasmConstVector consts;
  WasmCommand command;
  WasmCommandVector commands;
  WasmScript script;
} WasmToken;

#define WASM_STYPE WasmToken
#define WASM_LTYPE WasmLocation
#define YYSTYPE WASM_STYPE
#define YYLTYPE WASM_LTYPE

int wasm_lex(WasmToken*, WasmLocation*, WasmScanner, WasmParser*);
void wasm_print_memory(const void* start,
                       size_t size,
                       size_t offset,
                       int print_chars,
                       const char* desc);

#endif /* WASM_INTERNAL_H */

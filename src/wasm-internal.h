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

#include "wasm.h"

#define FATAL(...) fprintf(stderr, __VA_ARGS__), exit(1)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
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
  WasmTarget target;
  WasmTargetVector targets;
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
  WasmExport export;
  WasmModuleFieldVector module_fields;
  WasmModule* module;
  WasmConst const_;
  WasmConstVector consts;
  WasmCommand* command;
  WasmCommandVector commands;
  WasmScript script;
} WasmToken;

#define WASM_STYPE WasmToken
#define WASM_LTYPE WasmLocation
#define YYSTYPE WASM_STYPE
#define YYLTYPE WASM_LTYPE

void wasm_error(WasmLocation*, WasmScanner, WasmParser*, const char*, ...);
int wasm_lexer_lex(WasmToken*, WasmLocation*, WasmScanner, WasmParser*);
void wasm_print_memory(const void* start,
                       size_t size,
                       size_t offset,
                       int print_chars,
                       const char* desc);

int wasm_string_slices_are_equal(const WasmStringSlice*,
                                 const WasmStringSlice*);
WasmBinding* wasm_insert_binding(struct WasmAllocator*,
                                 WasmBindingHash*,
                                 const WasmStringSlice*);
int wasm_hash_entry_is_free(WasmBindingHashEntry*);
void wasm_destroy_string_slice(struct WasmAllocator*, WasmStringSlice*);

/* more destruction functions. not needed unless you're creating your own AST
 elements */
void wasm_destroy_case_vector_and_elements(struct WasmAllocator*, WasmCaseVector*);
void wasm_destroy_case(struct WasmAllocator*, WasmCase*);
void wasm_destroy_command_vector_and_elements(struct WasmAllocator*,
                                              WasmCommandVector*);
void wasm_destroy_command(struct WasmAllocator*, WasmCommand*);
void wasm_destroy_export(struct WasmAllocator*, WasmExport*);
void wasm_destroy_expr_ptr_vector_and_elements(struct WasmAllocator*,
                                               WasmExprPtrVector*);
void wasm_destroy_expr_ptr(struct WasmAllocator*, WasmExprPtr*);
void wasm_destroy_func_signature(struct WasmAllocator*, WasmFuncSignature*);
void wasm_destroy_func_type(struct WasmAllocator*, WasmFuncType*);
void wasm_destroy_func(struct WasmAllocator*, WasmFunc*);
void wasm_destroy_import(struct WasmAllocator*, WasmImport*);
void wasm_destroy_memory(struct WasmAllocator*, WasmMemory*);
void wasm_destroy_module_field_vector_and_elements(struct WasmAllocator*,
                                                   WasmModuleFieldVector*);
void wasm_destroy_module(struct WasmAllocator*, WasmModule*);
void wasm_destroy_segment_vector_and_elements(struct WasmAllocator*,
                                              WasmSegmentVector*);
void wasm_destroy_segment(struct WasmAllocator*, WasmSegment*);
void wasm_destroy_target_vector_and_elements(struct WasmAllocator*,
                                             WasmTargetVector*);
void wasm_destroy_target(struct WasmAllocator*, WasmTarget*);
void wasm_destroy_type_bindings(struct WasmAllocator*, WasmTypeBindings*);
void wasm_destroy_var_vector_and_elements(struct WasmAllocator*,
                                          WasmVarVector*);
void wasm_destroy_var(struct WasmAllocator*, WasmVar*);

/* convenience functions for looking through the AST */
int wasm_get_index_from_var(const WasmBindingHash* bindings,
                            const WasmVar* var);
int wasm_get_func_index_by_var(const WasmModule* module, const WasmVar* var);
int wasm_get_func_type_index_by_var(const WasmModule* module,
                                    const WasmVar* var);
int wasm_get_global_index_by_var(const WasmModule* module, const WasmVar* var);
int wasm_get_import_index_by_var(const WasmModule* module, const WasmVar* var);
int wasm_get_local_index_by_var(const WasmFunc* func, const WasmVar* var);

WasmFuncPtr wasm_get_func_by_var(const WasmModule* module, const WasmVar* var);
WasmFuncTypePtr wasm_get_func_type_by_var(const WasmModule* module,
                                          const WasmVar* var);
WasmImportPtr wasm_get_import_by_var(const WasmModule* module,
                                     const WasmVar* var);
WasmExportPtr wasm_get_export_by_name(const WasmModule* module,
                                      const WasmStringSlice* name);
int wasm_func_is_exported(const WasmModule* module, const WasmFunc* func);

WasmResult wasm_extend_type_bindings(struct WasmAllocator*,
                                     WasmTypeBindings* dst,
                                     WasmTypeBindings* src) WARN_UNUSED;

#endif /* WASM_INTERNAL_H */

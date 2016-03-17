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

#ifndef WASM_AST_H_
#define WASM_AST_H_

#include <stddef.h>
#include <stdint.h>

#include "wasm-common.h"
#include "wasm-vector.h"

WASM_DECLARE_VECTOR(type, WasmType)

typedef enum WasmMemSize {
  WASM_MEM_SIZE_8 = 8,
  WASM_MEM_SIZE_16 = 16,
  WASM_MEM_SIZE_32 = 32,
  WASM_MEM_SIZE_64 = 64,
} WasmMemSize;

typedef struct WasmUnaryOp {
  WasmType type;
  WasmUnaryOpType op_type;
} WasmUnaryOp;

typedef struct WasmBinaryOp {
  WasmType type;
  WasmBinaryOpType op_type;
} WasmBinaryOp;

typedef struct WasmCompareOp {
  WasmType type;
  WasmCompareOpType op_type;
} WasmCompareOp;

typedef struct WasmConvertOp {
  WasmType type;
  WasmConvertOpType op_type;
  WasmType type2;
} WasmConvertOp;

typedef struct WasmMemOp {
  WasmType type;
  WasmMemOpType op_type;
  WasmMemSize size;
} WasmMemOp;

typedef enum WasmVarType {
  WASM_VAR_TYPE_INDEX,
  WASM_VAR_TYPE_NAME,
} WasmVarType;

typedef struct WasmVar {
  WasmLocation loc;
  WasmVarType type;
  union {
    int index;
    WasmStringSlice name;
  };
} WasmVar;
WASM_DECLARE_VECTOR(var, WasmVar);

typedef WasmStringSlice WasmLabel;

typedef struct WasmExpr* WasmExprPtr;
WASM_DECLARE_VECTOR(expr_ptr, WasmExprPtr);

typedef struct WasmConst {
  WasmLocation loc;
  WasmType type;
  union {
    uint32_t u32;
    uint64_t u64;
    uint32_t f32_bits;
    uint64_t f64_bits;
  };
} WasmConst;
WASM_DECLARE_VECTOR(const, WasmConst);

typedef enum WasmExprType {
  WASM_EXPR_TYPE_BINARY,
  WASM_EXPR_TYPE_BLOCK,
  WASM_EXPR_TYPE_BR,
  WASM_EXPR_TYPE_BR_IF,
  WASM_EXPR_TYPE_BR_TABLE,
  WASM_EXPR_TYPE_CALL,
  WASM_EXPR_TYPE_CALL_IMPORT,
  WASM_EXPR_TYPE_CALL_INDIRECT,
  WASM_EXPR_TYPE_COMPARE,
  WASM_EXPR_TYPE_CONST,
  WASM_EXPR_TYPE_CONVERT,
  WASM_EXPR_TYPE_GET_LOCAL,
  WASM_EXPR_TYPE_GROW_MEMORY,
  WASM_EXPR_TYPE_IF,
  WASM_EXPR_TYPE_IF_ELSE,
  WASM_EXPR_TYPE_LOAD,
  WASM_EXPR_TYPE_LOOP,
  WASM_EXPR_TYPE_MEMORY_SIZE,
  WASM_EXPR_TYPE_NOP,
  WASM_EXPR_TYPE_RETURN,
  WASM_EXPR_TYPE_SELECT,
  WASM_EXPR_TYPE_SET_LOCAL,
  WASM_EXPR_TYPE_STORE,
  WASM_EXPR_TYPE_UNARY,
  WASM_EXPR_TYPE_UNREACHABLE,
} WasmExprType;

typedef struct WasmBinding {
  WasmLocation loc;
  WasmStringSlice name;
  int index;
} WasmBinding;

typedef struct WasmBindingHashEntry {
  WasmBinding binding;
  struct WasmBindingHashEntry* next;
  struct WasmBindingHashEntry* prev; /* only valid when this entry is unused */
} WasmBindingHashEntry;
WASM_DECLARE_VECTOR(binding_hash_entry, WasmBindingHashEntry);

typedef struct WasmBindingHash {
  WasmBindingHashEntryVector entries;
  WasmBindingHashEntry* free_head;
} WasmBindingHash;

typedef struct WasmBlock {
  WasmLabel label;
  WasmExprPtrVector exprs;
  int used;
} WasmBlock;

typedef struct WasmExpr WasmExpr;
struct WasmExpr {
  WasmLocation loc;
  WasmExprType type;
  union {
    WasmBlock block;
    struct { WasmExprPtr cond; WasmBlock true_, false_; } if_else;
    struct { WasmExprPtr cond; WasmBlock true_; } if_;
    struct { WasmVar var; WasmExprPtr cond, expr; } br_if;
    struct {
      WasmLabel inner, outer;
      WasmExprPtrVector exprs;
      int inner_used, outer_used;
    } loop;
    struct { WasmVar var; WasmExprPtr expr; } br;
    struct { WasmExprPtr expr; } return_;
    struct {
      WasmExprPtr expr;
      WasmVarVector targets;
      WasmVar default_target;
    } br_table;
    struct { WasmVar var; WasmExprPtrVector args; } call;
    struct {
      WasmVar var;
      WasmExprPtr expr;
      WasmExprPtrVector args;
    } call_indirect;
    struct { WasmVar var; } get_local;
    struct { WasmVar var; WasmExprPtr expr; } set_local;
    struct {
      WasmMemOp op;
      uint32_t align;
      uint64_t offset;
      WasmExprPtr addr;
    } load;
    struct {
      WasmMemOp op;
      uint32_t align;
      uint64_t offset;
      WasmExprPtr addr, value;
    } store;
    WasmConst const_;
    struct { WasmUnaryOp op; WasmExprPtr expr; } unary;
    struct { WasmBinaryOp op; WasmExprPtr left, right; } binary;
    struct { WasmExprPtr cond, true_, false_; } select;
    struct { WasmCompareOp op; WasmExprPtr left, right; } compare;
    struct { WasmConvertOp op; WasmExprPtr expr; } convert;
    struct { WasmExprPtr expr; } grow_memory;
  };
};

typedef struct WasmTypeBindings {
  WasmTypeVector types;
  WasmBindingHash bindings;
} WasmTypeBindings;

enum {
  WASM_FUNC_FLAG_HAS_FUNC_TYPE = 1,
  WASM_FUNC_FLAG_HAS_SIGNATURE = 2,
};
typedef uint32_t WasmFuncFlags;

typedef struct WasmFunc {
  WasmLocation loc;
  WasmFuncFlags flags;
  WasmStringSlice name;
  WasmVar type_var;
  WasmTypeBindings params;
  WasmType result_type;
  WasmTypeBindings locals;
  WasmExprPtrVector exprs;

  /* combined from params and locals; the binding names share memory with the
   originals */
  WasmTypeBindings params_and_locals;
} WasmFunc;
typedef WasmFunc* WasmFuncPtr;
WASM_DECLARE_VECTOR(func_ptr, WasmFuncPtr);

typedef struct WasmSegment {
  WasmLocation loc;
  uint32_t addr;
  void* data;
  size_t size;
} WasmSegment;
WASM_DECLARE_VECTOR(segment, WasmSegment);

typedef struct WasmMemory {
  WasmLocation loc;
  uint32_t initial_pages;
  uint32_t max_pages;
  WasmSegmentVector segments;
} WasmMemory;

typedef struct WasmFuncSignature {
  WasmType result_type;
  WasmTypeVector param_types;
} WasmFuncSignature;

typedef struct WasmFuncType {
  WasmStringSlice name;
  WasmFuncSignature sig;
} WasmFuncType;
typedef WasmFuncType* WasmFuncTypePtr;
WASM_DECLARE_VECTOR(func_type_ptr, WasmFuncTypePtr);

typedef enum WasmImportType {
  WASM_IMPORT_HAS_TYPE,
  WASM_IMPORT_HAS_FUNC_SIGNATURE,
} WasmImportType;

typedef struct WasmImport {
  WasmLocation loc;
  WasmImportType import_type;
  WasmStringSlice name;
  WasmStringSlice module_name;
  WasmStringSlice func_name;
  WasmVar type_var;
  WasmFuncSignature func_sig;
} WasmImport;
typedef WasmImport* WasmImportPtr;
WASM_DECLARE_VECTOR(import_ptr, WasmImportPtr);

typedef struct WasmExport {
  WasmStringSlice name;
  WasmVar var;
} WasmExport;
typedef WasmExport* WasmExportPtr;
WASM_DECLARE_VECTOR(export_ptr, WasmExportPtr);

typedef struct WasmExportMemory {
  WasmStringSlice name;
} WasmExportMemory;

typedef enum WasmModuleFieldType {
  WASM_MODULE_FIELD_TYPE_FUNC,
  WASM_MODULE_FIELD_TYPE_IMPORT,
  WASM_MODULE_FIELD_TYPE_EXPORT,
  WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY,
  WASM_MODULE_FIELD_TYPE_TABLE,
  WASM_MODULE_FIELD_TYPE_FUNC_TYPE,
  WASM_MODULE_FIELD_TYPE_MEMORY,
  WASM_MODULE_FIELD_TYPE_START,
} WasmModuleFieldType;

typedef struct WasmModuleField {
  WasmLocation loc;
  WasmModuleFieldType type;
  union {
    WasmFunc func;
    WasmImport import;
    WasmExport export_;
    WasmExportMemory export_memory;
    WasmVarVector table;
    WasmFuncType func_type;
    WasmMemory memory;
    WasmVar start;
  };
} WasmModuleField;
WASM_DECLARE_VECTOR(module_field, WasmModuleField);

typedef struct WasmModule {
  WasmLocation loc;
  WasmModuleFieldVector fields;

  /* cached for convenience */
  WasmFuncPtrVector funcs;
  WasmImportPtrVector imports;
  WasmExportPtrVector exports;
  WasmFuncTypePtrVector func_types;
  WasmVarVector* table;
  WasmMemory* memory;
  WasmVar start;
  WasmExportMemory* export_memory;

  WasmBindingHash func_bindings;
  WasmBindingHash import_bindings;
  WasmBindingHash export_bindings;
  WasmBindingHash func_type_bindings;
} WasmModule;

typedef enum WasmCommandType {
  WASM_COMMAND_TYPE_MODULE,
  WASM_COMMAND_TYPE_INVOKE,
  WASM_COMMAND_TYPE_ASSERT_INVALID,
  WASM_COMMAND_TYPE_ASSERT_RETURN,
  WASM_COMMAND_TYPE_ASSERT_RETURN_NAN,
  WASM_COMMAND_TYPE_ASSERT_TRAP,
} WasmCommandType;

typedef struct WasmCommandInvoke {
  WasmLocation loc;
  WasmStringSlice name;
  WasmConstVector args;
} WasmCommandInvoke;

typedef struct WasmCommand {
  WasmCommandType type;
  union {
    WasmModule module;
    WasmCommandInvoke invoke;
    struct { WasmCommandInvoke invoke; WasmConst expected; } assert_return;
    struct { WasmCommandInvoke invoke; } assert_return_nan;
    struct { WasmCommandInvoke invoke; WasmStringSlice text; } assert_trap;
    struct { WasmModule module; WasmStringSlice text; } assert_invalid;
  };
} WasmCommand;
WASM_DECLARE_VECTOR(command, WasmCommand);

typedef struct WasmScript {
  struct WasmAllocator* allocator;
  WasmCommandVector commands;
} WasmScript;

WASM_EXTERN_C_BEGIN
WasmBinding* wasm_insert_binding(struct WasmAllocator*,
                                 WasmBindingHash*,
                                 const WasmStringSlice*);
int wasm_hash_entry_is_free(WasmBindingHashEntry*);

/* WasmExpr creation functions */
WasmExpr* wasm_new_binary_expr(struct WasmAllocator*);
WasmExpr* wasm_new_block_expr(struct WasmAllocator*);
WasmExpr* wasm_new_br_expr(struct WasmAllocator*);
WasmExpr* wasm_new_br_if_expr(struct WasmAllocator*);
WasmExpr* wasm_new_br_table_expr(struct WasmAllocator*);
WasmExpr* wasm_new_call_expr(struct WasmAllocator*);
WasmExpr* wasm_new_call_import_expr(struct WasmAllocator*);
WasmExpr* wasm_new_call_indirect_expr(struct WasmAllocator*);
WasmExpr* wasm_new_compare_expr(struct WasmAllocator*);
WasmExpr* wasm_new_const_expr(struct WasmAllocator*);
WasmExpr* wasm_new_convert_expr(struct WasmAllocator*);
WasmExpr* wasm_new_get_local_expr(struct WasmAllocator*);
WasmExpr* wasm_new_grow_memory_expr(struct WasmAllocator*);
WasmExpr* wasm_new_if_else_expr(struct WasmAllocator*);
WasmExpr* wasm_new_if_expr(struct WasmAllocator*);
WasmExpr* wasm_new_load_expr(struct WasmAllocator*);
WasmExpr* wasm_new_loop_expr(struct WasmAllocator*);
WasmExpr* wasm_new_return_expr(struct WasmAllocator*);
WasmExpr* wasm_new_select_expr(struct WasmAllocator*);
WasmExpr* wasm_new_set_local_expr(struct WasmAllocator*);
WasmExpr* wasm_new_store_expr(struct WasmAllocator*);
WasmExpr* wasm_new_unary_expr(struct WasmAllocator*);
/* for nop, unreachable and memory_size */
WasmExpr* wasm_new_empty_expr(struct WasmAllocator*, WasmExprType);

/* destruction functions. not needed unless you're creating your own AST
 elements */
void wasm_destroy_script(struct WasmScript*);
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
                                     WasmTypeBindings* src) WASM_WARN_UNUSED;
WASM_EXTERN_C_END

#endif /* WASM_AST_H_ */

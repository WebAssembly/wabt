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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "wasm-common.h"
#include "wasm-vector.h"

WASM_DEFINE_VECTOR(type, WasmType)

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
WASM_DEFINE_VECTOR(var, WasmVar);

typedef WasmStringSlice WasmLabel;
WASM_DEFINE_VECTOR(string_slice, WasmStringSlice);

typedef struct WasmExpr* WasmExprPtr;
WASM_DEFINE_VECTOR(expr_ptr, WasmExprPtr);

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
WASM_DEFINE_VECTOR(const, WasmConst);

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
  WASM_EXPR_TYPE_CURRENT_MEMORY,
  WASM_EXPR_TYPE_GET_LOCAL,
  WASM_EXPR_TYPE_GROW_MEMORY,
  WASM_EXPR_TYPE_IF,
  WASM_EXPR_TYPE_IF_ELSE,
  WASM_EXPR_TYPE_LOAD,
  WASM_EXPR_TYPE_LOOP,
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
WASM_DEFINE_VECTOR(binding_hash_entry, WasmBindingHashEntry);

typedef struct WasmBindingHash {
  WasmBindingHashEntryVector entries;
  WasmBindingHashEntry* free_head;
} WasmBindingHash;

typedef struct WasmBlock {
  WasmLabel label;
  WasmExprPtrVector exprs;
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
    } loop;
    struct { WasmVar var; WasmExprPtr expr; } br;
    struct { WasmExprPtr expr; } return_;
    struct {
      WasmExprPtr key;
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
      WasmOpcode opcode;
      uint32_t align;
      uint64_t offset;
      WasmExprPtr addr;
    } load;
    struct {
      WasmOpcode opcode;
      uint32_t align;
      uint64_t offset;
      WasmExprPtr addr, value;
    } store;
    WasmConst const_;
    struct { WasmOpcode opcode; WasmExprPtr expr; } unary;
    struct { WasmOpcode opcode; WasmExprPtr left, right; } binary;
    struct { WasmExprPtr cond, true_, false_; } select;
    struct { WasmOpcode opcode; WasmExprPtr left, right; } compare;
    struct { WasmOpcode opcode; WasmExprPtr expr; } convert;
    struct { WasmExprPtr expr; } grow_memory;
  };
};

typedef struct WasmBoundType {
  WasmLocation loc;
  WasmStringSlice name;
  WasmType type;
} WasmBoundType;

/* only used for parsing */
typedef enum WasmFuncFieldType {
  WASM_FUNC_FIELD_TYPE_EXPRS,
  WASM_FUNC_FIELD_TYPE_PARAM_TYPES,
  WASM_FUNC_FIELD_TYPE_BOUND_PARAM,
  WASM_FUNC_FIELD_TYPE_RESULT_TYPE,
  WASM_FUNC_FIELD_TYPE_LOCAL_TYPES,
  WASM_FUNC_FIELD_TYPE_BOUND_LOCAL,
} WasmFuncFieldType;

typedef struct WasmFuncField {
  WasmFuncFieldType type;
  union {
    WasmExprPtrVector exprs;
    WasmTypeVector types;     /* WASM_FUNC_FIELD_TYPE_{PARAM,LOCAL}_TYPES */
    WasmBoundType bound_type; /* WASM_FUNC_FIELD_TYPE_BOUND_{LOCAL, PARAM} */
    WasmType result_type;
  };
  struct WasmFuncField* next;
} WasmFuncField;

typedef struct WasmFuncSignature {
  WasmType result_type;
  WasmTypeVector param_types;
} WasmFuncSignature;

typedef struct WasmFuncType {
  WasmStringSlice name;
  WasmFuncSignature sig;
} WasmFuncType;
typedef WasmFuncType* WasmFuncTypePtr;
WASM_DEFINE_VECTOR(func_type_ptr, WasmFuncTypePtr);

enum {
  WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE = 1,
  WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE = 2,
};
typedef uint32_t WasmFuncDeclarationFlags;

typedef struct WasmFuncDeclaration {
  WasmFuncDeclarationFlags flags;
  WasmVar type_var;
  WasmFuncSignature sig;
} WasmFuncDeclaration;

typedef struct WasmFunc {
  WasmLocation loc;
  WasmStringSlice name;
  WasmFuncDeclaration decl;
  WasmTypeVector local_types;
  WasmBindingHash param_bindings;
  WasmBindingHash local_bindings;
  WasmExprPtrVector exprs;
} WasmFunc;
typedef WasmFunc* WasmFuncPtr;
WASM_DEFINE_VECTOR(func_ptr, WasmFuncPtr);

typedef struct WasmImport {
  WasmLocation loc;
  WasmStringSlice name;
  WasmStringSlice module_name;
  WasmStringSlice func_name;
  WasmFuncDeclaration decl;
} WasmImport;
typedef WasmImport* WasmImportPtr;
WASM_DEFINE_VECTOR(import_ptr, WasmImportPtr);

typedef struct WasmSegment {
  WasmLocation loc;
  uint32_t addr;
  void* data;
  size_t size;
} WasmSegment;
WASM_DEFINE_VECTOR(segment, WasmSegment);

typedef struct WasmMemory {
  WasmLocation loc;
  uint32_t initial_pages;
  uint32_t max_pages;
  WasmSegmentVector segments;
} WasmMemory;

typedef struct WasmExport {
  WasmStringSlice name;
  WasmVar var;
} WasmExport;
typedef WasmExport* WasmExportPtr;
WASM_DEFINE_VECTOR(export_ptr, WasmExportPtr);

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
  struct WasmModuleField* next;
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

typedef struct WasmModule {
  WasmLocation loc;
  WasmModuleField* first_field;
  WasmModuleField* last_field;

  /* cached for convenience */
  WasmFuncPtrVector funcs;
  WasmImportPtrVector imports;
  WasmExportPtrVector exports;
  WasmFuncTypePtrVector func_types;
  WasmVarVector* table;
  WasmMemory* memory;
  WasmVar* start;
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
  WASM_NUM_COMMAND_TYPES,
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
WASM_DEFINE_VECTOR(command, WasmCommand);

typedef struct WasmScript {
  struct WasmAllocator* allocator;
  WasmCommandVector commands;
} WasmScript;

typedef struct WasmExprVisitor {
  void* user_data;
  WasmResult (*begin_binary_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_binary_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_block_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_block_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_br_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_br_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_br_if_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_br_if_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_br_table_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_br_table_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_call_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_call_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_call_import_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_call_import_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_call_indirect_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_call_indirect_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_compare_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_compare_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_const_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_convert_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_convert_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_current_memory_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_get_local_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_grow_memory_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_grow_memory_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_if_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_if_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_if_else_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_if_else_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_load_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_load_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_loop_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_loop_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_nop_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_return_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_return_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_select_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_select_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_set_local_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_set_local_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_store_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_store_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_unary_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_unary_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_unreachable_expr)(WasmExpr*, void* user_data);
} WasmExprVisitor;

WASM_EXTERN_C_BEGIN
WasmBinding* wasm_insert_binding(struct WasmAllocator*,
                                 WasmBindingHash*,
                                 const WasmStringSlice*);
WasmBool wasm_hash_entry_is_free(const WasmBindingHashEntry*);

WasmModuleField* wasm_append_module_field(struct WasmAllocator*, WasmModule*);

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
/* for nop, unreachable and current_memory */
WasmExpr* wasm_new_empty_expr(struct WasmAllocator*, WasmExprType);

/* destruction functions. not needed unless you're creating your own AST
 elements */
void wasm_destroy_script(struct WasmScript*);
void wasm_destroy_block(struct WasmAllocator*, struct WasmBlock*);
void wasm_destroy_command_vector_and_elements(struct WasmAllocator*,
                                              WasmCommandVector*);
void wasm_destroy_command(struct WasmAllocator*, WasmCommand*);
void wasm_destroy_export(struct WasmAllocator*, WasmExport*);
void wasm_destroy_expr_ptr_vector_and_elements(struct WasmAllocator*,
                                               WasmExprPtrVector*);
void wasm_destroy_expr_ptr(struct WasmAllocator*, WasmExprPtr*);
void wasm_destroy_func_declaration(WasmAllocator*, WasmFuncDeclaration*);
void wasm_destroy_func_fields(struct WasmAllocator*, WasmFuncField*);
void wasm_destroy_func_signature(struct WasmAllocator*, WasmFuncSignature*);
void wasm_destroy_func_type(struct WasmAllocator*, WasmFuncType*);
void wasm_destroy_func(struct WasmAllocator*, WasmFunc*);
void wasm_destroy_import(struct WasmAllocator*, WasmImport*);
void wasm_destroy_memory(struct WasmAllocator*, WasmMemory*);
void wasm_destroy_module(struct WasmAllocator*, WasmModule*);
void wasm_destroy_segment_vector_and_elements(struct WasmAllocator*,
                                              WasmSegmentVector*);
void wasm_destroy_segment(struct WasmAllocator*, WasmSegment*);
void wasm_destroy_var_vector_and_elements(struct WasmAllocator*,
                                          WasmVarVector*);
void wasm_destroy_var(struct WasmAllocator*, WasmVar*);

/* traversal functions */
WasmResult wasm_visit_func(WasmFunc* func, WasmExprVisitor*);

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

WasmResult wasm_make_type_binding_reverse_mapping(
    struct WasmAllocator*,
    const WasmTypeVector*,
    const WasmBindingHash*,
    WasmStringSliceVector* out_reverse_mapping);

static WASM_INLINE WasmBool
wasm_decl_has_func_type(const WasmFuncDeclaration* decl) {
  return decl->flags & WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
}

static WASM_INLINE WasmBool
wasm_decl_has_signature(const WasmFuncDeclaration* decl) {
  return decl->flags & WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
}


static WASM_INLINE size_t wasm_get_num_params(const WasmFunc* func) {
  return func->decl.sig.param_types.size;
}

static WASM_INLINE size_t wasm_get_num_params_and_locals(const WasmFunc* func) {
  return wasm_get_num_params(func) + func->local_types.size;
}

static WASM_INLINE WasmType wasm_get_param_type(const WasmFunc* func,
                                                int index) {
  assert((size_t)index < wasm_get_num_params(func));
  return func->decl.sig.param_types.data[index];
}

static WASM_INLINE WasmType wasm_get_result_type(const WasmFunc* func) {
  return func->decl.sig.result_type;
}

static WASM_INLINE WasmType
wasm_get_func_type_result_type(const WasmFuncType* func_type) {
  return func_type->sig.result_type;
}

static WASM_INLINE WasmType
wasm_get_func_type_param_type(const WasmFuncType* func_type, int index) {
  return func_type->sig.param_types.data[index];
}

static WASM_INLINE size_t
wasm_get_func_type_num_params(const WasmFuncType* func_type) {
  return func_type->sig.param_types.size;
}

WASM_EXTERN_C_END

#endif /* WASM_AST_H_ */

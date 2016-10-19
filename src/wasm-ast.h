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
#include "wasm-type-vector.h"
#include "wasm-vector.h"

typedef enum WasmVarType {
  WASM_VAR_TYPE_INDEX,
  WASM_VAR_TYPE_NAME,
} WasmVarType;

typedef struct WasmVar {
  WasmLocation loc;
  WasmVarType type;
  union {
    int64_t index;
    WasmStringSlice name;
  };
} WasmVar;
WASM_DEFINE_VECTOR(var, WasmVar);

typedef WasmStringSlice WasmLabel;
WASM_DEFINE_VECTOR(string_slice, WasmStringSlice);

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
  WASM_EXPR_TYPE_CALL_INDIRECT,
  WASM_EXPR_TYPE_COMPARE,
  WASM_EXPR_TYPE_CONST,
  WASM_EXPR_TYPE_CONVERT,
  WASM_EXPR_TYPE_CURRENT_MEMORY,
  WASM_EXPR_TYPE_DROP,
  WASM_EXPR_TYPE_GET_GLOBAL,
  WASM_EXPR_TYPE_GET_LOCAL,
  WASM_EXPR_TYPE_GROW_MEMORY,
  WASM_EXPR_TYPE_IF,
  WASM_EXPR_TYPE_LOAD,
  WASM_EXPR_TYPE_LOOP,
  WASM_EXPR_TYPE_NOP,
  WASM_EXPR_TYPE_RETURN,
  WASM_EXPR_TYPE_SELECT,
  WASM_EXPR_TYPE_SET_GLOBAL,
  WASM_EXPR_TYPE_SET_LOCAL,
  WASM_EXPR_TYPE_STORE,
  WASM_EXPR_TYPE_TEE_LOCAL,
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

typedef WasmTypeVector WasmBlockSignature;

typedef struct WasmBlock {
  WasmLabel label;
  WasmBlockSignature sig;
  struct WasmExpr* first;
} WasmBlock;

typedef struct WasmExpr WasmExpr;
struct WasmExpr {
  WasmLocation loc;
  WasmExprType type;
  WasmExpr* next;
  union {
    struct { WasmOpcode opcode; } binary, compare, convert, unary;
    WasmBlock block, loop;
    struct { WasmVar var; } br, br_if;
    struct { WasmVarVector targets; WasmVar default_target; } br_table;
    struct { WasmVar var; } call, call_indirect;
    WasmConst const_;
    struct { WasmVar var; } get_global, set_global;
    struct { WasmVar var; } get_local, set_local, tee_local;
    struct { WasmBlock true_; struct WasmExpr* false_; } if_;
    struct { WasmOpcode opcode; uint32_t align; uint64_t offset; } load, store;
  };
};

typedef struct WasmFuncSignature {
  WasmTypeVector param_types;
  WasmTypeVector result_types;
} WasmFuncSignature;

typedef struct WasmFuncType {
  WasmStringSlice name;
  WasmFuncSignature sig;
} WasmFuncType;
typedef WasmFuncType* WasmFuncTypePtr;
WASM_DEFINE_VECTOR(func_type_ptr, WasmFuncTypePtr);

enum {
  WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE = 1,
  /* set if the signature is owned by module */
  WASM_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE = 2,
};
typedef uint32_t WasmFuncDeclarationFlags;

typedef struct WasmFuncDeclaration {
  WasmFuncDeclarationFlags flags;
  WasmVar type_var;
  WasmFuncSignature sig;
} WasmFuncDeclaration;

typedef struct WasmFunc {
  WasmStringSlice name;
  WasmFuncDeclaration decl;
  WasmTypeVector local_types;
  WasmBindingHash param_bindings;
  WasmBindingHash local_bindings;
  WasmExpr* first_expr;
} WasmFunc;
typedef WasmFunc* WasmFuncPtr;
WASM_DEFINE_VECTOR(func_ptr, WasmFuncPtr);

typedef struct WasmGlobal {
  WasmStringSlice name;
  WasmType type;
  WasmBool mutable_;
  WasmExpr* init_expr;
} WasmGlobal;
typedef WasmGlobal* WasmGlobalPtr;
WASM_DEFINE_VECTOR(global_ptr, WasmGlobalPtr);

typedef struct WasmTable {
  WasmStringSlice name;
  WasmLimits elem_limits;
} WasmTable;
typedef WasmTable* WasmTablePtr;
WASM_DEFINE_VECTOR(table_ptr, WasmTablePtr);

typedef struct WasmElemSegment {
  WasmVar table_var;
  WasmExpr* offset;
  WasmVarVector vars;
} WasmElemSegment;
typedef WasmElemSegment* WasmElemSegmentPtr;
WASM_DEFINE_VECTOR(elem_segment_ptr, WasmElemSegmentPtr);

typedef struct WasmMemory {
  WasmStringSlice name;
  WasmLimits page_limits;
} WasmMemory;
typedef WasmMemory* WasmMemoryPtr;
WASM_DEFINE_VECTOR(memory_ptr, WasmMemoryPtr);

typedef struct WasmDataSegment {
  WasmVar memory_var;
  WasmExpr* offset;
  void* data;
  size_t size;
} WasmDataSegment;
typedef WasmDataSegment* WasmDataSegmentPtr;
WASM_DEFINE_VECTOR(data_segment_ptr, WasmDataSegmentPtr);

typedef struct WasmImport {
  WasmStringSlice module_name;
  WasmStringSlice field_name;
  WasmExternalKind kind;
  union {
    /* an imported func is has the type WasmFunc so it can be more easily
     * included in the Module's vector of funcs; but only the
     * WasmFuncDeclaration will have any useful information */
    WasmFunc func;
    WasmTable table;
    WasmMemory memory;
    WasmGlobal global;
  };
} WasmImport;
typedef WasmImport* WasmImportPtr;
WASM_DEFINE_VECTOR(import_ptr, WasmImportPtr);

typedef struct WasmExport {
  WasmStringSlice name;
  WasmExternalKind kind;
  WasmVar var;
} WasmExport;
typedef WasmExport* WasmExportPtr;
WASM_DEFINE_VECTOR(export_ptr, WasmExportPtr);

typedef enum WasmModuleFieldType {
  WASM_MODULE_FIELD_TYPE_FUNC,
  WASM_MODULE_FIELD_TYPE_GLOBAL,
  WASM_MODULE_FIELD_TYPE_IMPORT,
  WASM_MODULE_FIELD_TYPE_EXPORT,
  WASM_MODULE_FIELD_TYPE_FUNC_TYPE,
  WASM_MODULE_FIELD_TYPE_TABLE,
  WASM_MODULE_FIELD_TYPE_ELEM_SEGMENT,
  WASM_MODULE_FIELD_TYPE_MEMORY,
  WASM_MODULE_FIELD_TYPE_DATA_SEGMENT,
  WASM_MODULE_FIELD_TYPE_START,
} WasmModuleFieldType;

typedef struct WasmModuleField {
  WasmLocation loc;
  WasmModuleFieldType type;
  struct WasmModuleField* next;
  union {
    WasmFunc func;
    WasmGlobal global;
    WasmImport import;
    WasmExport export_;
    WasmFuncType func_type;
    WasmTable table;
    WasmElemSegment elem_segment;
    WasmMemory memory;
    WasmDataSegment data_segment;
    WasmVar start;
  };
} WasmModuleField;

typedef struct WasmModule {
  WasmStringSlice name;
  WasmModuleField* first_field;
  WasmModuleField* last_field;

  uint32_t num_func_imports;
  uint32_t num_table_imports;
  uint32_t num_memory_imports;
  uint32_t num_global_imports;

  /* cached for convenience; the pointers are shared with values that are
   * stored in either WasmModuleField or WasmImport. */
  WasmFuncPtrVector funcs;
  WasmGlobalPtrVector globals;
  WasmImportPtrVector imports;
  WasmExportPtrVector exports;
  WasmFuncTypePtrVector func_types;
  WasmTablePtrVector tables;
  WasmElemSegmentPtrVector elem_segments;
  WasmMemoryPtrVector memories;
  WasmDataSegmentPtrVector data_segments;
  WasmVar* start;

  WasmBindingHash func_bindings;
  WasmBindingHash global_bindings;
  WasmBindingHash export_bindings;
  WasmBindingHash func_type_bindings;
  WasmBindingHash table_bindings;
  WasmBindingHash memory_bindings;
} WasmModule;

typedef enum WasmRawModuleType {
  WASM_RAW_MODULE_TYPE_TEXT,
  WASM_RAW_MODULE_TYPE_BINARY,
} WasmRawModuleType;

/* "raw" means that the binary module has not yet been decoded. This is only
 * necessary when embedded in assert_invalid. In that case we want to defer
 * decoding errors until wasm_check_assert_invalid is called. This isn't needed
 * when parsing text, as assert_invalid always assumes that text parsing
 * succeeds. */
typedef struct WasmRawModule {
  WasmLocation loc;
  WasmRawModuleType type;
  union {
    WasmModule* text;
    struct {
      WasmStringSlice name;
      void* data;
      size_t size;
    } binary;
  };
} WasmRawModule;

typedef enum WasmActionType {
  WASM_ACTION_TYPE_INVOKE,
  WASM_ACTION_TYPE_GET,
} WasmActionType;

typedef struct WasmActionInvoke {
  WasmStringSlice name;
  WasmConstVector args;
} WasmActionInvoke;

typedef struct WasmActionGet {
  WasmStringSlice name;
} WasmActionGet;

typedef struct WasmAction {
  WasmLocation loc;
  WasmActionType type;
  WasmStringSlice module_var_name;
  union {
    WasmActionInvoke invoke;
    WasmActionGet get;
  };
} WasmAction;

typedef enum WasmCommandType {
  WASM_COMMAND_TYPE_MODULE,
  WASM_COMMAND_TYPE_ACTION,
  WASM_COMMAND_TYPE_REGISTER,
  WASM_COMMAND_TYPE_ASSERT_MALFORMED,
  WASM_COMMAND_TYPE_ASSERT_INVALID,
  WASM_COMMAND_TYPE_ASSERT_UNLINKABLE,
  WASM_COMMAND_TYPE_ASSERT_RETURN,
  WASM_COMMAND_TYPE_ASSERT_RETURN_NAN,
  WASM_COMMAND_TYPE_ASSERT_TRAP,
  WASM_NUM_COMMAND_TYPES,
} WasmCommandType;

typedef struct WasmCommand {
  WasmCommandType type;
  union {
    WasmModule module;
    WasmAction action;
    struct { WasmStringSlice module_name, module_var_name; } register_;
    struct { WasmAction action; WasmConstVector expected; } assert_return;
    struct { WasmAction action; } assert_return_nan;
    struct { WasmAction action; WasmStringSlice text; } assert_trap;
    struct {
      WasmRawModule module;
      WasmStringSlice text;
    } assert_malformed, assert_invalid, assert_unlinkable;
  };
} WasmCommand;
WASM_DEFINE_VECTOR(command, WasmCommand);

typedef struct WasmScript {
  struct WasmAllocator* allocator;
  WasmCommandVector commands;
} WasmScript;

typedef struct WasmExprVisitor {
  void* user_data;
  WasmResult (*on_binary_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_block_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_block_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_br_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_br_if_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_br_table_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_call_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_call_indirect_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_compare_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_const_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_convert_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_current_memory_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_drop_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_get_global_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_get_local_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_grow_memory_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_if_expr)(WasmExpr*, void* user_data);
  WasmResult (*after_if_true_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_if_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_load_expr)(WasmExpr*, void* user_data);
  WasmResult (*begin_loop_expr)(WasmExpr*, void* user_data);
  WasmResult (*end_loop_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_nop_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_return_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_select_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_set_global_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_set_local_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_store_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_tee_local_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_unary_expr)(WasmExpr*, void* user_data);
  WasmResult (*on_unreachable_expr)(WasmExpr*, void* user_data);
} WasmExprVisitor;

WASM_EXTERN_C_BEGIN
WasmBinding* wasm_insert_binding(struct WasmAllocator*,
                                 WasmBindingHash*,
                                 const WasmStringSlice*);

WasmBool wasm_hash_entry_is_free(const WasmBindingHashEntry*);

WasmModuleField* wasm_append_module_field(struct WasmAllocator*, WasmModule*);
/* ownership of the function signature is passed to the module */
WasmFuncType* wasm_append_implicit_func_type(struct WasmAllocator*,
                                             WasmLocation*,
                                             WasmModule*,
                                             WasmFuncSignature*);

/* WasmExpr creation functions */
WasmExpr* wasm_new_binary_expr(struct WasmAllocator*);
WasmExpr* wasm_new_block_expr(struct WasmAllocator*);
WasmExpr* wasm_new_br_expr(struct WasmAllocator*);
WasmExpr* wasm_new_br_if_expr(struct WasmAllocator*);
WasmExpr* wasm_new_br_table_expr(struct WasmAllocator*);
WasmExpr* wasm_new_call_expr(struct WasmAllocator*);
WasmExpr* wasm_new_call_indirect_expr(struct WasmAllocator*);
WasmExpr* wasm_new_compare_expr(struct WasmAllocator*);
WasmExpr* wasm_new_const_expr(struct WasmAllocator*);
WasmExpr* wasm_new_convert_expr(struct WasmAllocator*);
WasmExpr* wasm_new_current_memory_expr(struct WasmAllocator*);
WasmExpr* wasm_new_drop_expr(struct WasmAllocator*);
WasmExpr* wasm_new_get_global_expr(struct WasmAllocator*);
WasmExpr* wasm_new_get_local_expr(struct WasmAllocator*);
WasmExpr* wasm_new_grow_memory_expr(struct WasmAllocator*);
WasmExpr* wasm_new_if_expr(struct WasmAllocator*);
WasmExpr* wasm_new_load_expr(struct WasmAllocator*);
WasmExpr* wasm_new_loop_expr(struct WasmAllocator*);
WasmExpr* wasm_new_nop_expr(struct WasmAllocator*);
WasmExpr* wasm_new_return_expr(struct WasmAllocator*);
WasmExpr* wasm_new_select_expr(struct WasmAllocator*);
WasmExpr* wasm_new_set_global_expr(struct WasmAllocator*);
WasmExpr* wasm_new_set_local_expr(struct WasmAllocator*);
WasmExpr* wasm_new_store_expr(struct WasmAllocator*);
WasmExpr* wasm_new_tee_local_expr(struct WasmAllocator*);
WasmExpr* wasm_new_unary_expr(struct WasmAllocator*);
WasmExpr* wasm_new_unreachable_expr(struct WasmAllocator*);

/* destruction functions. not needed unless you're creating your own AST
 elements */
void wasm_destroy_script(struct WasmScript*);
void wasm_destroy_action(struct WasmAllocator*, struct WasmAction*);
void wasm_destroy_block(struct WasmAllocator*, struct WasmBlock*);
void wasm_destroy_command_vector_and_elements(struct WasmAllocator*,
                                              WasmCommandVector*);
void wasm_destroy_command(struct WasmAllocator*, WasmCommand*);
void wasm_destroy_data_segment(struct WasmAllocator*, WasmDataSegment*);
void wasm_destroy_elem_segment(struct WasmAllocator*, WasmElemSegment*);
void wasm_destroy_export(struct WasmAllocator*, WasmExport*);
void wasm_destroy_expr(struct WasmAllocator*, WasmExpr*);
void wasm_destroy_expr_list(struct WasmAllocator*, WasmExpr*);
void wasm_destroy_func_declaration(struct WasmAllocator*, WasmFuncDeclaration*);
void wasm_destroy_func_signature(struct WasmAllocator*, WasmFuncSignature*);
void wasm_destroy_func_type(struct WasmAllocator*, WasmFuncType*);
void wasm_destroy_func(struct WasmAllocator*, WasmFunc*);
void wasm_destroy_import(struct WasmAllocator*, WasmImport*);
void wasm_destroy_memory(struct WasmAllocator*, WasmMemory*);
void wasm_destroy_module(struct WasmAllocator*, WasmModule*);
void wasm_destroy_raw_module(struct WasmAllocator*, WasmRawModule*);
void wasm_destroy_table(struct WasmAllocator*, WasmTable*);
void wasm_destroy_var_vector_and_elements(struct WasmAllocator*,
                                          WasmVarVector*);
void wasm_destroy_var(struct WasmAllocator*, WasmVar*);

/* traversal functions */
WasmResult wasm_visit_func(WasmFunc* func, WasmExprVisitor*);

/* convenience functions for looking through the AST */
int wasm_get_index_from_var(const WasmBindingHash* bindings,
                            const WasmVar* var);
int wasm_get_func_index_by_var(const WasmModule* module, const WasmVar* var);
int wasm_get_global_index_by_var(const WasmModule* func, const WasmVar* var);
int wasm_get_func_type_index_by_var(const WasmModule* module,
                                    const WasmVar* var);
int wasm_get_func_type_index_by_sig(const WasmModule* module,
                                    const WasmFuncSignature* sig);
int wasm_get_func_type_index_by_decl(const WasmModule* module,
                                     const WasmFuncDeclaration* decl);
int wasm_get_table_index_by_var(const WasmModule* module, const WasmVar* var);
int wasm_get_memory_index_by_var(const WasmModule* module, const WasmVar* var);
int wasm_get_import_index_by_var(const WasmModule* module, const WasmVar* var);
int wasm_get_local_index_by_var(const WasmFunc* func, const WasmVar* var);

WasmFuncPtr wasm_get_func_by_var(const WasmModule* module, const WasmVar* var);
WasmGlobalPtr wasm_get_global_by_var(const WasmModule* func,
                                     const WasmVar* var);
WasmFuncTypePtr wasm_get_func_type_by_var(const WasmModule* module,
                                          const WasmVar* var);
WasmTablePtr wasm_get_table_by_var(const WasmModule* module,
                                   const WasmVar* var);
WasmMemoryPtr wasm_get_memory_by_var(const WasmModule* module,
                                     const WasmVar* var);
WasmImportPtr wasm_get_import_by_var(const WasmModule* module,
                                     const WasmVar* var);
WasmExportPtr wasm_get_export_by_name(const WasmModule* module,
                                      const WasmStringSlice* name);
WasmModule* wasm_get_first_module(const WasmScript* script);

void wasm_make_type_binding_reverse_mapping(
    struct WasmAllocator*,
    const WasmTypeVector*,
    const WasmBindingHash*,
    WasmStringSliceVector* out_reverse_mapping);

static WASM_INLINE WasmBool
wasm_decl_has_func_type(const WasmFuncDeclaration* decl) {
  return decl->flags & WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
}

static WASM_INLINE WasmBool
wasm_signatures_are_equal(const WasmFuncSignature* sig1,
                          const WasmFuncSignature* sig2) {
  return wasm_type_vectors_are_equal(&sig1->param_types, &sig2->param_types) &&
         wasm_type_vectors_are_equal(&sig1->result_types, &sig2->result_types);
}

static WASM_INLINE size_t wasm_get_num_params(const WasmFunc* func) {
  return func->decl.sig.param_types.size;
}

static WASM_INLINE size_t wasm_get_num_results(const WasmFunc* func) {
  return func->decl.sig.result_types.size;
}

static WASM_INLINE size_t wasm_get_num_locals(const WasmFunc* func) {
  return func->local_types.size;
}

static WASM_INLINE size_t wasm_get_num_params_and_locals(const WasmFunc* func) {
  return wasm_get_num_params(func) + wasm_get_num_locals(func);
}

static WASM_INLINE WasmType wasm_get_param_type(const WasmFunc* func,
                                                int index) {
  assert((size_t)index < func->decl.sig.param_types.size);
  return func->decl.sig.param_types.data[index];
}

static WASM_INLINE WasmType wasm_get_local_type(const WasmFunc* func,
                                                int index) {
  assert((size_t)index < wasm_get_num_locals(func));
  return func->local_types.data[index];
}

static WASM_INLINE WasmType wasm_get_result_type(const WasmFunc* func,
                                                 int index) {
  assert((size_t)index < func->decl.sig.result_types.size);
  return func->decl.sig.result_types.data[index];
}

static WASM_INLINE WasmType
wasm_get_func_type_param_type(const WasmFuncType* func_type, int index) {
  return func_type->sig.param_types.data[index];
}

static WASM_INLINE size_t
wasm_get_func_type_num_params(const WasmFuncType* func_type) {
  return func_type->sig.param_types.size;
}

static WASM_INLINE WasmType
wasm_get_func_type_result_type(const WasmFuncType* func_type, int index) {
  return func_type->sig.result_types.data[index];
}

static WASM_INLINE size_t
wasm_get_func_type_num_results(const WasmFuncType* func_type) {
  return func_type->sig.result_types.size;
}

WASM_EXTERN_C_END

#endif /* WASM_AST_H_ */

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

#ifndef WABT_AST_H_
#define WABT_AST_H_

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "binding-hash.h"
#include "common.h"
#include "type-vector.h"
#include "vector.h"

enum WabtVarType {
  WABT_VAR_TYPE_INDEX,
  WABT_VAR_TYPE_NAME,
};

struct WabtVar {
  WabtLocation loc;
  WabtVarType type;
  union {
    int64_t index;
    WabtStringSlice name;
  };
};
WABT_DEFINE_VECTOR(var, WabtVar);

typedef WabtStringSlice WabtLabel;
WABT_DEFINE_VECTOR(string_slice, WabtStringSlice);

struct WabtConst {
  WabtLocation loc;
  WabtType type;
  union {
    uint32_t u32;
    uint64_t u64;
    uint32_t f32_bits;
    uint64_t f64_bits;
  };
};
WABT_DEFINE_VECTOR(const, WabtConst);

enum WabtExprType {
  WABT_EXPR_TYPE_BINARY,
  WABT_EXPR_TYPE_BLOCK,
  WABT_EXPR_TYPE_BR,
  WABT_EXPR_TYPE_BR_IF,
  WABT_EXPR_TYPE_BR_TABLE,
  WABT_EXPR_TYPE_CALL,
  WABT_EXPR_TYPE_CALL_INDIRECT,
  WABT_EXPR_TYPE_COMPARE,
  WABT_EXPR_TYPE_CONST,
  WABT_EXPR_TYPE_CONVERT,
  WABT_EXPR_TYPE_CURRENT_MEMORY,
  WABT_EXPR_TYPE_DROP,
  WABT_EXPR_TYPE_GET_GLOBAL,
  WABT_EXPR_TYPE_GET_LOCAL,
  WABT_EXPR_TYPE_GROW_MEMORY,
  WABT_EXPR_TYPE_IF,
  WABT_EXPR_TYPE_LOAD,
  WABT_EXPR_TYPE_LOOP,
  WABT_EXPR_TYPE_NOP,
  WABT_EXPR_TYPE_RETURN,
  WABT_EXPR_TYPE_SELECT,
  WABT_EXPR_TYPE_SET_GLOBAL,
  WABT_EXPR_TYPE_SET_LOCAL,
  WABT_EXPR_TYPE_STORE,
  WABT_EXPR_TYPE_TEE_LOCAL,
  WABT_EXPR_TYPE_UNARY,
  WABT_EXPR_TYPE_UNREACHABLE,
};

typedef WabtTypeVector WabtBlockSignature;

struct WabtBlock {
  WabtLabel label;
  WabtBlockSignature sig;
  struct WabtExpr* first;
};

struct WabtExpr {
  WabtLocation loc;
  WabtExprType type;
  WabtExpr* next;
  union {
    struct { WabtOpcode opcode; } binary, compare, convert, unary;
    WabtBlock block, loop;
    struct { WabtVar var; } br, br_if;
    struct { WabtVarVector targets; WabtVar default_target; } br_table;
    struct { WabtVar var; } call, call_indirect;
    WabtConst const_;
    struct { WabtVar var; } get_global, set_global;
    struct { WabtVar var; } get_local, set_local, tee_local;
    struct { WabtBlock true_; struct WabtExpr* false_; } if_;
    struct { WabtOpcode opcode; uint32_t align; uint64_t offset; } load, store;
  };
};

struct WabtFuncSignature {
  WabtTypeVector param_types;
  WabtTypeVector result_types;
};

struct WabtFuncType {
  WabtStringSlice name;
  WabtFuncSignature sig;
};
typedef WabtFuncType* WabtFuncTypePtr;
WABT_DEFINE_VECTOR(func_type_ptr, WabtFuncTypePtr);

enum {
  WABT_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE = 1,
  /* set if the signature is owned by module */
  WABT_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE = 2,
};
typedef uint32_t WabtFuncDeclarationFlags;

struct WabtFuncDeclaration {
  WabtFuncDeclarationFlags flags;
  WabtVar type_var;
  WabtFuncSignature sig;
};

struct WabtFunc {
  WabtStringSlice name;
  WabtFuncDeclaration decl;
  WabtTypeVector local_types;
  WabtBindingHash param_bindings;
  WabtBindingHash local_bindings;
  WabtExpr* first_expr;
};
typedef WabtFunc* WabtFuncPtr;
WABT_DEFINE_VECTOR(func_ptr, WabtFuncPtr);

struct WabtGlobal {
  WabtStringSlice name;
  WabtType type;
  bool mutable_;
  WabtExpr* init_expr;
};
typedef WabtGlobal* WabtGlobalPtr;
WABT_DEFINE_VECTOR(global_ptr, WabtGlobalPtr);

struct WabtTable {
  WabtStringSlice name;
  WabtLimits elem_limits;
};
typedef WabtTable* WabtTablePtr;
WABT_DEFINE_VECTOR(table_ptr, WabtTablePtr);

struct WabtElemSegment {
  WabtVar table_var;
  WabtExpr* offset;
  WabtVarVector vars;
};
typedef WabtElemSegment* WabtElemSegmentPtr;
WABT_DEFINE_VECTOR(elem_segment_ptr, WabtElemSegmentPtr);

struct WabtMemory {
  WabtStringSlice name;
  WabtLimits page_limits;
};
typedef WabtMemory* WabtMemoryPtr;
WABT_DEFINE_VECTOR(memory_ptr, WabtMemoryPtr);

struct WabtDataSegment {
  WabtVar memory_var;
  WabtExpr* offset;
  void* data;
  size_t size;
};
typedef WabtDataSegment* WabtDataSegmentPtr;
WABT_DEFINE_VECTOR(data_segment_ptr, WabtDataSegmentPtr);

struct WabtImport {
  WabtStringSlice module_name;
  WabtStringSlice field_name;
  WabtExternalKind kind;
  union {
    /* an imported func is has the type WabtFunc so it can be more easily
     * included in the Module's vector of funcs; but only the
     * WabtFuncDeclaration will have any useful information */
    WabtFunc func;
    WabtTable table;
    WabtMemory memory;
    WabtGlobal global;
  };
};
typedef WabtImport* WabtImportPtr;
WABT_DEFINE_VECTOR(import_ptr, WabtImportPtr);

struct WabtExport {
  WabtStringSlice name;
  WabtExternalKind kind;
  WabtVar var;
};
typedef WabtExport* WabtExportPtr;
WABT_DEFINE_VECTOR(export_ptr, WabtExportPtr);

enum WabtModuleFieldType {
  WABT_MODULE_FIELD_TYPE_FUNC,
  WABT_MODULE_FIELD_TYPE_GLOBAL,
  WABT_MODULE_FIELD_TYPE_IMPORT,
  WABT_MODULE_FIELD_TYPE_EXPORT,
  WABT_MODULE_FIELD_TYPE_FUNC_TYPE,
  WABT_MODULE_FIELD_TYPE_TABLE,
  WABT_MODULE_FIELD_TYPE_ELEM_SEGMENT,
  WABT_MODULE_FIELD_TYPE_MEMORY,
  WABT_MODULE_FIELD_TYPE_DATA_SEGMENT,
  WABT_MODULE_FIELD_TYPE_START,
};

struct WabtModuleField {
  WabtLocation loc;
  WabtModuleFieldType type;
  struct WabtModuleField* next;
  union {
    WabtFunc func;
    WabtGlobal global;
    WabtImport import;
    WabtExport export_;
    WabtFuncType func_type;
    WabtTable table;
    WabtElemSegment elem_segment;
    WabtMemory memory;
    WabtDataSegment data_segment;
    WabtVar start;
  };
};

struct WabtModule {
  WabtLocation loc;
  WabtStringSlice name;
  WabtModuleField* first_field;
  WabtModuleField* last_field;

  uint32_t num_func_imports;
  uint32_t num_table_imports;
  uint32_t num_memory_imports;
  uint32_t num_global_imports;

  /* cached for convenience; the pointers are shared with values that are
   * stored in either WabtModuleField or WabtImport. */
  WabtFuncPtrVector funcs;
  WabtGlobalPtrVector globals;
  WabtImportPtrVector imports;
  WabtExportPtrVector exports;
  WabtFuncTypePtrVector func_types;
  WabtTablePtrVector tables;
  WabtElemSegmentPtrVector elem_segments;
  WabtMemoryPtrVector memories;
  WabtDataSegmentPtrVector data_segments;
  WabtVar* start;

  WabtBindingHash func_bindings;
  WabtBindingHash global_bindings;
  WabtBindingHash export_bindings;
  WabtBindingHash func_type_bindings;
  WabtBindingHash table_bindings;
  WabtBindingHash memory_bindings;
};

enum WabtRawModuleType {
  WABT_RAW_MODULE_TYPE_TEXT,
  WABT_RAW_MODULE_TYPE_BINARY,
};

/* "raw" means that the binary module has not yet been decoded. This is only
 * necessary when embedded in assert_invalid. In that case we want to defer
 * decoding errors until wabt_check_assert_invalid is called. This isn't needed
 * when parsing text, as assert_invalid always assumes that text parsing
 * succeeds. */
struct WabtRawModule {
  WabtRawModuleType type;
  union {
    WabtModule* text;
    struct {
      WabtLocation loc;
      WabtStringSlice name;
      void* data;
      size_t size;
    } binary;
  };
};

enum WabtActionType {
  WABT_ACTION_TYPE_INVOKE,
  WABT_ACTION_TYPE_GET,
};

struct WabtActionInvoke {
  WabtStringSlice name;
  WabtConstVector args;
};

struct WabtActionGet {
  WabtStringSlice name;
};

struct WabtAction {
  WabtLocation loc;
  WabtActionType type;
  WabtVar module_var;
  union {
    WabtActionInvoke invoke;
    WabtActionGet get;
  };
};

enum WabtCommandType {
  WABT_COMMAND_TYPE_MODULE,
  WABT_COMMAND_TYPE_ACTION,
  WABT_COMMAND_TYPE_REGISTER,
  WABT_COMMAND_TYPE_ASSERT_MALFORMED,
  WABT_COMMAND_TYPE_ASSERT_INVALID,
  /* This is a module that is invalid, but cannot be written as a binary module
   * (e.g. it has unresolvable names.) */
  WABT_COMMAND_TYPE_ASSERT_INVALID_NON_BINARY,
  WABT_COMMAND_TYPE_ASSERT_UNLINKABLE,
  WABT_COMMAND_TYPE_ASSERT_UNINSTANTIABLE,
  WABT_COMMAND_TYPE_ASSERT_RETURN,
  WABT_COMMAND_TYPE_ASSERT_RETURN_NAN,
  WABT_COMMAND_TYPE_ASSERT_TRAP,
  WABT_COMMAND_TYPE_ASSERT_EXHAUSTION,
  WABT_NUM_COMMAND_TYPES,
};

struct WabtCommand {
  WabtCommandType type;
  union {
    WabtModule module;
    WabtAction action;
    struct { WabtStringSlice module_name; WabtVar var; } register_;
    struct { WabtAction action; WabtConstVector expected; } assert_return;
    struct { WabtAction action; } assert_return_nan;
    struct { WabtAction action; WabtStringSlice text; } assert_trap;
    struct {
      WabtRawModule module;
      WabtStringSlice text;
    } assert_malformed, assert_invalid, assert_unlinkable,
        assert_uninstantiable;
  };
};
WABT_DEFINE_VECTOR(command, WabtCommand);

struct WabtScript {
  WabtCommandVector commands;
  WabtBindingHash module_bindings;
};

struct WabtExprVisitor {
  void* user_data;
  WabtResult (*on_binary_expr)(WabtExpr*, void* user_data);
  WabtResult (*begin_block_expr)(WabtExpr*, void* user_data);
  WabtResult (*end_block_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_br_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_br_if_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_br_table_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_call_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_call_indirect_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_compare_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_const_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_convert_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_current_memory_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_drop_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_get_global_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_get_local_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_grow_memory_expr)(WabtExpr*, void* user_data);
  WabtResult (*begin_if_expr)(WabtExpr*, void* user_data);
  WabtResult (*after_if_true_expr)(WabtExpr*, void* user_data);
  WabtResult (*end_if_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_load_expr)(WabtExpr*, void* user_data);
  WabtResult (*begin_loop_expr)(WabtExpr*, void* user_data);
  WabtResult (*end_loop_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_nop_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_return_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_select_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_set_global_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_set_local_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_store_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_tee_local_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_unary_expr)(WabtExpr*, void* user_data);
  WabtResult (*on_unreachable_expr)(WabtExpr*, void* user_data);
};

WABT_EXTERN_C_BEGIN
WabtModuleField* wabt_append_module_field(WabtModule*);
/* ownership of the function signature is passed to the module */
WabtFuncType* wabt_append_implicit_func_type(WabtLocation*,
                                             WabtModule*,
                                             WabtFuncSignature*);

/* WabtExpr creation functions */
WabtExpr* wabt_new_binary_expr(void);
WabtExpr* wabt_new_block_expr(void);
WabtExpr* wabt_new_br_expr(void);
WabtExpr* wabt_new_br_if_expr(void);
WabtExpr* wabt_new_br_table_expr(void);
WabtExpr* wabt_new_call_expr(void);
WabtExpr* wabt_new_call_indirect_expr(void);
WabtExpr* wabt_new_compare_expr(void);
WabtExpr* wabt_new_const_expr(void);
WabtExpr* wabt_new_convert_expr(void);
WabtExpr* wabt_new_current_memory_expr(void);
WabtExpr* wabt_new_drop_expr(void);
WabtExpr* wabt_new_get_global_expr(void);
WabtExpr* wabt_new_get_local_expr(void);
WabtExpr* wabt_new_grow_memory_expr(void);
WabtExpr* wabt_new_if_expr(void);
WabtExpr* wabt_new_load_expr(void);
WabtExpr* wabt_new_loop_expr(void);
WabtExpr* wabt_new_nop_expr(void);
WabtExpr* wabt_new_return_expr(void);
WabtExpr* wabt_new_select_expr(void);
WabtExpr* wabt_new_set_global_expr(void);
WabtExpr* wabt_new_set_local_expr(void);
WabtExpr* wabt_new_store_expr(void);
WabtExpr* wabt_new_tee_local_expr(void);
WabtExpr* wabt_new_unary_expr(void);
WabtExpr* wabt_new_unreachable_expr(void);

/* destruction functions. not needed unless you're creating your own AST
 elements */
void wabt_destroy_script(struct WabtScript*);
void wabt_destroy_action(struct WabtAction*);
void wabt_destroy_block(struct WabtBlock*);
void wabt_destroy_command_vector_and_elements(WabtCommandVector*);
void wabt_destroy_command(WabtCommand*);
void wabt_destroy_data_segment(WabtDataSegment*);
void wabt_destroy_elem_segment(WabtElemSegment*);
void wabt_destroy_export(WabtExport*);
void wabt_destroy_expr(WabtExpr*);
void wabt_destroy_expr_list(WabtExpr*);
void wabt_destroy_func_declaration(WabtFuncDeclaration*);
void wabt_destroy_func_signature(WabtFuncSignature*);
void wabt_destroy_func_type(WabtFuncType*);
void wabt_destroy_func(WabtFunc*);
void wabt_destroy_import(WabtImport*);
void wabt_destroy_memory(WabtMemory*);
void wabt_destroy_module(WabtModule*);
void wabt_destroy_raw_module(WabtRawModule*);
void wabt_destroy_table(WabtTable*);
void wabt_destroy_var_vector_and_elements(WabtVarVector*);
void wabt_destroy_var(WabtVar*);

/* traversal functions */
WabtResult wabt_visit_func(WabtFunc* func, WabtExprVisitor*);
WabtResult wabt_visit_expr_list(WabtExpr* expr, WabtExprVisitor*);

/* convenience functions for looking through the AST */
int wabt_get_index_from_var(const WabtBindingHash* bindings,
                            const WabtVar* var);
int wabt_get_func_index_by_var(const WabtModule* module, const WabtVar* var);
int wabt_get_global_index_by_var(const WabtModule* func, const WabtVar* var);
int wabt_get_func_type_index_by_var(const WabtModule* module,
                                    const WabtVar* var);
int wabt_get_func_type_index_by_sig(const WabtModule* module,
                                    const WabtFuncSignature* sig);
int wabt_get_func_type_index_by_decl(const WabtModule* module,
                                     const WabtFuncDeclaration* decl);
int wabt_get_table_index_by_var(const WabtModule* module, const WabtVar* var);
int wabt_get_memory_index_by_var(const WabtModule* module, const WabtVar* var);
int wabt_get_import_index_by_var(const WabtModule* module, const WabtVar* var);
int wabt_get_local_index_by_var(const WabtFunc* func, const WabtVar* var);
int wabt_get_module_index_by_var(const WabtScript* script, const WabtVar* var);

WabtFuncPtr wabt_get_func_by_var(const WabtModule* module, const WabtVar* var);
WabtGlobalPtr wabt_get_global_by_var(const WabtModule* func,
                                     const WabtVar* var);
WabtFuncTypePtr wabt_get_func_type_by_var(const WabtModule* module,
                                          const WabtVar* var);
WabtTablePtr wabt_get_table_by_var(const WabtModule* module,
                                   const WabtVar* var);
WabtMemoryPtr wabt_get_memory_by_var(const WabtModule* module,
                                     const WabtVar* var);
WabtImportPtr wabt_get_import_by_var(const WabtModule* module,
                                     const WabtVar* var);
WabtExportPtr wabt_get_export_by_name(const WabtModule* module,
                                      const WabtStringSlice* name);
WabtModule* wabt_get_first_module(const WabtScript* script);
WabtModule* wabt_get_module_by_var(const WabtScript* script,
                                   const WabtVar* var);

void wabt_make_type_binding_reverse_mapping(
    const WabtTypeVector*,
    const WabtBindingHash*,
    WabtStringSliceVector* out_reverse_mapping);

typedef void (*WabtDuplicateBindingCallback)(WabtBindingHashEntry* a,
                                             WabtBindingHashEntry* b,
                                             void* user_data);
void wabt_find_duplicate_bindings(const WabtBindingHash*,
                                  WabtDuplicateBindingCallback callback,
                                  void* user_data);

static WABT_INLINE bool
wabt_decl_has_func_type(const WabtFuncDeclaration* decl) {
  return static_cast<bool>(
      (decl->flags & WABT_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE) != 0);
}

static WABT_INLINE bool
wabt_signatures_are_equal(const WabtFuncSignature* sig1,
                          const WabtFuncSignature* sig2) {
  return static_cast<bool>(
      wabt_type_vectors_are_equal(&sig1->param_types, &sig2->param_types) &&
      wabt_type_vectors_are_equal(&sig1->result_types, &sig2->result_types));
}

static WABT_INLINE size_t wabt_get_num_params(const WabtFunc* func) {
  return func->decl.sig.param_types.size;
}

static WABT_INLINE size_t wabt_get_num_results(const WabtFunc* func) {
  return func->decl.sig.result_types.size;
}

static WABT_INLINE size_t wabt_get_num_locals(const WabtFunc* func) {
  return func->local_types.size;
}

static WABT_INLINE size_t wabt_get_num_params_and_locals(const WabtFunc* func) {
  return wabt_get_num_params(func) + wabt_get_num_locals(func);
}

static WABT_INLINE WabtType wabt_get_param_type(const WabtFunc* func,
                                                int index) {
  assert(static_cast<size_t>(index) < func->decl.sig.param_types.size);
  return func->decl.sig.param_types.data[index];
}

static WABT_INLINE WabtType wabt_get_local_type(const WabtFunc* func,
                                                int index) {
  assert(static_cast<size_t>(index) < wabt_get_num_locals(func));
  return func->local_types.data[index];
}

static WABT_INLINE WabtType wabt_get_result_type(const WabtFunc* func,
                                                 int index) {
  assert(static_cast<size_t>(index) < func->decl.sig.result_types.size);
  return func->decl.sig.result_types.data[index];
}

static WABT_INLINE WabtType
wabt_get_func_type_param_type(const WabtFuncType* func_type, int index) {
  return func_type->sig.param_types.data[index];
}

static WABT_INLINE size_t
wabt_get_func_type_num_params(const WabtFuncType* func_type) {
  return func_type->sig.param_types.size;
}

static WABT_INLINE WabtType
wabt_get_func_type_result_type(const WabtFuncType* func_type, int index) {
  return func_type->sig.result_types.data[index];
}

static WABT_INLINE size_t
wabt_get_func_type_num_results(const WabtFuncType* func_type) {
  return func_type->sig.result_types.size;
}

static WABT_INLINE const WabtLocation* wabt_get_raw_module_location(
    const WabtRawModule* raw) {
  switch (raw->type) {
    case WABT_RAW_MODULE_TYPE_BINARY: return &raw->binary.loc;
    case WABT_RAW_MODULE_TYPE_TEXT: return &raw->text->loc;
    default:
      assert(0);
      return nullptr;
  }
}

WABT_EXTERN_C_END

#endif /* WABT_AST_H_ */

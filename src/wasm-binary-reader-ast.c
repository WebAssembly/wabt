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

#include "wasm-binary-reader-ast.h"

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "wasm-allocator.h"
#include "wasm-ast.h"
#include "wasm-binary-reader.h"
#include "wasm-common.h"

#define CHECK_RESULT(expr) \
  do {                     \
    if (WASM_FAILED(expr)) \
      return WASM_ERROR;   \
  } while (0)

typedef enum LabelType {
  LABEL_TYPE_FUNC,
  LABEL_TYPE_BLOCK,
  LABEL_TYPE_LOOP,
  LABEL_TYPE_IF,
  LABEL_TYPE_ELSE,
} LabelType;

typedef struct LabelNode {
  LabelType label_type;
  WasmExpr** first;
  WasmExpr* last;
} LabelNode;
WASM_DEFINE_VECTOR(label_node, LabelNode);

typedef struct Context {
  WasmAllocator* allocator;
  WasmBinaryErrorHandler* error_handler;
  WasmModule* module;

  WasmFunc* current_func;
  LabelNodeVector label_stack;
  uint32_t max_depth;
  WasmExpr** current_init_expr;
} Context;

static void handle_error(Context* ctx, uint32_t offset, const char* message);

static void WASM_PRINTF_FORMAT(2, 3)
    print_error(Context* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  handle_error(ctx, WASM_UNKNOWN_OFFSET, buffer);
}

static void push_label(Context* ctx, LabelType label_type, WasmExpr** first) {
  LabelNode label;
  label.label_type = label_type;
  label.first = first;
  label.last = NULL;
  ctx->max_depth++;
  wasm_append_label_node_value(ctx->allocator, &ctx->label_stack, &label);
}

static WasmResult pop_label(Context* ctx) {
  if (ctx->label_stack.size == 0) {
    print_error(ctx, "popping empty label stack");
    return WASM_ERROR;
  }

  ctx->max_depth--;
  ctx->label_stack.size--;
  return WASM_OK;
}

static WasmResult get_label_at(Context* ctx,
                               LabelNode** label,
                               uint32_t depth) {
  if (depth > ctx->label_stack.size) {
    print_error(ctx, "accessing stack depth: %u > max: %" PRIzd, depth,
                ctx->label_stack.size);
    return WASM_ERROR;
  }

  *label = &ctx->label_stack.data[ctx->label_stack.size - depth - 1];
  return WASM_OK;
}

static WasmResult top_label(Context* ctx, LabelNode** label) {
  return get_label_at(ctx, label, 0);
}

static void dup_name(Context* ctx,
                     WasmStringSlice* name,
                     WasmStringSlice* out_name) {
  if (name->length > 0) {
    *out_name = wasm_dup_string_slice(ctx->allocator, *name);
  } else {
    WASM_ZERO_MEMORY(*out_name);
  }
}

static WasmResult append_expr(Context* ctx, WasmExpr* expr) {
  LabelNode* label;
  CHECK_RESULT(top_label(ctx, &label));
  if (*label->first) {
    label->last->next = expr;
    label->last = expr;
  } else {
    *label->first = label->last = expr;
  }
  return WASM_OK;
}

static void handle_error(Context* ctx, uint32_t offset, const char* message) {
  if (ctx->error_handler->on_error) {
    ctx->error_handler->on_error(offset, message,
                                 ctx->error_handler->user_data);
  }
}

static void on_error(WasmBinaryReaderContext* reader_context,
                     const char* message) {
  Context* ctx = reader_context->user_data;
  handle_error(ctx, reader_context->offset, message);
}

static WasmResult on_signature_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_reserve_func_type_ptrs(ctx->allocator, &ctx->module->func_types, count);
  return WASM_OK;
}

static WasmResult on_signature(uint32_t index,
                               uint32_t param_count,
                               WasmType* param_types,
                               uint32_t result_count,
                               WasmType* result_types,
                               void* user_data) {
  Context* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;

  WasmFuncType* func_type = &field->func_type;
  WASM_ZERO_MEMORY(*func_type);

  wasm_reserve_types(ctx->allocator, &func_type->sig.param_types, param_count);
  func_type->sig.param_types.size = param_count;
  memcpy(func_type->sig.param_types.data, param_types,
         param_count * sizeof(WasmType));

  wasm_reserve_types(ctx->allocator, &func_type->sig.result_types,
                     result_count);
  func_type->sig.result_types.size = result_count;
  memcpy(func_type->sig.result_types.data, result_types,
         result_count * sizeof(WasmType));

  assert(index < ctx->module->func_types.capacity);
  WasmFuncTypePtr* func_type_ptr =
      wasm_append_func_type_ptr(ctx->allocator, &ctx->module->func_types);
  *func_type_ptr = func_type;
  return WASM_OK;
}

static WasmResult on_import_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_reserve_import_ptrs(ctx->allocator, &ctx->module->imports, count);
  return WASM_OK;
}

static WasmResult on_import(uint32_t index,
                            WasmStringSlice module_name,
                            WasmStringSlice field_name,
                            void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->imports.capacity);

  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_IMPORT;

  WasmImport* import = &field->import;
  WASM_ZERO_MEMORY(*import);
  import->module_name = wasm_dup_string_slice(ctx->allocator, module_name);
  import->field_name = wasm_dup_string_slice(ctx->allocator, field_name);

  WasmImportPtr* import_ptr =
      wasm_append_import_ptr(ctx->allocator, &ctx->module->imports);
  *import_ptr = import;
  return WASM_OK;
}

static WasmResult on_import_func(uint32_t index,
                                 uint32_t sig_index,
                                 void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->imports.size - 1);
  assert(sig_index < ctx->module->func_types.size);
  WasmImport* import = ctx->module->imports.data[index];

  import->kind = WASM_EXTERNAL_KIND_FUNC;
  import->func.decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE |
                            WASM_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
  import->func.decl.type_var.type = WASM_VAR_TYPE_INDEX;
  import->func.decl.type_var.index = sig_index;
  import->func.decl.sig = ctx->module->func_types.data[sig_index]->sig;

  WasmFuncPtr func_ptr = &import->func;
  wasm_append_func_ptr_value(ctx->allocator, &ctx->module->funcs, &func_ptr);
  ctx->module->num_func_imports++;
  return WASM_OK;
}

static WasmResult on_import_table(uint32_t index,
                                  uint32_t elem_type,
                                  const WasmLimits* elem_limits,
                                  void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->imports.size - 1);
  WasmImport* import = ctx->module->imports.data[index];
  import->kind = WASM_EXTERNAL_KIND_TABLE;
  import->table.elem_limits = *elem_limits;

  WasmTablePtr table_ptr = &import->table;
  wasm_append_table_ptr_value(ctx->allocator, &ctx->module->tables, &table_ptr);
  ctx->module->num_table_imports++;
  return WASM_OK;
}

static WasmResult on_import_memory(uint32_t index,
                                   const WasmLimits* page_limits,
                                   void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->imports.size - 1);
  WasmImport* import = ctx->module->imports.data[index];
  import->kind = WASM_EXTERNAL_KIND_MEMORY;
  import->memory.page_limits = *page_limits;

  WasmMemoryPtr memory_ptr = &import->memory;
  wasm_append_memory_ptr_value(ctx->allocator, &ctx->module->memories,
                               &memory_ptr);
  ctx->module->num_memory_imports++;
  return WASM_OK;
}

static WasmResult on_import_global(uint32_t index,
                                   WasmType type,
                                   WasmBool mutable_,
                                   void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->imports.size - 1);
  WasmImport* import = ctx->module->imports.data[index];
  import->kind = WASM_EXTERNAL_KIND_GLOBAL;
  import->global.type = type;
  import->global.mutable_ = mutable_;

  WasmGlobalPtr global_ptr = &import->global;
  wasm_append_global_ptr_value(ctx->allocator, &ctx->module->globals,
                               &global_ptr);
  ctx->module->num_global_imports++;
  return WASM_OK;
}

static WasmResult on_function_signatures_count(uint32_t count,
                                               void* user_data) {
  Context* ctx = user_data;
  wasm_reserve_func_ptrs(ctx->allocator, &ctx->module->funcs, count);
  return WASM_OK;
}

static WasmResult on_function_signature(uint32_t index,
                                        uint32_t sig_index,
                                        void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->funcs.capacity);
  assert(sig_index < ctx->module->func_types.size);

  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_FUNC;

  WasmFunc* func = &field->func;
  WASM_ZERO_MEMORY(*func);
  func->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE |
                     WASM_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
  func->decl.type_var.type = WASM_VAR_TYPE_INDEX;
  func->decl.type_var.index = sig_index;
  func->decl.sig = ctx->module->func_types.data[sig_index]->sig;

  WasmFuncPtr* func_ptr =
      wasm_append_func_ptr(ctx->allocator, &ctx->module->funcs);
  *func_ptr = func;
  return WASM_OK;
}

static WasmResult on_table_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_reserve_table_ptrs(ctx->allocator, &ctx->module->tables, count);
  return WASM_OK;
}

static WasmResult on_table(uint32_t index,
                           uint32_t elem_type,
                           const WasmLimits* elem_limits,
                           void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->tables.capacity);

  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_TABLE;

  WasmTable* table = &field->table;
  WASM_ZERO_MEMORY(*table);
  table->elem_limits = *elem_limits;

  WasmTablePtr* table_ptr =
      wasm_append_table_ptr(ctx->allocator, &ctx->module->tables);
  *table_ptr = table;
  return WASM_OK;
}

static WasmResult on_memory_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_reserve_memory_ptrs(ctx->allocator, &ctx->module->memories, count);
  return WASM_OK;
}

static WasmResult on_memory(uint32_t index,
                            const WasmLimits* page_limits,
                            void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->memories.capacity);

  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_MEMORY;

  WasmMemory* memory = &field->memory;
  WASM_ZERO_MEMORY(*memory);
  memory->page_limits = *page_limits;

  WasmMemoryPtr* memory_ptr =
      wasm_append_memory_ptr(ctx->allocator, &ctx->module->memories);
  *memory_ptr = memory;
  return WASM_OK;
}

static WasmResult on_global_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_reserve_global_ptrs(ctx->allocator, &ctx->module->globals, count);
  return WASM_OK;
}

static WasmResult begin_global(uint32_t index,
                               WasmType type,
                               WasmBool mutable_,
                               void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->globals.capacity);

  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_GLOBAL;

  WasmGlobal* global = &field->global;
  WASM_ZERO_MEMORY(*global);
  global->type = type;
  global->mutable_ = mutable_;

  WasmGlobalPtr* global_ptr =
      wasm_append_global_ptr(ctx->allocator, &ctx->module->globals);
  *global_ptr = global;
  return WASM_OK;
}

static WasmResult begin_global_init_expr(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  assert(ctx->module->num_global_imports + index ==
         ctx->module->globals.size - 1);
  WasmGlobal* global =
      ctx->module->globals.data[index + ctx->module->num_global_imports];
  ctx->current_init_expr = &global->init_expr;
  return WASM_OK;
}

static WasmResult end_global_init_expr(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  ctx->current_init_expr = NULL;
  return WASM_OK;
}

static WasmResult on_export_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_reserve_export_ptrs(ctx->allocator, &ctx->module->exports, count);
  return WASM_OK;
}

static WasmResult on_export(uint32_t index,
                            WasmExternalKind kind,
                            uint32_t item_index,
                            WasmStringSlice name,
                            void* user_data) {
  Context* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_EXPORT;

  WasmExport* export = &field->export_;
  WASM_ZERO_MEMORY(*export);
  export->name = wasm_dup_string_slice(ctx->allocator, name);
  switch (kind) {
    case WASM_EXTERNAL_KIND_FUNC:
      assert(item_index < ctx->module->funcs.size);
      break;
    case WASM_EXTERNAL_KIND_TABLE:
      assert(item_index < ctx->module->tables.size);
      break;
    case WASM_EXTERNAL_KIND_MEMORY:
      assert(item_index < ctx->module->memories.size);
      break;
    case WASM_EXTERNAL_KIND_GLOBAL:
      assert(item_index < ctx->module->globals.size);
      break;
    case WASM_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
  export->var.type = WASM_VAR_TYPE_INDEX;
  export->var.index = item_index;
  export->kind = kind;

  assert(index < ctx->module->exports.capacity);
  WasmExportPtr* export_ptr =
      wasm_append_export_ptr(ctx->allocator, &ctx->module->exports);
  *export_ptr = export;
  return WASM_OK;
}

static WasmResult on_start_function(uint32_t func_index, void* user_data) {
  Context* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_START;

  field->start.type = WASM_VAR_TYPE_INDEX;
  assert(func_index < ctx->module->funcs.size);
  field->start.index = func_index;

  ctx->module->start = &field->start;
  return WASM_OK;
}

static WasmResult on_function_bodies_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  assert(ctx->module->num_func_imports + count == ctx->module->funcs.size);
  (void)ctx;
  return WASM_OK;
}

static WasmResult begin_function_body(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->funcs.size);
  ctx->current_func =
      ctx->module->funcs.data[index + ctx->module->num_func_imports];
  push_label(ctx, LABEL_TYPE_FUNC, &ctx->current_func->first_expr);
  return WASM_OK;
}

static WasmResult on_local_decl(uint32_t decl_index,
                                uint32_t count,
                                WasmType type,
                                void* user_data) {
  Context* ctx = user_data;
  size_t old_local_count = ctx->current_func->local_types.size;
  size_t new_local_count = old_local_count + count;
  wasm_reserve_types(ctx->allocator, &ctx->current_func->local_types,
                     new_local_count);
  WASM_STATIC_ASSERT(sizeof(WasmType) == sizeof(uint8_t));
  WasmTypeVector* types = &ctx->current_func->local_types;
  memset(&types->data[old_local_count], type, count);
  types->size = new_local_count;
  return WASM_OK;
}

static WasmResult on_binary_expr(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_binary_expr(ctx->allocator);
  expr->binary.opcode = opcode;
  return append_expr(ctx, expr);
}

static WasmResult on_block_expr(uint32_t num_types,
                                WasmType* sig_types,
                                void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_block_expr(ctx->allocator);
  WasmTypeVector src;
  WASM_ZERO_MEMORY(src);
  src.size = num_types;
  src.data = sig_types;
  wasm_extend_types(ctx->allocator, &expr->block.sig, &src);
  append_expr(ctx, expr);
  push_label(ctx, LABEL_TYPE_BLOCK, &expr->block.first);
  return WASM_OK;
}

static WasmResult on_br_expr(uint32_t depth, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_br_expr(ctx->allocator);
  expr->br.var.type = WASM_VAR_TYPE_INDEX;
  expr->br.var.index = depth;
  return append_expr(ctx, expr);
}

static WasmResult on_br_if_expr(uint32_t depth, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_br_if_expr(ctx->allocator);
  expr->br_if.var.type = WASM_VAR_TYPE_INDEX;
  expr->br_if.var.index = depth;
  return append_expr(ctx, expr);
}

static WasmResult on_br_table_expr(WasmBinaryReaderContext* context,
                                   uint32_t num_targets,
                                   uint32_t* target_depths,
                                   uint32_t default_target_depth) {
  Context* ctx = context->user_data;
  WasmExpr* expr = wasm_new_br_table_expr(ctx->allocator);
  wasm_reserve_vars(ctx->allocator, &expr->br_table.targets, num_targets);
  expr->br_table.targets.size = num_targets;
  uint32_t i;
  for (i = 0; i < num_targets; ++i) {
    WasmVar* var = &expr->br_table.targets.data[i];
    var->type = WASM_VAR_TYPE_INDEX;
    var->index = target_depths[i];
  }
  expr->br_table.default_target.type = WASM_VAR_TYPE_INDEX;
  expr->br_table.default_target.index = default_target_depth;
  return append_expr(ctx, expr);
}

static WasmResult on_call_expr(uint32_t func_index, void* user_data) {
  Context* ctx = user_data;
  assert(func_index < ctx->module->funcs.size);
  WasmExpr* expr = wasm_new_call_expr(ctx->allocator);
  expr->call.var.type = WASM_VAR_TYPE_INDEX;
  expr->call.var.index = func_index;
  return append_expr(ctx, expr);
}

static WasmResult on_call_indirect_expr(uint32_t sig_index, void* user_data) {
  Context* ctx = user_data;
  assert(sig_index < ctx->module->func_types.size);
  WasmExpr* expr = wasm_new_call_indirect_expr(ctx->allocator);
  expr->call_indirect.var.type = WASM_VAR_TYPE_INDEX;
  expr->call_indirect.var.index = sig_index;
  return append_expr(ctx, expr);
}

static WasmResult on_compare_expr(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_compare_expr(ctx->allocator);
  expr->compare.opcode = opcode;
  return append_expr(ctx, expr);
}

static WasmResult on_convert_expr(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_convert_expr(ctx->allocator);
  expr->convert.opcode = opcode;
  return append_expr(ctx, expr);
}

static WasmResult on_current_memory_expr(void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_current_memory_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WasmResult on_drop_expr(void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_drop_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WasmResult on_else_expr(void* user_data) {
  Context* ctx = user_data;
  LabelNode* label;
  CHECK_RESULT(top_label(ctx, &label));
  if (label->label_type != LABEL_TYPE_IF) {
    print_error(ctx, "else expression without matching if");
    return WASM_ERROR;
  }

  LabelNode* parent_label;
  CHECK_RESULT(get_label_at(ctx, &parent_label, 1));
  assert(parent_label->last->type == WASM_EXPR_TYPE_IF);

  label->label_type = LABEL_TYPE_ELSE;
  label->first = &parent_label->last->if_.false_;
  label->last = NULL;
  return WASM_OK;
}

static WasmResult on_end_expr(void* user_data) {
  Context* ctx = user_data;
  return pop_label(ctx);
}

static WasmResult on_f32_const_expr(uint32_t value_bits, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_const_expr(ctx->allocator);
  expr->const_.type = WASM_TYPE_F32;
  expr->const_.f32_bits = value_bits;
  return append_expr(ctx, expr);
}

static WasmResult on_f64_const_expr(uint64_t value_bits, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_const_expr(ctx->allocator);
  expr->const_.type = WASM_TYPE_F64;
  expr->const_.f64_bits = value_bits;
  return append_expr(ctx, expr);
}

static WasmResult on_get_global_expr(uint32_t global_index, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_get_global_expr(ctx->allocator);
  expr->get_global.var.type = WASM_VAR_TYPE_INDEX;
  expr->get_global.var.index = global_index;
  return append_expr(ctx, expr);
}

static WasmResult on_get_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_get_local_expr(ctx->allocator);
  expr->get_local.var.type = WASM_VAR_TYPE_INDEX;
  expr->get_local.var.index = local_index;
  return append_expr(ctx, expr);
}

static WasmResult on_grow_memory_expr(void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_grow_memory_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WasmResult on_i32_const_expr(uint32_t value, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_const_expr(ctx->allocator);
  expr->const_.type = WASM_TYPE_I32;
  expr->const_.u32 = value;
  return append_expr(ctx, expr);
}

static WasmResult on_i64_const_expr(uint64_t value, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_const_expr(ctx->allocator);
  expr->const_.type = WASM_TYPE_I64;
  expr->const_.u64 = value;
  return append_expr(ctx, expr);
}

static WasmResult on_if_expr(uint32_t num_types,
                             WasmType* sig_types,
                             void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_if_expr(ctx->allocator);
  WasmTypeVector src;
  WASM_ZERO_MEMORY(src);
  src.size = num_types;
  src.data = sig_types;
  wasm_extend_types(ctx->allocator, &expr->if_.true_.sig, &src);
  append_expr(ctx, expr);
  push_label(ctx, LABEL_TYPE_IF, &expr->if_.true_.first);
  return WASM_OK;
}

static WasmResult on_load_expr(WasmOpcode opcode,
                               uint32_t alignment_log2,
                               uint32_t offset,
                               void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_load_expr(ctx->allocator);
  expr->load.opcode = opcode;
  expr->load.align = 1 << alignment_log2;
  expr->load.offset = offset;
  return append_expr(ctx, expr);
}

static WasmResult on_loop_expr(uint32_t num_types,
                               WasmType* sig_types,
                               void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_loop_expr(ctx->allocator);
  WasmTypeVector src;
  WASM_ZERO_MEMORY(src);
  src.size = num_types;
  src.data = sig_types;
  wasm_extend_types(ctx->allocator, &expr->loop.sig, &src);
  append_expr(ctx, expr);
  push_label(ctx, LABEL_TYPE_LOOP, &expr->loop.first);
  return WASM_OK;
}

static WasmResult on_nop_expr(void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_nop_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WasmResult on_return_expr(void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_return_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WasmResult on_select_expr(void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_select_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WasmResult on_set_global_expr(uint32_t global_index, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_set_global_expr(ctx->allocator);
  expr->set_global.var.type = WASM_VAR_TYPE_INDEX;
  expr->set_global.var.index = global_index;
  return append_expr(ctx, expr);
}

static WasmResult on_set_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_set_local_expr(ctx->allocator);
  expr->set_local.var.type = WASM_VAR_TYPE_INDEX;
  expr->set_local.var.index = local_index;
  return append_expr(ctx, expr);
}

static WasmResult on_store_expr(WasmOpcode opcode,
                                uint32_t alignment_log2,
                                uint32_t offset,
                                void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_store_expr(ctx->allocator);
  expr->store.opcode = opcode;
  expr->store.align = 1 << alignment_log2;
  expr->store.offset = offset;
  return append_expr(ctx, expr);
}

static WasmResult on_tee_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_tee_local_expr(ctx->allocator);
  expr->tee_local.var.type = WASM_VAR_TYPE_INDEX;
  expr->tee_local.var.index = local_index;
  return append_expr(ctx, expr);
}

static WasmResult on_unary_expr(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_unary_expr(ctx->allocator);
  expr->unary.opcode = opcode;
  return append_expr(ctx, expr);
}

static WasmResult on_unreachable_expr(void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_unreachable_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WasmResult end_function_body(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(pop_label(ctx));
  ctx->current_func = NULL;
  return WASM_OK;
}

static WasmResult on_elem_segment_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_reserve_elem_segment_ptrs(ctx->allocator, &ctx->module->elem_segments,
                                 count);
  return WASM_OK;
}

static WasmResult begin_elem_segment(uint32_t index,
                                     uint32_t table_index,
                                     void* user_data) {
  Context* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_ELEM_SEGMENT;

  WasmElemSegment* segment = &field->elem_segment;
  WASM_ZERO_MEMORY(*segment);
  segment->table_var.type = WASM_VAR_TYPE_INDEX;
  segment->table_var.index = table_index;

  assert(index == ctx->module->elem_segments.size);
  assert(index < ctx->module->elem_segments.capacity);
  WasmElemSegmentPtr* segment_ptr =
      wasm_append_elem_segment_ptr(ctx->allocator, &ctx->module->elem_segments);
  *segment_ptr = segment;
  return WASM_OK;
}

static WasmResult begin_elem_segment_init_expr(uint32_t index,
                                               void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->elem_segments.size - 1);
  WasmElemSegment* segment = ctx->module->elem_segments.data[index];
  ctx->current_init_expr = &segment->offset;
  return WASM_OK;
}

static WasmResult end_elem_segment_init_expr(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  ctx->current_init_expr = NULL;
  return WASM_OK;
}

static WasmResult on_elem_segment_function_index_count(uint32_t index,
                                                       uint32_t count,
                                                       void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->elem_segments.size - 1);
  WasmElemSegment* segment = ctx->module->elem_segments.data[index];
  wasm_reserve_vars(ctx->allocator, &segment->vars, count);
  return WASM_OK;
}

static WasmResult on_elem_segment_function_index(uint32_t index,
                                                 uint32_t func_index,
                                                 void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->elem_segments.size - 1);
  WasmElemSegment* segment = ctx->module->elem_segments.data[index];
  WasmVar* var = wasm_append_var(ctx->allocator, &segment->vars);
  var->type = WASM_VAR_TYPE_INDEX;
  var->index = func_index;
  return WASM_OK;
}

static WasmResult on_data_segment_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_reserve_data_segment_ptrs(ctx->allocator, &ctx->module->data_segments,
                                 count);
  return WASM_OK;
}

static WasmResult begin_data_segment(uint32_t index,
                                     uint32_t memory_index,
                                     void* user_data) {
  Context* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_DATA_SEGMENT;

  WasmDataSegment* segment = &field->data_segment;
  WASM_ZERO_MEMORY(*segment);
  segment->memory_var.type = WASM_VAR_TYPE_INDEX;
  segment->memory_var.index = memory_index;

  assert(index == ctx->module->data_segments.size);
  assert(index < ctx->module->data_segments.capacity);
  WasmDataSegmentPtr* segment_ptr =
      wasm_append_data_segment_ptr(ctx->allocator, &ctx->module->data_segments);
  *segment_ptr = segment;
  return WASM_OK;
}

static WasmResult begin_data_segment_init_expr(uint32_t index,
                                               void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->data_segments.size - 1);
  WasmDataSegment* segment = ctx->module->data_segments.data[index];
  ctx->current_init_expr = &segment->offset;
  return WASM_OK;
}

static WasmResult end_data_segment_init_expr(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  ctx->current_init_expr = NULL;
  return WASM_OK;
}

static WasmResult on_data_segment_data(uint32_t index,
                                       const void* data,
                                       uint32_t size,
                                       void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->data_segments.size - 1);
  WasmDataSegment* segment = ctx->module->data_segments.data[index];
  segment->data = wasm_alloc(ctx->allocator, size, WASM_DEFAULT_ALIGN);
  segment->size = size;
  memcpy(segment->data, data, size);
  return WASM_OK;
}

static WasmResult on_function_names_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  if (count > ctx->module->funcs.size) {
    print_error(
        ctx, "expected function name count (%u) <= function count (%" PRIzd ")",
        count, ctx->module->funcs.size);
    return WASM_ERROR;
  }
  return WASM_OK;
}

static WasmResult on_function_name(uint32_t index,
                                   WasmStringSlice name,
                                   void* user_data) {
  Context* ctx = user_data;

  WasmStringSlice new_name;
  dup_name(ctx, &name, &new_name);

  WasmBinding* binding = wasm_insert_binding(
      ctx->allocator, &ctx->module->func_bindings, &new_name);
  binding->index = index;

  WasmFunc* func = ctx->module->funcs.data[index];
  func->name = new_name;
  return WASM_OK;
}

static WasmResult on_local_names_count(uint32_t index,
                                       uint32_t count,
                                       void* user_data) {
  Context* ctx = user_data;
  WasmModule* module = ctx->module;
  assert(index < module->funcs.size);
  WasmFunc* func = module->funcs.data[index];
  uint32_t num_params_and_locals = wasm_get_num_params_and_locals(func);
  if (count > num_params_and_locals) {
    print_error(ctx, "expected local name count (%d) <= local count (%d)",
                count, num_params_and_locals);
    return WASM_ERROR;
  }
  return WASM_OK;
}

static WasmResult on_init_expr_f32_const_expr(uint32_t index,
                                              uint32_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_const_expr(ctx->allocator);
  expr->const_.type = WASM_TYPE_F32;
  expr->const_.f32_bits = value;
  *ctx->current_init_expr = expr;
  return WASM_OK;
}

static WasmResult on_init_expr_f64_const_expr(uint32_t index,
                                              uint64_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_const_expr(ctx->allocator);
  expr->const_.type = WASM_TYPE_F64;
  expr->const_.f64_bits = value;
  *ctx->current_init_expr = expr;
  return WASM_OK;
}

static WasmResult on_init_expr_get_global_expr(uint32_t index,
                                               uint32_t global_index,
                                               void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_get_global_expr(ctx->allocator);
  expr->get_global.var.type = WASM_VAR_TYPE_INDEX;
  expr->get_global.var.index = global_index;
  *ctx->current_init_expr = expr;
  return WASM_OK;
}

static WasmResult on_init_expr_i32_const_expr(uint32_t index,
                                              uint32_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_const_expr(ctx->allocator);
  expr->const_.type = WASM_TYPE_I32;
  expr->const_.u32 = value;
  *ctx->current_init_expr = expr;
  return WASM_OK;
}

static WasmResult on_init_expr_i64_const_expr(uint32_t index,
                                              uint64_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_const_expr(ctx->allocator);
  expr->const_.type = WASM_TYPE_I64;
  expr->const_.u64 = value;
  *ctx->current_init_expr = expr;
  return WASM_OK;
}

static WasmResult on_local_name(uint32_t func_index,
                                uint32_t local_index,
                                WasmStringSlice name,
                                void* user_data) {
  Context* ctx = user_data;
  WasmModule* module = ctx->module;
  WasmFunc* func = module->funcs.data[func_index];
  uint32_t num_params = wasm_get_num_params(func);
  WasmStringSlice new_name;
  dup_name(ctx, &name, &new_name);
  WasmBindingHash* bindings;
  WasmBinding* binding;
  uint32_t index;
  if (local_index < num_params) {
    /* param name */
    bindings = &func->param_bindings;
    index = local_index;
  } else {
    /* local name */
    bindings = &func->local_bindings;
    index = local_index - num_params;
  }
  binding = wasm_insert_binding(ctx->allocator, bindings, &new_name);
  binding->index = index;
  return WASM_OK;
}

static WasmBinaryReader s_binary_reader = {
    .user_data = NULL,
    .on_error = on_error,

    .on_signature_count = on_signature_count,
    .on_signature = on_signature,

    .on_import_count = on_import_count,
    .on_import = on_import,
    .on_import_func = on_import_func,
    .on_import_table = on_import_table,
    .on_import_memory = on_import_memory,
    .on_import_global = on_import_global,

    .on_function_signatures_count = on_function_signatures_count,
    .on_function_signature = on_function_signature,

    .on_table_count = on_table_count,
    .on_table = on_table,

    .on_memory_count = on_memory_count,
    .on_memory = on_memory,

    .on_global_count = on_global_count,
    .begin_global = begin_global,
    .begin_global_init_expr = begin_global_init_expr,
    .end_global_init_expr = end_global_init_expr,

    .on_export_count = on_export_count,
    .on_export = on_export,

    .on_start_function = on_start_function,

    .on_function_bodies_count = on_function_bodies_count,
    .begin_function_body = begin_function_body,
    .on_local_decl = on_local_decl,
    .on_binary_expr = on_binary_expr,
    .on_block_expr = on_block_expr,
    .on_br_expr = on_br_expr,
    .on_br_if_expr = on_br_if_expr,
    .on_br_table_expr = on_br_table_expr,
    .on_call_expr = on_call_expr,
    .on_call_indirect_expr = on_call_indirect_expr,
    .on_compare_expr = on_compare_expr,
    .on_convert_expr = on_convert_expr,
    .on_current_memory_expr = on_current_memory_expr,
    .on_drop_expr = on_drop_expr,
    .on_else_expr = on_else_expr,
    .on_end_expr = on_end_expr,
    .on_f32_const_expr = on_f32_const_expr,
    .on_f64_const_expr = on_f64_const_expr,
    .on_get_global_expr = on_get_global_expr,
    .on_get_local_expr = on_get_local_expr,
    .on_grow_memory_expr = on_grow_memory_expr,
    .on_i32_const_expr = on_i32_const_expr,
    .on_i64_const_expr = on_i64_const_expr,
    .on_if_expr = on_if_expr,
    .on_load_expr = on_load_expr,
    .on_loop_expr = on_loop_expr,
    .on_nop_expr = on_nop_expr,
    .on_return_expr = on_return_expr,
    .on_select_expr = on_select_expr,
    .on_set_global_expr = on_set_global_expr,
    .on_set_local_expr = on_set_local_expr,
    .on_store_expr = on_store_expr,
    .on_tee_local_expr = on_tee_local_expr,
    .on_unary_expr = on_unary_expr,
    .on_unreachable_expr = on_unreachable_expr,
    .end_function_body = end_function_body,

    .on_elem_segment_count = on_elem_segment_count,
    .begin_elem_segment = begin_elem_segment,
    .begin_elem_segment_init_expr = begin_elem_segment_init_expr,
    .end_elem_segment_init_expr = end_elem_segment_init_expr,
    .on_elem_segment_function_index_count =
        on_elem_segment_function_index_count,
    .on_elem_segment_function_index = on_elem_segment_function_index,

    .on_data_segment_count = on_data_segment_count,
    .begin_data_segment = begin_data_segment,
    .begin_data_segment_init_expr = begin_data_segment_init_expr,
    .end_data_segment_init_expr = end_data_segment_init_expr,
    .on_data_segment_data = on_data_segment_data,

    .on_function_names_count = on_function_names_count,
    .on_function_name = on_function_name,
    .on_local_names_count = on_local_names_count,
    .on_local_name = on_local_name,

    .on_init_expr_f32_const_expr = on_init_expr_f32_const_expr,
    .on_init_expr_f64_const_expr = on_init_expr_f64_const_expr,
    .on_init_expr_get_global_expr = on_init_expr_get_global_expr,
    .on_init_expr_i32_const_expr = on_init_expr_i32_const_expr,
    .on_init_expr_i64_const_expr = on_init_expr_i64_const_expr,
};

static void wasm_destroy_label_node(WasmAllocator* allocator, LabelNode* node) {
  if (*node->first)
    wasm_destroy_expr_list(allocator, *node->first);
}

WasmResult wasm_read_binary_ast(struct WasmAllocator* allocator,
                                const void* data,
                                size_t size,
                                const WasmReadBinaryOptions* options,
                                WasmBinaryErrorHandler* error_handler,
                                struct WasmModule* out_module) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.error_handler = error_handler;
  ctx.module = out_module;

  WasmBinaryReader reader;
  WASM_ZERO_MEMORY(reader);
  reader = s_binary_reader;
  reader.user_data = &ctx;

  WasmResult result =
      wasm_read_binary(allocator, data, size, &reader, 1, options);
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, ctx.label_stack, label_node);
  return result;
}

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

#include "binary-reader-ast.h"

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "allocator.h"
#include "ast.h"
#include "binary-reader.h"
#include "common.h"

#define CHECK_RESULT(expr) \
  do {                     \
    if (WABT_FAILED(expr)) \
      return WABT_ERROR;   \
  } while (0)

typedef struct LabelNode {
  WabtLabelType label_type;
  WabtExpr** first;
  WabtExpr* last;
} LabelNode;
WABT_DEFINE_VECTOR(label_node, LabelNode);

typedef struct Context {
  WabtAllocator* allocator;
  WabtBinaryErrorHandler* error_handler;
  WabtModule* module;

  WabtFunc* current_func;
  LabelNodeVector label_stack;
  uint32_t max_depth;
  WabtExpr** current_init_expr;
} Context;

static void handle_error(Context* ctx, uint32_t offset, const char* message);

static void WABT_PRINTF_FORMAT(2, 3)
    print_error(Context* ctx, const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  handle_error(ctx, WABT_UNKNOWN_OFFSET, buffer);
}

static void push_label(Context* ctx,
                       WabtLabelType label_type,
                       WabtExpr** first) {
  LabelNode label;
  label.label_type = label_type;
  label.first = first;
  label.last = NULL;
  ctx->max_depth++;
  wabt_append_label_node_value(ctx->allocator, &ctx->label_stack, &label);
}

static WabtResult pop_label(Context* ctx) {
  if (ctx->label_stack.size == 0) {
    print_error(ctx, "popping empty label stack");
    return WABT_ERROR;
  }

  ctx->max_depth--;
  ctx->label_stack.size--;
  return WABT_OK;
}

static WabtResult get_label_at(Context* ctx,
                               LabelNode** label,
                               uint32_t depth) {
  if (depth >= ctx->label_stack.size) {
    print_error(ctx, "accessing stack depth: %u >= max: %" PRIzd, depth,
                ctx->label_stack.size);
    return WABT_ERROR;
  }

  *label = &ctx->label_stack.data[ctx->label_stack.size - depth - 1];
  return WABT_OK;
}

static WabtResult top_label(Context* ctx, LabelNode** label) {
  return get_label_at(ctx, label, 0);
}

static void dup_name(Context* ctx,
                     WabtStringSlice* name,
                     WabtStringSlice* out_name) {
  if (name->length > 0) {
    *out_name = wabt_dup_string_slice(ctx->allocator, *name);
  } else {
    WABT_ZERO_MEMORY(*out_name);
  }
}

static WabtResult append_expr(Context* ctx, WabtExpr* expr) {
  LabelNode* label;
  if (WABT_FAILED(top_label(ctx, &label))) {
    wabt_free(ctx->allocator, expr);
    return WABT_ERROR;
  }
  if (*label->first) {
    label->last->next = expr;
    label->last = expr;
  } else {
    *label->first = label->last = expr;
  }
  return WABT_OK;
}

static void handle_error(Context* ctx, uint32_t offset, const char* message) {
  if (ctx->error_handler->on_error) {
    ctx->error_handler->on_error(offset, message,
                                 ctx->error_handler->user_data);
  }
}

static void on_error(WabtBinaryReaderContext* reader_context,
                     const char* message) {
  Context* ctx = reader_context->user_data;
  handle_error(ctx, reader_context->offset, message);
}

static WabtResult on_signature_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wabt_reserve_func_type_ptrs(ctx->allocator, &ctx->module->func_types, count);
  return WABT_OK;
}

static WabtResult on_signature(uint32_t index,
                               uint32_t param_count,
                               WabtType* param_types,
                               uint32_t result_count,
                               WabtType* result_types,
                               void* user_data) {
  Context* ctx = user_data;
  WabtModuleField* field =
      wabt_append_module_field(ctx->allocator, ctx->module);
  field->type = WABT_MODULE_FIELD_TYPE_FUNC_TYPE;

  WabtFuncType* func_type = &field->func_type;
  WABT_ZERO_MEMORY(*func_type);

  wabt_reserve_types(ctx->allocator, &func_type->sig.param_types, param_count);
  func_type->sig.param_types.size = param_count;
  memcpy(func_type->sig.param_types.data, param_types,
         param_count * sizeof(WabtType));

  wabt_reserve_types(ctx->allocator, &func_type->sig.result_types,
                     result_count);
  func_type->sig.result_types.size = result_count;
  memcpy(func_type->sig.result_types.data, result_types,
         result_count * sizeof(WabtType));

  assert(index < ctx->module->func_types.capacity);
  WabtFuncTypePtr* func_type_ptr =
      wabt_append_func_type_ptr(ctx->allocator, &ctx->module->func_types);
  *func_type_ptr = func_type;
  return WABT_OK;
}

static WabtResult on_import_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wabt_reserve_import_ptrs(ctx->allocator, &ctx->module->imports, count);
  return WABT_OK;
}

static WabtResult on_import(uint32_t index,
                            WabtStringSlice module_name,
                            WabtStringSlice field_name,
                            void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->imports.capacity);

  WabtModuleField* field =
      wabt_append_module_field(ctx->allocator, ctx->module);
  field->type = WABT_MODULE_FIELD_TYPE_IMPORT;

  WabtImport* import = &field->import;
  WABT_ZERO_MEMORY(*import);
  import->module_name = wabt_dup_string_slice(ctx->allocator, module_name);
  import->field_name = wabt_dup_string_slice(ctx->allocator, field_name);

  WabtImportPtr* import_ptr =
      wabt_append_import_ptr(ctx->allocator, &ctx->module->imports);
  *import_ptr = import;
  return WABT_OK;
}

static WabtResult on_import_func(uint32_t import_index,
                                 uint32_t func_index,
                                 uint32_t sig_index,
                                 void* user_data) {
  Context* ctx = user_data;
  assert(import_index == ctx->module->imports.size - 1);
  assert(sig_index < ctx->module->func_types.size);
  WabtImport* import = ctx->module->imports.data[import_index];

  import->kind = WABT_EXTERNAL_KIND_FUNC;
  import->func.decl.flags = WABT_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE |
                            WABT_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
  import->func.decl.type_var.type = WABT_VAR_TYPE_INDEX;
  import->func.decl.type_var.index = sig_index;
  import->func.decl.sig = ctx->module->func_types.data[sig_index]->sig;

  WabtFuncPtr func_ptr = &import->func;
  wabt_append_func_ptr_value(ctx->allocator, &ctx->module->funcs, &func_ptr);
  ctx->module->num_func_imports++;
  return WABT_OK;
}

static WabtResult on_import_table(uint32_t import_index,
                                  uint32_t table_index,
                                  WabtType elem_type,
                                  const WabtLimits* elem_limits,
                                  void* user_data) {
  Context* ctx = user_data;
  assert(import_index == ctx->module->imports.size - 1);
  WabtImport* import = ctx->module->imports.data[import_index];
  import->kind = WABT_EXTERNAL_KIND_TABLE;
  import->table.elem_limits = *elem_limits;

  WabtTablePtr table_ptr = &import->table;
  wabt_append_table_ptr_value(ctx->allocator, &ctx->module->tables, &table_ptr);
  ctx->module->num_table_imports++;
  return WABT_OK;
}

static WabtResult on_import_memory(uint32_t import_index,
                                   uint32_t memory_index,
                                   const WabtLimits* page_limits,
                                   void* user_data) {
  Context* ctx = user_data;
  assert(import_index == ctx->module->imports.size - 1);
  WabtImport* import = ctx->module->imports.data[import_index];
  import->kind = WABT_EXTERNAL_KIND_MEMORY;
  import->memory.page_limits = *page_limits;

  WabtMemoryPtr memory_ptr = &import->memory;
  wabt_append_memory_ptr_value(ctx->allocator, &ctx->module->memories,
                               &memory_ptr);
  ctx->module->num_memory_imports++;
  return WABT_OK;
}

static WabtResult on_import_global(uint32_t import_index,
                                   uint32_t global_index,
                                   WabtType type,
                                   WabtBool mutable_,
                                   void* user_data) {
  Context* ctx = user_data;
  assert(import_index == ctx->module->imports.size - 1);
  WabtImport* import = ctx->module->imports.data[import_index];
  import->kind = WABT_EXTERNAL_KIND_GLOBAL;
  import->global.type = type;
  import->global.mutable_ = mutable_;

  WabtGlobalPtr global_ptr = &import->global;
  wabt_append_global_ptr_value(ctx->allocator, &ctx->module->globals,
                               &global_ptr);
  ctx->module->num_global_imports++;
  return WABT_OK;
}

static WabtResult on_function_signatures_count(uint32_t count,
                                               void* user_data) {
  Context* ctx = user_data;
  wabt_reserve_func_ptrs(ctx->allocator, &ctx->module->funcs, count);
  return WABT_OK;
}

static WabtResult on_function_signature(uint32_t index,
                                        uint32_t sig_index,
                                        void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->funcs.capacity);
  assert(sig_index < ctx->module->func_types.size);

  WabtModuleField* field =
      wabt_append_module_field(ctx->allocator, ctx->module);
  field->type = WABT_MODULE_FIELD_TYPE_FUNC;

  WabtFunc* func = &field->func;
  WABT_ZERO_MEMORY(*func);
  func->decl.flags = WABT_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE |
                     WABT_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
  func->decl.type_var.type = WABT_VAR_TYPE_INDEX;
  func->decl.type_var.index = sig_index;
  func->decl.sig = ctx->module->func_types.data[sig_index]->sig;

  WabtFuncPtr* func_ptr =
      wabt_append_func_ptr(ctx->allocator, &ctx->module->funcs);
  *func_ptr = func;
  return WABT_OK;
}

static WabtResult on_table_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wabt_reserve_table_ptrs(ctx->allocator, &ctx->module->tables, count);
  return WABT_OK;
}

static WabtResult on_table(uint32_t index,
                           WabtType elem_type,
                           const WabtLimits* elem_limits,
                           void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->tables.capacity);

  WabtModuleField* field =
      wabt_append_module_field(ctx->allocator, ctx->module);
  field->type = WABT_MODULE_FIELD_TYPE_TABLE;

  WabtTable* table = &field->table;
  WABT_ZERO_MEMORY(*table);
  table->elem_limits = *elem_limits;

  WabtTablePtr* table_ptr =
      wabt_append_table_ptr(ctx->allocator, &ctx->module->tables);
  *table_ptr = table;
  return WABT_OK;
}

static WabtResult on_memory_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wabt_reserve_memory_ptrs(ctx->allocator, &ctx->module->memories, count);
  return WABT_OK;
}

static WabtResult on_memory(uint32_t index,
                            const WabtLimits* page_limits,
                            void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->memories.capacity);

  WabtModuleField* field =
      wabt_append_module_field(ctx->allocator, ctx->module);
  field->type = WABT_MODULE_FIELD_TYPE_MEMORY;

  WabtMemory* memory = &field->memory;
  WABT_ZERO_MEMORY(*memory);
  memory->page_limits = *page_limits;

  WabtMemoryPtr* memory_ptr =
      wabt_append_memory_ptr(ctx->allocator, &ctx->module->memories);
  *memory_ptr = memory;
  return WABT_OK;
}

static WabtResult on_global_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wabt_reserve_global_ptrs(ctx->allocator, &ctx->module->globals, count);
  return WABT_OK;
}

static WabtResult begin_global(uint32_t index,
                               WabtType type,
                               WabtBool mutable_,
                               void* user_data) {
  Context* ctx = user_data;
  assert(index - ctx->module->num_global_imports <
         ctx->module->globals.capacity);

  WabtModuleField* field =
      wabt_append_module_field(ctx->allocator, ctx->module);
  field->type = WABT_MODULE_FIELD_TYPE_GLOBAL;

  WabtGlobal* global = &field->global;
  WABT_ZERO_MEMORY(*global);
  global->type = type;
  global->mutable_ = mutable_;

  WabtGlobalPtr* global_ptr =
      wabt_append_global_ptr(ctx->allocator, &ctx->module->globals);
  *global_ptr = global;
  return WABT_OK;
}

static WabtResult begin_global_init_expr(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->globals.size - 1);
  WabtGlobal* global = ctx->module->globals.data[index];
  ctx->current_init_expr = &global->init_expr;
  return WABT_OK;
}

static WabtResult end_global_init_expr(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  ctx->current_init_expr = NULL;
  return WABT_OK;
}

static WabtResult on_export_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wabt_reserve_export_ptrs(ctx->allocator, &ctx->module->exports, count);
  return WABT_OK;
}

static WabtResult on_export(uint32_t index,
                            WabtExternalKind kind,
                            uint32_t item_index,
                            WabtStringSlice name,
                            void* user_data) {
  Context* ctx = user_data;
  WabtModuleField* field =
      wabt_append_module_field(ctx->allocator, ctx->module);
  field->type = WABT_MODULE_FIELD_TYPE_EXPORT;

  WabtExport* export = &field->export_;
  WABT_ZERO_MEMORY(*export);
  export->name = wabt_dup_string_slice(ctx->allocator, name);
  switch (kind) {
    case WABT_EXTERNAL_KIND_FUNC:
      assert(item_index < ctx->module->funcs.size);
      break;
    case WABT_EXTERNAL_KIND_TABLE:
      assert(item_index < ctx->module->tables.size);
      break;
    case WABT_EXTERNAL_KIND_MEMORY:
      assert(item_index < ctx->module->memories.size);
      break;
    case WABT_EXTERNAL_KIND_GLOBAL:
      assert(item_index < ctx->module->globals.size);
      break;
    case WABT_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
  export->var.type = WABT_VAR_TYPE_INDEX;
  export->var.index = item_index;
  export->kind = kind;

  assert(index < ctx->module->exports.capacity);
  WabtExportPtr* export_ptr =
      wabt_append_export_ptr(ctx->allocator, &ctx->module->exports);
  *export_ptr = export;
  return WABT_OK;
}

static WabtResult on_start_function(uint32_t func_index, void* user_data) {
  Context* ctx = user_data;
  WabtModuleField* field =
      wabt_append_module_field(ctx->allocator, ctx->module);
  field->type = WABT_MODULE_FIELD_TYPE_START;

  field->start.type = WABT_VAR_TYPE_INDEX;
  assert(func_index < ctx->module->funcs.size);
  field->start.index = func_index;

  ctx->module->start = &field->start;
  return WABT_OK;
}

static WabtResult on_function_bodies_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  assert(ctx->module->num_func_imports + count == ctx->module->funcs.size);
  (void)ctx;
  return WABT_OK;
}

static WabtResult begin_function_body(WabtBinaryReaderContext* context,
                                      uint32_t index) {
  Context* ctx = context->user_data;
  assert(index < ctx->module->funcs.size);
  ctx->current_func = ctx->module->funcs.data[index];
  push_label(ctx, WABT_LABEL_TYPE_FUNC, &ctx->current_func->first_expr);
  return WABT_OK;
}

static WabtResult on_local_decl(uint32_t decl_index,
                                uint32_t count,
                                WabtType type,
                                void* user_data) {
  Context* ctx = user_data;
  size_t old_local_count = ctx->current_func->local_types.size;
  size_t new_local_count = old_local_count + count;
  wabt_reserve_types(ctx->allocator, &ctx->current_func->local_types,
                     new_local_count);
  WabtTypeVector* types = &ctx->current_func->local_types;
  size_t i;
  for (i = 0; i < count; ++i)
    types->data[old_local_count + i] = type;
  types->size = new_local_count;
  return WABT_OK;
}

static WabtResult on_binary_expr(WabtOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_binary_expr(ctx->allocator);
  expr->binary.opcode = opcode;
  return append_expr(ctx, expr);
}

static WabtResult on_block_expr(uint32_t num_types,
                                WabtType* sig_types,
                                void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_block_expr(ctx->allocator);
  WabtTypeVector src;
  WABT_ZERO_MEMORY(src);
  src.size = num_types;
  src.data = sig_types;
  wabt_extend_types(ctx->allocator, &expr->block.sig, &src);
  append_expr(ctx, expr);
  push_label(ctx, WABT_LABEL_TYPE_BLOCK, &expr->block.first);
  return WABT_OK;
}

static WabtResult on_br_expr(uint32_t depth, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_br_expr(ctx->allocator);
  expr->br.var.type = WABT_VAR_TYPE_INDEX;
  expr->br.var.index = depth;
  return append_expr(ctx, expr);
}

static WabtResult on_br_if_expr(uint32_t depth, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_br_if_expr(ctx->allocator);
  expr->br_if.var.type = WABT_VAR_TYPE_INDEX;
  expr->br_if.var.index = depth;
  return append_expr(ctx, expr);
}

static WabtResult on_br_table_expr(WabtBinaryReaderContext* context,
                                   uint32_t num_targets,
                                   uint32_t* target_depths,
                                   uint32_t default_target_depth) {
  Context* ctx = context->user_data;
  WabtExpr* expr = wabt_new_br_table_expr(ctx->allocator);
  wabt_reserve_vars(ctx->allocator, &expr->br_table.targets, num_targets);
  expr->br_table.targets.size = num_targets;
  uint32_t i;
  for (i = 0; i < num_targets; ++i) {
    WabtVar* var = &expr->br_table.targets.data[i];
    var->type = WABT_VAR_TYPE_INDEX;
    var->index = target_depths[i];
  }
  expr->br_table.default_target.type = WABT_VAR_TYPE_INDEX;
  expr->br_table.default_target.index = default_target_depth;
  return append_expr(ctx, expr);
}

static WabtResult on_call_expr(uint32_t func_index, void* user_data) {
  Context* ctx = user_data;
  assert(func_index < ctx->module->funcs.size);
  WabtExpr* expr = wabt_new_call_expr(ctx->allocator);
  expr->call.var.type = WABT_VAR_TYPE_INDEX;
  expr->call.var.index = func_index;
  return append_expr(ctx, expr);
}

static WabtResult on_call_indirect_expr(uint32_t sig_index, void* user_data) {
  Context* ctx = user_data;
  assert(sig_index < ctx->module->func_types.size);
  WabtExpr* expr = wabt_new_call_indirect_expr(ctx->allocator);
  expr->call_indirect.var.type = WABT_VAR_TYPE_INDEX;
  expr->call_indirect.var.index = sig_index;
  return append_expr(ctx, expr);
}

static WabtResult on_compare_expr(WabtOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_compare_expr(ctx->allocator);
  expr->compare.opcode = opcode;
  return append_expr(ctx, expr);
}

static WabtResult on_convert_expr(WabtOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_convert_expr(ctx->allocator);
  expr->convert.opcode = opcode;
  return append_expr(ctx, expr);
}

static WabtResult on_current_memory_expr(void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_current_memory_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WabtResult on_drop_expr(void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_drop_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WabtResult on_else_expr(void* user_data) {
  Context* ctx = user_data;
  LabelNode* label;
  CHECK_RESULT(top_label(ctx, &label));
  if (label->label_type != WABT_LABEL_TYPE_IF) {
    print_error(ctx, "else expression without matching if");
    return WABT_ERROR;
  }

  LabelNode* parent_label;
  CHECK_RESULT(get_label_at(ctx, &parent_label, 1));
  assert(parent_label->last->type == WABT_EXPR_TYPE_IF);

  label->label_type = WABT_LABEL_TYPE_ELSE;
  label->first = &parent_label->last->if_.false_;
  label->last = NULL;
  return WABT_OK;
}

static WabtResult on_end_expr(void* user_data) {
  Context* ctx = user_data;
  return pop_label(ctx);
}

static WabtResult on_f32_const_expr(uint32_t value_bits, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_const_expr(ctx->allocator);
  expr->const_.type = WABT_TYPE_F32;
  expr->const_.f32_bits = value_bits;
  return append_expr(ctx, expr);
}

static WabtResult on_f64_const_expr(uint64_t value_bits, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_const_expr(ctx->allocator);
  expr->const_.type = WABT_TYPE_F64;
  expr->const_.f64_bits = value_bits;
  return append_expr(ctx, expr);
}

static WabtResult on_get_global_expr(uint32_t global_index, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_get_global_expr(ctx->allocator);
  expr->get_global.var.type = WABT_VAR_TYPE_INDEX;
  expr->get_global.var.index = global_index;
  return append_expr(ctx, expr);
}

static WabtResult on_get_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_get_local_expr(ctx->allocator);
  expr->get_local.var.type = WABT_VAR_TYPE_INDEX;
  expr->get_local.var.index = local_index;
  return append_expr(ctx, expr);
}

static WabtResult on_grow_memory_expr(void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_grow_memory_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WabtResult on_i32_const_expr(uint32_t value, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_const_expr(ctx->allocator);
  expr->const_.type = WABT_TYPE_I32;
  expr->const_.u32 = value;
  return append_expr(ctx, expr);
}

static WabtResult on_i64_const_expr(uint64_t value, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_const_expr(ctx->allocator);
  expr->const_.type = WABT_TYPE_I64;
  expr->const_.u64 = value;
  return append_expr(ctx, expr);
}

static WabtResult on_if_expr(uint32_t num_types,
                             WabtType* sig_types,
                             void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_if_expr(ctx->allocator);
  WabtTypeVector src;
  WABT_ZERO_MEMORY(src);
  src.size = num_types;
  src.data = sig_types;
  wabt_extend_types(ctx->allocator, &expr->if_.true_.sig, &src);
  append_expr(ctx, expr);
  push_label(ctx, WABT_LABEL_TYPE_IF, &expr->if_.true_.first);
  return WABT_OK;
}

static WabtResult on_load_expr(WabtOpcode opcode,
                               uint32_t alignment_log2,
                               uint32_t offset,
                               void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_load_expr(ctx->allocator);
  expr->load.opcode = opcode;
  expr->load.align = 1 << alignment_log2;
  expr->load.offset = offset;
  return append_expr(ctx, expr);
}

static WabtResult on_loop_expr(uint32_t num_types,
                               WabtType* sig_types,
                               void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_loop_expr(ctx->allocator);
  WabtTypeVector src;
  WABT_ZERO_MEMORY(src);
  src.size = num_types;
  src.data = sig_types;
  wabt_extend_types(ctx->allocator, &expr->loop.sig, &src);
  append_expr(ctx, expr);
  push_label(ctx, WABT_LABEL_TYPE_LOOP, &expr->loop.first);
  return WABT_OK;
}

static WabtResult on_nop_expr(void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_nop_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WabtResult on_return_expr(void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_return_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WabtResult on_select_expr(void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_select_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WabtResult on_set_global_expr(uint32_t global_index, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_set_global_expr(ctx->allocator);
  expr->set_global.var.type = WABT_VAR_TYPE_INDEX;
  expr->set_global.var.index = global_index;
  return append_expr(ctx, expr);
}

static WabtResult on_set_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_set_local_expr(ctx->allocator);
  expr->set_local.var.type = WABT_VAR_TYPE_INDEX;
  expr->set_local.var.index = local_index;
  return append_expr(ctx, expr);
}

static WabtResult on_store_expr(WabtOpcode opcode,
                                uint32_t alignment_log2,
                                uint32_t offset,
                                void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_store_expr(ctx->allocator);
  expr->store.opcode = opcode;
  expr->store.align = 1 << alignment_log2;
  expr->store.offset = offset;
  return append_expr(ctx, expr);
}

static WabtResult on_tee_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_tee_local_expr(ctx->allocator);
  expr->tee_local.var.type = WABT_VAR_TYPE_INDEX;
  expr->tee_local.var.index = local_index;
  return append_expr(ctx, expr);
}

static WabtResult on_unary_expr(WabtOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_unary_expr(ctx->allocator);
  expr->unary.opcode = opcode;
  return append_expr(ctx, expr);
}

static WabtResult on_unreachable_expr(void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_unreachable_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WabtResult end_function_body(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(pop_label(ctx));
  ctx->current_func = NULL;
  return WABT_OK;
}

static WabtResult on_elem_segment_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wabt_reserve_elem_segment_ptrs(ctx->allocator, &ctx->module->elem_segments,
                                 count);
  return WABT_OK;
}

static WabtResult begin_elem_segment(uint32_t index,
                                     uint32_t table_index,
                                     void* user_data) {
  Context* ctx = user_data;
  WabtModuleField* field =
      wabt_append_module_field(ctx->allocator, ctx->module);
  field->type = WABT_MODULE_FIELD_TYPE_ELEM_SEGMENT;

  WabtElemSegment* segment = &field->elem_segment;
  WABT_ZERO_MEMORY(*segment);
  segment->table_var.type = WABT_VAR_TYPE_INDEX;
  segment->table_var.index = table_index;

  assert(index == ctx->module->elem_segments.size);
  assert(index < ctx->module->elem_segments.capacity);
  WabtElemSegmentPtr* segment_ptr =
      wabt_append_elem_segment_ptr(ctx->allocator, &ctx->module->elem_segments);
  *segment_ptr = segment;
  return WABT_OK;
}

static WabtResult begin_elem_segment_init_expr(uint32_t index,
                                               void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->elem_segments.size - 1);
  WabtElemSegment* segment = ctx->module->elem_segments.data[index];
  ctx->current_init_expr = &segment->offset;
  return WABT_OK;
}

static WabtResult end_elem_segment_init_expr(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  ctx->current_init_expr = NULL;
  return WABT_OK;
}

static WabtResult on_elem_segment_function_index_count(
    WabtBinaryReaderContext* context,
    uint32_t index,
    uint32_t count) {
  Context* ctx = context->user_data;
  assert(index == ctx->module->elem_segments.size - 1);
  WabtElemSegment* segment = ctx->module->elem_segments.data[index];
  wabt_reserve_vars(ctx->allocator, &segment->vars, count);
  return WABT_OK;
}

static WabtResult on_elem_segment_function_index(uint32_t index,
                                                 uint32_t func_index,
                                                 void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->elem_segments.size - 1);
  WabtElemSegment* segment = ctx->module->elem_segments.data[index];
  WabtVar* var = wabt_append_var(ctx->allocator, &segment->vars);
  var->type = WABT_VAR_TYPE_INDEX;
  var->index = func_index;
  return WABT_OK;
}

static WabtResult on_data_segment_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wabt_reserve_data_segment_ptrs(ctx->allocator, &ctx->module->data_segments,
                                 count);
  return WABT_OK;
}

static WabtResult begin_data_segment(uint32_t index,
                                     uint32_t memory_index,
                                     void* user_data) {
  Context* ctx = user_data;
  WabtModuleField* field =
      wabt_append_module_field(ctx->allocator, ctx->module);
  field->type = WABT_MODULE_FIELD_TYPE_DATA_SEGMENT;

  WabtDataSegment* segment = &field->data_segment;
  WABT_ZERO_MEMORY(*segment);
  segment->memory_var.type = WABT_VAR_TYPE_INDEX;
  segment->memory_var.index = memory_index;

  assert(index == ctx->module->data_segments.size);
  assert(index < ctx->module->data_segments.capacity);
  WabtDataSegmentPtr* segment_ptr =
      wabt_append_data_segment_ptr(ctx->allocator, &ctx->module->data_segments);
  *segment_ptr = segment;
  return WABT_OK;
}

static WabtResult begin_data_segment_init_expr(uint32_t index,
                                               void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->data_segments.size - 1);
  WabtDataSegment* segment = ctx->module->data_segments.data[index];
  ctx->current_init_expr = &segment->offset;
  return WABT_OK;
}

static WabtResult end_data_segment_init_expr(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  ctx->current_init_expr = NULL;
  return WABT_OK;
}

static WabtResult on_data_segment_data(uint32_t index,
                                       const void* data,
                                       uint32_t size,
                                       void* user_data) {
  Context* ctx = user_data;
  assert(index == ctx->module->data_segments.size - 1);
  WabtDataSegment* segment = ctx->module->data_segments.data[index];
  segment->data = wabt_alloc(ctx->allocator, size, WABT_DEFAULT_ALIGN);
  segment->size = size;
  memcpy(segment->data, data, size);
  return WABT_OK;
}

static WabtResult on_function_names_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  if (count > ctx->module->funcs.size) {
    print_error(
        ctx, "expected function name count (%u) <= function count (%" PRIzd ")",
        count, ctx->module->funcs.size);
    return WABT_ERROR;
  }
  return WABT_OK;
}

static WabtResult on_function_name(uint32_t index,
                                   WabtStringSlice name,
                                   void* user_data) {
  Context* ctx = user_data;

  WabtStringSlice new_name;
  dup_name(ctx, &name, &new_name);

  WabtBinding* binding = wabt_insert_binding(
      ctx->allocator, &ctx->module->func_bindings, &new_name);
  binding->index = index;

  WabtFunc* func = ctx->module->funcs.data[index];
  func->name = new_name;
  return WABT_OK;
}

static WabtResult on_local_names_count(uint32_t index,
                                       uint32_t count,
                                       void* user_data) {
  Context* ctx = user_data;
  WabtModule* module = ctx->module;
  assert(index < module->funcs.size);
  WabtFunc* func = module->funcs.data[index];
  uint32_t num_params_and_locals = wabt_get_num_params_and_locals(func);
  if (count > num_params_and_locals) {
    print_error(ctx, "expected local name count (%d) <= local count (%d)",
                count, num_params_and_locals);
    return WABT_ERROR;
  }
  return WABT_OK;
}

static WabtResult on_init_expr_f32_const_expr(uint32_t index,
                                              uint32_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_const_expr(ctx->allocator);
  expr->const_.type = WABT_TYPE_F32;
  expr->const_.f32_bits = value;
  *ctx->current_init_expr = expr;
  return WABT_OK;
}

static WabtResult on_init_expr_f64_const_expr(uint32_t index,
                                              uint64_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_const_expr(ctx->allocator);
  expr->const_.type = WABT_TYPE_F64;
  expr->const_.f64_bits = value;
  *ctx->current_init_expr = expr;
  return WABT_OK;
}

static WabtResult on_init_expr_get_global_expr(uint32_t index,
                                               uint32_t global_index,
                                               void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_get_global_expr(ctx->allocator);
  expr->get_global.var.type = WABT_VAR_TYPE_INDEX;
  expr->get_global.var.index = global_index;
  *ctx->current_init_expr = expr;
  return WABT_OK;
}

static WabtResult on_init_expr_i32_const_expr(uint32_t index,
                                              uint32_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_const_expr(ctx->allocator);
  expr->const_.type = WABT_TYPE_I32;
  expr->const_.u32 = value;
  *ctx->current_init_expr = expr;
  return WABT_OK;
}

static WabtResult on_init_expr_i64_const_expr(uint32_t index,
                                              uint64_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  WabtExpr* expr = wabt_new_const_expr(ctx->allocator);
  expr->const_.type = WABT_TYPE_I64;
  expr->const_.u64 = value;
  *ctx->current_init_expr = expr;
  return WABT_OK;
}

static WabtResult on_local_name(uint32_t func_index,
                                uint32_t local_index,
                                WabtStringSlice name,
                                void* user_data) {
  Context* ctx = user_data;
  WabtModule* module = ctx->module;
  WabtFunc* func = module->funcs.data[func_index];
  uint32_t num_params = wabt_get_num_params(func);
  WabtStringSlice new_name;
  dup_name(ctx, &name, &new_name);
  WabtBindingHash* bindings;
  WabtBinding* binding;
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
  binding = wabt_insert_binding(ctx->allocator, bindings, &new_name);
  binding->index = index;
  return WABT_OK;
}

static WabtBinaryReader s_binary_reader = {
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

static void wabt_destroy_label_node(WabtAllocator* allocator, LabelNode* node) {
  if (*node->first)
    wabt_destroy_expr_list(allocator, *node->first);
}

WabtResult wabt_read_binary_ast(struct WabtAllocator* allocator,
                                const void* data,
                                size_t size,
                                const WabtReadBinaryOptions* options,
                                WabtBinaryErrorHandler* error_handler,
                                struct WabtModule* out_module) {
  Context ctx;
  WABT_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.error_handler = error_handler;
  ctx.module = out_module;

  WabtBinaryReader reader;
  WABT_ZERO_MEMORY(reader);
  reader = s_binary_reader;
  reader.user_data = &ctx;

  WabtResult result =
      wabt_read_binary(allocator, data, size, &reader, 1, options);
  WABT_DESTROY_VECTOR_AND_ELEMENTS(allocator, ctx.label_stack, label_node);
  if (WABT_FAILED(result))
    wabt_destroy_module(allocator, out_module);
  return result;
}

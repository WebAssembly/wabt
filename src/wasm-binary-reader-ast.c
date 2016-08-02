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

#if LOG
#define V(rtype, type1, type2, mem_size, code, NAME, text) [code] = text,
static const char* s_opcode_name[] = {WASM_FOREACH_OPCODE(V)};
#undef V
#endif

typedef struct LabelNode {
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
} Context;

static void on_error(uint32_t offset, const char* message, void* user_data);

static void WASM_PRINTF_FORMAT(2, 3)
    print_error(Context* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  on_error(WASM_UNKNOWN_OFFSET, buffer, ctx);
}

static void push_label(Context* ctx, WasmExpr** first) {
  LabelNode label;
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

static WasmResult top_label(Context* ctx, LabelNode** label) {
  if (ctx->label_stack.size == 0) {
    print_error(ctx, "accessing empty label stack");
    return WASM_ERROR;
  }

  *label = &ctx->label_stack.data[ctx->label_stack.size - 1];
  return WASM_OK;
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

static void on_error(uint32_t offset, const char* message, void* user_data) {
  Context* ctx = user_data;
  if (ctx->error_handler->on_error) {
    ctx->error_handler->on_error(offset, message,
                                 ctx->error_handler->user_data);
  }
}

static WasmResult begin_memory_section(void* user_data) {
  Context* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_MEMORY;
  assert(ctx->module->memory == NULL);
  ctx->module->memory = &field->memory;
  return WASM_OK;
}

static WasmResult on_memory_initial_size_pages(uint32_t pages,
                                               void* user_data) {
  Context* ctx = user_data;
  ctx->module->memory->initial_pages = pages;
  return WASM_OK;
}

static WasmResult on_memory_max_size_pages(uint32_t pages, void* user_data) {
  Context* ctx = user_data;
  ctx->module->memory->max_pages = pages;
  return WASM_OK;
}

static WasmResult on_memory_exported(WasmBool exported, void* user_data) {
  Context* ctx = user_data;
  if (exported) {
    WasmModuleField* field =
        wasm_append_module_field(ctx->allocator, ctx->module);
    field->type = WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY;
    /* TODO(binji): refactor */
    field->export_memory.name.start = wasm_strndup(ctx->allocator, "memory", 6);
    field->export_memory.name.length = 6;
    assert(ctx->module->export_memory == NULL);
    ctx->module->export_memory = &field->export_memory;
  }
  return WASM_OK;
}

static WasmResult on_data_segment_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_reserve_segments(ctx->allocator, &ctx->module->memory->segments, count);
  return WASM_OK;
}

static WasmResult on_data_segment(uint32_t index,
                                  uint32_t address,
                                  const void* data,
                                  uint32_t size,
                                  void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->memory->segments.capacity);
  WasmSegment* segment =
      wasm_append_segment(ctx->allocator, &ctx->module->memory->segments);
  segment->addr = address;
  segment->data = wasm_alloc(ctx->allocator, size, WASM_DEFAULT_ALIGN);
  segment->size = size;
  memcpy(segment->data, data, size);
  return WASM_OK;
}

static WasmResult on_signature_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_reserve_func_type_ptrs(ctx->allocator, &ctx->module->func_types, count);
  return WASM_OK;
}

static WasmResult on_signature(uint32_t index,
                               WasmType result_type,
                               uint32_t param_count,
                               WasmType* param_types,
                               void* user_data) {
  Context* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;

  WasmFuncType* func_type = &field->func_type;
  WASM_ZERO_MEMORY(*func_type);
  func_type->sig.result_type = result_type;

  wasm_reserve_types(ctx->allocator, &func_type->sig.param_types, param_count);
  func_type->sig.param_types.size = param_count;
  memcpy(func_type->sig.param_types.data, param_types,
         param_count * sizeof(WasmType));

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
                            uint32_t sig_index,
                            WasmStringSlice module_name,
                            WasmStringSlice function_name,
                            void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->imports.capacity);
  assert(sig_index < ctx->module->func_types.size);

  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_IMPORT;

  WasmImport* import = &field->import;
  WASM_ZERO_MEMORY(*import);
  import->module_name = wasm_dup_string_slice(ctx->allocator, module_name);
  import->func_name = wasm_dup_string_slice(ctx->allocator, function_name);
  import->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE |
                       WASM_FUNC_DECLARATION_FLAG_SHARED_SIGNATURE;
  import->decl.type_var.type = WASM_VAR_TYPE_INDEX;
  import->decl.type_var.index = sig_index;
  import->decl.sig = ctx->module->func_types.data[sig_index]->sig;

  WasmImportPtr* import_ptr =
      wasm_append_import_ptr(ctx->allocator, &ctx->module->imports);
  *import_ptr = import;
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

static WasmResult on_function_bodies_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  assert(count == ctx->module->funcs.size);
  (void)ctx;
  return WASM_OK;
}

static WasmResult begin_function_body(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->funcs.size);
  ctx->current_func = ctx->module->funcs.data[index];
  push_label(ctx, &ctx->current_func->first_expr);
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

static WasmResult on_block_expr(void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_block_expr(ctx->allocator);
  append_expr(ctx, expr);
  push_label(ctx, &expr->block.first);
  return WASM_OK;
}

static WasmResult on_br_expr(uint8_t arity, uint32_t depth, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_br_expr(ctx->allocator);
  expr->br.var.type = WASM_VAR_TYPE_INDEX;
  expr->br.arity = arity;
  expr->br.var.index = depth;
  return append_expr(ctx, expr);
}

static WasmResult on_br_if_expr(uint8_t arity,
                                uint32_t depth,
                                void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_br_if_expr(ctx->allocator);
  expr->br_if.arity = arity;
  expr->br_if.var.type = WASM_VAR_TYPE_INDEX;
  expr->br_if.var.index = depth;
  return append_expr(ctx, expr);
}

static WasmResult on_br_table_expr(uint8_t arity,
                                   uint32_t num_targets,
                                   uint32_t* target_depths,
                                   uint32_t default_target_depth,
                                   void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_br_table_expr(ctx->allocator);
  wasm_reserve_vars(ctx->allocator, &expr->br_table.targets, num_targets);
  expr->br_table.arity = arity;
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

static WasmResult on_call_expr(uint32_t arity,
                               uint32_t func_index,
                               void* user_data) {
  Context* ctx = user_data;
  assert(func_index < ctx->module->funcs.size);
  WasmExpr* expr = wasm_new_call_expr(ctx->allocator);
  expr->call.var.type = WASM_VAR_TYPE_INDEX;
  expr->call.var.index = func_index;
  return append_expr(ctx, expr);
}

static WasmResult on_call_import_expr(uint32_t arity,
                                      uint32_t import_index,
                                      void* user_data) {
  Context* ctx = user_data;
  assert(import_index < ctx->module->imports.size);
  WasmExpr* expr = wasm_new_call_import_expr(ctx->allocator);
  expr->call.var.type = WASM_VAR_TYPE_INDEX;
  expr->call.var.index = import_index;
  return append_expr(ctx, expr);
}

static WasmResult on_call_indirect_expr(uint32_t arity,
                                        uint32_t sig_index,
                                        void* user_data) {
  Context* ctx = user_data;
  assert(sig_index < ctx->module->func_types.size);
  WasmExpr* expr = wasm_new_call_indirect_expr(ctx->allocator);
  expr->call_indirect.var.type = WASM_VAR_TYPE_INDEX;
  expr->call_indirect.var.index = sig_index;
  return append_expr(ctx, expr);
}

static WasmResult on_compare_expr(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  WasmExpr *expr = wasm_new_compare_expr(ctx->allocator);
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
  /* destroy the if expr, replace it with an if_else */
  /* TODO(binji): remove if_else and just have false branch be an empty block
   * for if without else? */
  Context* ctx = user_data;
  CHECK_RESULT(pop_label(ctx));
  LabelNode* label;
  CHECK_RESULT(top_label(ctx, &label));
  WasmExpr* if_expr = label->last;
  if (if_expr->type != WASM_EXPR_TYPE_IF) {
    print_error(ctx, "else expression without matching if");
    return WASM_ERROR;
  }

  WasmExpr* if_else_expr = wasm_new_if_else_expr(ctx->allocator);
  if_else_expr->if_else.true_.first = if_expr->if_.true_.first;
  wasm_free(ctx->allocator, if_expr);
  push_label(ctx, &if_else_expr->if_else.false_.first);
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

static WasmResult on_if_expr(void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_if_expr(ctx->allocator);
  append_expr(ctx, expr);
  push_label(ctx, &expr->if_.true_.first);
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

static WasmResult on_loop_expr(void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_loop_expr(ctx->allocator);
  append_expr(ctx, expr);
  push_label(ctx, &expr->loop.first);
  return WASM_OK;
}

static WasmResult on_nop_expr(void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_nop_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WasmResult on_return_expr(uint8_t arity, void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_return_expr(ctx->allocator);
  return append_expr(ctx, expr);
}

static WasmResult on_select_expr(void* user_data) {
  Context* ctx = user_data;
  WasmExpr* expr = wasm_new_select_expr(ctx->allocator);
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

static WasmResult on_function_table_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_TABLE;

  assert(ctx->module->table == NULL);
  ctx->module->table = &field->table;

  wasm_reserve_vars(ctx->allocator, &field->table, count);
  return WASM_OK;
}

static WasmResult on_function_table_entry(uint32_t index,
                                          uint32_t func_index,
                                          void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->table->capacity);
  WasmVar* func_var = wasm_append_var(ctx->allocator, ctx->module->table);
  func_var->type = WASM_VAR_TYPE_INDEX;
  assert(func_index < ctx->module->funcs.size);
  func_var->index = func_index;
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

static WasmResult on_export_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_reserve_export_ptrs(ctx->allocator, &ctx->module->exports, count);
  return WASM_OK;
}

static WasmResult on_export(uint32_t index,
                            uint32_t func_index,
                            WasmStringSlice name,
                            void* user_data) {
  Context* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  field->type = WASM_MODULE_FIELD_TYPE_EXPORT;

  WasmExport* export = &field->export_;
  WASM_ZERO_MEMORY(*export);
  export->name = wasm_dup_string_slice(ctx->allocator, name);
  export->var.type = WASM_VAR_TYPE_INDEX;
  assert(func_index < ctx->module->funcs.size);
  export->var.index = func_index;

  assert(index < ctx->module->exports.capacity);
  WasmExportPtr* export_ptr =
      wasm_append_export_ptr(ctx->allocator, &ctx->module->exports);
  *export_ptr = export;
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
  uint32_t num_params_and_locals = wasm_get_num_params_and_locals(module, func);
  if (count > num_params_and_locals) {
    print_error(ctx, "expected local name count (%d) <= local count (%d)",
                count, num_params_and_locals);
    return WASM_ERROR;
  }
  return WASM_OK;
}

static WasmResult on_local_name(uint32_t func_index,
                                uint32_t local_index,
                                WasmStringSlice name,
                                void* user_data) {
  Context* ctx = user_data;
  WasmModule* module = ctx->module;
  WasmFunc* func = module->funcs.data[func_index];
  uint32_t num_params = wasm_get_num_params(module, func);
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
    .on_error = &on_error,

    .begin_memory_section = &begin_memory_section,
    .on_memory_initial_size_pages = &on_memory_initial_size_pages,
    .on_memory_max_size_pages = &on_memory_max_size_pages,
    .on_memory_exported = &on_memory_exported,

    .on_data_segment_count = &on_data_segment_count,
    .on_data_segment = &on_data_segment,

    .on_signature_count = &on_signature_count,
    .on_signature = &on_signature,

    .on_import_count = &on_import_count,
    .on_import = &on_import,

    .on_function_signatures_count = &on_function_signatures_count,
    .on_function_signature = &on_function_signature,

    .on_function_bodies_count = &on_function_bodies_count,
    .begin_function_body = &begin_function_body,
    .on_local_decl = &on_local_decl,
    .on_binary_expr = &on_binary_expr,
    .on_block_expr = &on_block_expr,
    .on_br_expr = &on_br_expr,
    .on_br_if_expr = &on_br_if_expr,
    .on_br_table_expr = &on_br_table_expr,
    .on_call_expr = &on_call_expr,
    .on_call_import_expr = &on_call_import_expr,
    .on_call_indirect_expr = &on_call_indirect_expr,
    .on_compare_expr = &on_compare_expr,
    .on_convert_expr = &on_convert_expr,
    .on_current_memory_expr = &on_current_memory_expr,
    .on_drop_expr = &on_drop_expr,
    .on_else_expr = &on_else_expr,
    .on_end_expr = &on_end_expr,
    .on_f32_const_expr = &on_f32_const_expr,
    .on_f64_const_expr = &on_f64_const_expr,
    .on_get_local_expr = &on_get_local_expr,
    .on_grow_memory_expr = &on_grow_memory_expr,
    .on_i32_const_expr = &on_i32_const_expr,
    .on_i64_const_expr = &on_i64_const_expr,
    .on_if_expr = &on_if_expr,
    .on_load_expr = &on_load_expr,
    .on_loop_expr = &on_loop_expr,
    .on_nop_expr = &on_nop_expr,
    .on_return_expr = &on_return_expr,
    .on_select_expr = &on_select_expr,
    .on_set_local_expr = &on_set_local_expr,
    .on_store_expr = &on_store_expr,
    .on_tee_local_expr = &on_tee_local_expr,
    .on_unary_expr = &on_unary_expr,
    .on_unreachable_expr = &on_unreachable_expr,
    .end_function_body = &end_function_body,

    .on_function_table_count = &on_function_table_count,
    .on_function_table_entry = &on_function_table_entry,

    .on_start_function = &on_start_function,

    .on_export_count = &on_export_count,
    .on_export = &on_export,

    .on_function_names_count = &on_function_names_count,
    .on_function_name = &on_function_name,
    .on_local_names_count = &on_local_names_count,
    .on_local_name = &on_local_name,
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

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
#include <stdint.h>
#include <stdio.h>

#include "wasm-allocator.h"
#include "wasm-ast.h"
#include "wasm-binary-reader.h"
#include "wasm-common.h"

/* TODO(binji): use on_error instead of fprintf */
#define CHECK_ALLOC_(cond)                                               \
  do {                                                                   \
    if (!(cond)) {                                                       \
      fprintf(stderr, "%s:%d: allocation failed\n", __FILE__, __LINE__); \
      return WASM_ERROR;                                                 \
    }                                                                    \
  } while (0)

#define CHECK_ALLOC(e) CHECK_ALLOC_((e) == WASM_OK)
#define CHECK_ALLOC_NULL(v) CHECK_ALLOC_(v)
#define CHECK_ALLOC_NULL_STR(v) CHECK_ALLOC_((v).start)

typedef struct WasmExprNode {
  WasmExpr* expr;
  uint32_t index;
  uint32_t total;
} WasmExprNode;
WASM_DECLARE_VECTOR(expr_node, WasmExprNode);
WASM_DEFINE_VECTOR(expr_node, WasmExprNode);

typedef struct WasmReadAstContext {
  WasmAllocator* allocator;
  WasmModule* module;

  WasmFuncTypePtrVector func_types;
  WasmFunc* current_func;
  WasmExprNodeVector expr_stack;
} WasmReadAstContext;

static WasmStringSlice dup_string_slice(WasmAllocator* allocator,
                                        WasmStringSlice str) {
  WasmStringSlice result;
  result.start = wasm_strndup(allocator, str.start, str.length);
  result.length = str.length;
  return result;
}

static void on_error(const char* message, void* user_data) {
  fprintf(stderr, "error: %s\n", message);
}

static WasmResult begin_memory_section(void* user_data) {
  WasmReadAstContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(field);
  field->type = WASM_MODULE_FIELD_TYPE_MEMORY;
  assert(ctx->module->memory == NULL);
  ctx->module->memory = &field->memory;
  return WASM_OK;
}

static WasmResult on_memory_initial_size_pages(uint32_t pages,
                                               void* user_data) {
  WasmReadAstContext* ctx = user_data;
  ctx->module->memory->initial_pages = pages;
  return WASM_OK;
}

static WasmResult on_memory_max_size_pages(uint32_t pages, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  ctx->module->memory->max_pages = pages;
  return WASM_OK;
}

static WasmResult on_memory_exported(int exported, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  if (exported) {
    WasmModuleField* field =
        wasm_append_module_field(ctx->allocator, ctx->module);
    CHECK_ALLOC_NULL(field);
    field->type = WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY;
    /* TODO(binji): refactor */
    field->export_memory.name.start =
        wasm_strndup(ctx->allocator, "memory", 6);
    field->export_memory.name.length = 6;
    assert(ctx->module->export_memory == NULL);
    ctx->module->export_memory = &field->export_memory;
  }
  return WASM_OK;
}

static WasmResult on_data_segment_count(uint32_t count, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  CHECK_ALLOC(wasm_reserve_segments(ctx->allocator,
                                    &ctx->module->memory->segments, count));
  ctx->module->memory->segments.size = count;
  return WASM_OK;
}

static WasmResult on_data_segment(uint32_t index,
                                  uint32_t address,
                                  const void* data,
                                  uint32_t size,
                                  void* user_data) {
  WasmReadAstContext* ctx = user_data;
  assert(index < ctx->module->memory->segments.capacity);
  WasmSegment* segment =
      wasm_append_segment(ctx->allocator, &ctx->module->memory->segments);
  CHECK_ALLOC_NULL(segment);
  segment->addr = address;
  segment->data = wasm_alloc(ctx->allocator, size, WASM_DEFAULT_ALIGN);
  segment->size = size;
  memcpy(segment->data, data, size);
  return WASM_OK;
}

static WasmResult on_signature_count(uint32_t count, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  CHECK_ALLOC(wasm_reserve_func_type_ptrs(ctx->allocator,
                                          &ctx->module->func_types, count));
  return WASM_OK;
}

static WasmResult on_signature(uint32_t index,
                               WasmType result_type,
                               uint32_t param_count,
                               WasmType* param_types,
                               void* user_data) {
  WasmReadAstContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(field);
  field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;

  WasmFuncType* func_type = &field->func_type;
  WASM_ZERO_MEMORY(*func_type);
  func_type->sig.result_type = result_type;

  CHECK_ALLOC(wasm_reserve_types(ctx->allocator, &func_type->sig.param_types,
                                 param_count));
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
  WasmReadAstContext* ctx = user_data;
  CHECK_ALLOC(
      wasm_reserve_import_ptrs(ctx->allocator, &ctx->module->imports, count));
  return WASM_OK;
}

static WasmResult on_import(uint32_t index,
                            uint32_t sig_index,
                            WasmStringSlice module_name,
                            WasmStringSlice function_name,
                            void* user_data) {
  WasmReadAstContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(field);
  field->type = WASM_MODULE_FIELD_TYPE_IMPORT;

  WasmImport* import = &field->import;
  WASM_ZERO_MEMORY(*import);
  import->import_type = WASM_IMPORT_HAS_TYPE;
  CHECK_ALLOC_NULL_STR(import->module_name =
                           dup_string_slice(ctx->allocator, module_name));
  CHECK_ALLOC_NULL_STR(import->func_name =
                           dup_string_slice(ctx->allocator, function_name));
  import->type_var.type = WASM_VAR_TYPE_INDEX;
  assert(sig_index < ctx->module->func_types.size);
  import->type_var.index = sig_index;

  assert(index < ctx->module->imports.capacity);
  WasmImportPtr* import_ptr =
      wasm_append_import_ptr(ctx->allocator, &ctx->module->imports);
  *import_ptr = import;
  return WASM_OK;
}

static WasmResult on_function_signatures_count(uint32_t count,
                                               void* user_data) {
  WasmReadAstContext* ctx = user_data;
  CHECK_ALLOC(
      wasm_reserve_func_ptrs(ctx->allocator, &ctx->module->funcs, count));
  return WASM_OK;
}

static WasmResult on_function_signature(uint32_t index,
                                        uint32_t sig_index,
                                        void* user_data) {
  WasmReadAstContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(field);
  field->type = WASM_MODULE_FIELD_TYPE_FUNC;

  WasmFunc* func = &field->func;
  WASM_ZERO_MEMORY(*func);
  func->flags = WASM_FUNC_FLAG_HAS_FUNC_TYPE;
  func->type_var.type = WASM_VAR_TYPE_INDEX;
  assert(sig_index < ctx->module->func_types.size);
  func->type_var.index = sig_index;

  assert(index < ctx->module->funcs.capacity);
  WasmFuncPtr* func_ptr =
      wasm_append_func_ptr(ctx->allocator, &ctx->module->funcs);
  *func_ptr = func;
  return WASM_OK;
}

static WasmResult on_function_bodies_count(uint32_t count, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  assert(count == ctx->module->funcs.size);
  return WASM_OK;
}

static WasmResult begin_function_body(uint32_t index, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  assert(index < ctx->module->funcs.size);
  ctx->current_func = ctx->module->funcs.data[index];
  return WASM_OK;
}

static WasmResult end_function_body(uint32_t index, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  ctx->current_func = NULL;
  return WASM_OK;
}

static WasmResult on_local_decl_count(uint32_t count, void* user_data) {
  return WASM_OK;
}

static WasmResult on_local_decl(uint32_t decl_index,
                                uint32_t count,
                                WasmType type,
                                void* user_data) {
  WasmReadAstContext* ctx = user_data;
  assert(ctx->current_func);
  size_t old_local_count = ctx->current_func->locals.types.size;
  size_t new_local_count = old_local_count + count;
  CHECK_ALLOC(wasm_reserve_types(
      ctx->allocator, &ctx->current_func->locals.types, new_local_count));
  WASM_STATIC_ASSERT(sizeof(WasmType) == sizeof(uint8_t));
  WasmTypeVector* types = &ctx->current_func->locals.types;
  memset(&types->data[old_local_count], type, count);
  types->size = new_local_count;
  return WASM_OK;
}

static WasmResult reduce(WasmReadAstContext* ctx, WasmExpr* expr) {
  int done = 1;
  while (!done) {
    done = 1;
    if (ctx->expr_stack.size == 0) {
      WasmExprPtr* expr_ptr =
          wasm_append_expr_ptr(ctx->allocator, &ctx->current_func->exprs);
      CHECK_ALLOC_NULL(expr_ptr);
      *expr_ptr = expr;
    } else {
      WasmExprNode* top = &ctx->expr_stack.data[ctx->expr_stack.size - 1];
      assert(top->index < top->total);

      switch (top->expr->type) {
        case WASM_EXPR_TYPE_BINARY:
          if (top->index == 0)
            top->expr->binary.left = expr;
          else
            top->expr->binary.right = expr;
          break;

        case WASM_EXPR_TYPE_BLOCK: {
          WasmExprPtr* expr_ptr =
              wasm_append_expr_ptr(ctx->allocator, &top->expr->block.exprs);
          CHECK_ALLOC_NULL(expr_ptr);
          *expr_ptr = expr;
          break;
        }

        case WASM_EXPR_TYPE_BR:
          top->expr->br.expr = expr;
          break;

        case WASM_EXPR_TYPE_BR_IF:
          top->expr->br_if.expr = expr;
          break;

        case WASM_EXPR_TYPE_BR_TABLE:
          top->expr->br_table.expr = expr;
          break;

        case WASM_EXPR_TYPE_CALL:
        case WASM_EXPR_TYPE_CALL_IMPORT: {
          WasmExprPtr* expr_ptr =
              wasm_append_expr_ptr(ctx->allocator, &top->expr->call.args);
          CHECK_ALLOC_NULL(expr_ptr);
          *expr_ptr = expr;
          break;
        }

        case WASM_EXPR_TYPE_CALL_INDIRECT: {
          if (top->index == 0) {
            top->expr->call_indirect.expr = expr;
          } else {
            WasmExprPtr* expr_ptr = wasm_append_expr_ptr(
                ctx->allocator, &top->expr->call_indirect.args);
            CHECK_ALLOC_NULL(expr_ptr);
            *expr_ptr = expr;
          }
          break;
        }

        case WASM_EXPR_TYPE_COMPARE:
          if (top->index == 0)
            top->expr->compare.left = expr;
          else
            top->expr->compare.right = expr;
          break;

        case WASM_EXPR_TYPE_CONVERT:
          top->expr->convert.expr = expr;
          break;

        case WASM_EXPR_TYPE_GROW_MEMORY:
          top->expr->grow_memory.expr = expr;
          break;

        case WASM_EXPR_TYPE_IF:
          if (top->index == 0) {
            top->expr->if_.cond = expr;
          } else {
            top->expr->if_.true_ = expr;
          }
          break;

        case WASM_EXPR_TYPE_IF_ELSE:
          if (top->index == 0) {
            top->expr->if_else.cond = expr;
          } else if (top->index == 1) {
            top->expr->if_else.true_ = expr;
          } else {
            top->expr->if_else.false_ = expr;
          }
          break;

        case WASM_EXPR_TYPE_LOAD:
          top->expr->load.addr = expr;
          break;

        case WASM_EXPR_TYPE_LOOP: {
          WasmExprPtr* expr_ptr =
              wasm_append_expr_ptr(ctx->allocator, &top->expr->loop.exprs);
          CHECK_ALLOC_NULL(expr_ptr);
          *expr_ptr = expr;
          break;
        }

        case WASM_EXPR_TYPE_RETURN:
          top->expr->return_.expr = expr;
          break;

        case WASM_EXPR_TYPE_SELECT:
          if (top->index == 0)
            top->expr->select.true_ = expr;
          else if (top->index == 1)
            top->expr->select.false_ = expr;
          else
            top->expr->select.cond = expr;
          break;

        case WASM_EXPR_TYPE_SET_LOCAL:
          top->expr->set_local.expr = expr;
          break;

        case WASM_EXPR_TYPE_STORE:
          if (top->index == 0)
            top->expr->store.addr = expr;
          else
            top->expr->store.value = expr;
          break;

        case WASM_EXPR_TYPE_UNARY:
          top->expr->unary.expr = expr;
          break;


        default:
        case WASM_EXPR_TYPE_CONST:
        case WASM_EXPR_TYPE_GET_LOCAL:
        case WASM_EXPR_TYPE_MEMORY_SIZE:
        case WASM_EXPR_TYPE_NOP:
        case WASM_EXPR_TYPE_UNREACHABLE:
          assert(0);
          return WASM_ERROR;
      }

      if (++top->index == top->total) {
        /* "recurse" and reduce the current expr */
        expr = top->expr;
        ctx->expr_stack.size--;
        done = 0;
      }
    }
  }

  return WASM_OK;
}

static WasmResult on_binary_expr(WasmOpcode opcode, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_binary_expr(ctx->allocator);
  CHECK_ALLOC_NULL(expr);
  expr->binary = opcode_to_binary_op(opcode);

  WasmExprNode* node = wasm_append_expr_node(ctx->allocator, &ctx->expr_stack);
  CHECK_ALLOC_NULL(node);
  node->expr = expr;
  node->total = 2;
  return WASM_OK;
}

static WasmResult on_block_expr(uint32_t count, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_block_expr(ctx->allocator);
  CHECK_ALLOC_NULL(expr);

  if (count > 0) {
    WasmExprNode* node =
        wasm_append_expr_node(ctx->allocator, &ctx->expr_stack);
    CHECK_ALLOC_NULL(node);
    node->expr = expr;
    node->total = count;
    return WASM_OK;
  } else {
    return reduce(ctx, expr);
  }
}

static WasmResult on_br_expr(uint32_t depth, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_block_expr(ctx->allocator);
  CHECK_ALLOC_NULL(expr);
  expr->br.
}

static WasmResult on_br_if_expr(uint32_t depth, void* user_data) {
  return WASM_OK;
}

static WasmResult on_call_expr(uint32_t func_index, void* user_data) {
  return WASM_OK;
}

static WasmResult on_call_import_expr(uint32_t import_index, void* user_data) {
  return WASM_OK;
}

static WasmResult on_call_indirect_expr(uint32_t sig_index, void* user_data) {
  return WASM_OK;
}

static WasmResult on_compare_expr(WasmOpcode opcode, void* user_data) {
  return WASM_OK;
}

static WasmResult on_i32_const_expr(uint32_t value, void* user_data) {
  return WASM_OK;
}

static WasmResult on_i64_const_expr(uint64_t value, void* user_data) {
  return WASM_OK;
}

static WasmResult on_f32_const_expr(uint32_t value_bits, void* user_data) {
  return WASM_OK;
}

static WasmResult on_f64_const_expr(uint64_t value_bits, void* user_data) {
  return WASM_OK;
}

static WasmResult on_convert_expr(WasmOpcode opcode, void* user_data) {
  return WASM_OK;
}

static WasmResult on_get_local_expr(uint32_t local_index, void* user_data) {
  return WASM_OK;
}

static WasmResult on_grow_memory_expr(void* user_data) {
  return WASM_OK;
}

static WasmResult on_if_expr(void* user_data) {
  return WASM_OK;
}

static WasmResult on_if_else_expr(void* user_data) {
  return WASM_OK;
}

static WasmResult on_load_expr(WasmOpcode opcode,
                               uint32_t alignment_log2,
                               uint32_t offset,
                               void* user_data) {
  return WASM_OK;
}

static WasmResult on_loop_expr(uint32_t count, void* user_data) {
  return WASM_OK;
}

static WasmResult on_memory_size_expr(void* user_data) {
  return WASM_OK;
}

static WasmResult on_nop_expr(void* user_data) {
  return WASM_OK;
}

static WasmResult on_return_expr(void* user_data) {
  return WASM_OK;
}

static WasmResult on_select_expr(void* user_data) {
  return WASM_OK;
}

static WasmResult on_set_local_expr(uint32_t local_index, void* user_data) {
  return WASM_OK;
}

static WasmResult on_store_expr(WasmOpcode opcode,
                                uint32_t alignment_log2,
                                uint32_t offset,
                                void* user_data) {
  return WASM_OK;
}

static WasmResult on_br_table_expr(uint32_t num_targets,
                                   uint32_t* target_depths,
                                   uint32_t default_target_depth,
                                   void* user_data) {
  return WASM_OK;
}

static WasmResult on_unary_expr(WasmOpcode opcode, void* user_data) {
  return WASM_OK;
}

static WasmResult on_unreachable_expr(void* user_data) {
  return WASM_OK;
}

static WasmResult on_function_table_count(uint32_t count, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(field);
  field->type = WASM_MODULE_FIELD_TYPE_TABLE;

  assert(ctx->module->table == NULL);
  ctx->module->table = &field->table;

  CHECK_ALLOC(wasm_reserve_vars(ctx->allocator, &field->table, count));
  return WASM_OK;
}

static WasmResult on_function_table_entry(uint32_t index,
                                          uint32_t func_index,
                                          void* user_data) {
  WasmReadAstContext* ctx = user_data;
  WasmVar* func_var = &ctx->module->table->data[index];
  func_var->type = WASM_VAR_TYPE_INDEX;
  assert(func_index < ctx->module->funcs.size);
  func_var->index = func_index;
  return WASM_OK;
}

static WasmResult on_start_function(uint32_t func_index, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(field);
  field->type = WASM_MODULE_FIELD_TYPE_START;

  field->start.type = WASM_VAR_TYPE_INDEX;
  assert(func_index < ctx->module->funcs.size);
  field->start.index = func_index;

  ctx->module->start = field->start;
  return WASM_OK;
}

static WasmResult on_export_count(uint32_t count, void* user_data) {
  WasmReadAstContext* ctx = user_data;
  CHECK_ALLOC(
      wasm_reserve_export_ptrs(ctx->allocator, &ctx->module->exports, count));
  return WASM_OK;
}

static WasmResult on_export(uint32_t index,
                            uint32_t func_index,
                            WasmStringSlice name,
                            void* user_data) {
  WasmReadAstContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(field);
  field->type = WASM_MODULE_FIELD_TYPE_EXPORT;

  WasmExport* export = &field->export_;
  WASM_ZERO_MEMORY(*export);
  CHECK_ALLOC_NULL_STR(export->name = dup_string_slice(ctx->allocator, name));
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
  return WASM_OK;
}

static WasmResult on_function_name(uint32_t index,
                                   WasmStringSlice name,
                                   void* user_data) {
  return WASM_OK;
}

static WasmResult on_local_names_count(uint32_t index,
                                       uint32_t count,
                                       void* user_data) {
  return WASM_OK;
}

static WasmResult on_local_name(uint32_t func_index,
                                uint32_t local_index,
                                WasmStringSlice name,
                                void* user_data) {
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
    .on_local_decl_count = &on_local_decl_count,
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
    .on_i32_const_expr = &on_i32_const_expr,
    .on_i64_const_expr = &on_i64_const_expr,
    .on_f32_const_expr = &on_f32_const_expr,
    .on_f64_const_expr = &on_f64_const_expr,
    .on_convert_expr = &on_convert_expr,
    .on_get_local_expr = &on_get_local_expr,
    .on_grow_memory_expr = &on_grow_memory_expr,
    .on_if_expr = &on_if_expr,
    .on_if_else_expr = &on_if_else_expr,
    .on_load_expr = &on_load_expr,
    .on_loop_expr = &on_loop_expr,
    .on_memory_size_expr = &on_memory_size_expr,
    .on_nop_expr = &on_nop_expr,
    .on_return_expr = &on_return_expr,
    .on_select_expr = &on_select_expr,
    .on_set_local_expr = &on_set_local_expr,
    .on_store_expr = &on_store_expr,
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

WasmResult wasm_read_binary_ast(struct WasmAllocator* allocator,
                                const void* data,
                                size_t size,
                                struct WasmModule* out_module) {
  WasmReadAstContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.module = out_module;

  WasmBinaryReader reader;
  WASM_ZERO_MEMORY(reader);
  reader = s_binary_reader;
  reader.user_data = &ctx;

  WasmResult result = wasm_read_binary(allocator, data, size, &reader);
  return result;
}

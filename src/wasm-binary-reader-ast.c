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
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "wasm-allocator.h"
#include "wasm-ast.h"
#include "wasm-binary-reader.h"
#include "wasm-common.h"

#define LOG 0

#if LOG
#define LOGF(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOGF(...) (void)0
#endif

#define CHECK_ALLOC_(cond)                                              \
  do {                                                                  \
    if (!(cond)) {                                                      \
      print_error(ctx, "%s:%d: allocation failed", __FILE__, __LINE__); \
      return WASM_ERROR;                                                \
    }                                                                   \
  } while (0)

#define CHECK_ALLOC(e) CHECK_ALLOC_(WASM_SUCCEEDED(e))
#define CHECK_ALLOC_NULL(v) CHECK_ALLOC_((v))
#define CHECK_ALLOC_NULL_STR(v) CHECK_ALLOC_((v).start)

#define CHECK_RESULT(expr) \
  do {                     \
    if (WASM_FAILED(expr)) \
      return WASM_ERROR;   \
  } while (0)

#define CHECK_DEPTH(depth)                                            \
  do {                                                                \
    if ((depth) >= ctx->max_depth) {                                  \
      print_error(ctx, "invalid depth: %d (max %" PRIzd ")", (depth), \
                  ctx->max_depth);                                    \
      return WASM_ERROR;                                              \
    }                                                                 \
  } while (0)

#define CHECK_LOCAL(local_index)                                          \
  do {                                                                    \
    assert(wasm_decl_has_func_type(&ctx->current_func->decl));            \
    uint32_t max_local_index =                                            \
        wasm_get_num_params_and_locals(ctx->current_func);                \
    if ((local_index) >= max_local_index) {                               \
      print_error(ctx, "invalid local_index: %d (max %d)", (local_index), \
                  max_local_index);                                       \
      return WASM_ERROR;                                                  \
    }                                                                     \
  } while (0)

typedef enum WasmLabelType {
  WASM_LABEL_TYPE_BLOCK,
  WASM_LABEL_TYPE_IF,
  WASM_LABEL_TYPE_ELSE,
  WASM_LABEL_TYPE_LOOP,
} WasmLabelType;

typedef struct WasmLabelNode {
  WasmLabelType type;
  uint32_t expr_stack_size;
  WasmExpr* expr;
} WasmLabelNode;
WASM_DEFINE_VECTOR(label_node, WasmLabelNode);

typedef struct WasmContext {
  WasmAllocator* allocator;
  WasmBinaryErrorHandler* error_handler;
  WasmModule* module;

  WasmFunc* current_func;
  WasmExprPtrVector expr_stack;
  WasmLabelNodeVector label_stack;
  uint32_t max_depth;
} WasmContext;

static void on_error(uint32_t offset, const char* message, void* user_data);

static void WASM_PRINTF_FORMAT(2, 3)
    print_error(WasmContext* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  on_error(WASM_UNKNOWN_OFFSET, buffer, ctx);
}

static WasmResult dup_name(WasmContext* ctx,
                           WasmStringSlice* name,
                           WasmStringSlice* out_name) {
  if (name->length > 0) {
    *out_name = wasm_dup_string_slice(ctx->allocator, *name);
    CHECK_ALLOC_NULL_STR(*out_name);
  } else {
    WASM_ZERO_MEMORY(*out_name);
  }
  return WASM_OK;
}

static WasmResult push_expr(WasmContext* ctx, WasmExpr* expr) {
  return wasm_append_expr_ptr_value(ctx->allocator, &ctx->expr_stack, &expr);
}

static WasmResult pop_expr(WasmContext* ctx, WasmExpr** expr) {
  if (ctx->expr_stack.size == 0) {
    print_error(ctx, "popping empty stack");
    return WASM_ERROR;
  }

  *expr = ctx->expr_stack.data[--ctx->expr_stack.size];
  return WASM_OK;
}

static WasmResult pop_into_expr_vector(WasmContext* ctx,
                                       uint32_t new_expr_stack_size,
                                       WasmExprPtrVector* exprs) {
  uint32_t start = new_expr_stack_size;
  uint32_t end = ctx->expr_stack.size;
  assert(start <= end);
  uint32_t i;
  for (i = start; i < end; ++i) {
    WasmExpr* arg = ctx->expr_stack.data[i];
    CHECK_RESULT(wasm_append_expr_ptr_value(ctx->allocator, exprs, &arg));
  }
  ctx->expr_stack.size = new_expr_stack_size;
  return WASM_OK;
}

static WasmResult pop_into_args(WasmContext* ctx,
                                WasmFuncSignature* sig,
                                WasmExprPtrVector* args) {
  uint32_t num_params = sig->param_types.size;
  if (ctx->expr_stack.size < num_params) {
    print_error(ctx,
                "call requires %" PRIzd " args, but only %" PRIzd " on stack",
                num_params, ctx->expr_stack.size);
    return WASM_ERROR;
  }

  uint32_t i;
  for (i = 0; i < num_params; ++i) {
    WasmExpr* arg = ctx->expr_stack.data[ctx->expr_stack.size - num_params + i];
    CHECK_RESULT(wasm_append_expr_ptr_value(ctx->allocator, args, &arg));
  }
  ctx->expr_stack.size -= num_params;
  return WASM_OK;
}

void on_error(uint32_t offset, const char* message, void* user_data) {
  WasmContext* ctx = user_data;
  if (ctx->error_handler->on_error) {
    ctx->error_handler->on_error(offset, message,
                                 ctx->error_handler->user_data);
  }
}

static WasmResult begin_memory_section(void* user_data) {
  WasmContext* ctx = user_data;
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
  WasmContext* ctx = user_data;
  ctx->module->memory->initial_pages = pages;
  return WASM_OK;
}

static WasmResult on_memory_max_size_pages(uint32_t pages, void* user_data) {
  WasmContext* ctx = user_data;
  ctx->module->memory->max_pages = pages;
  return WASM_OK;
}

static WasmResult on_memory_exported(WasmBool exported, void* user_data) {
  WasmContext* ctx = user_data;
  if (exported) {
    WasmModuleField* field =
        wasm_append_module_field(ctx->allocator, ctx->module);
    CHECK_ALLOC_NULL(field);
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
  WasmContext* ctx = user_data;
  CHECK_ALLOC(wasm_reserve_segments(ctx->allocator,
                                    &ctx->module->memory->segments, count));
  return WASM_OK;
}

static WasmResult on_data_segment(uint32_t index,
                                  uint32_t address,
                                  const void* data,
                                  uint32_t size,
                                  void* user_data) {
  WasmContext* ctx = user_data;
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
  WasmContext* ctx = user_data;
  CHECK_ALLOC(wasm_reserve_func_type_ptrs(ctx->allocator,
                                          &ctx->module->func_types, count));
  return WASM_OK;
}

static WasmResult on_signature(uint32_t index,
                               WasmType result_type,
                               uint32_t param_count,
                               WasmType* param_types,
                               void* user_data) {
  WasmContext* ctx = user_data;
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
  WasmContext* ctx = user_data;
  CHECK_ALLOC(
      wasm_reserve_import_ptrs(ctx->allocator, &ctx->module->imports, count));
  return WASM_OK;
}

static WasmResult on_import(uint32_t index,
                            uint32_t sig_index,
                            WasmStringSlice module_name,
                            WasmStringSlice function_name,
                            void* user_data) {
  WasmContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(field);
  field->type = WASM_MODULE_FIELD_TYPE_IMPORT;

  WasmImport* import = &field->import;
  WASM_ZERO_MEMORY(*import);
  import->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
  CHECK_ALLOC_NULL_STR(import->module_name =
                           wasm_dup_string_slice(ctx->allocator, module_name));
  CHECK_ALLOC_NULL_STR(
      import->func_name = wasm_dup_string_slice(ctx->allocator, function_name));
  import->decl.type_var.type = WASM_VAR_TYPE_INDEX;
  assert(sig_index < ctx->module->func_types.size);
  import->decl.type_var.index = sig_index;

  assert(index < ctx->module->imports.capacity);
  WasmImportPtr* import_ptr =
      wasm_append_import_ptr(ctx->allocator, &ctx->module->imports);
  *import_ptr = import;
  return WASM_OK;
}

static WasmResult on_function_signatures_count(uint32_t count,
                                               void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_ALLOC(
      wasm_reserve_func_ptrs(ctx->allocator, &ctx->module->funcs, count));
  return WASM_OK;
}

static WasmResult on_function_signature(uint32_t index,
                                        uint32_t sig_index,
                                        void* user_data) {
  WasmContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(field);
  field->type = WASM_MODULE_FIELD_TYPE_FUNC;

  WasmFunc* func = &field->func;
  WASM_ZERO_MEMORY(*func);
  func->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE |
                     WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
  func->decl.type_var.type = WASM_VAR_TYPE_INDEX;
  assert(sig_index < ctx->module->func_types.size);
  func->decl.type_var.index = sig_index;

  /* copy the signature from the function type */
  WasmFuncSignature* sig = &ctx->module->func_types.data[sig_index]->sig;
  size_t i;
  for (i = 0; i < sig->param_types.size; ++i) {
    CHECK_ALLOC(wasm_append_type_value(ctx->allocator,
                                       &func->decl.sig.param_types,
                                       &sig->param_types.data[i]));
  }
  func->decl.sig.result_type = sig->result_type;

  assert(index < ctx->module->funcs.capacity);
  WasmFuncPtr* func_ptr =
      wasm_append_func_ptr(ctx->allocator, &ctx->module->funcs);
  *func_ptr = func;
  return WASM_OK;
}

static WasmResult on_function_bodies_count(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  assert(count == ctx->module->funcs.size);
  (void)ctx;
  return WASM_OK;
}

static WasmResult begin_function_body(uint32_t index, void* user_data) {
  WasmContext* ctx = user_data;
  assert(index < ctx->module->funcs.size);
  ctx->current_func = ctx->module->funcs.data[index];
  LOGF("func %d\n", index);
  return WASM_OK;
}

static WasmResult end_function_body(uint32_t index, void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_RESULT(pop_into_expr_vector(ctx, 0, &ctx->current_func->exprs));
  ctx->current_func = NULL;
  return WASM_OK;
}

static WasmResult on_local_decl(uint32_t decl_index,
                                uint32_t count,
                                WasmType type,
                                void* user_data) {
  WasmContext* ctx = user_data;
  size_t old_local_count = ctx->current_func->local_types.size;
  size_t new_local_count = old_local_count + count;
  CHECK_ALLOC(wasm_reserve_types(
      ctx->allocator, &ctx->current_func->local_types, new_local_count));
  WASM_STATIC_ASSERT(sizeof(WasmType) == sizeof(uint8_t));
  WasmTypeVector* types = &ctx->current_func->local_types;
  memset(&types->data[old_local_count], type, count);
  types->size = new_local_count;
  return WASM_OK;
}

static WasmResult push_label(WasmContext* ctx,
                             WasmLabelType type,
                             WasmExpr* expr) {
  WasmLabelNode label;
  label.type = type;
  label.expr_stack_size = ctx->expr_stack.size;
  label.expr = expr;
  ctx->max_depth += type == WASM_LABEL_TYPE_LOOP ? 2 : 1;
  return wasm_append_label_node_value(ctx->allocator, &ctx->label_stack,
                                      &label);
}

static WasmResult pop_label(WasmContext* ctx) {
  if (ctx->label_stack.size == 0) {
    print_error(ctx, "popping empty label stack");
    return WASM_ERROR;
  }

  WasmLabelType type = ctx->label_stack.data[ctx->label_stack.size - 1].type;
  ctx->max_depth -= type == WASM_LABEL_TYPE_LOOP ? 2 : 1;
  ctx->label_stack.size--;
  return WASM_OK;
}

static WasmResult top_label(WasmContext* ctx, WasmLabelNode** label) {
  if (ctx->label_stack.size == 0) {
    print_error(ctx, "accessing empty label stack");
    return WASM_ERROR;
  }

  *label = &ctx->label_stack.data[ctx->label_stack.size - 1];
  return WASM_OK;
}

static WasmResult on_unary_expr(WasmOpcode opcode, void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result, *expr;
  CHECK_RESULT(pop_expr(ctx, &expr));
  CHECK_ALLOC_NULL(result = wasm_new_unary_expr(ctx->allocator));
  result->unary.opcode = opcode;
  result->unary.expr = expr;
  return push_expr(ctx, result);
}

static WasmResult on_binary_expr(WasmOpcode opcode, void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result, *left, *right;
  CHECK_RESULT(pop_expr(ctx, &right));
  CHECK_RESULT(pop_expr(ctx, &left));
  CHECK_ALLOC_NULL(result = wasm_new_binary_expr(ctx->allocator));
  result->binary.opcode = opcode;
  result->binary.left = left;
  result->binary.right = right;
  return push_expr(ctx, result);
}

static WasmResult on_compare_expr(WasmOpcode opcode, void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result, *left, *right;
  CHECK_RESULT(pop_expr(ctx, &right));
  CHECK_RESULT(pop_expr(ctx, &left));
  CHECK_ALLOC_NULL(result = wasm_new_compare_expr(ctx->allocator));
  result->compare.opcode = opcode;
  result->compare.left = left;
  result->compare.right = right;
  return push_expr(ctx, result);
}

static WasmResult on_convert_expr(WasmOpcode opcode, void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result, *expr;
  CHECK_RESULT(pop_expr(ctx, &expr));
  CHECK_ALLOC_NULL(result = wasm_new_convert_expr(ctx->allocator));
  result->convert.opcode = opcode;
  result->convert.expr = expr;
  return push_expr(ctx, result);
}

static WasmResult on_block_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result;
  CHECK_ALLOC_NULL(result = wasm_new_block_expr(ctx->allocator));
  return push_label(ctx, WASM_LABEL_TYPE_BLOCK, result);
}

static WasmResult on_loop_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result;
  CHECK_ALLOC_NULL(result = wasm_new_loop_expr(ctx->allocator));
  return push_label(ctx, WASM_LABEL_TYPE_LOOP, result);
}

static WasmResult on_if_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *cond, *result;
  CHECK_RESULT(pop_expr(ctx, &cond));
  CHECK_ALLOC_NULL(result = wasm_new_if_expr(ctx->allocator));
  result->if_.cond = cond;
  return push_label(ctx, WASM_LABEL_TYPE_IF, result);
}

static WasmResult on_else_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmLabelNode* label;
  CHECK_RESULT(top_label(ctx, &label));
  if (label->type != WASM_LABEL_TYPE_IF) {
    print_error(ctx, "else expression without matching if");
    return WASM_ERROR;
  }

  /* destroy the if expr, replace it with an if_else */
  /* TODO(binji): remove if_else and just have false branch be an empty block
   * for if without else? */
  WasmExpr* cond = label->expr->if_.cond;
  label->expr->if_.cond = NULL;
  wasm_free(ctx->allocator, label->expr);

  WasmExpr* result;
  CHECK_ALLOC_NULL(result = wasm_new_if_else_expr(ctx->allocator));
  result->if_else.cond = cond;
  CHECK_RESULT(pop_into_expr_vector(ctx, label->expr_stack_size,
                                    &result->if_else.true_.exprs));

  label->type = WASM_LABEL_TYPE_ELSE;
  label->expr = result;
  return WASM_OK;
}

static WasmResult on_end_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmLabelNode* label;
  CHECK_RESULT(top_label(ctx, &label));
  WasmExpr* expr = label->expr;

  switch (label->type) {
    case WASM_LABEL_TYPE_IF:
      CHECK_RESULT(pop_into_expr_vector(ctx, label->expr_stack_size,
                                        &expr->if_.true_.exprs));
      break;

    case WASM_LABEL_TYPE_ELSE:
      CHECK_RESULT(pop_into_expr_vector(ctx, label->expr_stack_size,
                                        &expr->if_else.false_.exprs));
      break;

    case WASM_LABEL_TYPE_BLOCK:
      CHECK_RESULT(pop_into_expr_vector(ctx, label->expr_stack_size,
                                        &expr->block.exprs));
      break;

    case WASM_LABEL_TYPE_LOOP:
      CHECK_RESULT(
          pop_into_expr_vector(ctx, label->expr_stack_size, &expr->loop.exprs));
      break;

    default:
      assert(0);
      return WASM_ERROR;
  }

  CHECK_RESULT(pop_label(ctx));
  return push_expr(ctx, expr);
}

static WasmResult on_br_expr(uint32_t depth, void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result, *expr;
  CHECK_RESULT(pop_expr(ctx, &expr));
  CHECK_ALLOC_NULL(result = wasm_new_br_expr(ctx->allocator));
  result->br.var.type = WASM_VAR_TYPE_INDEX;
  CHECK_DEPTH(depth);
  result->br.var.index = depth;
  result->br.expr = expr;
  return push_expr(ctx, result);
}

static WasmResult on_br_if_expr(uint32_t depth, void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result, *cond, *expr;
  CHECK_RESULT(pop_expr(ctx, &cond));
  CHECK_RESULT(pop_expr(ctx, &expr));
  CHECK_ALLOC_NULL(result = wasm_new_br_if_expr(ctx->allocator));
  result->br_if.var.type = WASM_VAR_TYPE_INDEX;
  CHECK_DEPTH(depth);
  result->br_if.var.index = depth;
  result->br_if.cond = cond;
  result->br_if.expr = expr;
  return push_expr(ctx, result);
}

static WasmResult on_br_table_expr(uint32_t num_targets,
                                   uint32_t* target_depths,
                                   uint32_t default_target_depth,
                                   void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result, *expr, *key;
  CHECK_RESULT(pop_expr(ctx, &key));
  CHECK_RESULT(pop_expr(ctx, &expr));
  CHECK_ALLOC_NULL(result = wasm_new_br_table_expr(ctx->allocator));
  CHECK_ALLOC(wasm_reserve_vars(ctx->allocator, &result->br_table.targets,
                                num_targets));
  result->br_table.key = key;
  result->br_table.expr = expr;
  result->br_table.targets.size = num_targets;
  uint32_t i;
  for (i = 0; i < num_targets; ++i) {
    WasmVar* var = &result->br_table.targets.data[i];
    var->type = WASM_VAR_TYPE_INDEX;
    CHECK_DEPTH(target_depths[i]);
    var->index = target_depths[i];
  }
  result->br_table.default_target.type = WASM_VAR_TYPE_INDEX;
  CHECK_DEPTH(default_target_depth);
  result->br_table.default_target.index = default_target_depth;
  return push_expr(ctx, result);
}

static WasmResult on_call_expr(uint32_t func_index, void* user_data) {
  WasmContext* ctx = user_data;
  assert(func_index < ctx->module->funcs.size);
  WasmFunc* func = ctx->module->funcs.data[func_index];
  uint32_t sig_index = func->decl.type_var.index;
  assert(sig_index < ctx->module->func_types.size);
  WasmFuncType* func_type = ctx->module->func_types.data[sig_index];

  WasmExpr* result;
  CHECK_ALLOC_NULL(result = wasm_new_call_expr(ctx->allocator));
  result->call.var.type = WASM_VAR_TYPE_INDEX;
  result->call.var.index = func_index;
  CHECK_RESULT(pop_into_args(ctx, &func_type->sig, &result->call.args));
  return push_expr(ctx, result);
}

static WasmResult on_call_import_expr(uint32_t import_index, void* user_data) {
  WasmContext* ctx = user_data;
  assert(import_index < ctx->module->imports.size);
  WasmImport* import = ctx->module->imports.data[import_index];
  uint32_t sig_index = import->decl.type_var.index;
  assert(sig_index < ctx->module->func_types.size);
  WasmFuncType* func_type = ctx->module->func_types.data[sig_index];

  WasmExpr* result;
  CHECK_ALLOC_NULL(result = wasm_new_call_import_expr(ctx->allocator));
  result->call.var.type = WASM_VAR_TYPE_INDEX;
  result->call.var.index = import_index;
  CHECK_RESULT(pop_into_args(ctx, &func_type->sig, &result->call.args));
  return push_expr(ctx, result);
}

static WasmResult on_call_indirect_expr(uint32_t sig_index, void* user_data) {
  WasmContext* ctx = user_data;
  assert(sig_index < ctx->module->func_types.size);
  WasmFuncType* func_type = ctx->module->func_types.data[sig_index];

  WasmExpr* result;
  CHECK_ALLOC_NULL(result = wasm_new_call_indirect_expr(ctx->allocator));
  result->call_indirect.var.type = WASM_VAR_TYPE_INDEX;
  result->call_indirect.var.index = sig_index;
  CHECK_RESULT(
      pop_into_args(ctx, &func_type->sig, &result->call_indirect.args));
  CHECK_RESULT(pop_expr(ctx, &result->call_indirect.expr));
  return push_expr(ctx, result);
}

static WasmResult on_i32_const_expr(uint32_t value, void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr* result = wasm_new_const_expr(ctx->allocator);
  CHECK_ALLOC_NULL(result);
  result->const_.type = WASM_TYPE_I32;
  result->const_.u32 = value;
  return push_expr(ctx, result);
}

static WasmResult on_i64_const_expr(uint64_t value, void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr* result = wasm_new_const_expr(ctx->allocator);
  CHECK_ALLOC_NULL(result);
  result->const_.type = WASM_TYPE_I64;
  result->const_.u64 = value;
  return push_expr(ctx, result);
}

static WasmResult on_f32_const_expr(uint32_t value_bits, void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr* result = wasm_new_const_expr(ctx->allocator);
  CHECK_ALLOC_NULL(result);
  result->const_.type = WASM_TYPE_F32;
  result->const_.f32_bits = value_bits;
  return push_expr(ctx, result);
}

static WasmResult on_f64_const_expr(uint64_t value_bits, void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr* result = wasm_new_const_expr(ctx->allocator);
  CHECK_ALLOC_NULL(result);
  result->const_.type = WASM_TYPE_F64;
  result->const_.f64_bits = value_bits;
  return push_expr(ctx, result);
}

static WasmResult on_get_local_expr(uint32_t local_index, void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr* result;
  CHECK_ALLOC_NULL(result = wasm_new_get_local_expr(ctx->allocator));
  result->get_local.var.type = WASM_VAR_TYPE_INDEX;
  result->get_local.var.index = local_index;
  CHECK_LOCAL(local_index);
  return push_expr(ctx, result);
}

static WasmResult on_set_local_expr(uint32_t local_index, void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result, *expr;
  CHECK_RESULT(pop_expr(ctx, &expr));
  CHECK_ALLOC_NULL(result = wasm_new_set_local_expr(ctx->allocator));
  result->set_local.var.type = WASM_VAR_TYPE_INDEX;
  result->set_local.var.index = local_index;
  result->set_local.expr = expr;
  CHECK_LOCAL(local_index);
  return push_expr(ctx, result);
}

static WasmResult on_grow_memory_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result, *expr;
  CHECK_RESULT(pop_expr(ctx, &expr));
  CHECK_ALLOC_NULL(result = wasm_new_grow_memory_expr(ctx->allocator));
  result->grow_memory.expr = expr;
  return push_expr(ctx, result);
}

static WasmResult on_memory_size_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr* result;
  CHECK_ALLOC_NULL(
      result = wasm_new_empty_expr(ctx->allocator, WASM_EXPR_TYPE_MEMORY_SIZE));
  return push_expr(ctx, result);
}

static WasmResult on_load_expr(WasmOpcode opcode,
                               uint32_t alignment_log2,
                               uint32_t offset,
                               void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr* result, *addr;
  CHECK_RESULT(pop_expr(ctx, &addr));
  CHECK_ALLOC_NULL(result = wasm_new_load_expr(ctx->allocator));
  result->load.opcode = opcode;
  result->load.align = 1 << alignment_log2;
  result->load.offset = offset;
  result->load.addr = addr;
  return push_expr(ctx, result);
}

static WasmResult on_store_expr(WasmOpcode opcode,
                                uint32_t alignment_log2,
                                uint32_t offset,
                                void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result, *addr, *value;
  CHECK_RESULT(pop_expr(ctx, &value));
  CHECK_RESULT(pop_expr(ctx, &addr));
  CHECK_ALLOC_NULL(result = wasm_new_store_expr(ctx->allocator));
  result->store.opcode = opcode;
  result->store.align = 1 << alignment_log2;
  result->store.offset = offset;
  result->store.addr = addr;
  result->store.value = value;
  return push_expr(ctx, result);
}

static WasmResult on_nop_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr* result;
  CHECK_ALLOC_NULL(result =
                       wasm_new_empty_expr(ctx->allocator, WASM_EXPR_TYPE_NOP));
  return push_expr(ctx, result);
}

static WasmResult on_return_expr(void* user_data) {
  WasmContext* ctx = user_data;
  uint32_t sig_index = ctx->current_func->decl.type_var.index;
  assert(sig_index < ctx->module->func_types.size);
  WasmFuncType* func_type = ctx->module->func_types.data[sig_index];

  WasmExpr *result, *expr = NULL;
  if (func_type->sig.result_type != WASM_TYPE_VOID)
    CHECK_RESULT(pop_expr(ctx, &expr));
  CHECK_ALLOC_NULL(result = wasm_new_return_expr(ctx->allocator));
  result->return_.expr = expr;
  return push_expr(ctx, result);
}

static WasmResult on_select_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr *result, *true_, *false_, *cond;
  CHECK_RESULT(pop_expr(ctx, &cond));
  CHECK_RESULT(pop_expr(ctx, &false_));
  CHECK_RESULT(pop_expr(ctx, &true_));
  CHECK_ALLOC_NULL(result = wasm_new_select_expr(ctx->allocator));
  result->select.cond = cond;
  result->select.true_ = true_;
  result->select.false_ = false_;
  return push_expr(ctx, result);
}

static WasmResult on_unreachable_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmExpr* result;
  CHECK_ALLOC_NULL(
      result = wasm_new_empty_expr(ctx->allocator, WASM_EXPR_TYPE_UNREACHABLE));
  return push_expr(ctx, result);
}

static WasmResult on_function_table_count(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
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
  WasmContext* ctx = user_data;
  assert(index < ctx->module->table->capacity);
  WasmVar* func_var = wasm_append_var(ctx->allocator, ctx->module->table);
  func_var->type = WASM_VAR_TYPE_INDEX;
  assert(func_index < ctx->module->funcs.size);
  func_var->index = func_index;
  return WASM_OK;
}

static WasmResult on_start_function(uint32_t func_index, void* user_data) {
  WasmContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(field);
  field->type = WASM_MODULE_FIELD_TYPE_START;

  field->start.type = WASM_VAR_TYPE_INDEX;
  assert(func_index < ctx->module->funcs.size);
  field->start.index = func_index;

  ctx->module->start = &field->start;
  return WASM_OK;
}

static WasmResult on_export_count(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_ALLOC(
      wasm_reserve_export_ptrs(ctx->allocator, &ctx->module->exports, count));
  return WASM_OK;
}

static WasmResult on_export(uint32_t index,
                            uint32_t func_index,
                            WasmStringSlice name,
                            void* user_data) {
  WasmContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(field);
  field->type = WASM_MODULE_FIELD_TYPE_EXPORT;

  WasmExport* export = &field->export_;
  WASM_ZERO_MEMORY(*export);
  CHECK_ALLOC_NULL_STR(export->name =
                           wasm_dup_string_slice(ctx->allocator, name));
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
  WasmContext* ctx = user_data;
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
  WasmContext* ctx = user_data;

  WasmStringSlice new_name;
  CHECK_RESULT(dup_name(ctx, &name, &new_name));

  WasmBinding* binding = wasm_insert_binding(
      ctx->allocator, &ctx->module->func_bindings, &new_name);
  CHECK_ALLOC_NULL(binding);
  binding->index = index;

  WasmFunc* func = ctx->module->funcs.data[index];
  func->name = new_name;
  return WASM_OK;
}

static WasmResult on_local_names_count(uint32_t index,
                                       uint32_t count,
                                       void* user_data) {
  WasmContext* ctx = user_data;
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

static WasmResult on_local_name(uint32_t func_index,
                                uint32_t local_index,
                                WasmStringSlice name,
                                void* user_data) {
  WasmContext* ctx = user_data;
  WasmModule* module = ctx->module;
  WasmFunc* func = module->funcs.data[func_index];
  uint32_t num_params = wasm_get_num_params(func);
  WasmStringSlice new_name;
  CHECK_RESULT(dup_name(ctx, &name, &new_name));
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
  CHECK_ALLOC_NULL(binding);
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
                                const WasmReadBinaryOptions* options,
                                WasmBinaryErrorHandler* error_handler,
                                struct WasmModule* out_module) {
  WasmContext ctx;
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
  wasm_destroy_label_node_vector(allocator, &ctx.label_stack);
  wasm_destroy_expr_ptr_vector(allocator, &ctx.expr_stack);
  return result;
}

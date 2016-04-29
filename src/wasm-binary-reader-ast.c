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

#define CHECK_ALLOC_(ctx, cond)                                           \
  do {                                                                    \
    if (!(cond)) {                                                        \
      print_error((ctx), "%s:%d: allocation failed", __FILE__, __LINE__); \
      return WASM_ERROR;                                                  \
    }                                                                     \
  } while (0)

#define CHECK_ALLOC(ctx, e) CHECK_ALLOC_((ctx), WASM_SUCCEEDED(e))
#define CHECK_ALLOC_NULL(ctx, v) CHECK_ALLOC_((ctx), (v))
#define CHECK_ALLOC_NULL_STR(ctx, v) CHECK_ALLOC_((ctx), (v).start)

#define CHECK_RESULT(expr) \
  if (WASM_FAILED(expr))   \
    return WASM_ERROR;     \
  else                     \
  (void)0

#define CHECK_DEPTH(ctx, depth)                                       \
  if ((depth) >= (ctx)->depth_stack.size) {                           \
    print_error((ctx), "invalid depth: %d (max %" PRIzd ")", (depth), \
                ((ctx)->depth_stack.size));                           \
    return WASM_ERROR;                                                \
  } else                                                              \
  (void)0

#define CHECK_LOCAL(ctx, local_index)                                       \
  do {                                                                      \
    assert(wasm_decl_has_func_type(&(ctx)->current_func->decl));            \
    uint32_t max_local_index =                                              \
        wasm_get_num_params_and_locals((ctx)->current_func);                \
    if ((local_index) >= max_local_index) {                                 \
      print_error((ctx), "invalid local_index: %d (max %d)", (local_index), \
                  max_local_index);                                         \
      return WASM_ERROR;                                                    \
    }                                                                       \
  } while (0)

typedef struct WasmExprNode {
  WasmExpr* expr;
  uint32_t index;
  uint32_t total;
} WasmExprNode;
WASM_DEFINE_VECTOR(expr_node, WasmExprNode);

typedef uint32_t WasmUint32;
WASM_DEFINE_VECTOR(uint32, WasmUint32);

typedef struct WasmContext {
  WasmAllocator* allocator;
  WasmBinaryErrorHandler* error_handler;
  WasmModule* module;

  WasmFunc* current_func;
  WasmExprNodeVector expr_stack;

  /* TODO(binji): remove all this if-block stuff when we switch to postorder */
  WasmUint32Vector depth_stack;
  uint32_t depth;
  /* currently there is a discrepancy between the wast format and the binary
   * format w.r.t. whether the if branches introduce new blocks. As a result,
   * if we see an if or if_else operator in the binary format that does not
   * have a block as the branch expression, we'll have to introduce a fake
   * block so that the break depths are correct for any nested br operators.
   *
   * |just_pushed_fake_depth| works as follows: we pre-emptively assume that an
   * expression does not have a nested block, and introduce a fake block. If
   * the next expression we see is a block, then we reverse that and add the
   * real block instead. */
  WasmBool just_pushed_fake_depth;
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
    CHECK_ALLOC_NULL_STR(ctx, *out_name);
  } else {
    WASM_ZERO_MEMORY(*out_name);
  }
  return WASM_OK;
}

/* TODO(binji): remove all this if-block stuff when we switch to postorder */
static WasmResult push_depth(WasmContext* ctx) {
  uint32_t* depth = wasm_append_uint32(ctx->allocator, &ctx->depth_stack);
  CHECK_ALLOC_NULL(ctx, depth);
  *depth = ctx->depth;
  ctx->depth++;
  return WASM_OK;
}

static void pop_depth(WasmContext* ctx) {
  ctx->depth--;
  ctx->depth_stack.size--;
}

static void push_fake_depth(WasmContext* ctx) {
  ctx->depth++;
  ctx->just_pushed_fake_depth = WASM_TRUE;
}

static void pop_fake_depth(WasmContext* ctx) {
  ctx->depth--;
}

static void pop_fake_depth_unless_block(WasmContext* ctx, WasmExpr* expr) {
  if (expr->type != WASM_EXPR_TYPE_BLOCK)
    pop_fake_depth(ctx);
}

static uint32_t translate_depth(WasmContext* ctx, uint32_t depth) {
  uint32_t index = ctx->depth_stack.size - depth - 1;
  assert(index < ctx->depth_stack.size);
  return ctx->depth - ctx->depth_stack.data[index] - 1;
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
  CHECK_ALLOC_NULL(ctx, field);
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
    CHECK_ALLOC_NULL(ctx, field);
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
  CHECK_ALLOC(ctx, wasm_reserve_segments(
                       ctx->allocator, &ctx->module->memory->segments, count));
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
  CHECK_ALLOC_NULL(ctx, segment);
  segment->addr = address;
  segment->data = wasm_alloc(ctx->allocator, size, WASM_DEFAULT_ALIGN);
  segment->size = size;
  memcpy(segment->data, data, size);
  return WASM_OK;
}

static WasmResult on_signature_count(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_ALLOC(ctx, wasm_reserve_func_type_ptrs(
                       ctx->allocator, &ctx->module->func_types, count));
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
  CHECK_ALLOC_NULL(ctx, field);
  field->type = WASM_MODULE_FIELD_TYPE_FUNC_TYPE;

  WasmFuncType* func_type = &field->func_type;
  WASM_ZERO_MEMORY(*func_type);
  func_type->sig.result_type = result_type;

  CHECK_ALLOC(ctx,
              wasm_reserve_types(ctx->allocator, &func_type->sig.param_types,
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
  CHECK_ALLOC(ctx, wasm_reserve_import_ptrs(ctx->allocator,
                                            &ctx->module->imports, count));
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
  CHECK_ALLOC_NULL(ctx, field);
  field->type = WASM_MODULE_FIELD_TYPE_IMPORT;

  WasmImport* import = &field->import;
  WASM_ZERO_MEMORY(*import);
  import->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
  CHECK_ALLOC_NULL_STR(ctx, import->module_name = wasm_dup_string_slice(
                                ctx->allocator, module_name));
  CHECK_ALLOC_NULL_STR(ctx, import->func_name = wasm_dup_string_slice(
                                ctx->allocator, function_name));
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
      ctx, wasm_reserve_func_ptrs(ctx->allocator, &ctx->module->funcs, count));
  return WASM_OK;
}

static WasmResult on_function_signature(uint32_t index,
                                        uint32_t sig_index,
                                        void* user_data) {
  WasmContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(ctx, field);
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
    CHECK_ALLOC(
        ctx, wasm_append_type_value(ctx->allocator, &func->decl.sig.param_types,
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
  if (ctx->expr_stack.size != 0) {
    print_error(ctx,
                "expression stack not empty on function exit! %" PRIzd " items",
                ctx->expr_stack.size);
    return WASM_ERROR;
  }
  ctx->current_func = NULL;
  return WASM_OK;
}

static WasmResult on_local_decl(uint32_t decl_index,
                                uint32_t count,
                                WasmType type,
                                void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);
  size_t old_local_count = ctx->current_func->local_types.size;
  size_t new_local_count = old_local_count + count;
  CHECK_ALLOC(
      ctx, wasm_reserve_types(ctx->allocator, &ctx->current_func->local_types,
                              new_local_count));
  WASM_STATIC_ASSERT(sizeof(WasmType) == sizeof(uint8_t));
  WasmTypeVector* types = &ctx->current_func->local_types;
  memset(&types->data[old_local_count], type, count);
  types->size = new_local_count;
  return WASM_OK;
}

static WasmResult reduce(WasmContext* ctx, WasmExpr* expr) {
  ctx->just_pushed_fake_depth = WASM_FALSE;
  WasmBool done = WASM_FALSE;
  while (!done) {
    done = WASM_TRUE;
    if (ctx->expr_stack.size == 0) {
      LOGF("reduce: <- %u\n", expr->type);

      WasmExprPtr* expr_ptr =
          wasm_append_expr_ptr(ctx->allocator, &ctx->current_func->exprs);
      CHECK_ALLOC_NULL(ctx, expr_ptr);
      *expr_ptr = expr;
    } else {
      WasmExprNode* top = &ctx->expr_stack.data[ctx->expr_stack.size - 1];
      assert(top->index < top->total);

      LOGF("reduce: %d(%d/%d) <- %d\n", top->expr->type, top->index,
              top->total, expr->type);

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
          CHECK_ALLOC_NULL(ctx, expr_ptr);
          *expr_ptr = expr;

          /* TODO(binji): remove after post order, only needed for if-block */
          if (top->index + 1 == top->total)
            pop_depth(ctx);
          break;
        }

        case WASM_EXPR_TYPE_BR:
          top->expr->br.expr = expr;
          break;

        case WASM_EXPR_TYPE_BR_IF:
          if (top->index == 0)
            top->expr->br_if.expr = expr;
          else
            top->expr->br_if.cond = expr;
          break;

        case WASM_EXPR_TYPE_BR_TABLE:
          top->expr->br_table.expr = expr;
          break;

        case WASM_EXPR_TYPE_CALL:
        case WASM_EXPR_TYPE_CALL_IMPORT: {
          WasmExprPtr* expr_ptr =
              wasm_append_expr_ptr(ctx->allocator, &top->expr->call.args);
          CHECK_ALLOC_NULL(ctx, expr_ptr);
          *expr_ptr = expr;
          break;
        }

        case WASM_EXPR_TYPE_CALL_INDIRECT: {
          if (top->index == 0) {
            top->expr->call_indirect.expr = expr;
          } else {
            WasmExprPtr* expr_ptr = wasm_append_expr_ptr(
                ctx->allocator, &top->expr->call_indirect.args);
            CHECK_ALLOC_NULL(ctx, expr_ptr);
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
            push_fake_depth(ctx);
          } else {
            top->expr->if_.true_ = expr;
            pop_fake_depth_unless_block(ctx, expr);
          }
          break;

        case WASM_EXPR_TYPE_IF_ELSE:
          if (top->index == 0) {
            top->expr->if_else.cond = expr;
            push_fake_depth(ctx);
          } else if (top->index == 1) {
            top->expr->if_else.true_ = expr;
            pop_fake_depth_unless_block(ctx, expr);
            push_fake_depth(ctx);
          } else {
            top->expr->if_else.false_ = expr;
            pop_fake_depth_unless_block(ctx, expr);
          }
          break;

        case WASM_EXPR_TYPE_LOAD:
          top->expr->load.addr = expr;
          break;

        case WASM_EXPR_TYPE_LOOP: {
          WasmExprPtr* expr_ptr =
              wasm_append_expr_ptr(ctx->allocator, &top->expr->loop.exprs);
          CHECK_ALLOC_NULL(ctx, expr_ptr);
          *expr_ptr = expr;

          /* TODO(binji): remove after post order, only needed for if-block */
          if (top->index + 1 == top->total) {
            pop_depth(ctx);
            pop_depth(ctx);
          }
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
        done = WASM_FALSE;
      }
    }
  }

  return WASM_OK;
}

static WasmResult shift(WasmContext* ctx, WasmExpr* expr, uint32_t count) {
  ctx->just_pushed_fake_depth = WASM_FALSE;
  if (count > 0) {
    LOGF("shift: %d %u\n", expr->type, count);
    WasmExprNode* node =
        wasm_append_expr_node(ctx->allocator, &ctx->expr_stack);
    CHECK_ALLOC_NULL(ctx, node);
    node->expr = expr;
    node->index = 0;
    node->total = count;
    return WASM_OK;
  } else {
    return reduce(ctx, expr);
  }
}

static WasmResult on_binary_expr(WasmOpcode opcode, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_binary_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->binary.opcode = opcode;
  return shift(ctx, expr, 2);
}

static WasmResult on_block_expr(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  if (ctx->just_pushed_fake_depth)
    pop_fake_depth(ctx);

  CHECK_RESULT(push_depth(ctx));

  WasmExpr* expr = wasm_new_block_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  return shift(ctx, expr, count);
}

static WasmResult on_br_expr(uint32_t depth, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_br_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->br.var.type = WASM_VAR_TYPE_INDEX;
  CHECK_DEPTH(ctx, depth);
  expr->br.var.index = translate_depth(ctx, depth);
  return shift(ctx, expr, 1);
}

static WasmResult on_br_if_expr(uint32_t depth, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_br_if_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->br_if.var.type = WASM_VAR_TYPE_INDEX;
  CHECK_DEPTH(ctx, depth);
  expr->br_if.var.index = translate_depth(ctx, depth);
  return shift(ctx, expr, 2);
}

static WasmResult on_br_table_expr(uint32_t num_targets,
                                   uint32_t* target_depths,
                                   uint32_t default_target_depth,
                                   void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_br_table_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  CHECK_ALLOC(ctx, wasm_reserve_vars(ctx->allocator, &expr->br_table.targets,
                                     num_targets));
  expr->br_table.targets.size = num_targets;
  uint32_t i;
  for (i = 0; i < num_targets; ++i) {
    WasmVar* var = &expr->br_table.targets.data[i];
    var->type = WASM_VAR_TYPE_INDEX;
    CHECK_DEPTH(ctx, target_depths[i]);
    var->index = translate_depth(ctx, target_depths[i]);
  }
  expr->br_table.default_target.type = WASM_VAR_TYPE_INDEX;
  CHECK_DEPTH(ctx, default_target_depth);
  expr->br_table.default_target.index =
      translate_depth(ctx, default_target_depth);
  return shift(ctx, expr, 1);
}

static WasmResult on_call_expr(uint32_t func_index, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);
  assert(func_index < ctx->module->funcs.size);
  WasmFunc* func = ctx->module->funcs.data[func_index];
  uint32_t sig_index = func->decl.type_var.index;
  assert(sig_index < ctx->module->func_types.size);
  WasmFuncType* func_type = ctx->module->func_types.data[sig_index];

  WasmExpr* expr = wasm_new_call_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->call.var.type = WASM_VAR_TYPE_INDEX;
  expr->call.var.index = func_index;
  uint32_t num_params = func_type->sig.param_types.size;
  return shift(ctx, expr, num_params);
}

static WasmResult on_call_import_expr(uint32_t import_index, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);
  assert(import_index < ctx->module->imports.size);
  WasmImport* import = ctx->module->imports.data[import_index];
  uint32_t sig_index = import->decl.type_var.index;
  assert(sig_index < ctx->module->func_types.size);
  WasmFuncType* func_type = ctx->module->func_types.data[sig_index];

  WasmExpr* expr = wasm_new_call_import_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->call.var.type = WASM_VAR_TYPE_INDEX;
  expr->call.var.index = import_index;
  uint32_t num_params = func_type->sig.param_types.size;
  return shift(ctx, expr, num_params);
}

static WasmResult on_call_indirect_expr(uint32_t sig_index, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);
  assert(sig_index < ctx->module->func_types.size);
  WasmFuncType* func_type = ctx->module->func_types.data[sig_index];

  WasmExpr* expr = wasm_new_call_indirect_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->call_indirect.var.type = WASM_VAR_TYPE_INDEX;
  expr->call_indirect.var.index = sig_index;
  uint32_t num_params = func_type->sig.param_types.size;
  return shift(ctx, expr, num_params + 1);
}

static WasmResult on_compare_expr(WasmOpcode opcode, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_compare_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->compare.opcode = opcode;
  return shift(ctx, expr, 2);
}

static WasmResult on_i32_const_expr(uint32_t value, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_const_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->const_.type = WASM_TYPE_I32;
  expr->const_.u32 = value;
  return reduce(ctx, expr);
}

static WasmResult on_i64_const_expr(uint64_t value, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_const_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->const_.type = WASM_TYPE_I64;
  expr->const_.u64 = value;
  return reduce(ctx, expr);
}

static WasmResult on_f32_const_expr(uint32_t value_bits, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_const_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->const_.type = WASM_TYPE_F32;
  expr->const_.f32_bits = value_bits;
  return reduce(ctx, expr);
}

static WasmResult on_f64_const_expr(uint64_t value_bits, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_const_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->const_.type = WASM_TYPE_F64;
  expr->const_.f64_bits = value_bits;
  return reduce(ctx, expr);
}

static WasmResult on_convert_expr(WasmOpcode opcode, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_convert_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->convert.opcode = opcode;
  return shift(ctx, expr, 1);
}

static WasmResult on_get_local_expr(uint32_t local_index, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_get_local_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->get_local.var.type = WASM_VAR_TYPE_INDEX;
  expr->get_local.var.index = local_index;
  CHECK_LOCAL(ctx, local_index);
  return reduce(ctx, expr);
}

static WasmResult on_grow_memory_expr(void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_grow_memory_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  return shift(ctx, expr, 1);
}

static WasmResult on_if_expr(void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_if_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  return shift(ctx, expr, 2);
}

static WasmResult on_if_else_expr(void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_if_else_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  return shift(ctx, expr, 3);
}

static WasmResult on_load_expr(WasmOpcode opcode,
                               uint32_t alignment_log2,
                               uint32_t offset,
                               void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_load_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->load.opcode = opcode;
  expr->load.align = 1 << alignment_log2;
  expr->load.offset = offset;
  return shift(ctx, expr, 1);
}

static WasmResult on_loop_expr(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  CHECK_RESULT(push_depth(ctx));
  CHECK_RESULT(push_depth(ctx));

  WasmExpr* expr = wasm_new_loop_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  return shift(ctx, expr, count);
}

static WasmResult on_memory_size_expr(void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr =
      wasm_new_empty_expr(ctx->allocator, WASM_EXPR_TYPE_MEMORY_SIZE);
  CHECK_ALLOC_NULL(ctx, expr);
  return reduce(ctx, expr);
}

static WasmResult on_nop_expr(void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_empty_expr(ctx->allocator, WASM_EXPR_TYPE_NOP);
  CHECK_ALLOC_NULL(ctx, expr);
  return reduce(ctx, expr);
}

static WasmResult on_return_expr(void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  uint32_t sig_index = ctx->current_func->decl.type_var.index;
  assert(sig_index < ctx->module->func_types.size);
  WasmFuncType* func_type = ctx->module->func_types.data[sig_index];

  WasmExpr* expr = wasm_new_return_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  return shift(ctx, expr, func_type->sig.result_type == WASM_TYPE_VOID ? 0 : 1);
}

static WasmResult on_select_expr(void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_select_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  return shift(ctx, expr, 3);
}

static WasmResult on_set_local_expr(uint32_t local_index, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_set_local_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->set_local.var.type = WASM_VAR_TYPE_INDEX;
  expr->set_local.var.index = local_index;
  CHECK_LOCAL(ctx, local_index);
  return shift(ctx, expr, 1);
}

static WasmResult on_store_expr(WasmOpcode opcode,
                                uint32_t alignment_log2,
                                uint32_t offset,
                                void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_store_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->store.opcode = opcode;
  expr->store.align = 1 << alignment_log2;
  expr->store.offset = offset;
  return shift(ctx, expr, 2);
}

static WasmResult on_unary_expr(WasmOpcode opcode, void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr = wasm_new_unary_expr(ctx->allocator);
  CHECK_ALLOC_NULL(ctx, expr);
  expr->unary.opcode = opcode;
  return shift(ctx, expr, 1);
}

static WasmResult on_unreachable_expr(void* user_data) {
  WasmContext* ctx = user_data;
  assert(ctx->current_func);

  WasmExpr* expr =
      wasm_new_empty_expr(ctx->allocator, WASM_EXPR_TYPE_UNREACHABLE);
  CHECK_ALLOC_NULL(ctx, expr);
  return reduce(ctx, expr);
}

static WasmResult on_function_table_count(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(ctx, field);
  field->type = WASM_MODULE_FIELD_TYPE_TABLE;

  assert(ctx->module->table == NULL);
  ctx->module->table = &field->table;

  CHECK_ALLOC(ctx, wasm_reserve_vars(ctx->allocator, &field->table, count));
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
  CHECK_ALLOC_NULL(ctx, field);
  field->type = WASM_MODULE_FIELD_TYPE_START;

  field->start.type = WASM_VAR_TYPE_INDEX;
  assert(func_index < ctx->module->funcs.size);
  field->start.index = func_index;

  ctx->module->start = &field->start;
  return WASM_OK;
}

static WasmResult on_export_count(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_ALLOC(ctx, wasm_reserve_export_ptrs(ctx->allocator,
                                            &ctx->module->exports, count));
  return WASM_OK;
}

static WasmResult on_export(uint32_t index,
                            uint32_t func_index,
                            WasmStringSlice name,
                            void* user_data) {
  WasmContext* ctx = user_data;
  WasmModuleField* field =
      wasm_append_module_field(ctx->allocator, ctx->module);
  CHECK_ALLOC_NULL(ctx, field);
  field->type = WASM_MODULE_FIELD_TYPE_EXPORT;

  WasmExport* export = &field->export_;
  WASM_ZERO_MEMORY(*export);
  CHECK_ALLOC_NULL_STR(
      ctx, export->name = wasm_dup_string_slice(ctx->allocator, name));
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
  CHECK_ALLOC_NULL(ctx, binding);
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
  CHECK_ALLOC_NULL(ctx, binding);
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

  WasmResult result = wasm_read_binary(allocator, data, size, &reader, options);
  wasm_destroy_expr_node_vector(allocator, &ctx.expr_stack);
  wasm_destroy_uint32_vector(allocator, &ctx.depth_stack);
  return result;
}

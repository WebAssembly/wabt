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

#include "wasm-check.h"

#include <alloca.h>
#include <assert.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>

#include "wasm-allocator.h"
#include "wasm-internal.h"

static const char* s_type_names[] = {
    "void",
    "i32",
    "i64",
    "f32",
    "f64",
};
STATIC_ASSERT(ARRAY_SIZE(s_type_names) == WASM_NUM_TYPES);

typedef enum WasmTypeSet {
  WASM_TYPE_SET_VOID = 0,
  WASM_TYPE_SET_I32 = 1 << 0,
  WASM_TYPE_SET_I64 = 1 << 1,
  WASM_TYPE_SET_F32 = 1 << 2,
  WASM_TYPE_SET_F64 = 1 << 3,
  WASM_TYPE_SET_ALL = (1 << 4) - 1,
} WasmTypeSet;

static const char* s_type_set_names[] = {
    "void",
    "i32",
    "i64",
    "i32 or i64",
    "f32",
    "i32 or f32",
    "i64 or f32",
    "i32, i64 or f32",
    "f64",
    "i32 or f64",
    "i64 or f64",
    "i32, i64 or f64",
    "f32 or f64",
    "i32, f32 or f64",
    "i64, f32 or f64",
    "i32, i64, f32 or f64",
};
STATIC_ASSERT(ARRAY_SIZE(s_type_set_names) == WASM_TYPE_SET_ALL + 1);
STATIC_ASSERT((1 << (WASM_TYPE_I32 - 1)) == WASM_TYPE_SET_I32);
STATIC_ASSERT((1 << (WASM_TYPE_I64 - 1)) == WASM_TYPE_SET_I64);
STATIC_ASSERT((1 << (WASM_TYPE_F32 - 1)) == WASM_TYPE_SET_F32);
STATIC_ASSERT((1 << (WASM_TYPE_F64 - 1)) == WASM_TYPE_SET_F64);

#define TYPE_TO_TYPE_SET(type) (1 << ((type) - 1))

typedef struct WasmLabelNode {
  WasmLabel* label;
  WasmType expected_type;
  const char* desc;
  struct WasmLabelNode* next;
} WasmLabelNode;

typedef struct WasmCheckContext {
  WasmAllocator* allocator;
  WasmLexer lexer;
  WasmModule* last_module;
  WasmLabelNode* top_label;
  int in_assert_invalid;
  int max_depth;
} WasmCheckContext;

static void print_error(WasmCheckContext* ctx,
                        WasmLocation* loc,
                        const char* fmt,
                        ...) {
  FILE* out = stderr;
  if (ctx->in_assert_invalid) {
    out = stdout;
    fprintf(out, "assert_invalid error:\n  ");
  }

  va_list args;
  va_start(args, fmt);
  wasm_vfprint_error(out, loc, ctx->lexer, fmt, args);
  va_end(args);
}

static int is_power_of_two(uint32_t x) {
  return x && ((x & (x - 1)) == 0);
}

static WasmResult check_duplicate_bindings(WasmCheckContext* ctx,
                                           WasmBindingHash* bindings,
                                           const char* desc) {
  WasmResult result = WASM_OK;
  int i;
  for (i = 0; i < bindings->entries.capacity; ++i) {
    WasmBindingHashEntry* entry = &bindings->entries.data[i];
    if (wasm_hash_entry_is_free(entry))
      continue;

    /* only follow the chain if this is the first entry in the chain */
    if (entry->prev != NULL)
      continue;

    WasmBindingHashEntry* a = entry;
    for (; a; a = a->next) {
      WasmBindingHashEntry* b = a->next;
      for (; b; b = b->next) {
        if (wasm_string_slices_are_equal(&a->binding.name, &b->binding.name)) {
          /* choose the location that is later in the file */
          WasmLocation* a_loc = &a->binding.loc;
          WasmLocation* b_loc = &b->binding.loc;
          WasmLocation* loc = a_loc->line > b_loc->line ? a_loc : b_loc;
          print_error(ctx, loc, "redefinition of %s \"%.*s\"", desc,
                      a->binding.name.length, a->binding.name.start);
          result = WASM_ERROR;
        }
      }
    }
  }
  return result;
}

static WasmResult check_var(WasmCheckContext* ctx,
                            WasmBindingHash* bindings,
                            int max_index,
                            WasmVar* var,
                            const char* desc,
                            int* out_index) {
  int index = wasm_get_index_from_var(bindings, var);
  if (index >= 0 && index < max_index) {
    if (out_index)
      *out_index = index;
    return WASM_OK;
  }
  if (var->type == WASM_VAR_TYPE_NAME)
    print_error(ctx, &var->loc, "undefined %s variable \"%.*s\"", desc,
                var->name.length, var->name.start);
  else
    print_error(ctx, &var->loc, "%s variable out of range (max %d)", desc,
                max_index);
  return WASM_ERROR;
}

static WasmResult check_func_var(WasmCheckContext* ctx,
                                 WasmModule* module,
                                 WasmVar* var,
                                 WasmFunc** out_func) {
  int index;
  if (check_var(ctx, &module->func_bindings, module->funcs.size, var,
                "function", &index) != WASM_OK)
    return WASM_ERROR;

  if (out_func)
    *out_func = module->funcs.data[index];
  return WASM_OK;
}

static WasmResult check_import_var(WasmCheckContext* ctx,
                                   WasmModule* module,
                                   WasmVar* var,
                                   WasmImport** out_import) {
  int index;
  if (check_var(ctx, &module->import_bindings, module->imports.size, var,
                "import", &index) != WASM_OK)
    return WASM_ERROR;

  if (out_import)
    *out_import = module->imports.data[index];
  return WASM_OK;
}

static WasmResult check_func_type_var(WasmCheckContext* ctx,
                                      WasmModule* module,
                                      WasmVar* var,
                                      WasmFuncType** out_func_type) {
  int index;
  if (check_var(ctx, &module->func_type_bindings, module->func_types.size, var,
                "function type", &index) != WASM_OK)
    return WASM_ERROR;

  if (out_func_type)
    *out_func_type = module->func_types.data[index];
  return WASM_OK;
}

static WasmResult check_local_var(WasmCheckContext* ctx,
                                  WasmFunc* func,
                                  WasmVar* var,
                                  WasmType* out_type) {
  int index;
  if (check_var(ctx, &func->params_and_locals.bindings,
                func->params_and_locals.types.size, var, "local",
                &index) != WASM_OK)
    return WASM_ERROR;

  if (out_type)
    *out_type = func->params_and_locals.types.data[index];
  return WASM_OK;
}

static WasmResult check_align(WasmCheckContext* ctx,
                              WasmLocation* loc,
                              uint32_t alignment) {
  if (alignment != WASM_USE_NATURAL_ALIGNMENT && !is_power_of_two(alignment)) {
    print_error(ctx, loc, "alignment must be power-of-two");
    return WASM_ERROR;
  }
  return WASM_OK;
}

static WasmResult check_offset(WasmCheckContext* ctx,
                               WasmLocation* loc,
                               uint64_t offset) {
  if (offset > UINT32_MAX) {
    print_error(ctx, loc, "offset must be less than or equal to 0xffffffff");
    return WASM_ERROR;
  }
  return WASM_OK;
}

static WasmResult check_type(WasmCheckContext* ctx,
                             WasmLocation* loc,
                             WasmType actual,
                             WasmType expected,
                             const char* desc) {
  if (expected != WASM_TYPE_VOID && actual != expected) {
    print_error(ctx, loc, "type mismatch%s. got %s, expected %s", desc,
                s_type_names[actual], s_type_names[expected]);
    return WASM_ERROR;
  }
  return WASM_OK;
}

static WasmResult check_type_set(WasmCheckContext* ctx,
                                 WasmLocation* loc,
                                 WasmType actual,
                                 WasmTypeSet expected,
                                 const char* desc) {
  if (expected != WASM_TYPE_SET_VOID &&
      (TYPE_TO_TYPE_SET(actual) & expected) == 0) {
    print_error(ctx, loc, "type mismatch%s. got %s, expected %s", desc,
                s_type_names[actual], s_type_set_names[expected]);
    return WASM_ERROR;
  }
  return WASM_OK;
}


static WasmResult check_type_exact(WasmCheckContext* ctx,
                                   WasmLocation* loc,
                                   WasmType actual,
                                   WasmType expected,
                                   const char* desc) {
  if (actual != expected) {
    print_error(ctx, loc, "type mismatch%s. got %s, expected %s", desc,
                s_type_names[actual], s_type_names[expected]);
    return WASM_ERROR;
  }
  return WASM_OK;
}

static WasmResult check_type_arg_exact(WasmCheckContext* ctx,
                                       WasmLocation* loc,
                                       WasmType actual,
                                       WasmType expected,
                                       const char* desc,
                                       int arg_index) {
  if (actual != expected) {
    print_error(ctx, loc,
                "type mismatch for argument %d of %s. got %s, expected %s",
                arg_index, desc, s_type_names[actual], s_type_names[expected]);
    return WASM_ERROR;
  }
  return WASM_OK;
}

static WasmLabelNode* find_label_by_name(WasmLabelNode* top_label,
                                         WasmStringSlice* name) {
  WasmLabelNode* node = top_label;
  while (node) {
    if (wasm_string_slices_are_equal(node->label, name))
      return node;
    node = node->next;
  }
  return NULL;
}

static WasmLabelNode* find_label_by_var(WasmLabelNode* top_label,
                                        WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME)
    return find_label_by_name(top_label, &var->name);

  WasmLabelNode* node = top_label;
  int i = 0;
  while (node && i != var->index) {
    node = node->next;
    i++;
  }
  return node;
}

static WasmResult check_label_var(WasmCheckContext* ctx,
                                  WasmLabelNode* top_label,
                                  WasmVar* var,
                                  WasmLabelNode** out_node) {
  WasmLabelNode* node = find_label_by_var(top_label, var);
  if (node) {
    if (out_node)
      *out_node = node;
    return WASM_OK;
  }
  if (var->type == WASM_VAR_TYPE_NAME)
    print_error(ctx, &var->loc, "undefined label variable \"%.*s\"",
                var->name.length, var->name.start);
  else
    print_error(ctx, &var->loc, "label variable out of range (max %d)",
                ctx->max_depth);
  return WASM_ERROR;
}

static WasmResult push_label(WasmCheckContext* ctx,
                             WasmLocation* loc,
                             WasmLabelNode* node,
                             WasmLabel* label,
                             WasmType expected_type,
                             const char* desc) {
  WasmResult result = WASM_OK;
  node->label = label;
  node->next = ctx->top_label;
  node->expected_type = expected_type;
  node->desc = desc;
  ctx->top_label = node;
  ctx->max_depth++;
  return result;
}

static void pop_label(WasmCheckContext* ctx) {
  ctx->max_depth--;
  ctx->top_label = ctx->top_label->next;
}

static WasmResult check_expr(WasmCheckContext* ctx,
                             WasmModule* module,
                             WasmFunc* func,
                             WasmExpr* expr,
                             WasmType expected_type,
                             const char* desc);

static WasmResult check_br(WasmCheckContext* ctx,
                           WasmLocation* loc,
                           WasmModule* module,
                           WasmFunc* func,
                           WasmVar* var,
                           WasmExpr* expr,
                           const char* desc) {
  WasmLabelNode* node;
  if (check_label_var(ctx, ctx->top_label, var, &node) != WASM_OK)
    return WASM_ERROR;

  if (node->expected_type != WASM_TYPE_VOID) {
    if (expr) {
      return check_expr(ctx, module, func, expr, node->expected_type, desc);
    } else {
      print_error(
          ctx, loc,
          "arity mismatch%s. label expects non-void, but br value is empty",
          desc);
      return WASM_ERROR;
    }
  } else if (expr) {
    print_error(
        ctx, loc,
        "arity mismatch%s. label expects void, but br value is non-empty",
        desc);
    return WASM_ERROR;
  } else {
    return WASM_OK;
  }
}

static WasmResult check_call(WasmCheckContext* ctx,
                             WasmLocation* loc,
                             WasmModule* module,
                             WasmFunc* func,
                             WasmTypeVector* param_types,
                             WasmType result_type,
                             WasmExprPtrVector* args,
                             WasmType expected_type,
                             const char* desc) {
  WasmResult result = WASM_OK;
  int actual_args = args->size;
  int expected_args = param_types->size;
  if (expected_args == actual_args) {
    char buffer[100];
    snprintf(buffer, 100, " of %s result", desc);
    result |= check_type(ctx, loc, result_type, expected_type, buffer);
    int i;
    for (i = 0; i < actual_args; ++i) {
      snprintf(buffer, 100, " of argument %d of %s", i, desc);
      result |= check_expr(ctx, module, func, args->data[i],
                           param_types->data[i], buffer);
    }
  } else {
    char* func_name = "";
    if (func->name.start) {
      size_t length = func->name.length + 10;
      func_name = alloca(length);
      snprintf(func_name, length, " \"%.*s\"", (int)func->name.length,
               func->name.start);
    }
    print_error(ctx, loc,
                "too %s parameters to function%s in %s. got %d, expected %d",
                actual_args > expected_args ? "many" : "few", func_name, desc,
                actual_args, expected_args);
    result = WASM_ERROR;
  }
  return result;
}

static WasmResult check_target(WasmCheckContext* ctx,
                               WasmBindingHash* bindings,
                               int num_cases,
                               WasmTarget* target) {
  switch (target->type) {
    case WASM_TARGET_TYPE_CASE:
      return check_var(ctx, bindings, num_cases, &target->var, "case", NULL);
    case WASM_TARGET_TYPE_BR:
      return check_label_var(ctx, ctx->top_label, &target->var, NULL);
    default:
      return WASM_ERROR;
  }
}

static WasmResult check_exprs(WasmCheckContext* ctx,
                              WasmModule* module,
                              WasmFunc* func,
                              WasmExprPtrVector* exprs,
                              WasmType expected_type,
                              const char* desc) {
  WasmResult result = WASM_OK;
  if (exprs->size > 0) {
    int i;
    WasmExprPtr* expr = &exprs->data[0];
    for (i = 0; i < exprs->size - 1; ++i, ++expr)
      result |= check_expr(ctx, module, func, *expr, WASM_TYPE_VOID, "");
    result |= check_expr(ctx, module, func, *expr, expected_type, desc);
  }
  return result;
}

static WasmResult check_expr(WasmCheckContext* ctx,
                             WasmModule* module,
                             WasmFunc* func,
                             WasmExpr* expr,
                             WasmType expected_type,
                             const char* desc) {
  WasmResult result = WASM_OK;
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY: {
      WasmType type = expr->binary.op.type;
      result |= check_type(ctx, &expr->loc, type, expected_type, desc);
      result |= check_expr(ctx, module, func, expr->binary.left, type,
                           " of argument 0 of binary op");
      result |= check_expr(ctx, module, func, expr->binary.right, type,
                           " of argument 1 of binary op");
      break;
    }
    case WASM_EXPR_TYPE_BLOCK: {
      WasmLabelNode node;
      result |= push_label(ctx, &expr->loc, &node, &expr->block.label,
                           expected_type, "block");
      result |= check_exprs(ctx, module, func, &expr->block.exprs,
                            expected_type, " of block");
      pop_label(ctx);
      break;
    }
    case WASM_EXPR_TYPE_BR: {
      result |= check_br(ctx, &expr->loc, module, func, &expr->br.var,
                         expr->br.expr, " of br value");
      break;
    }
    case WASM_EXPR_TYPE_BR_IF: {
      result |=
          check_type(ctx, &expr->loc, WASM_TYPE_VOID, expected_type, desc);
      result |= check_br(ctx, &expr->loc, module, func, &expr->br_if.var,
                         expr->br_if.expr, " of br_if value");
      result |= check_expr(ctx, module, func, expr->br_if.cond, WASM_TYPE_I32,
                           " of br_if condition");
      break;
    }
    case WASM_EXPR_TYPE_CALL: {
      WasmFunc* callee;
      if (check_func_var(ctx, module, &expr->call.var, &callee) == WASM_OK) {
        result |= check_call(ctx, &expr->loc, module, func,
                             &callee->params.types, callee->result_type,
                             &expr->call.args, expected_type, "call");
      } else {
        result = WASM_ERROR;
      }
      break;
    }
    case WASM_EXPR_TYPE_CALL_IMPORT: {
      WasmImport* import;
      if (check_import_var(ctx, module, &expr->call.var, &import) == WASM_OK) {
        WasmFuncSignature* sig = NULL;
        if (import->import_type == WASM_IMPORT_HAS_TYPE) {
          WasmFuncType* func_type;
          if (check_func_type_var(ctx, module, &import->type_var, &func_type) ==
              WASM_OK) {
            sig = &func_type->sig;
          } else {
            result = WASM_ERROR;
          }
        } else {
          assert(import->import_type == WASM_IMPORT_HAS_FUNC_SIGNATURE);
          sig = &import->func_sig;
        }
        if (sig) {
          result |= check_call(ctx, &expr->loc, module, func, &sig->param_types,
                               sig->result_type, &expr->call.args,
                               expected_type, "call_import");
        }
      } else {
        result = WASM_ERROR;
      }
      break;
    }
    case WASM_EXPR_TYPE_CALL_INDIRECT: {
      result |= check_expr(ctx, module, func, expr->call_indirect.expr,
                           WASM_TYPE_I32, " of function index");
      WasmFuncType* func_type;
      if (check_func_type_var(ctx, module, &expr->call_indirect.var,
                              &func_type) == WASM_OK) {
        WasmFuncSignature* sig = &func_type->sig;
        result |= check_call(ctx, &expr->loc, module, func, &sig->param_types,
                             sig->result_type, &expr->call_indirect.args,
                             expected_type, "call_indirect");
      } else {
        result = WASM_ERROR;
      }
      break;
    }
    case WASM_EXPR_TYPE_COMPARE: {
      WasmType type = expr->convert.op.type;
      result |= check_type(ctx, &expr->loc, WASM_TYPE_I32, expected_type, desc);
      result |= check_expr(ctx, module, func, expr->compare.left, type,
                           " of argument 0 of compare op");
      result |= check_expr(ctx, module, func, expr->compare.right, type,
                           " of argument 1 of compare op");
      break;
    }
    case WASM_EXPR_TYPE_CONST:
      result |=
          check_type(ctx, &expr->loc, expr->const_.type, expected_type, desc);
      break;
    case WASM_EXPR_TYPE_CONVERT: {
      result |= check_type(ctx, &expr->loc, expr->convert.op.type,
                           expected_type, desc);
      result |= check_expr(ctx, module, func, expr->convert.expr,
                           expr->convert.op.type2, " of convert op");
      break;
    }
    case WASM_EXPR_TYPE_GET_LOCAL: {
      WasmType type;
      if (check_local_var(ctx, func, &expr->get_local.var, &type) == WASM_OK)
        result |= check_type(ctx, &expr->loc, type, expected_type, desc);
      else
        result = WASM_ERROR;
      break;
    }
    case WASM_EXPR_TYPE_GROW_MEMORY:
      result |= check_type(ctx, &expr->loc, WASM_TYPE_I32, expected_type,
                           " in grow_memory");
      result |= check_expr(ctx, module, func, expr->grow_memory.expr,
                           WASM_TYPE_I32, " of grow_memory");
      break;
    case WASM_EXPR_TYPE_IF:
      result |= check_expr(ctx, module, func, expr->if_.cond, WASM_TYPE_I32,
                           " of condition");
      WasmLabelNode node;
      result |= push_label(ctx, &expr->loc, &node, &expr->if_.true_.label,
                           expected_type, "block");
      result |= check_exprs(ctx, module, func, &expr->if_.true_.exprs,
                            expected_type, " of if branch");
      pop_label(ctx);
      result |=
          check_type(ctx, &expr->loc, WASM_TYPE_VOID, expected_type, desc);
      break;
    case WASM_EXPR_TYPE_IF_ELSE:
      result |= check_expr(ctx, module, func, expr->if_else.cond, WASM_TYPE_I32,
                           " of condition");
      result |= push_label(ctx, &expr->loc, &node, &expr->if_else.true_.label,
                           expected_type, "block");
      result |= check_exprs(ctx, module, func, &expr->if_else.true_.exprs,
                            expected_type, " of if branch");
      pop_label(ctx);
      result |= push_label(ctx, &expr->loc, &node, &expr->if_else.false_.label,
                           expected_type, "block");
      result |= check_exprs(ctx, module, func, &expr->if_else.false_.exprs,
                            expected_type, " of if branch");
      pop_label(ctx);
      break;
    case WASM_EXPR_TYPE_LOAD: {
      WasmType type = expr->load.op.type;
      result |= check_align(ctx, &expr->loc, expr->load.align);
      result |= check_offset(ctx, &expr->loc, expr->load.offset);
      result |= check_type(ctx, &expr->loc, type, expected_type, desc);
      result |= check_expr(ctx, module, func, expr->load.addr, WASM_TYPE_I32,
                           " of load index");
      break;
    }
    case WASM_EXPR_TYPE_LOOP: {
      WasmLabelNode outer_node;
      WasmLabelNode inner_node;
      result |= push_label(ctx, &expr->loc, &outer_node, &expr->loop.outer,
                           expected_type, "loop outer label");
      result |= push_label(ctx, &expr->loc, &inner_node, &expr->loop.inner,
                           WASM_TYPE_VOID, "loop inner label");
      result |= check_exprs(ctx, module, func, &expr->loop.exprs, expected_type,
                            " of loop");
      pop_label(ctx);
      pop_label(ctx);
      break;
    }
    case WASM_EXPR_TYPE_MEMORY_SIZE:
      result |= check_type(ctx, &expr->loc, WASM_TYPE_I32, expected_type,
                           " in memory_size");
      break;
    case WASM_EXPR_TYPE_NOP:
      result |=
          check_type(ctx, &expr->loc, WASM_TYPE_VOID, expected_type, " in nop");
      break;
    case WASM_EXPR_TYPE_RETURN:
      if (expr->return_.expr) {
        if (func->result_type == WASM_TYPE_VOID) {
          print_error(ctx, &expr->loc,
                      "arity mismatch of return. function expects void, but "
                      "return value is non-empty");
          result = WASM_ERROR;
        } else {
          result |= check_expr(ctx, module, func, expr->return_.expr,
                               func->result_type, " of return");
        }
      } else {
        result |= check_type(ctx, &expr->loc, WASM_TYPE_VOID, func->result_type,
                             desc);
      }
      break;
    case WASM_EXPR_TYPE_SELECT:
      result |= check_expr(ctx, module, func, expr->select.cond, WASM_TYPE_I32,
                           " of condition");
      result |= check_expr(ctx, module, func, expr->select.true_,
                           expected_type, " of argument 0 of select op");
      result |= check_expr(ctx, module, func, expr->select.false_,
                           expected_type, " of argment 1 of select op");
      break;
    case WASM_EXPR_TYPE_SET_LOCAL: {
      WasmType type;
      if (check_local_var(ctx, func, &expr->set_local.var, &type) == WASM_OK) {
        result |= check_type(ctx, &expr->loc, type, expected_type, desc);
        result |= check_expr(ctx, module, func, expr->set_local.expr, type,
                             " of set_local");
      } else {
        result = WASM_ERROR;
      }
      break;
    }
    case WASM_EXPR_TYPE_STORE: {
      WasmType type = expr->store.op.type;
      result |= check_align(ctx, &expr->loc, expr->store.align);
      result |= check_offset(ctx, &expr->loc, expr->store.offset);
      result |= check_type(ctx, &expr->loc, type, expected_type, desc);
      result |= check_expr(ctx, module, func, expr->store.addr, WASM_TYPE_I32,
                           " of store index");
      result |= check_expr(ctx, module, func, expr->store.value, type,
                           " of store value");
      break;
    }
    case WASM_EXPR_TYPE_TABLESWITCH: {
      result |= check_duplicate_bindings(ctx, &expr->tableswitch.case_bindings,
                                         "case");

      WasmLabelNode node;
      result |= push_label(ctx, &expr->loc, &node, &expr->tableswitch.label,
                           expected_type, "tableswitch");
      result |= check_expr(ctx, module, func, expr->tableswitch.expr,
                           WASM_TYPE_I32, " of key");
      int i;
      for (i = 0; i < expr->tableswitch.targets.size; ++i) {
        WasmTarget* target = &expr->tableswitch.targets.data[i];
        result |= check_target(ctx, &expr->tableswitch.case_bindings,
                               expr->tableswitch.cases.size, target);
      }
      result |= check_target(ctx, &expr->tableswitch.case_bindings,
                             expr->tableswitch.cases.size,
                             &expr->tableswitch.default_target);
      int num_cases = expr->tableswitch.cases.size;
      if (num_cases > 0) {
        WasmCase* case_ = &expr->tableswitch.cases.data[0];
        for (i = 0; i < num_cases - 1; ++i, ++case_) {
          result |=
              check_exprs(ctx, module, func, &case_->exprs, WASM_TYPE_VOID, "");
        }
        result |= check_exprs(ctx, module, func, &case_->exprs, expected_type,
                              " of tableswitch case");
      }
      pop_label(ctx);
      break;
    }
    case WASM_EXPR_TYPE_UNARY: {
      WasmType type = expr->unary.op.type;
      result |= check_type(ctx, &expr->loc, type, expected_type, desc);
      result |= check_expr(ctx, module, func, expr->unary.expr,
                           expr->unary.op.type, " of unary op");
      break;
    }
    case WASM_EXPR_TYPE_UNREACHABLE:
      break;
  }
  return result;
}

static WasmResult check_func(WasmCheckContext* ctx,
                             WasmModule* module,
                             WasmLocation* loc,
                             WasmFunc* func) {
  WasmResult result = WASM_OK;
  if (func->flags & WASM_FUNC_FLAG_HAS_FUNC_TYPE) {
    WasmFuncType* func_type;
    if (check_func_type_var(ctx, module, &func->type_var, &func_type) ==
        WASM_OK) {
      if (func->flags & WASM_FUNC_FLAG_HAS_SIGNATURE) {
        result |= check_type_exact(ctx, &func->loc, func->result_type,
                                   func_type->sig.result_type, "");
        if (func->params.types.size == func_type->sig.param_types.size) {
          int i;
          for (i = 0; i < func->params.types.size; ++i) {
            result |= check_type_arg_exact(
                ctx, &func->loc, func->params.types.data[i],
                func_type->sig.param_types.data[i], "function", i);
          }
        } else {
          print_error(ctx, &func->loc, "expected %d parameters, got %d",
                      func_type->sig.param_types.size, func->params.types.size);
          result = WASM_ERROR;
        }
      } else {
        /* copy signature from type var */
        func->result_type = func_type->sig.result_type;
        result |= wasm_extend_types(ctx->allocator, &func->params.types,
                                    &func_type->sig.param_types);
      }
    } else {
      result = WASM_ERROR;
    }
  }

  result |= check_duplicate_bindings(ctx, &func->params.bindings, "parameter");
  result |= check_duplicate_bindings(ctx, &func->locals.bindings, "local");

  result |= wasm_extend_type_bindings(ctx->allocator, &func->params_and_locals,
                                      &func->params);
  result |= wasm_extend_type_bindings(ctx->allocator, &func->params_and_locals,
                                      &func->locals);

  result |= check_exprs(ctx, module, func, &func->exprs, func->result_type,
                        " of function result");

  return result;
}

static WasmResult check_import(WasmCheckContext* ctx,
                               WasmModule* module,
                               WasmLocation* loc,
                               WasmImport* import) {
  if (import->import_type != WASM_IMPORT_HAS_TYPE)
    return WASM_OK;

  return check_func_type_var(ctx, module, &import->type_var, NULL);
}

static WasmResult check_export(WasmCheckContext* ctx,
                               WasmModule* module,
                               WasmExport* export) {
  return check_func_var(ctx, module, &export->var, NULL);
}

static WasmResult check_table(WasmCheckContext* ctx,
                              WasmModule* module,
                              WasmVarVector* table) {
  WasmResult result = WASM_OK;
  int i;
  for (i = 0; i < table->size; ++i)
    result |= check_func_var(ctx, module, &table->data[i], NULL);
  return result;
}

static WasmResult check_memory(WasmCheckContext* ctx,
                               WasmModule* module,
                               WasmMemory* memory) {
  WasmResult result = WASM_OK;
  if (memory->max_size < memory->initial_size) {
    print_error(
        ctx, &memory->loc,
        "max size (%u) must be greater than or equal to initial size (%u)",
        memory->max_size, memory->initial_size);
    result = WASM_ERROR;
  }
  int i;
  uint32_t last_end = 0;
  for (i = 0; i < memory->segments.size; ++i) {
    WasmSegment* segment = &memory->segments.data[i];
    if (segment->addr < last_end) {
      print_error(ctx, &segment->loc,
                  "address (%u) less than end of previous segment (%u)",
                  segment->addr, last_end);
      result = WASM_ERROR;
    }
    if (segment->addr > memory->initial_size) {
      print_error(ctx, &segment->loc,
                  "address (%u) greater than initial memory size (%u)",
                  segment->addr, memory->initial_size);
      result = WASM_ERROR;
    } else if (segment->addr + segment->size > memory->initial_size) {
      print_error(ctx, &segment->loc,
                  "segment ends past the end of initial memory size (%u)",
                  memory->initial_size);
      result = WASM_ERROR;
    }
    last_end = segment->addr + segment->size;
  }
  return result;
}

static WasmResult check_module(WasmCheckContext* ctx, WasmModule* module) {
  WasmResult result = WASM_OK;
  WasmLocation* export_memory_loc = NULL;
  int i;
  int seen_memory = 0;
  int seen_export_memory = 0;
  int seen_table = 0;
  int seen_start = 0;
  for (i = 0; i < module->fields.size; ++i) {
    WasmModuleField* field = &module->fields.data[i];
    switch (field->type) {
      case WASM_MODULE_FIELD_TYPE_FUNC:
        result |= check_func(ctx, module, &field->loc, &field->func);
        break;
      case WASM_MODULE_FIELD_TYPE_IMPORT:
        result |= check_import(ctx, module, &field->loc, &field->import);
        break;
      case WASM_MODULE_FIELD_TYPE_EXPORT:
        result |= check_export(ctx, module, &field->export_);
        break;
      case WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY:
        seen_export_memory = 1;
        export_memory_loc = &field->loc;
        break;
      case WASM_MODULE_FIELD_TYPE_TABLE:
        if (seen_table) {
          print_error(ctx, &field->loc, "only one table allowed");
          result |= WASM_ERROR;
        }
        result |= check_table(ctx, module, &field->table);
        seen_table = 1;
        break;
      case WASM_MODULE_FIELD_TYPE_MEMORY:
        if (seen_memory) {
          print_error(ctx, &field->loc, "only one memory block allowed");
          result |= WASM_ERROR;
        }
        result |= check_memory(ctx, module, &field->memory);
        seen_memory = 1;
        break;

      case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
        break;
      case WASM_MODULE_FIELD_TYPE_START: {
        if (seen_start) {
          print_error(ctx, &field->loc, "only one start function allowed");
          result |= WASM_ERROR;
        }
        WasmFunc* start_func = NULL;
        result |= check_func_var(ctx, module, &field->start, &start_func);
        if (start_func) {
          if (start_func->params.types.size != 0) {
            print_error(ctx, &field->loc, "start function must be nullary");
            result |= WASM_ERROR;
          }
          if (start_func->result_type != WASM_TYPE_VOID) {
            print_error(ctx, &field->loc,
                        "start function must not return anything");
            result |= WASM_ERROR;
          }
        }
        seen_start = 1;
        break;
      }
    }
  }

  if (seen_export_memory && !seen_memory) {
    print_error(ctx, export_memory_loc, "no memory to export");
    result |= WASM_ERROR;
  }

  result |= check_duplicate_bindings(ctx, &module->func_bindings, "function");
  result |= check_duplicate_bindings(ctx, &module->import_bindings, "import");
  result |= check_duplicate_bindings(ctx, &module->export_bindings, "export");
  result |= check_duplicate_bindings(ctx, &module->func_type_bindings,
                                     "function type");

  return result;
}

static WasmResult check_invoke(WasmCheckContext* ctx,
                               WasmCommandInvoke* invoke,
                               WasmTypeSet return_type) {
  if (!ctx->last_module) {
    print_error(ctx, &invoke->loc,
                "invoke must occur after a module definition");
    return WASM_ERROR;
  }

  WasmExport* export = wasm_get_export_by_name(ctx->last_module, &invoke->name);
  if (!export) {
    print_error(ctx, &invoke->loc, "unknown function export \"%.*s\"",
                invoke->name.length, invoke->name.start);
    return WASM_ERROR;
  }

  WasmFunc* func = wasm_get_func_by_var(ctx->last_module, &export->var);
  if (!func) {
    /* this error will have already been reported, just skip it */
    return WASM_ERROR;
  }

  int actual_args = invoke->args.size;
  int expected_args = func->params.types.size;
  if (expected_args != actual_args) {
    print_error(ctx, &invoke->loc,
                "too %s parameters to function. got %d, expected %d",
                actual_args > expected_args ? "many" : "few", actual_args,
                expected_args);
    return WASM_ERROR;
  }
  WasmResult result =
      check_type_set(ctx, &invoke->loc, func->result_type, return_type, "");
  int i;
  for (i = 0; i < actual_args; ++i) {
    WasmConst* const_ = &invoke->args.data[i];
    result |= check_type_arg_exact(ctx, &const_->loc, const_->type,
                                   func->params.types.data[i], "invoke", i);
  }
  return result;
}

static WasmResult check_command(WasmCheckContext* ctx, WasmCommand* command) {
  switch (command->type) {
    case WASM_COMMAND_TYPE_MODULE:
      ctx->last_module = &command->module;
      return check_module(ctx, &command->module);
    case WASM_COMMAND_TYPE_INVOKE:
      return check_invoke(ctx, &command->invoke, WASM_TYPE_SET_VOID);
    case WASM_COMMAND_TYPE_ASSERT_INVALID: {
      ctx->in_assert_invalid = 1;
      WasmResult module_ok = check_module(ctx, &command->assert_invalid.module);
      ctx->in_assert_invalid = 0;
      if (module_ok == WASM_OK) {
        /* error, module should be invalid */
        print_error(ctx, &command->assert_invalid.module.loc,
                    "expected module to be invalid");
        return WASM_ERROR;
      }
      return WASM_OK;
    }
    case WASM_COMMAND_TYPE_ASSERT_RETURN:
      return check_invoke(
          ctx, &command->assert_return.invoke,
          TYPE_TO_TYPE_SET(command->assert_return.expected.type));
    case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
      return check_invoke(ctx, &command->assert_return_nan.invoke,
                          WASM_TYPE_SET_F32 | WASM_TYPE_SET_F64);
    case WASM_COMMAND_TYPE_ASSERT_TRAP:
      return check_invoke(ctx, &command->assert_trap.invoke,
                          WASM_TYPE_SET_VOID);
  }
}

WasmResult wasm_check_script(WasmLexer lexer, WasmScript* script) {
  WasmCheckContext ctx;
  ZERO_MEMORY(ctx);
  ctx.lexer = lexer;
  ctx.allocator = script->allocator;
  WasmResult result = WASM_OK;
  int i;
  for (i = 0; i < script->commands.size; ++i)
    result |= check_command(&ctx, &script->commands.data[i]);
  return result;
}

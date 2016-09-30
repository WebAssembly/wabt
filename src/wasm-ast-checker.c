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

#include "wasm-ast-checker.h"
#include "wasm-config.h"

#include <assert.h>
#include <inttypes.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>

#include "wasm-allocator.h"
#include "wasm-ast-parser-lexer-shared.h"
#include "wasm-binary-reader-ast.h"
#include "wasm-binary-reader.h"

#define WASM_TYPE_ANY WASM_NUM_TYPES

static const char* s_type_names[] = {
    "void", "i32", "i64", "f32", "f64", "any",
};
WASM_STATIC_ASSERT(WASM_ARRAY_SIZE(s_type_names) == WASM_NUM_TYPES + 1);

#define V(rtype, type1, type2, mem_size, code, NAME, text) \
  [code] = WASM_TYPE_##rtype,
static WasmType s_opcode_rtype[] = {WASM_FOREACH_OPCODE(V)};
#undef V

#define V(rtype, type1, type2, mem_size, code, NAME, text) \
  [code] = WASM_TYPE_##type1,
static WasmType s_opcode_type1[] = {WASM_FOREACH_OPCODE(V)};
#undef V

#define V(rtype, type1, type2, mem_size, code, NAME, text) \
  [code] = WASM_TYPE_##type2,
static WasmType s_opcode_type2[] = {WASM_FOREACH_OPCODE(V)};
#undef V

#define V(rtype, type1, type2, mem_size, code, NAME, text) [code] = text,
static const char* s_opcode_name[] = {WASM_FOREACH_OPCODE(V)};
#undef V

typedef enum CheckStyle {
  CHECK_STYLE_NAME,
  CHECK_STYLE_FULL,
} CheckStyle;

typedef enum LabelType {
  LABEL_TYPE_FUNC,
  LABEL_TYPE_BLOCK,
  LABEL_TYPE_LOOP,
  LABEL_TYPE_IF,
} LabelType;

typedef struct LabelNode {
  LabelType label_type;
  const WasmLabel* label;
  const WasmTypeVector* sig;
  struct LabelNode* next;
  size_t type_stack_limit;
} LabelNode;

typedef enum ActionResultKind {
  ACTION_RESULT_KIND_ERROR,
  ACTION_RESULT_KIND_TYPES,
  ACTION_RESULT_KIND_TYPE,
} ActionResultKind;

typedef struct ActionResult {
  ActionResultKind kind;
  union {
    const WasmTypeVector* types;
    WasmType type;
  };
} ActionResult;

typedef struct Context {
  CheckStyle check_style;
  WasmSourceErrorHandler* error_handler;
  WasmAllocator* allocator;
  WasmAstLexer* lexer;
  const WasmModule* current_module;
  const WasmFunc* current_func;
  int current_global_index;
  WasmTypeVector type_stack;
  LabelNode* top_label;
  int max_depth;
  WasmResult result;
} Context;

static void WASM_PRINTF_FORMAT(4, 5) print_error(Context* ctx,
                                                 CheckStyle check_style,
                                                 const WasmLocation* loc,
                                                 const char* fmt,
                                                 ...) {
  if (check_style <= ctx->check_style) {
    ctx->result = WASM_ERROR;
    va_list args;
    va_start(args, fmt);
    wasm_ast_format_error(ctx->error_handler, loc, ctx->lexer, fmt, args);
    va_end(args);
  }
}

static WasmBool is_power_of_two(uint32_t x) {
  return x && ((x & (x - 1)) == 0);
}

static void check_duplicate_bindings(Context* ctx,
                                     const WasmBindingHash* bindings,
                                     const char* desc) {
  size_t i;
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
          print_error(ctx, CHECK_STYLE_NAME, loc,
                      "redefinition of %s \"" PRIstringslice "\"", desc,
                      WASM_PRINTF_STRING_SLICE_ARG(a->binding.name));
        }
      }
    }
  }
}

static WasmResult check_var(Context* ctx,
                            const WasmBindingHash* bindings,
                            int max_index,
                            const WasmVar* var,
                            const char* desc,
                            int* out_index) {
  int index = wasm_get_index_from_var(bindings, var);
  if (index >= 0 && index < max_index) {
    if (out_index)
      *out_index = index;
    return WASM_OK;
  }
  if (var->type == WASM_VAR_TYPE_NAME) {
    print_error(ctx, CHECK_STYLE_NAME, &var->loc,
                "undefined %s variable \"" PRIstringslice "\"", desc,
                WASM_PRINTF_STRING_SLICE_ARG(var->name));
  } else {
    print_error(ctx, CHECK_STYLE_NAME, &var->loc,
                "%s variable out of range (max %d)", desc, max_index);
  }
  return WASM_ERROR;
}

static WasmResult check_func_var(Context* ctx,
                                 const WasmVar* var,
                                 const WasmFunc** out_func) {
  int index;
  if (WASM_FAILED(check_var(ctx, &ctx->current_module->func_bindings,
                            ctx->current_module->funcs.size, var, "function",
                            &index))) {
    return WASM_ERROR;
  }

  if (out_func)
    *out_func = ctx->current_module->funcs.data[index];
  return WASM_OK;
}

static WasmResult check_global_var(Context* ctx,
                                   const WasmVar* var,
                                   const WasmGlobal** out_global,
                                   int* out_global_index) {
  int index;
  if (WASM_FAILED(check_var(ctx, &ctx->current_module->global_bindings,
                            ctx->current_module->globals.size, var, "global",
                            &index))) {
    return WASM_ERROR;
  }

  if (out_global)
    *out_global = ctx->current_module->globals.data[index];
  if (out_global_index)
    *out_global_index = index;
  return WASM_OK;
}

static WasmResult check_func_type_var(Context* ctx,
                                      const WasmVar* var,
                                      const WasmFuncType** out_func_type) {
  int index;
  if (WASM_FAILED(check_var(ctx, &ctx->current_module->func_type_bindings,
                            ctx->current_module->func_types.size, var,
                            "function type", &index))) {
    return WASM_ERROR;
  }

  if (out_func_type)
    *out_func_type = ctx->current_module->func_types.data[index];
  return WASM_OK;
}

static WasmResult check_table_var(Context* ctx,
                                  const WasmVar* var,
                                  const WasmTable** out_table) {
  int index;
  if (WASM_FAILED(check_var(ctx, &ctx->current_module->table_bindings,
                            ctx->current_module->tables.size, var, "table",
                            &index))) {
    return WASM_ERROR;
  }

  if (out_table)
    *out_table = ctx->current_module->tables.data[index];
  return WASM_OK;
}

static WasmResult check_memory_var(Context* ctx,
                                   const WasmVar* var,
                                   const WasmMemory** out_memory) {
  int index;
  if (WASM_FAILED(check_var(ctx, &ctx->current_module->memory_bindings,
                            ctx->current_module->memories.size, var, "memory",
                            &index))) {
    return WASM_ERROR;
  }

  if (out_memory)
    *out_memory = ctx->current_module->memories.data[index];
  return WASM_OK;
}

static WasmResult check_local_var(Context* ctx,
                                  const WasmVar* var,
                                  WasmType* out_type) {
  const WasmFunc* func = ctx->current_func;
  int max_index = wasm_get_num_params_and_locals(func);
  int index = wasm_get_local_index_by_var(func, var);
  if (index >= 0 && index < max_index) {
    if (out_type) {
      int num_params = wasm_get_num_params(func);
      if (index < num_params) {
        *out_type = wasm_get_param_type(func, index);
      } else {
        *out_type = ctx->current_func->local_types.data[index - num_params];
      }
    }
    return WASM_OK;
  }

  if (var->type == WASM_VAR_TYPE_NAME) {
    print_error(ctx, CHECK_STYLE_NAME, &var->loc,
                "undefined local variable \"" PRIstringslice "\"",
                WASM_PRINTF_STRING_SLICE_ARG(var->name));
  } else {
    print_error(ctx, CHECK_STYLE_NAME, &var->loc,
                "local variable out of range (max %d)", max_index);
  }
  return WASM_ERROR;
}

static void check_align(Context* ctx,
                        const WasmLocation* loc,
                        uint32_t alignment) {
  if (alignment != WASM_USE_NATURAL_ALIGNMENT && !is_power_of_two(alignment))
    print_error(ctx, CHECK_STYLE_FULL, loc, "alignment must be power-of-two");
}

static void check_offset(Context* ctx,
                         const WasmLocation* loc,
                         uint64_t offset) {
  if (offset > UINT32_MAX) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "offset must be less than or equal to 0xffffffff");
  }
}

static LabelNode* find_label_by_name(LabelNode* top_label,
                                     const WasmStringSlice* name) {
  LabelNode* node = top_label;
  while (node) {
    if (wasm_string_slices_are_equal(node->label, name))
      return node;
    node = node->next;
  }
  return NULL;
}

static LabelNode* find_label_by_var(LabelNode* top_label, const WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME)
    return find_label_by_name(top_label, &var->name);

  LabelNode* node = top_label;
  int i = 0;
  while (node && i != var->index) {
    node = node->next;
    i++;
  }
  return node;
}

static WasmResult check_label_var(Context* ctx,
                                  LabelNode* top_label,
                                  const WasmVar* var,
                                  LabelNode** out_node) {
  LabelNode* node = find_label_by_var(top_label, var);
  if (node) {
    if (out_node)
      *out_node = node;
    return WASM_OK;
  }

  if (var->type == WASM_VAR_TYPE_NAME) {
    print_error(ctx, CHECK_STYLE_NAME, &var->loc,
                "undefined label variable \"" PRIstringslice "\"",
                WASM_PRINTF_STRING_SLICE_ARG(var->name));
  } else {
    print_error(ctx, CHECK_STYLE_NAME, &var->loc,
                "label variable out of range (max %d)", ctx->max_depth);
  }

  return WASM_ERROR;
}

static void push_label(Context* ctx,
                       const WasmLocation* loc,
                       LabelNode* node,
                       LabelType label_type,
                       const WasmLabel* label,
                       const WasmTypeVector* sig) {
  node->label_type = label_type;
  node->label = label;
  node->next = ctx->top_label;
  node->sig = sig;
  node->type_stack_limit = ctx->type_stack.size;
  ctx->top_label = node;
  ctx->max_depth++;
}

static void pop_label(Context* ctx) {
  ctx->max_depth--;
  ctx->top_label = ctx->top_label->next;
}

static size_t type_stack_limit(Context* ctx) {
  assert(ctx->top_label != NULL);
  return ctx->top_label->type_stack_limit;
}

static WasmResult check_type_stack_limit(Context* ctx,
                                         const WasmLocation* loc,
                                         size_t expected,
                                         const char* desc) {
  size_t limit = type_stack_limit(ctx);
  size_t avail = ctx->type_stack.size - limit;
  if (expected > avail) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "type stack size too small at %s. got %" PRIzd
                ", expected at least %" PRIzd,
                desc, avail, expected);
    return WASM_ERROR;
  }
  return WASM_OK;
}

static WasmResult check_type_stack_limit_exact(Context* ctx,
                                               const WasmLocation* loc,
                                               size_t expected,
                                               const char* desc) {
  size_t limit = type_stack_limit(ctx);
  size_t avail = ctx->type_stack.size - limit;
  if (expected != avail) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "type stack at end of %s is %" PRIzd ". expected %" PRIzd, desc,
                avail, expected);
    return WASM_ERROR;
  }
  return WASM_OK;
}

static void reset_type_stack_to_limit(Context* ctx) {
  ctx->type_stack.size = type_stack_limit(ctx);
}

static void push_type(Context* ctx, WasmType type) {
  if (type != WASM_TYPE_VOID)
    wasm_append_type_value(ctx->allocator, &ctx->type_stack, &type);
}

static void push_types(Context* ctx, const WasmTypeVector* types) {
  size_t i;
  for (i = 0; i < types->size; ++i)
    push_type(ctx, types->data[i]);
}

static WasmType peek_type(Context* ctx, size_t depth, int arity) {
  if (arity > 0) {
    if (depth < ctx->type_stack.size - type_stack_limit(ctx)) {
      return ctx->type_stack.data[ctx->type_stack.size - depth - 1];
    } else {
      /* return any type to allow type-checking to continue; the caller should
       * have already raised an error about the stack underflow. */
      return WASM_TYPE_I32;
    }
  } else {
    return WASM_TYPE_VOID;
  }
}

static WasmType top_type(Context* ctx) {
  return peek_type(ctx, 0, 1);
}

static WasmType pop_type(Context* ctx) {
  WasmType result = top_type(ctx);
  if (ctx->type_stack.size > type_stack_limit(ctx))
    ctx->type_stack.size--;
  return result;
}

static void drop_types(Context* ctx, size_t count) {
  assert(ctx->type_stack.size >= type_stack_limit(ctx) + count);
  ctx->type_stack.size -= count;
}

static void check_type(Context* ctx,
                       const WasmLocation* loc,
                       WasmType actual,
                       WasmType expected,
                       const char* desc) {
  if (actual != expected) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "type mismatch at %s. got %s, expected %s", desc,
                s_type_names[actual], s_type_names[expected]);
  }
}

static void check_type_index(Context* ctx,
                             const WasmLocation* loc,
                             WasmType actual,
                             WasmType expected,
                             const char* desc,
                             int index,
                             const char* index_kind) {
  if (actual != expected) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "type mismatch for %s %d of %s. got %s, expected %s",
                index_kind, index, desc, s_type_names[actual],
                s_type_names[expected]);
  }
}

static void check_types(Context* ctx,
                        const WasmLocation* loc,
                        const WasmTypeVector* actual,
                        const WasmTypeVector* expected,
                        const char* desc,
                        const char* index_kind) {
  if (actual->size == expected->size) {
    size_t i;
    for (i = 0; i < actual->size; ++i) {
      check_type_index(ctx, loc, actual->data[i], expected->data[i], desc, i,
                       index_kind);
    }
  } else {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "expected %" PRIzd " %ss, got %" PRIzd, expected->size,
                index_kind, actual->size);
  }
}

static void check_const_types(Context* ctx,
                              const WasmLocation* loc,
                              const WasmTypeVector* actual,
                              const WasmConstVector* expected,
                              const char* desc) {
  if (actual->size == expected->size) {
    size_t i;
    for (i = 0; i < actual->size; ++i) {
      check_type_index(ctx, loc, actual->data[i], expected->data[i].type, desc,
                       i, "result");
    }
  } else {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "expected %" PRIzd " results, got %" PRIzd, expected->size,
                actual->size);
  }
}

static void check_const_type(Context* ctx,
                             const WasmLocation* loc,
                             WasmType actual,
                             const WasmConstVector* expected,
                             const char* desc) {
  assert(actual != WASM_TYPE_ANY);
  WasmTypeVector actual_types;

  WASM_ZERO_MEMORY(actual_types);
  actual_types.size = actual == WASM_TYPE_VOID ? 0 : 1;
  actual_types.data = &actual;
  check_const_types(ctx, loc, &actual_types, expected, desc);
}

static void pop_and_check_1_type(Context* ctx,
                                 const WasmLocation* loc,
                                 WasmType expected,
                                 const char* desc) {
  if (WASM_SUCCEEDED(check_type_stack_limit(ctx, loc, 1, desc))) {
    WasmType actual = pop_type(ctx);
    check_type(ctx, loc, actual, expected, desc);
  }
}

static void pop_and_check_2_types(Context* ctx,
                                  const WasmLocation* loc,
                                  WasmType expected1,
                                  WasmType expected2,
                                  const char* desc) {
  if (WASM_SUCCEEDED(check_type_stack_limit(ctx, loc, 2, desc))) {
    WasmType actual2 = pop_type(ctx);
    WasmType actual1 = pop_type(ctx);
    check_type(ctx, loc, actual1, expected1, desc);
    check_type(ctx, loc, actual2, expected2, desc);
  }
}

static void check_opcode1(Context* ctx,
                          const WasmLocation* loc,
                          WasmOpcode opcode) {
  pop_and_check_1_type(ctx, loc, s_opcode_type1[opcode], s_opcode_name[opcode]);
  push_type(ctx, s_opcode_rtype[opcode]);
}

static void check_opcode2(Context* ctx,
                          const WasmLocation* loc,
                          WasmOpcode opcode) {
  pop_and_check_2_types(ctx, loc, s_opcode_type1[opcode],
                        s_opcode_type2[opcode], s_opcode_name[opcode]);
  push_type(ctx, s_opcode_rtype[opcode]);
}

static void check_n_types(Context* ctx,
                          const WasmLocation* loc,
                          const WasmTypeVector* expected,
                          const char* desc) {
  if (WASM_SUCCEEDED(check_type_stack_limit(ctx, loc, expected->size, desc))) {
    size_t i;
    for (i = 0; i < expected->size; ++i) {
      WasmType actual = ctx->type_stack.data[i];
      check_type(ctx, loc, actual, expected->data[expected->size - i - 1],
                 desc);
    }
  }
}

static void check_assert_return_nan_type(Context* ctx,
                                         const WasmLocation* loc,
                                         WasmType actual,
                                         const char* desc) {
  /* when using assert_return_nan, the result can be either a f32 or f64 type
   * so we special case it here. */
  if (actual != WASM_TYPE_F32 && actual != WASM_TYPE_F64) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "type mismatch at %s. got %s, expected f32 or f64", desc,
                s_type_names[actual]);
  }
}

static WasmResult check_br(Context* ctx,
                           const WasmLocation* loc,
                           const WasmVar* var,
                           const WasmBlockSignature** out_sig,
                           const char* desc) {
  LabelNode* node;
  WasmResult result = check_label_var(ctx, ctx->top_label, var, &node);
  if (WASM_SUCCEEDED(result)) {
    if (node->label_type != LABEL_TYPE_LOOP) {
      check_n_types(ctx, loc, node->sig, desc);
    }

    if (out_sig)
      *out_sig = node->sig;
  }
  return result;
}

static void check_call(Context* ctx,
                       const WasmLocation* loc,
                       const WasmStringSlice* callee_name,
                       const WasmFuncSignature* sig,
                       const char* desc) {
  size_t expected_args = sig->param_types.size;
  size_t limit = type_stack_limit(ctx);
  size_t avail = ctx->type_stack.size - limit;
  size_t i;
  if (expected_args <= avail) {
    for (i = 0; i < sig->param_types.size; ++i) {
      WasmType actual =
          ctx->type_stack.data[ctx->type_stack.size - expected_args + i];
      WasmType expected = sig->param_types.data[i];
      check_type_index(ctx, loc, actual, expected, desc, i, "argument");
    }
    ctx->type_stack.size -= expected_args;
  } else {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "type stack size too small at %s. got %" PRIzd
                ", expected at least %" PRIzd,
                desc, avail, expected_args);
    ctx->type_stack.size = limit;
  }
  for (i = 0; i < sig->result_types.size; ++i)
    push_type(ctx, sig->result_types.data[i]);
}

static void check_expr(Context* ctx, const WasmExpr* expr);

static void check_block(Context* ctx,
                        const WasmLocation* loc,
                        const WasmExpr* first,
                        const char* desc) {
  if (first) {
    WasmBool check_result = WASM_TRUE;
    const WasmExpr* expr;
    for (expr = first; expr; expr = expr->next) {
      check_expr(ctx, expr);
      /* stop typechecking if we hit unreachable code */
      if (top_type(ctx) == WASM_TYPE_ANY) {
        check_result = WASM_FALSE;
        break;
      }
    }
    if (check_result) {
      assert(ctx->top_label != NULL);
      check_n_types(ctx, loc, ctx->top_label->sig, desc);
      check_type_stack_limit_exact(ctx, loc, ctx->top_label->sig->size, desc);
    }
    ctx->type_stack.size = ctx->top_label->type_stack_limit;
  }
}

static void check_expr(Context* ctx, const WasmExpr* expr) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      check_opcode2(ctx, &expr->loc, expr->binary.opcode);
      break;

    case WASM_EXPR_TYPE_BLOCK: {
      LabelNode node;
      push_label(ctx, &expr->loc, &node, LABEL_TYPE_BLOCK, &expr->block.label,
                 &expr->block.sig);
      check_block(ctx, &expr->loc, expr->block.first, "block");
      pop_label(ctx);
      push_types(ctx, &expr->block.sig);
      break;
    }

    case WASM_EXPR_TYPE_BR:
      check_br(ctx, &expr->loc, &expr->br.var, NULL, "br value");
      reset_type_stack_to_limit(ctx);
      push_type(ctx, WASM_TYPE_ANY);
      break;

    case WASM_EXPR_TYPE_BR_IF: {
      const WasmBlockSignature* sig;
      pop_and_check_1_type(ctx, &expr->loc, WASM_TYPE_I32, "br_if condition");
      if (WASM_SUCCEEDED(check_br(ctx, &expr->loc, &expr->br_if.var, &sig,
                                  "br_if value"))) {
        drop_types(ctx, sig->size);
      }
      break;
    }

    case WASM_EXPR_TYPE_BR_TABLE: {
      pop_and_check_1_type(ctx, &expr->loc, WASM_TYPE_I32, "br_table key");
      size_t i;
      for (i = 0; i < expr->br_table.targets.size; ++i) {
        check_br(ctx, &expr->loc, &expr->br_table.targets.data[i], NULL,
                 "br_table target");
      }
      check_br(ctx, &expr->loc, &expr->br_table.default_target, NULL,
               "br_table default target");
      reset_type_stack_to_limit(ctx);
      push_type(ctx, WASM_TYPE_ANY);
      break;
    }

    case WASM_EXPR_TYPE_CALL: {
      const WasmFunc* callee;
      if (WASM_SUCCEEDED(check_func_var(ctx, &expr->call.var, &callee)))
        check_call(ctx, &expr->loc, &callee->name, &callee->decl.sig, "call");
      break;
    }

    case WASM_EXPR_TYPE_CALL_INDIRECT: {
      const WasmFuncType* func_type;
      if (ctx->current_module->tables.size == 0) {
        print_error(ctx, CHECK_STYLE_FULL, &expr->loc,
                    "found call_indirect operator, but no table");
      }
      if (WASM_SUCCEEDED(
              check_func_type_var(ctx, &expr->call_indirect.var, &func_type))) {
        WasmType type = pop_type(ctx);
        check_type(ctx, &expr->loc, type, WASM_TYPE_I32,
                   "call_indirect function index");
        check_call(ctx, &expr->loc, NULL, &func_type->sig, "call_indirect");
      }
      break;
    }

    case WASM_EXPR_TYPE_COMPARE:
      check_opcode2(ctx, &expr->loc, expr->compare.opcode);
      break;

    case WASM_EXPR_TYPE_CONST:
      push_type(ctx, expr->const_.type);
      break;

    case WASM_EXPR_TYPE_CONVERT:
      check_opcode1(ctx, &expr->loc, expr->convert.opcode);
      break;

    case WASM_EXPR_TYPE_DROP:
      if (WASM_SUCCEEDED(check_type_stack_limit(ctx, &expr->loc, 1, "drop")))
        pop_type(ctx);
      break;

    case WASM_EXPR_TYPE_GET_GLOBAL: {
      const WasmGlobal* global;
      if (WASM_SUCCEEDED(
              check_global_var(ctx, &expr->get_global.var, &global, NULL)))
        push_type(ctx, global->type);
      break;
    }

    case WASM_EXPR_TYPE_GET_LOCAL: {
      WasmType type;
      if (WASM_SUCCEEDED(check_local_var(ctx, &expr->get_local.var, &type)))
        push_type(ctx, type);
      break;
    }

    case WASM_EXPR_TYPE_GROW_MEMORY:
      check_opcode1(ctx, &expr->loc, WASM_OPCODE_GROW_MEMORY);
      break;

    case WASM_EXPR_TYPE_IF: {
      LabelNode node;
      pop_and_check_1_type(ctx, &expr->loc, WASM_TYPE_I32, "if condition");
      push_label(ctx, &expr->loc, &node, LABEL_TYPE_IF, &expr->if_.true_.label,
                 &expr->if_.true_.sig);
      check_block(ctx, &expr->loc, expr->if_.true_.first, "if true branch");
      check_block(ctx, &expr->loc, expr->if_.false_, "if false branch");
      pop_label(ctx);
      push_types(ctx, &expr->if_.true_.sig);
      break;
    }

    case WASM_EXPR_TYPE_LOAD:
      check_align(ctx, &expr->loc, expr->load.align);
      check_offset(ctx, &expr->loc, expr->load.offset);
      check_opcode1(ctx, &expr->loc, expr->load.opcode);
      break;

    case WASM_EXPR_TYPE_LOOP: {
      LabelNode node;
      push_label(ctx, &expr->loc, &node, LABEL_TYPE_LOOP, &expr->loop.label,
                 &expr->loop.sig);
      check_block(ctx, &expr->loc, expr->loop.first, "loop");
      pop_label(ctx);
      push_types(ctx, &expr->block.sig);
      break;
    }

    case WASM_EXPR_TYPE_CURRENT_MEMORY:
      push_type(ctx, WASM_TYPE_I32);
      break;

    case WASM_EXPR_TYPE_NOP:
      break;

    case WASM_EXPR_TYPE_RETURN: {
      check_n_types(ctx, &expr->loc, &ctx->current_func->decl.sig.result_types,
                    "return");
      reset_type_stack_to_limit(ctx);
      push_type(ctx, WASM_TYPE_ANY);
      break;
    }

    case WASM_EXPR_TYPE_SELECT: {
      pop_and_check_1_type(ctx, &expr->loc, WASM_TYPE_I32, "select");
      if (WASM_SUCCEEDED(
              check_type_stack_limit(ctx, &expr->loc, 2, "select"))) {
        WasmType type1 = pop_type(ctx);
        WasmType type2 = pop_type(ctx);
        check_type(ctx, &expr->loc, type2, type1, "select");
        push_type(ctx, type1);
      }
      break;
    }

    case WASM_EXPR_TYPE_SET_GLOBAL: {
      WasmType type = WASM_TYPE_I32;
      const WasmGlobal* global;
      if (WASM_SUCCEEDED(
              check_global_var(ctx, &expr->set_global.var, &global, NULL))) {
        type = global->type;
      }
      pop_and_check_1_type(ctx, &expr->loc, type, "set_global");
      break;
    }

    case WASM_EXPR_TYPE_SET_LOCAL: {
      WasmType type = WASM_TYPE_I32;
      check_local_var(ctx, &expr->set_local.var, &type);
      pop_and_check_1_type(ctx, &expr->loc, type, "set_local");
      break;
    }

    case WASM_EXPR_TYPE_STORE:
      check_align(ctx, &expr->loc, expr->store.align);
      check_offset(ctx, &expr->loc, expr->store.offset);
      check_opcode2(ctx, &expr->loc, expr->store.opcode);
      break;

    case WASM_EXPR_TYPE_TEE_LOCAL: {
      WasmType local_type = WASM_TYPE_I32;
      check_local_var(ctx, &expr->tee_local.var, &local_type);
      if (check_type_stack_limit(ctx, &expr->loc, 1, "tee_local"))
        check_type(ctx, &expr->loc, top_type(ctx), local_type, "tee_local");
      break;
    }

    case WASM_EXPR_TYPE_UNARY:
      check_opcode1(ctx, &expr->loc, expr->unary.opcode);
      break;

    case WASM_EXPR_TYPE_UNREACHABLE:
      reset_type_stack_to_limit(ctx);
      push_type(ctx, WASM_TYPE_ANY);
      break;
  }
}

static void check_func_signature_matches_func_type(
    Context* ctx,
    const WasmLocation* loc,
    const WasmFuncSignature* sig,
    const WasmFuncType* func_type) {
  check_types(ctx, loc, &sig->result_types, &func_type->sig.result_types,
              "function", "result");
  check_types(ctx, loc, &sig->param_types, &func_type->sig.param_types,
              "function", "argument");
}

static void check_func(Context* ctx,
                       const WasmLocation* loc,
                       const WasmFunc* func) {
  ctx->current_func = func;
  if (wasm_get_num_results(func) > 1) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "multiple result values not currently supported.");
    /* don't run any other checks, the won't test the result_type properly */
    return;
  }
  if (wasm_decl_has_func_type(&func->decl)) {
    const WasmFuncType* func_type;
    if (WASM_SUCCEEDED(
            check_func_type_var(ctx, &func->decl.type_var, &func_type))) {
      check_func_signature_matches_func_type(ctx, loc, &func->decl.sig,
                                             func_type);
    }
  }

  check_duplicate_bindings(ctx, &func->param_bindings, "parameter");
  check_duplicate_bindings(ctx, &func->local_bindings, "local");
  /* The function has an implicit label; branching to it is equivalent to the
   * returning from the function. */
  LabelNode node;
  WasmLabel label = wasm_empty_string_slice();
  push_label(ctx, loc, &node, LABEL_TYPE_FUNC, &label,
             &func->decl.sig.result_types);
  check_block(ctx, loc, func->first_expr, "function");
  pop_label(ctx);
  ctx->current_func = NULL;
}

static void print_const_expr_error(Context* ctx,
                                   const WasmLocation* loc,
                                   const char* desc) {
  print_error(ctx, CHECK_STYLE_FULL, loc,
              "invalid %s, must be a constant expression; either *.const or "
              "get_global.",
              desc);
}

static WasmResult eval_const_expr_i32(Context* ctx,
                                      const WasmExpr* expr,
                                      const char* desc,
                                      uint32_t* out_value) {
  if (expr->next != NULL) {
    expr = expr->next;
    goto invalid_expr;
  }

  switch (expr->type) {
    case WASM_EXPR_TYPE_CONST:
      if (expr->const_.type != WASM_TYPE_I32) {
        print_error(ctx, CHECK_STYLE_FULL, &expr->loc,
                    "%s must be of type i32; got %s.", desc,
                    s_type_names[expr->const_.type]);
        return WASM_ERROR;
      }
      *out_value = expr->const_.u32;
      return WASM_OK;

    case WASM_EXPR_TYPE_GET_GLOBAL: {
      const WasmGlobal* global = NULL;
      if (WASM_FAILED(
              check_global_var(ctx, &expr->get_global.var, &global, NULL)))
        return WASM_ERROR;

      if (global->type != WASM_TYPE_I32) {
        print_error(ctx, CHECK_STYLE_FULL, &expr->loc,
                    "%s must be of type i32; got %s.", desc,
                    s_type_names[expr->const_.type]);
        return WASM_ERROR;
      }

      /* TODO(binji): handle infinite recursion */
      return eval_const_expr_i32(ctx, global->init_expr, desc, out_value);
    }

    invalid_expr:
    default:
      print_const_expr_error(ctx, &expr->loc, desc);
      return WASM_ERROR;
  }
}

static void check_global(Context* ctx,
                         const WasmLocation* loc,
                         const WasmGlobal* global) {
  static const char s_desc[] = "global initializer expression";
  WasmType type = WASM_TYPE_VOID;
  WasmExpr* expr = global->init_expr;
  if (!expr)
    return;

  if (expr->next != NULL) {
    print_const_expr_error(ctx, loc, s_desc);
    return;
  }

  switch (expr->type) {
    case WASM_EXPR_TYPE_CONST:
      type = expr->const_.type;
      break;

    case WASM_EXPR_TYPE_GET_GLOBAL: {
      const WasmGlobal* ref_global = NULL;
      int ref_global_index;
      if (WASM_FAILED(check_global_var(ctx, &expr->get_global.var, &ref_global,
                                       &ref_global_index))) {
        return;
      }

      type = ref_global->type;
      /* globals can only reference previously defined globals */
      if (ref_global_index >= ctx->current_global_index) {
        print_error(ctx, CHECK_STYLE_FULL, loc,
                    "global can only be defined in terms of a previously "
                    "defined global.");
      }
      break;
    }

    default:
      print_const_expr_error(ctx, loc, s_desc);
      return;
  }

  check_type(ctx, &expr->loc, type, global->type, s_desc);
}

static void check_limits(Context* ctx,
                         const WasmLocation* loc,
                         const WasmLimits* limits,
                         uint64_t absolute_max,
                         const char* desc) {
  if (limits->initial > absolute_max) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "initial %s (%" PRIu64 ") must be <= (%" PRIu64 ")", desc,
                limits->initial, absolute_max);
  }

  if (limits->has_max) {
    if (limits->max > absolute_max) {
      print_error(ctx, CHECK_STYLE_FULL, loc,
                  "max %s (%" PRIu64 ") must be <= (%" PRIu64 ")", desc,
                  limits->max, absolute_max);
    }

    if (limits->max < limits->initial) {
      print_error(ctx, CHECK_STYLE_FULL, loc,
                  "max %s (%" PRIu64 ") must be >= initial %s (%" PRIu64 ")",
                  desc, limits->max, desc, limits->initial);
    }
  }
}

static void check_table(Context* ctx,
                        const WasmLocation* loc,
                        const WasmTable* table) {
  check_limits(ctx, loc, &table->elem_limits, UINT32_MAX, "elems");
}

static void check_elem_segments(Context* ctx, const WasmModule* module) {
  WasmModuleField* field;
  uint32_t last_end = 0;
  for (field = module->first_field; field; field = field->next) {
    if (field->type != WASM_MODULE_FIELD_TYPE_ELEM_SEGMENT)
      continue;

    WasmElemSegment* elem_segment = &field->elem_segment;
    const WasmTable* table;
    if (!WASM_SUCCEEDED(
            check_table_var(ctx, &elem_segment->table_var, &table)))
      continue;

    uint32_t offset = 0;
    if (WASM_SUCCEEDED(eval_const_expr_i32(
            ctx, elem_segment->offset, "elem segment expression", &offset))) {
      if (offset < last_end) {
        print_error(
            ctx, CHECK_STYLE_FULL, &field->loc,
            "segment offset (%u) less than end of previous segment (%u)",
            offset, last_end);
      }

      uint64_t max = table->elem_limits.initial;
      if ((uint64_t)offset + elem_segment->vars.size > max) {
        print_error(ctx, CHECK_STYLE_FULL, &field->loc,
                    "segment ends past the end of the table (%" PRIu64 ")",
                    max);
      }

      last_end = offset + elem_segment->vars.size;
    }
  }
}

static void check_memory(Context* ctx,
                         const WasmLocation* loc,
                         const WasmMemory* memory) {
  check_limits(ctx, loc, &memory->page_limits, WASM_MAX_PAGES, "pages");
}

static void check_data_segments(Context* ctx, const WasmModule* module) {
  WasmModuleField* field;
  uint32_t last_end = 0;
  for (field = module->first_field; field; field = field->next) {
    if (field->type != WASM_MODULE_FIELD_TYPE_DATA_SEGMENT)
      continue;

    WasmDataSegment* data_segment = &field->data_segment;
    const WasmMemory* memory;
    if (!WASM_SUCCEEDED(
            check_memory_var(ctx, &data_segment->memory_var, &memory)))
      continue;

    uint32_t offset = 0;
    if (WASM_SUCCEEDED(eval_const_expr_i32(
            ctx, data_segment->offset, "data segment expression", &offset))) {
      if (offset < last_end) {
        print_error(
            ctx, CHECK_STYLE_FULL, &field->loc,
            "segment offset (%u) less than end of previous segment (%u)",
            offset, last_end);
      }

      uint32_t max = memory->page_limits.initial;
      const uint64_t memory_initial_size = (uint64_t)max * WASM_PAGE_SIZE;
      if ((uint64_t)offset + data_segment->size > memory_initial_size) {
        print_error(ctx, CHECK_STYLE_FULL, &field->loc,
                    "segment ends past the end of memory (%" PRIu64 ")",
                    memory_initial_size);
      }

      last_end = offset + data_segment->size;
    }
  }
}

static void check_import(Context* ctx,
                         const WasmLocation* loc,
                         const WasmImport* import) {
  switch (import->kind) {
    case WASM_EXTERNAL_KIND_FUNC:
      if (wasm_decl_has_func_type(&import->func.decl))
        check_func_type_var(ctx, &import->func.decl.type_var, NULL);
      break;
    case WASM_EXTERNAL_KIND_TABLE:
      check_table(ctx, loc, &import->table);
      break;
    case WASM_EXTERNAL_KIND_MEMORY:
      check_memory(ctx, loc, &import->memory);
      break;
    case WASM_EXTERNAL_KIND_GLOBAL:
      check_global(ctx, loc, &import->global);
      break;
    case WASM_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
}

static void check_export(Context* ctx, const WasmExport* export_) {
  switch (export_->kind) {
    case WASM_EXTERNAL_KIND_FUNC:
      check_func_var(ctx, &export_->var, NULL);
      break;
    case WASM_EXTERNAL_KIND_TABLE:
      check_table_var(ctx, &export_->var, NULL);
      break;
    case WASM_EXTERNAL_KIND_MEMORY:
      check_memory_var(ctx, &export_->var, NULL);
      break;
    case WASM_EXTERNAL_KIND_GLOBAL:
      check_global_var(ctx, &export_->var, NULL, NULL);
      break;
    case WASM_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
}

static void check_module(Context* ctx, const WasmModule* module) {
  WasmBool seen_memory = WASM_FALSE;
  WasmBool seen_table = WASM_FALSE;
  WasmBool seen_start = WASM_FALSE;

  ctx->current_module = module;
  ctx->current_global_index = 0;

  WasmModuleField* field;
  for (field = module->first_field; field != NULL; field = field->next) {
    switch (field->type) {
      case WASM_MODULE_FIELD_TYPE_FUNC:
        check_func(ctx, &field->loc, &field->func);
        break;

      case WASM_MODULE_FIELD_TYPE_GLOBAL:
        check_global(ctx, &field->loc, &field->global);
        ctx->current_global_index++;
        break;

      case WASM_MODULE_FIELD_TYPE_IMPORT:
        check_import(ctx, &field->loc, &field->import);
        break;

      case WASM_MODULE_FIELD_TYPE_EXPORT:
        check_export(ctx, &field->export_);
        break;

      case WASM_MODULE_FIELD_TYPE_TABLE:
        if (seen_table) {
          print_error(ctx, CHECK_STYLE_FULL, &field->loc,
                      "only one table allowed");
        }
        check_table(ctx, &field->loc, &field->table);
        seen_table = WASM_TRUE;
        break;

      case WASM_MODULE_FIELD_TYPE_ELEM_SEGMENT:
        /* checked below */
        break;

      case WASM_MODULE_FIELD_TYPE_MEMORY:
        if (seen_memory) {
          print_error(ctx, CHECK_STYLE_FULL, &field->loc,
                      "only one memory block allowed");
        }
        check_memory(ctx, &field->loc, &field->memory);
        seen_memory = WASM_TRUE;
        break;

      case WASM_MODULE_FIELD_TYPE_DATA_SEGMENT:
        /* checked below */
        break;

      case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
        break;

      case WASM_MODULE_FIELD_TYPE_START: {
        if (seen_start) {
          print_error(ctx, CHECK_STYLE_FULL, &field->loc,
                      "only one start function allowed");
        }

        const WasmFunc* start_func = NULL;
        check_func_var(ctx, &field->start, &start_func);
        if (start_func) {
          if (wasm_get_num_params(start_func) != 0) {
            print_error(ctx, CHECK_STYLE_FULL, &field->loc,
                        "start function must be nullary");
          }

          if (wasm_get_num_results(start_func) != 0) {
            print_error(ctx, CHECK_STYLE_FULL, &field->loc,
                        "start function must not return anything");
          }
        }
        seen_start = WASM_TRUE;
        break;
      }
    }
  }

  check_elem_segments(ctx, module);
  check_data_segments(ctx, module);

  check_duplicate_bindings(ctx, &module->func_bindings, "function");
  check_duplicate_bindings(ctx, &module->global_bindings, "global");
  check_duplicate_bindings(ctx, &module->export_bindings, "export");
  check_duplicate_bindings(ctx, &module->func_type_bindings, "function type");
  check_duplicate_bindings(ctx, &module->table_bindings, "table");
  check_duplicate_bindings(ctx, &module->memory_bindings, "memory");
}

typedef struct BinaryErrorCallbackData {
  Context* ctx;
  WasmLocation* loc;
} BinaryErrorCallbackData;

static void on_read_binary_error(uint32_t offset,
                                 const char* error,
                                 void* user_data) {
  BinaryErrorCallbackData* data = user_data;
  if (offset == WASM_UNKNOWN_OFFSET) {
    print_error(data->ctx, CHECK_STYLE_FULL, data->loc,
                "error in binary module: %s", error);
  } else {
    print_error(data->ctx, CHECK_STYLE_FULL, data->loc,
                "error in binary module: @0x%08x: %s", offset, error);
  }
}

static void check_raw_module(Context* ctx, WasmRawModule* raw) {
  if (raw->type == WASM_RAW_MODULE_TYPE_TEXT) {
    check_module(ctx, raw->text);
  } else {
    WasmModule module;
    WASM_ZERO_MEMORY(module);
    WasmReadBinaryOptions options = WASM_READ_BINARY_OPTIONS_DEFAULT;
    BinaryErrorCallbackData user_data;
    user_data.ctx = ctx;
    user_data.loc = &raw->loc;
    WasmBinaryErrorHandler error_handler;
    error_handler.on_error = on_read_binary_error;
    error_handler.user_data = &user_data;
    if (WASM_SUCCEEDED(wasm_read_binary_ast(ctx->allocator, raw->binary.data,
                                            raw->binary.size, &options,
                                            &error_handler, &module))) {
      check_module(ctx, &module);
    } else {
      /* error will have been printed by the error_handler */
    }
    wasm_destroy_module(ctx->allocator, &module);
  }
}

/* returns the result type of the invoked function, checked by the caller;
 * returning NULL means that another error occured first, so the result type
 * should be ignored. */
static const WasmTypeVector* check_invoke(Context* ctx,
                                          const WasmAction* action) {
  const WasmActionInvoke* invoke = &action->invoke;
  const WasmModule* module = ctx->current_module;
  if (!module) {
    print_error(ctx, CHECK_STYLE_FULL, &action->loc,
                "invoke must occur after a module definition");
    return NULL;
  }

  WasmExport* export = wasm_get_export_by_name(module, &invoke->name);
  if (!export) {
    print_error(ctx, CHECK_STYLE_NAME, &action->loc,
                "unknown function export \"" PRIstringslice "\"",
                WASM_PRINTF_STRING_SLICE_ARG(invoke->name));
    return NULL;
  }

  WasmFunc* func = wasm_get_func_by_var(module, &export->var);
  if (!func) {
    /* this error will have already been reported, just skip it */
    return NULL;
  }

  size_t actual_args = invoke->args.size;
  size_t expected_args = wasm_get_num_params(func);
  if (expected_args != actual_args) {
    print_error(ctx, CHECK_STYLE_FULL, &action->loc,
                "too %s parameters to function. got %" PRIzd
                ", expected %" PRIzd,
                actual_args > expected_args ? "many" : "few", actual_args,
                expected_args);
    return NULL;
  }
  size_t i;
  for (i = 0; i < actual_args; ++i) {
    WasmConst* const_ = &invoke->args.data[i];
    check_type_index(ctx, &const_->loc, const_->type,
                     wasm_get_param_type(func, i), "invoke", i, "argument");
  }

  return &func->decl.sig.result_types;
}

static WasmType check_get(Context* ctx, const WasmAction* action) {
  /* TODO */
  return WASM_TYPE_ANY;
}

static ActionResult check_action(Context* ctx, const WasmAction* action) {
  ActionResult result;
  WASM_ZERO_MEMORY(result);

  switch (action->type) {
    case WASM_ACTION_TYPE_INVOKE:
      result.types = check_invoke(ctx, action);
      result.kind =
          result.types ? ACTION_RESULT_KIND_TYPES : ACTION_RESULT_KIND_ERROR;
      break;

    case WASM_ACTION_TYPE_GET:
      result.kind = ACTION_RESULT_KIND_TYPE;
      result.type = check_get(ctx, action);
      break;
  }

  return result;
}

static void check_command(Context* ctx, const WasmCommand* command) {
  switch (command->type) {
    case WASM_COMMAND_TYPE_MODULE:
      check_module(ctx, &command->module);
      break;

    case WASM_COMMAND_TYPE_ACTION:
      /* ignore result type */
      check_action(ctx, &command->action);
      break;

    case WASM_COMMAND_TYPE_REGISTER:
    case WASM_COMMAND_TYPE_ASSERT_INVALID:
      /* ignore */
      break;

    case WASM_COMMAND_TYPE_ASSERT_RETURN: {
      const WasmAction* action = &command->assert_return.action;
      ActionResult result = check_action(ctx, action);
      switch (result.kind) {
        case ACTION_RESULT_KIND_TYPES:
          check_const_types(ctx, &action->loc, result.types,
                            &command->assert_return.expected, "action");
          break;

        case ACTION_RESULT_KIND_TYPE:
          check_const_type(ctx, &action->loc, result.type,
                           &command->assert_return.expected, "action");
          break;

        case ACTION_RESULT_KIND_ERROR:
          /* error occurred, don't do any further checks */
          break;
      }
      break;
    }

    case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN: {
      const WasmAction* action = &command->assert_return_nan.action;
      ActionResult result = check_action(ctx, action);

      /* a valid result type will either be f32 or f64; convert a TYPES result
       * into a TYPE result, so it is easier to check below. WASM_TYPE_ANY is
       * used to specify a type that should not be checked (because an earlier
       * error occurred). */
      if (result.kind == ACTION_RESULT_KIND_TYPES) {
        if (result.types->size == 1) {
          result.kind = ACTION_RESULT_KIND_TYPE;
          result.type = result.types->data[0];
        } else {
          print_error(ctx, CHECK_STYLE_FULL, &action->loc,
                      "expected 1 result, got %" PRIzd, result.types->size);
          result.type = WASM_TYPE_ANY;
        }
      }

      if (result.kind == ACTION_RESULT_KIND_TYPE &&
          result.type != WASM_TYPE_ANY)
        check_assert_return_nan_type(ctx, &action->loc, result.type, "action");
      break;
    }

    case WASM_COMMAND_TYPE_ASSERT_TRAP:
      /* ignore result type */
      check_action(ctx, &command->assert_trap.action);
      break;

    case WASM_NUM_COMMAND_TYPES:
      assert(0);
      break;
  }
}

static void wasm_destroy_context(Context* ctx) {
  wasm_destroy_type_vector(ctx->allocator, &ctx->type_stack);
}

WasmResult wasm_check_names(WasmAllocator* allocator,
                            WasmAstLexer* lexer,
                            const struct WasmScript* script,
                            WasmSourceErrorHandler* error_handler) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.check_style = CHECK_STYLE_NAME;
  ctx.allocator = allocator;
  ctx.lexer = lexer;
  ctx.error_handler = error_handler;
  ctx.result = WASM_OK;
  size_t i;
  for (i = 0; i < script->commands.size; ++i)
    check_command(&ctx, &script->commands.data[i]);
  wasm_destroy_context(&ctx);
  return ctx.result;
}

WasmResult wasm_check_ast(WasmAllocator* allocator,
                          WasmAstLexer* lexer,
                          const struct WasmScript* script,
                          WasmSourceErrorHandler* error_handler) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.check_style = CHECK_STYLE_FULL;
  ctx.allocator = allocator;
  ctx.lexer = lexer;
  ctx.error_handler = error_handler;
  ctx.result = WASM_OK;
  size_t i;
  for (i = 0; i < script->commands.size; ++i)
    check_command(&ctx, &script->commands.data[i]);
  wasm_destroy_context(&ctx);
  return ctx.result;
}

WasmResult wasm_check_assert_invalid(
    WasmAllocator* allocator,
    WasmAstLexer* lexer,
    const struct WasmScript* script,
    WasmSourceErrorHandler* assert_invalid_error_handler,
    WasmSourceErrorHandler* error_handler) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.check_style = CHECK_STYLE_FULL;
  ctx.allocator = allocator;
  ctx.lexer = lexer;
  ctx.error_handler = error_handler;
  ctx.result = WASM_OK;

  size_t i;
  for (i = 0; i < script->commands.size; ++i) {
    WasmCommand* command = &script->commands.data[i];
    if (command->type != WASM_COMMAND_TYPE_ASSERT_INVALID)
      continue;

    Context ai_ctx;
    WASM_ZERO_MEMORY(ai_ctx);
    ai_ctx.check_style = CHECK_STYLE_FULL;
    ai_ctx.allocator = allocator;
    ai_ctx.lexer = lexer;
    ai_ctx.error_handler = assert_invalid_error_handler;
    ai_ctx.result = WASM_OK;
    check_raw_module(&ai_ctx, &command->assert_invalid.module);
    wasm_destroy_context(&ai_ctx);

    if (WASM_SUCCEEDED(ai_ctx.result)) {
      print_error(&ctx, CHECK_STYLE_FULL, &command->assert_invalid.module.loc,
                  "expected module to be invalid");
    }
  }
  wasm_destroy_context(&ctx);
  return ctx.result;
}

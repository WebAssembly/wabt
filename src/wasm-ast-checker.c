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

typedef enum WasmCheckType {
  WASM_CHECK_TYPE_VOID = WASM_TYPE_VOID,
  WASM_CHECK_TYPE_I32 = WASM_TYPE_I32,
  WASM_CHECK_TYPE_I64 = WASM_TYPE_I64,
  WASM_CHECK_TYPE_F32 = WASM_TYPE_F32,
  WASM_CHECK_TYPE_F64 = WASM_TYPE_F64,
  WASM_CHECK_TYPE_ANY = WASM_NUM_TYPES,
  WASM_NUM_CHECK_TYPES,
  WASM_CHECK_TYPE____ = WASM_CHECK_TYPE_VOID, /* see table in wasm-common.h */
} WasmCheckType;
WASM_DEFINE_VECTOR(check_type, WasmCheckType);

static const char* s_type_names[] = {"void", "i32", "i64", "f32", "f64", "any"};
WASM_STATIC_ASSERT(WASM_ARRAY_SIZE(s_type_names) == WASM_NUM_CHECK_TYPES);

#define V(rtype, type1, type2, mem_size, code, NAME, text) \
  [code] = WASM_CHECK_TYPE_##rtype,
static WasmCheckType s_opcode_rtype[] = {WASM_FOREACH_OPCODE(V)};
#undef V

#define V(rtype, type1, type2, mem_size, code, NAME, text) \
  [code] = WASM_CHECK_TYPE_##type1,
static WasmCheckType s_opcode_type1[] = {WASM_FOREACH_OPCODE(V)};
#undef V

#define V(rtype, type1, type2, mem_size, code, NAME, text) \
  [code] = WASM_CHECK_TYPE_##type2,
static WasmCheckType s_opcode_type2[] = {WASM_FOREACH_OPCODE(V)};
#undef V

#define V(rtype, type1, type2, mem_size, code, NAME, text) [code] = text,
static const char* s_opcode_name[] = {WASM_FOREACH_OPCODE(V)};
#undef V

typedef size_t WasmSizeT;
WASM_DEFINE_VECTOR(size_t, WasmSizeT);

typedef enum CheckStyle {
  CHECK_STYLE_NAME,
  CHECK_STYLE_FULL,
} CheckStyle;

typedef struct LabelNode {
  const WasmLabel* label;
  WasmCheckType type;
  struct LabelNode* next;
} LabelNode;

typedef struct Context {
  CheckStyle check_style;
  WasmSourceErrorHandler* error_handler;
  WasmAllocator* allocator;
  WasmAstLexer* lexer;
  const WasmModule* current_module;
  const WasmFunc* current_func;
  WasmSizeTVector type_stack_limit;
  WasmCheckTypeVector type_stack;
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

static WasmResult check_import_var(Context* ctx,
                                   const WasmVar* var,
                                   const WasmImport** out_import) {
  int index;
  if (WASM_FAILED(check_var(ctx, &ctx->current_module->import_bindings,
                            ctx->current_module->imports.size, var, "import",
                            &index))) {
    return WASM_ERROR;
  }

  if (out_import)
    *out_import = ctx->current_module->imports.data[index];
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

static WasmResult check_local_var(Context* ctx,
                                  const WasmVar* var,
                                  WasmCheckType* out_type) {
  const WasmModule* module = ctx->current_module;
  const WasmFunc* func = ctx->current_func;
  int max_index = wasm_get_num_params_and_locals(module, func);
  int index = wasm_get_local_index_by_var(func, var);
  if (index >= 0 && index < max_index) {
    if (out_type) {
      int num_params = wasm_get_num_params(module, func);
      if (index < num_params) {
        *out_type = wasm_get_param_type(module, func, index);
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
                       const WasmLabel* label,
                       WasmCheckType type) {
  node->label = label;
  node->next = ctx->top_label;
  node->type = type;
  ctx->top_label = node;
  ctx->max_depth++;
}

static void pop_label(Context* ctx) {
  ctx->max_depth--;
  ctx->top_label = ctx->top_label->next;
}

static size_t type_stack_limit(Context* ctx) {
  return ctx->type_stack_limit.data[ctx->type_stack_limit.size - 1];
}

static size_t push_type_stack_limit(Context* ctx) {
  size_t limit = ctx->type_stack.size;
  wasm_append_size_t_value(ctx->allocator, &ctx->type_stack_limit, &limit);
  return limit;
}

static void pop_type_stack_limit(Context* ctx) {
  assert(ctx->type_stack_limit.size > 0);
  ctx->type_stack_limit.size--;
}

static void push_type(Context* ctx, WasmCheckType type) {
  if (type != WASM_CHECK_TYPE_VOID)
    wasm_append_check_type_value(ctx->allocator, &ctx->type_stack, &type);
}

static WasmCheckType peek_type(Context* ctx, size_t depth, int arity) {
  if (arity > 0) {
    if (depth < ctx->type_stack.size - type_stack_limit(ctx)) {
      return ctx->type_stack.data[ctx->type_stack.size - depth - 1];
    } else {
      /* return any type to allow type-checking to continue; the caller should
       * have already raised an error about the stack underflow. */
      return WASM_CHECK_TYPE_I32;
    }
  } else {
    return WASM_CHECK_TYPE_VOID;
  }
}

static WasmCheckType top_type(Context* ctx) {
  return peek_type(ctx, 0, 1);
}

static WasmCheckType pop_type(Context* ctx) {
  WasmCheckType result = top_type(ctx);
  if (ctx->type_stack.size > type_stack_limit(ctx))
    ctx->type_stack.size--;
  return result;
}

static void check_type(Context* ctx,
                       const WasmLocation* loc,
                       WasmCheckType actual,
                       WasmCheckType expected,
                       const char* desc) {
  if (actual != expected) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "type mismatch at %s. got %s, expected %s", desc,
                s_type_names[actual], s_type_names[expected]);
  }
}

static void check_type_arg(Context* ctx,
                           const WasmLocation* loc,
                           WasmCheckType actual,
                           WasmCheckType expected,
                           const char* desc,
                           int arg_index) {
  if (actual != expected) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "type mismatch for argument %d of %s. got %s, expected %s",
                arg_index, desc, s_type_names[actual], s_type_names[expected]);
  }
}

static void check_assert_return_nan_type(Context* ctx,
                                         const WasmLocation* loc,
                                         WasmCheckType actual,
                                         const char* desc) {
  /* when using assert_return_nan, the result can be either a f32 or f64 type
   * so we special case it here. */
  if (actual != WASM_CHECK_TYPE_F32 && actual != WASM_CHECK_TYPE_F64) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "type mismatch at %s. got %s, expected f32 or f64", desc,
                s_type_names[actual]);
  }
}

static WasmCheckType join_type(Context* ctx,
                               const WasmLocation* loc,
                               WasmCheckType t1,
                               const char* t1_desc,
                               WasmCheckType t2,
                               const char* t2_desc) {
  if (t1 == WASM_CHECK_TYPE_ANY) {
    return t2;
  } else if (t2 == WASM_CHECK_TYPE_ANY) {
    return t1;
  } else if (t1 == t2) {
    return t1;
  } else {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "unable to join type %s (%s) with type %s (%s).",
                s_type_names[t1], t1_desc, s_type_names[t2], t2_desc);
    return t1;
  }
}

static void unify_label_type(Context* ctx,
                             const WasmLocation* loc,
                             LabelNode* node,
                             WasmCheckType type,
                             const char* desc) {
  if (node->type == WASM_CHECK_TYPE_ANY) {
    node->type = type;
  } else if (type != WASM_CHECK_TYPE_ANY && node->type != type) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "type mismatch at %s. got %s, expected %s", desc,
                s_type_names[type], s_type_names[node->type]);
  }
}

static void transform_stack(Context* ctx,
                            const WasmLocation* loc,
                            const char* desc,
                            size_t before_size,
                            size_t after_size,
                            ...) {
  size_t i;
  va_list args;
  va_start(args, after_size);
  size_t limit = type_stack_limit(ctx);
  size_t avail = ctx->type_stack.size - limit;
  if (before_size <= avail) {
    for (i = 0; i < before_size; ++i) {
      WasmCheckType actual =
          ctx->type_stack.data[ctx->type_stack.size - before_size + i];
      WasmCheckType expected = va_arg(args, WasmCheckType);
      /* TODO(binji): could give a better location for the error by storing the
       * location in the type stack; i.e. where this type was added to the
       * stack */
      check_type(ctx, loc, actual, expected, desc);
    }
    ctx->type_stack.size -= before_size;
  } else {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "type stack size too small at %s. got %" PRIzd
                ", expected at least %" PRIzd,
                desc, avail, before_size);
    ctx->type_stack.size = limit;
  }
  assert(after_size <= 1);
  if (after_size > 0)
    push_type(ctx, va_arg(args, WasmCheckType));
  va_end(args);
}

static void check_br(Context* ctx,
                     const WasmLocation* loc,
                     const WasmVar* var,
                     WasmCheckType type,
                     const char* desc) {
  LabelNode* node;
  if (WASM_SUCCEEDED(check_label_var(ctx, ctx->top_label, var, &node))) {
    unify_label_type(ctx, loc, node, type, desc);
  }
}

static void check_call(Context* ctx,
                       const WasmLocation* loc,
                       const WasmStringSlice* callee_name,
                       const WasmFuncSignature* sig,
                       const char* desc) {
  size_t expected_args = sig->param_types.size;
  size_t limit = type_stack_limit(ctx);
  size_t avail = ctx->type_stack.size - limit;
  if (expected_args <= avail) {
    size_t i;
    for (i = 0; i < sig->param_types.size; ++i) {
      WasmCheckType actual =
          ctx->type_stack.data[ctx->type_stack.size - expected_args + i];
      WasmCheckType expected = sig->param_types.data[i];
      check_type_arg(ctx, loc, actual, expected, desc, i);
    }
    ctx->type_stack.size -= expected_args;
  } else {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "type stack size too small at %s. got %" PRIzd
                ", expected at least %" PRIzd,
                desc, avail, expected_args);
    ctx->type_stack.size = limit;
  }
  push_type(ctx, sig->result_type);
}

static void check_expr(Context* ctx, const WasmExpr* expr);

static WasmCheckType check_block(Context* ctx,
                                 const WasmLocation* loc,
                                 const WasmExpr* first,
                                 const char* desc) {
  if (first) {
    size_t limit = push_type_stack_limit(ctx);
    const WasmExpr* expr;
    for (expr = first; expr; expr = expr->next) {
      check_expr(ctx, expr);
      /* stop typechecking if we hit unreachable code */
      if (top_type(ctx) == WASM_CHECK_TYPE_ANY)
        break;
    }
    WasmCheckType result = top_type(ctx);
    if (result != WASM_CHECK_TYPE_ANY) {
      size_t result_arity = ctx->type_stack.size - limit;
      if (result_arity > 1) {
        print_error(ctx, CHECK_STYLE_FULL, loc,
                    "maximum arity for %s is 1, got %" PRIzd, desc,
                    result_arity);
      } else if (result_arity == 0) {
        result = WASM_CHECK_TYPE_VOID;
      }
    }
    ctx->type_stack.size = limit;
    pop_type_stack_limit(ctx);
    return result;
  } else {
    return WASM_CHECK_TYPE_VOID;
  }
}

static void check_expr(Context* ctx, const WasmExpr* expr) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY: {
      WasmOpcode opcode = expr->binary.opcode;
      WasmCheckType rtype = s_opcode_rtype[opcode];
      WasmCheckType type1 = s_opcode_type1[opcode];
      WasmCheckType type2 = s_opcode_type2[opcode];
      transform_stack(ctx, &expr->loc, s_opcode_name[opcode], 2, 1, type1,
                      type2, rtype);
      break;
    }

    case WASM_EXPR_TYPE_BLOCK: {
      LabelNode node;
      push_label(ctx, &expr->loc, &node, &expr->block.label,
                 WASM_CHECK_TYPE_ANY);
      WasmCheckType rtype =
          check_block(ctx, &expr->loc, expr->block.first, "block");
      rtype = join_type(ctx, &expr->loc, node.type,
                        "joined type of block's br* targets", rtype,
                        "block's result type");
      pop_label(ctx);
      push_type(ctx, rtype);
      break;
    }

    case WASM_EXPR_TYPE_BR: {
      WasmCheckType type = peek_type(ctx, 0, expr->br.arity);
      check_br(ctx, &expr->loc, &expr->br.var, type, "br value");
      if (expr->br.arity > 0) {
        transform_stack(ctx, &expr->loc, "br", 1, 1, type, WASM_CHECK_TYPE_ANY);
      } else {
        transform_stack(ctx, &expr->loc, "br", 0, 1, WASM_CHECK_TYPE_ANY);
      }
      break;
    }

    case WASM_EXPR_TYPE_BR_IF: {
      WasmCheckType type = peek_type(ctx, 1, expr->br_if.arity);
      check_br(ctx, &expr->loc, &expr->br_if.var, type, "br_if value");
      if (expr->br_if.arity > 0) {
        transform_stack(ctx, &expr->loc, "br_if", 2, 1, type,
                        WASM_CHECK_TYPE_I32, WASM_CHECK_TYPE_VOID);
      } else {
        transform_stack(ctx, &expr->loc, "br_if", 1, 1, WASM_CHECK_TYPE_I32,
                        WASM_CHECK_TYPE_VOID);
      }
      break;
    }

    case WASM_EXPR_TYPE_BR_TABLE: {
      WasmCheckType type = peek_type(ctx, 1, expr->br_table.arity);
      size_t i;
      for (i = 0; i < expr->br_table.targets.size; ++i) {
        check_br(ctx, &expr->loc, &expr->br_table.targets.data[i], type,
                 "br_table target");
      }
      check_br(ctx, &expr->loc, &expr->br_table.default_target, type,
               "br_table default target");
      if (expr->br_table.arity > 0) {
        transform_stack(ctx, &expr->loc, "br_table", 2, 1, type,
                        WASM_CHECK_TYPE_I32, WASM_CHECK_TYPE_ANY);
      } else {
        transform_stack(ctx, &expr->loc, "br_table", 1, 1, WASM_CHECK_TYPE_I32,
                        WASM_CHECK_TYPE_ANY);
      }
      break;
    }

    case WASM_EXPR_TYPE_CALL: {
      const WasmFunc* callee;
      if (WASM_SUCCEEDED(check_func_var(ctx, &expr->call.var, &callee))) {
        check_call(ctx, &expr->loc, &callee->name, &callee->decl.sig, "call");
      }
      break;
    }

    case WASM_EXPR_TYPE_CALL_IMPORT: {
      const WasmImport* import;
      if (WASM_SUCCEEDED(
              check_import_var(ctx, &expr->call_import.var, &import))) {
        check_call(ctx, &expr->loc, &import->name, &import->decl.sig,
                   "call_import");
      }
      break;
    }

    case WASM_EXPR_TYPE_CALL_INDIRECT: {
      const WasmFuncType* func_type;
      if (WASM_SUCCEEDED(
              check_func_type_var(ctx, &expr->call_indirect.var, &func_type))) {
        WasmCheckType type = pop_type(ctx);
        check_type(ctx, &expr->loc, type, WASM_CHECK_TYPE_I32,
                   "call_indirect function index");
        check_call(ctx, &expr->loc, NULL, &func_type->sig, "call_indirect");
      }
      break;
    }

    case WASM_EXPR_TYPE_COMPARE: {
      WasmOpcode opcode = expr->compare.opcode;
      WasmCheckType rtype = s_opcode_rtype[opcode];
      WasmCheckType type1 = s_opcode_type1[opcode];
      WasmCheckType type2 = s_opcode_type2[opcode];
      transform_stack(ctx, &expr->loc, s_opcode_name[opcode], 2, 1, type1,
                      type2, rtype);
      break;
    }

    case WASM_EXPR_TYPE_CONST:
      push_type(ctx, expr->const_.type);
      break;

    case WASM_EXPR_TYPE_CONVERT: {
      WasmOpcode opcode = expr->convert.opcode;
      WasmCheckType rtype = s_opcode_rtype[opcode];
      WasmCheckType type1 = s_opcode_type1[opcode];
      transform_stack(ctx, &expr->loc, s_opcode_name[opcode], 1, 1, type1,
                      rtype);
      break;
    }

    case WASM_EXPR_TYPE_DROP: {
      WasmType type = top_type(ctx);
      transform_stack(ctx, &expr->loc, "drop", 1, 0, type);
      break;
    }

    case WASM_EXPR_TYPE_GET_GLOBAL: {
      const WasmGlobal* global;
      if (WASM_SUCCEEDED(
              check_global_var(ctx, &expr->get_global.var, &global, NULL))) {
        push_type(ctx, global->type);
      }
      break;
    }

    case WASM_EXPR_TYPE_GET_LOCAL: {
      WasmCheckType type;
      if (WASM_SUCCEEDED(check_local_var(ctx, &expr->get_local.var, &type))) {
        push_type(ctx, type);
      }
      break;
    }

    case WASM_EXPR_TYPE_GROW_MEMORY:
      transform_stack(ctx, &expr->loc, "grow_memory", 1, 1, WASM_CHECK_TYPE_I32,
                      WASM_CHECK_TYPE_I32);
      break;

    case WASM_EXPR_TYPE_IF: {
      LabelNode node;
      push_label(ctx, &expr->loc, &node, &expr->if_.true_.label,
                 WASM_CHECK_TYPE_ANY);
      WasmCheckType rtype =
          check_block(ctx, &expr->loc, expr->if_.true_.first, "if");
      rtype = join_type(ctx, &expr->loc, rtype, "if true branch type",
                        WASM_CHECK_TYPE_VOID, "if false branch type");
      pop_label(ctx);
      transform_stack(ctx, &expr->loc, "if condition", 1, 1,
                      WASM_CHECK_TYPE_I32, rtype);
      break;
    }

    case WASM_EXPR_TYPE_IF_ELSE: {
      LabelNode node;
      push_label(ctx, &expr->loc, &node, &expr->if_else.true_.label,
                 WASM_CHECK_TYPE_ANY);
      WasmCheckType rtype_true = check_block(
          ctx, &expr->loc, expr->if_else.true_.first, "if true branch");
      pop_label(ctx);
      WasmCheckType true_type =
          join_type(ctx, &expr->loc, node.type,
                    "joined type of if true branch's br* targets", rtype_true,
                    "if true branch last operation type");
      push_label(ctx, &expr->loc, &node, &expr->if_else.false_.label,
                 WASM_CHECK_TYPE_ANY);
      WasmCheckType rtype_false = check_block(
          ctx, &expr->loc, expr->if_else.false_.first, "if false branch");
      WasmCheckType false_type =
          join_type(ctx, &expr->loc, node.type,
                    "joined type of if false branch's br* targets", rtype_false,
                    "if false branch last operation type");
      pop_label(ctx);
      WasmCheckType rtype =
          join_type(ctx, &expr->loc, true_type, "if true branch type",
                    false_type, "if false branch type");
      transform_stack(ctx, &expr->loc, "if condition", 1, 1,
                      WASM_CHECK_TYPE_I32, rtype);
      break;
    }

    case WASM_EXPR_TYPE_LOAD: {
      WasmOpcode opcode = expr->load.opcode;
      WasmCheckType rtype = s_opcode_rtype[opcode];
      WasmCheckType type1 = s_opcode_type1[opcode];
      check_align(ctx, &expr->loc, expr->load.align);
      check_offset(ctx, &expr->loc, expr->load.offset);
      transform_stack(ctx, &expr->loc, s_opcode_name[opcode], 1, 1, type1,
                      rtype);
      break;
    }

    case WASM_EXPR_TYPE_LOOP: {
      LabelNode node;
      push_label(ctx, &expr->loc, &node, &expr->loop.label,
                 WASM_CHECK_TYPE_VOID);
      WasmCheckType rtype =
          check_block(ctx, &expr->loc, expr->loop.first, "loop");
      pop_label(ctx);
      push_type(ctx, rtype);
      break;
    }

    case WASM_EXPR_TYPE_CURRENT_MEMORY:
      push_type(ctx, WASM_CHECK_TYPE_I32);
      break;

    case WASM_EXPR_TYPE_NOP:
      break;

    case WASM_EXPR_TYPE_RETURN: {
      WasmCheckType result_type =
          wasm_get_result_type(ctx->current_module, ctx->current_func);
      if (result_type != WASM_TYPE_VOID) {
        transform_stack(ctx, &expr->loc, "return", 1, 1, result_type,
                        WASM_CHECK_TYPE_ANY);
      } else {
        transform_stack(ctx, &expr->loc, "return", 0, 1, WASM_CHECK_TYPE_ANY);
      }
      break;
    }

    case WASM_EXPR_TYPE_SELECT: {
      WasmCheckType type = peek_type(ctx, 1, 1);
      transform_stack(ctx, &expr->loc, "select", 3, 1, type, type,
                      WASM_CHECK_TYPE_I32, type);
      break;
    }

    case WASM_EXPR_TYPE_SET_GLOBAL: {
      WasmCheckType type = WASM_TYPE_I32;
      const WasmGlobal* global;
      if (WASM_SUCCEEDED(
              check_global_var(ctx, &expr->set_global.var, &global, NULL))) {
        type = global->type;
      }
      transform_stack(ctx, &expr->loc, "set_global", 1, 0, type);
      break;
    }

    case WASM_EXPR_TYPE_SET_LOCAL: {
      WasmCheckType type = WASM_TYPE_I32;
      check_local_var(ctx, &expr->set_local.var, &type);
      transform_stack(ctx, &expr->loc, "set_local", 1, 0, type);
      break;
    }

    case WASM_EXPR_TYPE_STORE: {
      WasmOpcode opcode = expr->store.opcode;
      WasmCheckType type1 = s_opcode_type1[opcode];
      WasmCheckType type2 = s_opcode_type2[opcode];
      check_align(ctx, &expr->loc, expr->store.align);
      check_offset(ctx, &expr->loc, expr->store.offset);
      transform_stack(ctx, &expr->loc, s_opcode_name[opcode], 2, 0, type1,
                      type2);
      break;
    }

    case WASM_EXPR_TYPE_TEE_LOCAL: {
      WasmCheckType type = WASM_TYPE_I32;
      check_local_var(ctx, &expr->tee_local.var, &type);
      transform_stack(ctx, &expr->loc, "tee_local", 1, 1, type, type);
      break;
    }

    case WASM_EXPR_TYPE_UNARY: {
      WasmOpcode opcode = expr->unary.opcode;
      WasmCheckType rtype = s_opcode_rtype[opcode];
      WasmCheckType type1 = s_opcode_type1[opcode];
      transform_stack(ctx, &expr->loc, s_opcode_name[opcode], 1, 1, type1,
                      rtype);
      break;
    }

    case WASM_EXPR_TYPE_UNREACHABLE:
      push_type(ctx, WASM_CHECK_TYPE_ANY);
      break;
  }
}

static void check_func_signature_matches_func_type(
    Context* ctx,
    const WasmLocation* loc,
    const WasmFuncSignature* sig,
    const WasmFuncType* func_type) {
  size_t num_params = sig->param_types.size;
  /* special case this check to give a better error message */
  WasmCheckType func_type_result_type =
      wasm_get_func_type_result_type(func_type);
  if (sig->result_type != func_type_result_type) {
    print_error(
        ctx, CHECK_STYLE_FULL, loc,
        "type mismatch between function result type (%s) and function type "
        "result type (%s)\n",
        s_type_names[sig->result_type], s_type_names[func_type_result_type]);
  }
  if (num_params == wasm_get_func_type_num_params(func_type)) {
    size_t i;
    for (i = 0; i < num_params; ++i) {
      check_type_arg(ctx, loc, sig->param_types.data[i],
                     wasm_get_func_type_param_type(func_type, i), "function",
                     i);
    }
  } else {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "expected %" PRIzd " parameters, got %" PRIzd,
                wasm_get_func_type_num_params(func_type), num_params);
  }
}

static void check_func(Context* ctx,
                       const WasmLocation* loc,
                       const WasmFunc* func) {
  ctx->current_func = func;
  if (wasm_decl_has_func_type(&func->decl)) {
    const WasmFuncType* func_type;
    if (WASM_SUCCEEDED(
            check_func_type_var(ctx, &func->decl.type_var, &func_type))) {
      check_func_signature_matches_func_type(ctx, &func->loc, &func->decl.sig,
                                             func_type);
    }
  }

  check_duplicate_bindings(ctx, &func->param_bindings, "parameter");
  check_duplicate_bindings(ctx, &func->local_bindings, "local");
  /* The function has an implicit label; branching to it is equivalent to the
   * returning from the function. */
  LabelNode node;
  WasmLabel label = wasm_empty_string_slice();
  push_label(ctx, &func->loc, &node, &label, func->decl.sig.result_type);
  WasmCheckType rtype =
      check_block(ctx, &func->loc, func->first_expr, "function result");
  join_type(ctx, &func->loc, rtype, "type of last operation",
            func->decl.sig.result_type, "function signature result type");
  pop_label(ctx);
  ctx->current_func = NULL;
}

static void check_global(Context* ctx,
                         const WasmLocation* loc,
                         const WasmGlobal* global,
                         int global_index) {
  WasmCheckType type = WASM_CHECK_TYPE_VOID;
  WasmBool invalid_expr = WASM_TRUE;
  WasmExpr* expr = global->init_expr;

  if (expr->next == NULL) {
    switch (expr->type) {
      case WASM_EXPR_TYPE_CONST:
        type = expr->const_.type;
        invalid_expr = WASM_FALSE;
        break;

      case WASM_EXPR_TYPE_GET_GLOBAL: {
        const WasmGlobal* ref_global = NULL;
        int ref_global_index;
        if (WASM_SUCCEEDED(check_global_var(ctx, &expr->get_global.var,
                                            &ref_global, &ref_global_index))) {
          type = ref_global->type;
          /* globals can only reference previously defined globals */
          if (ref_global_index >= global_index) {
            print_error(ctx, CHECK_STYLE_FULL, loc,
                        "global can only be defined in terms of a previously "
                        "defined global.");
          }
        }
        invalid_expr = WASM_FALSE;
        break;
      }

      default:
        break;
    }
  }

  if (invalid_expr) {
    print_error(ctx, CHECK_STYLE_FULL, loc,
                "invalid global initializer expression. Must be *.const or "
                "get_global.");
  } else {
    check_type(ctx, &expr->loc, type, global->type,
               "global initializer expression");
  }
}

static void check_import(Context* ctx,
                         const WasmLocation* loc,
                         const WasmImport* import) {
  if (wasm_decl_has_func_type(&import->decl))
    check_func_type_var(ctx, &import->decl.type_var, NULL);
}

static void check_export(Context* ctx, const WasmExport* export) {
  check_func_var(ctx, &export->var, NULL);
}

static void check_table(Context* ctx, const WasmVarVector* table) {
  size_t i;
  for (i = 0; i < table->size; ++i)
    check_func_var(ctx, &table->data[i], NULL);
}

static void check_memory(Context* ctx, const WasmMemory* memory) {
  if (memory->initial_pages > WASM_MAX_PAGES) {
    print_error(ctx, CHECK_STYLE_FULL, &memory->loc,
                "initial pages (%" PRIu64 ") must be <= (%u)",
                memory->initial_pages, WASM_MAX_PAGES);
  }

  if (memory->max_pages > WASM_MAX_PAGES) {
    print_error(ctx, CHECK_STYLE_FULL, &memory->loc,
                "max pages (%" PRIu64 ") must be <= (%u)", memory->max_pages,
                WASM_MAX_PAGES);
  }

  if (memory->max_pages < memory->initial_pages) {
    print_error(ctx, CHECK_STYLE_FULL, &memory->loc,
                "max pages (%" PRIu64 ") must be >= initial pages (%" PRIu64
                ")",
                memory->max_pages, memory->initial_pages);
  }

  size_t i;
  uint32_t last_end = 0;
  for (i = 0; i < memory->segments.size; ++i) {
    const WasmSegment* segment = &memory->segments.data[i];
    if (segment->addr < last_end) {
      print_error(ctx, CHECK_STYLE_FULL, &segment->loc,
                  "address (%u) less than end of previous segment (%u)",
                  segment->addr, last_end);
    }
    if (segment->addr > memory->initial_pages * WASM_PAGE_SIZE) {
      print_error(ctx, CHECK_STYLE_FULL, &segment->loc,
                  "address (%u) greater than initial memory size (%" PRIu64 ")",
                  segment->addr, memory->initial_pages * WASM_PAGE_SIZE);
    } else if (segment->addr + segment->size >
               memory->initial_pages * WASM_PAGE_SIZE) {
      print_error(ctx, CHECK_STYLE_FULL, &segment->loc,
                  "segment ends past the end of initial memory size (%" PRIu64
                  ")",
                  memory->initial_pages * WASM_PAGE_SIZE);
    }
    last_end = segment->addr + segment->size;
  }
}

static void check_module(Context* ctx, const WasmModule* module) {
  WasmLocation* export_memory_loc = NULL;
  WasmBool seen_memory = WASM_FALSE;
  WasmBool seen_export_memory = WASM_FALSE;
  WasmBool seen_table = WASM_FALSE;
  WasmBool seen_start = WASM_FALSE;
  int global_index = 0;

  ctx->current_module = module;

  WasmModuleField* field;
  for (field = module->first_field; field != NULL; field = field->next) {
    switch (field->type) {
      case WASM_MODULE_FIELD_TYPE_FUNC:
        check_func(ctx, &field->loc, &field->func);
        break;

      case WASM_MODULE_FIELD_TYPE_GLOBAL:
        check_global(ctx, &field->loc, &field->global, global_index++);
        break;

      case WASM_MODULE_FIELD_TYPE_IMPORT:
        check_import(ctx, &field->loc, &field->import);
        break;

      case WASM_MODULE_FIELD_TYPE_EXPORT:
        check_export(ctx, &field->export_);
        break;

      case WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY:
        seen_export_memory = WASM_TRUE;
        export_memory_loc = &field->loc;
        break;

      case WASM_MODULE_FIELD_TYPE_TABLE:
        if (seen_table) {
          print_error(ctx, CHECK_STYLE_FULL, &field->loc,
                      "only one table allowed");
        }
        check_table(ctx, &field->table);
        seen_table = WASM_TRUE;
        break;

      case WASM_MODULE_FIELD_TYPE_MEMORY:
        if (seen_memory) {
          print_error(ctx, CHECK_STYLE_FULL, &field->loc,
                      "only one memory block allowed");
        }
        check_memory(ctx, &field->memory);
        seen_memory = WASM_TRUE;
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
          if (wasm_get_num_params(ctx->current_module, start_func) != 0) {
            print_error(ctx, CHECK_STYLE_FULL, &field->loc,
                        "start function must be nullary");
          }

          if (wasm_get_result_type(ctx->current_module, start_func) !=
              WASM_TYPE_VOID) {
            print_error(ctx, CHECK_STYLE_FULL, &field->loc,
                        "start function must not return anything");
          }
        }
        seen_start = WASM_TRUE;
        break;
      }
    }
  }

  if (seen_export_memory && !seen_memory) {
    print_error(ctx, CHECK_STYLE_FULL, export_memory_loc,
                "no memory to export");
  }

  check_duplicate_bindings(ctx, &module->func_bindings, "function");
  check_duplicate_bindings(ctx, &module->import_bindings, "import");
  check_duplicate_bindings(ctx, &module->export_bindings, "export");
  check_duplicate_bindings(ctx, &module->func_type_bindings, "function type");
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
 * returning WASM_CHECK_TYPE_ANY means that another error occured first, so the
 * result type should be ignored. */
static WasmCheckType check_invoke(Context* ctx,
                                  const WasmCommandInvoke* invoke) {
  const WasmModule* module = ctx->current_module;
  if (!module) {
    print_error(ctx, CHECK_STYLE_FULL, &invoke->loc,
                "invoke must occur after a module definition");
    return WASM_CHECK_TYPE_ANY;
  }

  WasmExport* export = wasm_get_export_by_name(module, &invoke->name);
  if (!export) {
    print_error(ctx, CHECK_STYLE_NAME, &invoke->loc,
                "unknown function export \"" PRIstringslice "\"",
                WASM_PRINTF_STRING_SLICE_ARG(invoke->name));
    return WASM_CHECK_TYPE_ANY;
  }

  WasmFunc* func = wasm_get_func_by_var(module, &export->var);
  if (!func) {
    /* this error will have already been reported, just skip it */
    return WASM_CHECK_TYPE_ANY;
  }

  size_t actual_args = invoke->args.size;
  size_t expected_args = wasm_get_num_params(module, func);
  if (expected_args != actual_args) {
    print_error(ctx, CHECK_STYLE_FULL, &invoke->loc,
                "too %s parameters to function. got %" PRIzd
                ", expected %" PRIzd,
                actual_args > expected_args ? "many" : "few", actual_args,
                expected_args);
    return WASM_CHECK_TYPE_ANY;
  }
  size_t i;
  for (i = 0; i < actual_args; ++i) {
    WasmConst* const_ = &invoke->args.data[i];
    check_type_arg(ctx, &const_->loc, const_->type,
                   wasm_get_param_type(module, func, i), "invoke", i);
  }

  return wasm_get_result_type(module, func);
}

static void check_command(Context* ctx, const WasmCommand* command) {
  switch (command->type) {
    case WASM_COMMAND_TYPE_MODULE:
      check_module(ctx, &command->module);
      break;

    case WASM_COMMAND_TYPE_INVOKE:
      /* ignore result type */
      check_invoke(ctx, &command->invoke);
      break;

    case WASM_COMMAND_TYPE_ASSERT_INVALID:
      /* ignore */
      break;

    case WASM_COMMAND_TYPE_ASSERT_RETURN: {
      const WasmCommandInvoke* invoke = &command->assert_return.invoke;
      WasmCheckType result_type = check_invoke(ctx, invoke);
      if (result_type != WASM_CHECK_TYPE_ANY) {
        check_type(ctx, &invoke->loc, result_type,
                   command->assert_return.expected.type, "invoke");
      }
      break;
    }

    case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN: {
      const WasmCommandInvoke* invoke = &command->assert_return_nan.invoke;
      WasmCheckType result_type = check_invoke(ctx, invoke);
      if (result_type != WASM_CHECK_TYPE_ANY) {
        check_assert_return_nan_type(ctx, &invoke->loc, result_type, "invoke");
      }
      break;
    }

    case WASM_COMMAND_TYPE_ASSERT_TRAP:
      /* ignore result type */
      check_invoke(ctx, &command->assert_trap.invoke);
      break;

    default:
      break;
  }
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

    if (WASM_SUCCEEDED(ai_ctx.result)) {
      print_error(&ctx, CHECK_STYLE_FULL, &command->assert_invalid.module.loc,
                  "expected module to be invalid");
    }
  }
  return ctx.result;
}

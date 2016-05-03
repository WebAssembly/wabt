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
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>

#include "wasm-ast-parser-lexer-shared.h"

static const char* s_type_names[] = {
    "void", "i32", "i64", "f32", "f64",
};
WASM_STATIC_ASSERT(WASM_ARRAY_SIZE(s_type_names) == WASM_NUM_TYPES);

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
WASM_STATIC_ASSERT(WASM_ARRAY_SIZE(s_type_set_names) == WASM_TYPE_SET_ALL + 1);
WASM_STATIC_ASSERT((1 << (WASM_TYPE_I32 - 1)) == WASM_TYPE_SET_I32);
WASM_STATIC_ASSERT((1 << (WASM_TYPE_I64 - 1)) == WASM_TYPE_SET_I64);
WASM_STATIC_ASSERT((1 << (WASM_TYPE_F32 - 1)) == WASM_TYPE_SET_F32);
WASM_STATIC_ASSERT((1 << (WASM_TYPE_F64 - 1)) == WASM_TYPE_SET_F64);

#define TYPE_TO_TYPE_SET(type) ((type) ? 1 << ((type)-1) : 0)

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

typedef struct WasmLabelNode {
  const WasmLabel* label;
  WasmType expected_type;
  const char* desc;
  struct WasmLabelNode* next;
} WasmLabelNode;

typedef struct WasmContext {
  WasmSourceErrorHandler* error_handler;
  WasmAstLexer* lexer;
  const WasmModule* current_module;
  const WasmFunc* current_func;
  WasmLabelNode* top_label;
  int max_depth;
  WasmResult result;
} WasmContext;

static void WASM_PRINTF_FORMAT(3, 4) print_error(WasmContext* ctx,
                                                 const WasmLocation* loc,
                                                 const char* fmt,
                                                 ...) {
  ctx->result = WASM_ERROR;
  va_list args;
  va_start(args, fmt);
  wasm_ast_format_error(ctx->error_handler, loc, ctx->lexer, fmt, args);
  va_end(args);
}

static WasmBool is_power_of_two(uint32_t x) {
  return x && ((x & (x - 1)) == 0);
}

static void check_duplicate_bindings(WasmContext* ctx,
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
          print_error(ctx, loc, "redefinition of %s \"" PRIstringslice "\"",
                      desc, WASM_PRINTF_STRING_SLICE_ARG(a->binding.name));
        }
      }
    }
  }
}

static WasmResult check_var(WasmContext* ctx,
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
  if (var->type == WASM_VAR_TYPE_NAME)
    print_error(ctx, &var->loc, "undefined %s variable \"" PRIstringslice "\"",
                desc, WASM_PRINTF_STRING_SLICE_ARG(var->name));
  else
    print_error(ctx, &var->loc, "%s variable out of range (max %d)", desc,
                max_index);
  return WASM_ERROR;
}

static WasmResult check_func_var(WasmContext* ctx,
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

static WasmResult check_import_var(WasmContext* ctx,
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

static WasmResult check_func_type_var(WasmContext* ctx,
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

static WasmResult check_local_var(WasmContext* ctx,
                                  const WasmVar* var,
                                  WasmType* out_type) {
  int max_index = wasm_get_num_params_and_locals(ctx->current_func);
  int index = wasm_get_local_index_by_var(ctx->current_func, var);
  if (index >= 0 && index < max_index) {
    if (out_type) {
      int num_params = wasm_get_num_params(ctx->current_func);
      if (index < num_params) {
        *out_type = wasm_get_param_type(ctx->current_func, index);
      } else {
        *out_type = ctx->current_func->local_types.data[index - num_params];
      }
    }
    return WASM_OK;
  }

  if (var->type == WASM_VAR_TYPE_NAME) {
    print_error(ctx, &var->loc,
                "undefined local variable \"" PRIstringslice "\"",
                WASM_PRINTF_STRING_SLICE_ARG(var->name));
  } else {
    print_error(ctx, &var->loc, "local variable out of range (max %d)",
                max_index);
  }
  return WASM_ERROR;
}

static void check_align(WasmContext* ctx,
                        const WasmLocation* loc,
                        uint32_t alignment) {
  if (alignment != WASM_USE_NATURAL_ALIGNMENT && !is_power_of_two(alignment))
    print_error(ctx, loc, "alignment must be power-of-two");
}

static void check_offset(WasmContext* ctx,
                         const WasmLocation* loc,
                         uint64_t offset) {
  if (offset > UINT32_MAX)
    print_error(ctx, loc, "offset must be less than or equal to 0xffffffff");
}

static void check_type(WasmContext* ctx,
                       const WasmLocation* loc,
                       WasmType actual,
                       WasmType expected,
                       const char* desc) {
  if (expected != WASM_TYPE_VOID && actual != expected) {
    print_error(ctx, loc, "type mismatch%s. got %s, expected %s", desc,
                s_type_names[actual], s_type_names[expected]);
  }
}

static void check_type_set(WasmContext* ctx,
                           const WasmLocation* loc,
                           WasmType actual,
                           WasmTypeSet expected,
                           const char* desc) {
  if (expected != WASM_TYPE_SET_VOID &&
      (TYPE_TO_TYPE_SET(actual) & expected) == 0) {
    print_error(ctx, loc, "type mismatch%s. got %s, expected %s", desc,
                s_type_names[actual], s_type_set_names[expected]);
  }
}

static void check_type_exact(WasmContext* ctx,
                             const WasmLocation* loc,
                             WasmType actual,
                             WasmType expected,
                             const char* desc) {
  if (actual != expected) {
    print_error(ctx, loc, "type mismatch%s. got %s, expected %s", desc,
                s_type_names[actual], s_type_names[expected]);
  }
}

static void check_type_arg_exact(WasmContext* ctx,
                                 const WasmLocation* loc,
                                 WasmType actual,
                                 WasmType expected,
                                 const char* desc,
                                 int arg_index) {
  if (actual != expected) {
    print_error(ctx, loc,
                "type mismatch for argument %d of %s. got %s, expected %s",
                arg_index, desc, s_type_names[actual], s_type_names[expected]);
  }
}

static WasmLabelNode* find_label_by_name(WasmLabelNode* top_label,
                                         const WasmStringSlice* name) {
  WasmLabelNode* node = top_label;
  while (node) {
    if (wasm_string_slices_are_equal(node->label, name))
      return node;
    node = node->next;
  }
  return NULL;
}

static WasmLabelNode* find_label_by_var(WasmLabelNode* top_label,
                                        const WasmVar* var) {
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

static WasmResult check_label_var(WasmContext* ctx,
                                  WasmLabelNode* top_label,
                                  const WasmVar* var,
                                  WasmLabelNode** out_node) {
  WasmLabelNode* node = find_label_by_var(top_label, var);
  if (node) {
    if (out_node)
      *out_node = node;
    return WASM_OK;
  }

  if (var->type == WASM_VAR_TYPE_NAME) {
    print_error(ctx, &var->loc,
                "undefined label variable \"" PRIstringslice "\"",
                WASM_PRINTF_STRING_SLICE_ARG(var->name));
  } else {
    print_error(ctx, &var->loc, "label variable out of range (max %d)",
                ctx->max_depth);
  }

  return WASM_ERROR;
}

static void push_label(WasmContext* ctx,
                       const WasmLocation* loc,
                       WasmLabelNode* node,
                       const WasmLabel* label,
                       WasmType expected_type,
                       const char* desc) {
  node->label = label;
  node->next = ctx->top_label;
  node->expected_type = expected_type;
  node->desc = desc;
  ctx->top_label = node;
  ctx->max_depth++;
}

static void pop_label(WasmContext* ctx) {
  ctx->max_depth--;
  ctx->top_label = ctx->top_label->next;
}

static void check_expr(WasmContext* ctx,
                       const WasmExpr* expr,
                       WasmType expected_type,
                       const char* desc);

static void check_expr_opt(WasmContext* ctx,
                           const WasmLocation* loc,
                           const WasmExpr* expr,
                           WasmType expected_type,
                           const char* desc) {
  if (expr)
    check_expr(ctx, expr, expected_type, desc);
  else
    check_type(ctx, loc, WASM_TYPE_VOID, expected_type, desc);
}

static void check_br(WasmContext* ctx,
                     const WasmLocation* loc,
                     const WasmVar* var,
                     const WasmExpr* expr,
                     const char* desc) {
  WasmLabelNode* node;
  if (WASM_FAILED(check_label_var(ctx, ctx->top_label, var, &node)))
    return;

  check_expr_opt(ctx, loc, expr, node->expected_type, desc);
}

static void check_call(WasmContext* ctx,
                       const WasmLocation* loc,
                       const WasmStringSlice* callee_name,
                       const WasmFuncSignature* sig,
                       const WasmExpr* first_arg,
                       size_t num_args,
                       WasmType expected_type,
                       const char* desc) {
  size_t expected_args = sig->param_types.size;
  if (expected_args == num_args) {
    char buffer[100];
    wasm_snprintf(buffer, 100, " of %s result", desc);
    check_type(ctx, loc, sig->result_type, expected_type, buffer);
    const WasmExpr* arg;
    size_t i;
    for (i = 0, arg = first_arg; i < num_args; ++i, arg = arg->next) {
      wasm_snprintf(buffer, 100, " of argument %" PRIzd " of %s", i, desc);
      check_expr(ctx, arg, sig->param_types.data[i], buffer);
    }
  } else {
    char* callee_name_str = "";
    if (callee_name && callee_name->start) {
      size_t length = callee_name->length + 10;
      callee_name_str = alloca(length);
      wasm_snprintf(callee_name_str, length, " \"" PRIstringslice "\"",
                    WASM_PRINTF_STRING_SLICE_ARG(*callee_name));
    }
    print_error(ctx, loc, "too %s parameters to function%s in %s. got %" PRIzd
                          ", expected %" PRIzd,
                num_args > expected_args ? "many" : "few", callee_name_str,
                desc, num_args, expected_args);
  }
}

static void check_expr_list(WasmContext* ctx,
                            const WasmExpr* first,
                            WasmType expected_type,
                            const char* desc) {
  const WasmExpr* expr;
  for (expr = first; expr; expr = expr->next) {
    if (expr->next)
      check_expr(ctx, expr, WASM_TYPE_VOID, "");
    else
      check_expr(ctx, expr, expected_type, desc);
  }
}

static void check_expr(WasmContext* ctx,
                       const WasmExpr* expr,
                       WasmType expected_type,
                       const char* desc) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY: {
      WasmType rtype = s_opcode_rtype[expr->binary.opcode];
      WasmType type1 = s_opcode_type1[expr->binary.opcode];
      WasmType type2 = s_opcode_type2[expr->binary.opcode];
      check_type(ctx, &expr->loc, rtype, expected_type, desc);
      check_expr(ctx, expr->binary.left, type1, " of argument 0 of binary op");
      check_expr(ctx, expr->binary.right, type2, " of argument 1 of binary op");
      break;
    }

    case WASM_EXPR_TYPE_BLOCK: {
      WasmLabelNode node;
      push_label(ctx, &expr->loc, &node, &expr->block.label, expected_type,
                 "block");
      check_expr_list(ctx, expr->block.first, expected_type, " of block");
      pop_label(ctx);
      break;
    }

    case WASM_EXPR_TYPE_BR: {
      check_br(ctx, &expr->loc, &expr->br.var, expr->br.expr, " of br value");
      break;
    }

    case WASM_EXPR_TYPE_BR_IF: {
      check_type(ctx, &expr->loc, WASM_TYPE_VOID, expected_type, desc);
      check_br(ctx, &expr->loc, &expr->br_if.var, expr->br_if.expr,
               " of br_if value");
      check_expr(ctx, expr->br_if.cond, WASM_TYPE_I32, " of br_if condition");
      break;
    }

    case WASM_EXPR_TYPE_CALL: {
      const WasmFunc* callee;
      if (WASM_SUCCEEDED(check_func_var(ctx, &expr->call.var, &callee))) {
        if (wasm_decl_has_func_type(&callee->decl))
          check_func_type_var(ctx, &callee->decl.type_var, NULL);

        check_call(ctx, &expr->loc, &callee->name, &callee->decl.sig,
                   expr->call.first_arg, expr->call.num_args, expected_type,
                   "call");
      }
      break;
    }

    case WASM_EXPR_TYPE_CALL_IMPORT: {
      const WasmImport* import;
      if (WASM_SUCCEEDED(check_import_var(ctx, &expr->call.var, &import))) {
        if (wasm_decl_has_func_type(&import->decl))
          check_func_type_var(ctx, &import->decl.type_var, NULL);

        check_call(ctx, &expr->loc, &import->name, &import->decl.sig,
                   expr->call.first_arg, expr->call.num_args, expected_type,
                   "call_import");
      }
      break;
    }

    case WASM_EXPR_TYPE_CALL_INDIRECT: {
      check_expr(ctx, expr->call_indirect.expr, WASM_TYPE_I32,
                 " of function index");
      const WasmFuncType* func_type;
      if (WASM_SUCCEEDED(
              check_func_type_var(ctx, &expr->call_indirect.var, &func_type))) {
        check_call(ctx, &expr->loc, NULL, &func_type->sig,
                   expr->call_indirect.first_arg, expr->call_indirect.num_args,
                   expected_type, "call_indirect");
      }
      break;
    }

    case WASM_EXPR_TYPE_COMPARE: {
      WasmType rtype = s_opcode_rtype[expr->compare.opcode];
      WasmType type1 = s_opcode_type1[expr->compare.opcode];
      WasmType type2 = s_opcode_type2[expr->compare.opcode];
      check_type(ctx, &expr->loc, rtype, expected_type, desc);
      check_expr(ctx, expr->compare.left, type1,
                 " of argument 0 of compare op");
      check_expr(ctx, expr->compare.right, type2,
                 " of argument 1 of compare op");
      break;
    }

    case WASM_EXPR_TYPE_CONST:
      check_type(ctx, &expr->loc, expr->const_.type, expected_type, desc);
      break;

    case WASM_EXPR_TYPE_CONVERT: {
      WasmType rtype = s_opcode_rtype[expr->convert.opcode];
      WasmType type1 = s_opcode_type1[expr->convert.opcode];
      check_type(ctx, &expr->loc, rtype, expected_type, desc);
      check_expr(ctx, expr->convert.expr, type1, " of convert op");
      break;
    }

    case WASM_EXPR_TYPE_GET_LOCAL: {
      WasmType type;
      if (WASM_SUCCEEDED(check_local_var(ctx, &expr->get_local.var, &type))) {
        check_type(ctx, &expr->loc, type, expected_type, desc);
      }
      break;
    }

    case WASM_EXPR_TYPE_GROW_MEMORY:
      check_type(ctx, &expr->loc, WASM_TYPE_I32, expected_type,
                 " in grow_memory");
      check_expr(ctx, expr->grow_memory.expr, WASM_TYPE_I32, " of grow_memory");
      break;

    case WASM_EXPR_TYPE_IF: {
      WasmLabelNode node;
      check_expr(ctx, expr->if_.cond, WASM_TYPE_I32, " of condition");
      push_label(ctx, &expr->loc, &node, &expr->if_.true_.label, expected_type,
                 "if branch");
      check_expr_list(ctx, expr->if_.true_.first, expected_type,
                      " of if branch");
      pop_label(ctx);
      check_type(ctx, &expr->loc, WASM_TYPE_VOID, expected_type, desc);
      break;
    }

    case WASM_EXPR_TYPE_IF_ELSE: {
      WasmLabelNode node;
      check_expr(ctx, expr->if_else.cond, WASM_TYPE_I32, " of condition");
      push_label(ctx, &expr->loc, &node, &expr->if_else.true_.label,
                 expected_type, "if true branch");
      check_expr_list(ctx, expr->if_else.true_.first, expected_type,
                      " of if branch");
      pop_label(ctx);
      push_label(ctx, &expr->loc, &node, &expr->if_else.false_.label,
                 expected_type, "if false branch");
      check_expr_list(ctx, expr->if_else.false_.first, expected_type,
                      " of if branch");
      pop_label(ctx);
      break;
    }

    case WASM_EXPR_TYPE_LOAD: {
      WasmType rtype = s_opcode_rtype[expr->load.opcode];
      WasmType type1 = s_opcode_type1[expr->load.opcode];
      check_align(ctx, &expr->loc, expr->load.align);
      check_offset(ctx, &expr->loc, expr->load.offset);
      check_type(ctx, &expr->loc, rtype, expected_type, desc);
      check_expr(ctx, expr->load.addr, type1, " of load index");
      break;
    }

    case WASM_EXPR_TYPE_LOOP: {
      WasmLabelNode outer_node;
      WasmLabelNode inner_node;
      push_label(ctx, &expr->loc, &outer_node, &expr->loop.outer, expected_type,
                 "loop outer label");
      push_label(ctx, &expr->loc, &inner_node, &expr->loop.inner,
                 WASM_TYPE_VOID, "loop inner label");
      check_expr_list(ctx, expr->loop.first, expected_type, " of loop");
      pop_label(ctx);
      pop_label(ctx);
      break;
    }

    case WASM_EXPR_TYPE_CURRENT_MEMORY:
      check_type(ctx, &expr->loc, WASM_TYPE_I32, expected_type,
                 " in current_memory");
      break;

    case WASM_EXPR_TYPE_NOP:
      check_type(ctx, &expr->loc, WASM_TYPE_VOID, expected_type, " in nop");
      break;

    case WASM_EXPR_TYPE_RETURN: {
      WasmType result_type = wasm_get_result_type(ctx->current_func);
      check_expr_opt(ctx, &expr->loc, expr->return_.expr, result_type,
                     " of return");
      break;
    }

    case WASM_EXPR_TYPE_SELECT:
      check_expr(ctx, expr->select.cond, WASM_TYPE_I32, " of condition");
      check_expr(ctx, expr->select.true_, expected_type,
                 " of argument 0 of select op");
      check_expr(ctx, expr->select.false_, expected_type,
                 " of argment 1 of select op");
      break;

    case WASM_EXPR_TYPE_SET_LOCAL: {
      WasmType type;
      if (WASM_SUCCEEDED(check_local_var(ctx, &expr->set_local.var, &type))) {
        check_type(ctx, &expr->loc, type, expected_type, desc);
        check_expr(ctx, expr->set_local.expr, type, " of set_local");
      }
      break;
    }

    case WASM_EXPR_TYPE_STORE: {
      WasmType rtype = s_opcode_rtype[expr->store.opcode];
      WasmType type1 = s_opcode_type1[expr->store.opcode];
      WasmType type2 = s_opcode_type2[expr->store.opcode];
      check_align(ctx, &expr->loc, expr->store.align);
      check_offset(ctx, &expr->loc, expr->store.offset);
      check_type(ctx, &expr->loc, rtype, expected_type, desc);
      check_expr(ctx, expr->store.addr, type1, " of store index");
      check_expr(ctx, expr->store.value, type2, " of store value");
      break;
    }

    case WASM_EXPR_TYPE_BR_TABLE: {
      check_expr(ctx, expr->br_table.key, WASM_TYPE_I32, " of key");
      size_t i;
      for (i = 0; i < expr->br_table.targets.size; ++i) {
        check_br(ctx, &expr->loc, &expr->br_table.targets.data[i],
                 expr->br_table.expr, " of br_table target");
      }
      check_br(ctx, &expr->loc, &expr->br_table.default_target,
               expr->br_table.expr, " of br_table default target");
      break;
    }

    case WASM_EXPR_TYPE_UNARY: {
      WasmType rtype = s_opcode_rtype[expr->unary.opcode];
      WasmType type1 = s_opcode_type1[expr->unary.opcode];
      check_type(ctx, &expr->loc, rtype, expected_type, desc);
      check_expr(ctx, expr->unary.expr, type1, " of unary op");
      break;
    }

    case WASM_EXPR_TYPE_UNREACHABLE:
      break;
  }
}

static void check_func_signature_matches_func_type(
    WasmContext* ctx,
    const WasmFunc* func,
    const WasmFuncType* func_type) {
  size_t num_params = wasm_get_num_params(func);
  check_type_exact(ctx, &func->loc, wasm_get_result_type(func),
                   wasm_get_func_type_result_type(func_type), "");
  if (num_params == wasm_get_func_type_num_params(func_type)) {
    size_t i;
    for (i = 0; i < num_params; ++i) {
      check_type_arg_exact(ctx, &func->loc, wasm_get_param_type(func, i),
                           wasm_get_func_type_param_type(func_type, i),
                           "function", i);
    }
  } else {
    print_error(ctx, &func->loc, "expected %" PRIzd " parameters, got %" PRIzd,
                wasm_get_func_type_num_params(func_type), num_params);
  }
}

static void check_func(WasmContext* ctx,
                       const WasmLocation* loc,
                       const WasmFunc* func) {
  ctx->current_func = func;
  if (wasm_decl_has_func_type(&func->decl)) {
    const WasmFuncType* func_type;
    if (WASM_SUCCEEDED(
            check_func_type_var(ctx, &func->decl.type_var, &func_type))) {
      if (wasm_decl_has_signature(&func->decl))
        check_func_signature_matches_func_type(ctx, ctx->current_func,
                                               func_type);
    }
  }

  check_duplicate_bindings(ctx, &func->param_bindings, "parameter");
  check_duplicate_bindings(ctx, &func->local_bindings, "local");
  check_expr_list(ctx, func->first_expr, wasm_get_result_type(func),
                  " of function result");
  ctx->current_func = NULL;
}

static void check_import(WasmContext* ctx,
                         const WasmLocation* loc,
                         const WasmImport* import) {
  if (wasm_decl_has_func_type(&import->decl))
    check_func_type_var(ctx, &import->decl.type_var, NULL);
}

static void check_export(WasmContext* ctx, const WasmExport* export) {
  check_func_var(ctx, &export->var, NULL);
}

static void check_table(WasmContext* ctx, const WasmVarVector* table) {
  size_t i;
  for (i = 0; i < table->size; ++i)
    check_func_var(ctx, &table->data[i], NULL);
}

static void check_memory(WasmContext* ctx, const WasmMemory* memory) {
  if (memory->initial_pages > WASM_MAX_PAGES) {
    print_error(ctx, &memory->loc, "initial pages (%u) must be less than (%u)",
                memory->initial_pages, WASM_MAX_PAGES);
  }

  if (memory->max_pages > WASM_MAX_PAGES) {
    print_error(ctx, &memory->loc, "max pages (%u) must be less than (%u)",
                memory->max_pages, WASM_MAX_PAGES);
  }

  if (memory->max_pages < memory->initial_pages) {
    print_error(
        ctx, &memory->loc,
        "max pages (%u) must be greater than or equal to initial pages (%u)",
        memory->max_pages, memory->initial_pages);
  }

  size_t i;
  uint32_t last_end = 0;
  for (i = 0; i < memory->segments.size; ++i) {
    const WasmSegment* segment = &memory->segments.data[i];
    if (segment->addr < last_end) {
      print_error(ctx, &segment->loc,
                  "address (%u) less than end of previous segment (%u)",
                  segment->addr, last_end);
    }
    if (segment->addr > memory->initial_pages * WASM_PAGE_SIZE) {
      print_error(ctx, &segment->loc,
                  "address (%u) greater than initial memory size (%u)",
                  segment->addr, memory->initial_pages * WASM_PAGE_SIZE);
    } else if (segment->addr + segment->size >
               memory->initial_pages * WASM_PAGE_SIZE) {
      print_error(ctx, &segment->loc,
                  "segment ends past the end of initial memory size (%u)",
                  memory->initial_pages * WASM_PAGE_SIZE);
    }
    last_end = segment->addr + segment->size;
  }
}

static void check_module(WasmContext* ctx, const WasmModule* module) {
  WasmLocation* export_memory_loc = NULL;
  WasmBool seen_memory = WASM_FALSE;
  WasmBool seen_export_memory = WASM_FALSE;
  WasmBool seen_table = WASM_FALSE;
  WasmBool seen_start = WASM_FALSE;

  ctx->current_module = module;

  WasmModuleField* field;
  for (field = module->first_field; field != NULL; field = field->next) {
    switch (field->type) {
      case WASM_MODULE_FIELD_TYPE_FUNC:
        check_func(ctx, &field->loc, &field->func);
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
        if (seen_table)
          print_error(ctx, &field->loc, "only one table allowed");
        check_table(ctx, &field->table);
        seen_table = WASM_TRUE;
        break;

      case WASM_MODULE_FIELD_TYPE_MEMORY:
        if (seen_memory)
          print_error(ctx, &field->loc, "only one memory block allowed");
        check_memory(ctx, &field->memory);
        seen_memory = WASM_TRUE;
        break;

      case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
        break;

      case WASM_MODULE_FIELD_TYPE_START: {
        if (seen_start)
          print_error(ctx, &field->loc, "only one start function allowed");

        const WasmFunc* start_func = NULL;
        check_func_var(ctx, &field->start, &start_func);
        if (start_func) {
          if (wasm_get_num_params(start_func) != 0)
            print_error(ctx, &field->loc, "start function must be nullary");

          if (wasm_get_result_type(start_func) != WASM_TYPE_VOID) {
            print_error(ctx, &field->loc,
                        "start function must not return anything");
          }
        }
        seen_start = WASM_TRUE;
        break;
      }
    }
  }

  if (seen_export_memory && !seen_memory)
    print_error(ctx, export_memory_loc, "no memory to export");

  check_duplicate_bindings(ctx, &module->func_bindings, "function");
  check_duplicate_bindings(ctx, &module->import_bindings, "import");
  check_duplicate_bindings(ctx, &module->export_bindings, "export");
  check_duplicate_bindings(ctx, &module->func_type_bindings, "function type");
}

static void check_invoke(WasmContext* ctx,
                         const WasmCommandInvoke* invoke,
                         WasmTypeSet return_type) {
  if (!ctx->current_module) {
    print_error(ctx, &invoke->loc,
                "invoke must occur after a module definition");
    return;
  }

  WasmExport* export =
      wasm_get_export_by_name(ctx->current_module, &invoke->name);
  if (!export) {
    print_error(ctx, &invoke->loc,
                "unknown function export \"" PRIstringslice "\"",
                WASM_PRINTF_STRING_SLICE_ARG(invoke->name));
    return;
  }

  WasmFunc* func = wasm_get_func_by_var(ctx->current_module, &export->var);
  if (!func) {
    /* this error will have already been reported, just skip it */
    return;
  }

  size_t actual_args = invoke->args.size;
  size_t expected_args = wasm_get_num_params(func);
  if (expected_args != actual_args) {
    print_error(ctx, &invoke->loc, "too %s parameters to function. got %" PRIzd
                                   ", expected %" PRIzd,
                actual_args > expected_args ? "many" : "few", actual_args,
                expected_args);
    return;
  }
  check_type_set(ctx, &invoke->loc, wasm_get_result_type(func), return_type,
                 "");
  size_t i;
  for (i = 0; i < actual_args; ++i) {
    WasmConst* const_ = &invoke->args.data[i];
    check_type_arg_exact(ctx, &const_->loc, const_->type,
                         wasm_get_param_type(func, i), "invoke", i);
  }
}

static void check_command(WasmContext* ctx, const WasmCommand* command) {
  switch (command->type) {
    case WASM_COMMAND_TYPE_MODULE:
      check_module(ctx, &command->module);
      break;

    case WASM_COMMAND_TYPE_INVOKE:
      check_invoke(ctx, &command->invoke, WASM_TYPE_SET_VOID);
      break;

    case WASM_COMMAND_TYPE_ASSERT_INVALID:
      /* ignore */
      break;

    case WASM_COMMAND_TYPE_ASSERT_RETURN:
      check_invoke(ctx, &command->assert_return.invoke,
                   TYPE_TO_TYPE_SET(command->assert_return.expected.type));
      break;

    case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
      check_invoke(ctx, &command->assert_return_nan.invoke,
                   WASM_TYPE_SET_F32 | WASM_TYPE_SET_F64);
      break;

    case WASM_COMMAND_TYPE_ASSERT_TRAP:
      check_invoke(ctx, &command->assert_trap.invoke, WASM_TYPE_SET_VOID);
      break;

    default:
      break;
  }
}

WasmResult wasm_check_ast(WasmAstLexer* lexer,
                          const struct WasmScript* script,
                          WasmSourceErrorHandler* error_handler) {
  WasmContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.lexer = lexer;
  ctx.error_handler = error_handler;
  ctx.result = WASM_OK;
  size_t i;
  for (i = 0; i < script->commands.size; ++i)
    check_command(&ctx, &script->commands.data[i]);
  return ctx.result;
}

WasmResult wasm_check_assert_invalid(
    WasmAstLexer* lexer,
    const struct WasmScript* script,
    WasmSourceErrorHandler* assert_invalid_error_handler,
    WasmSourceErrorHandler* error_handler) {
  WasmContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.lexer = lexer;
  ctx.error_handler = error_handler;
  ctx.result = WASM_OK;

  size_t i;
  for (i = 0; i < script->commands.size; ++i) {
    WasmCommand* command = &script->commands.data[i];
    if (command->type != WASM_COMMAND_TYPE_ASSERT_INVALID)
      continue;

    WasmContext ai_ctx;
    WASM_ZERO_MEMORY(ai_ctx);
    ai_ctx.lexer = lexer;
    ai_ctx.error_handler = assert_invalid_error_handler;
    ai_ctx.result = WASM_OK;
    check_module(&ai_ctx, &command->assert_invalid.module);

    if (WASM_SUCCEEDED(ai_ctx.result)) {
      print_error(&ctx, &command->assert_invalid.module.loc,
                  "expected module to be invalid");
    }
  }
  return ctx.result;
}

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

#include "validator.h"
#include "config.h"

#include <assert.h>
#include <inttypes.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>

#include "ast-parser-lexer-shared.h"
#include "binary-reader-ast.h"
#include "binary-reader.h"
#include "type-checker.h"

typedef enum ActionResultKind {
  ACTION_RESULT_KIND_ERROR,
  ACTION_RESULT_KIND_TYPES,
  ACTION_RESULT_KIND_TYPE,
} ActionResultKind;

typedef struct ActionResult {
  ActionResultKind kind;
  union {
    const WabtTypeVector* types;
    WabtType type;
  };
} ActionResult;

typedef struct Context {
  WabtSourceErrorHandler* error_handler;
  WabtAstLexer* lexer;
  const WabtScript* script;
  const WabtModule* current_module;
  const WabtFunc* current_func;
  int current_table_index;
  int current_memory_index;
  int current_global_index;
  int num_imported_globals;
  WabtTypeChecker typechecker;
  const WabtLocation* expr_loc; /* Cached for access by on_typechecker_error */
  WabtResult result;
} Context;

static void WABT_PRINTF_FORMAT(3, 4)
    print_error(Context* ctx, const WabtLocation* loc, const char* fmt, ...) {
  ctx->result = WABT_ERROR;
  va_list args;
  va_start(args, fmt);
  wabt_ast_format_error(ctx->error_handler, loc, ctx->lexer, fmt, args);
  va_end(args);
}

static void on_typechecker_error(const char* msg, void* user_data) {
  Context* ctx = (Context*)user_data;
  print_error(ctx, ctx->expr_loc, "%s", msg);
}

static bool is_power_of_two(uint32_t x) {
  return x && ((x & (x - 1)) == 0);
}

static uint32_t get_opcode_natural_alignment(WabtOpcode opcode) {
  uint32_t memory_size = wabt_get_opcode_memory_size(opcode);
  assert(memory_size != 0);
  return memory_size;
}

static WabtResult check_var(Context* ctx,
                            int max_index,
                            const WabtVar* var,
                            const char* desc,
                            int* out_index) {
  assert(var->type == WABT_VAR_TYPE_INDEX);
  if (var->index >= 0 && var->index < max_index) {
    if (out_index)
      *out_index = var->index;
    return WABT_OK;
  }
  print_error(ctx, &var->loc, "%s variable out of range (max %d)", desc,
              max_index);
  return WABT_ERROR;
}

static WabtResult check_func_var(Context* ctx,
                                 const WabtVar* var,
                                 const WabtFunc** out_func) {
  int index;
  if (WABT_FAILED(check_var(ctx, ctx->current_module->funcs.size, var,
                            "function", &index))) {
    return WABT_ERROR;
  }

  if (out_func)
    *out_func = ctx->current_module->funcs.data[index];
  return WABT_OK;
}

static WabtResult check_global_var(Context* ctx,
                                   const WabtVar* var,
                                   const WabtGlobal** out_global,
                                   int* out_global_index) {
  int index;
  if (WABT_FAILED(check_var(ctx, ctx->current_module->globals.size, var,
                            "global", &index))) {
    return WABT_ERROR;
  }

  if (out_global)
    *out_global = ctx->current_module->globals.data[index];
  if (out_global_index)
    *out_global_index = index;
  return WABT_OK;
}

static WabtType get_global_var_type_or_any(Context* ctx, const WabtVar* var) {
  const WabtGlobal* global;
  if (WABT_SUCCEEDED(check_global_var(ctx, var, &global, nullptr)))
    return global->type;
  return WABT_TYPE_ANY;
}

static WabtResult check_func_type_var(Context* ctx,
                                      const WabtVar* var,
                                      const WabtFuncType** out_func_type) {
  int index;
  if (WABT_FAILED(check_var(ctx, ctx->current_module->func_types.size, var,
                            "function type", &index))) {
    return WABT_ERROR;
  }

  if (out_func_type)
    *out_func_type = ctx->current_module->func_types.data[index];
  return WABT_OK;
}

static WabtResult check_table_var(Context* ctx,
                                  const WabtVar* var,
                                  const WabtTable** out_table) {
  int index;
  if (WABT_FAILED(check_var(ctx, ctx->current_module->tables.size, var, "table",
                            &index))) {
    return WABT_ERROR;
  }

  if (out_table)
    *out_table = ctx->current_module->tables.data[index];
  return WABT_OK;
}

static WabtResult check_memory_var(Context* ctx,
                                   const WabtVar* var,
                                   const WabtMemory** out_memory) {
  int index;
  if (WABT_FAILED(check_var(ctx, ctx->current_module->memories.size, var,
                            "memory", &index))) {
    return WABT_ERROR;
  }

  if (out_memory)
    *out_memory = ctx->current_module->memories.data[index];
  return WABT_OK;
}

static WabtResult check_local_var(Context* ctx,
                                  const WabtVar* var,
                                  WabtType* out_type) {
  const WabtFunc* func = ctx->current_func;
  int max_index = wabt_get_num_params_and_locals(func);
  int index = wabt_get_local_index_by_var(func, var);
  if (index >= 0 && index < max_index) {
    if (out_type) {
      int num_params = wabt_get_num_params(func);
      if (index < num_params) {
        *out_type = wabt_get_param_type(func, index);
      } else {
        *out_type = ctx->current_func->local_types.data[index - num_params];
      }
    }
    return WABT_OK;
  }

  if (var->type == WABT_VAR_TYPE_NAME) {
    print_error(ctx, &var->loc,
                "undefined local variable \"" PRIstringslice "\"",
                WABT_PRINTF_STRING_SLICE_ARG(var->name));
  } else {
    print_error(ctx, &var->loc, "local variable out of range (max %d)",
                max_index);
  }
  return WABT_ERROR;
}

static WabtType get_local_var_type_or_any(Context* ctx, const WabtVar* var) {
  WabtType type = WABT_TYPE_ANY;
  check_local_var(ctx, var, &type);
  return type;
}

static void check_align(Context* ctx,
                        const WabtLocation* loc,
                        uint32_t alignment,
                        uint32_t natural_alignment) {
  if (alignment != WABT_USE_NATURAL_ALIGNMENT) {
    if (!is_power_of_two(alignment))
      print_error(ctx, loc, "alignment must be power-of-two");
    if (alignment > natural_alignment) {
      print_error(ctx, loc,
                  "alignment must not be larger than natural alignment (%u)",
                  natural_alignment);
    }
  }
}

static void check_offset(Context* ctx,
                         const WabtLocation* loc,
                         uint64_t offset) {
  if (offset > UINT32_MAX) {
    print_error(ctx, loc, "offset must be less than or equal to 0xffffffff");
  }
}

static void check_type(Context* ctx,
                       const WabtLocation* loc,
                       WabtType actual,
                       WabtType expected,
                       const char* desc) {
  if (expected != actual) {
    print_error(ctx, loc, "type mismatch at %s. got %s, expected %s", desc,
                wabt_get_type_name(actual), wabt_get_type_name(expected));
  }
}

static void check_type_index(Context* ctx,
                             const WabtLocation* loc,
                             WabtType actual,
                             WabtType expected,
                             const char* desc,
                             int index,
                             const char* index_kind) {
  if (expected != actual && expected != WABT_TYPE_ANY &&
      actual != WABT_TYPE_ANY) {
    print_error(ctx, loc, "type mismatch for %s %d of %s. got %s, expected %s",
                index_kind, index, desc, wabt_get_type_name(actual),
                wabt_get_type_name(expected));
  }
}

static void check_types(Context* ctx,
                        const WabtLocation* loc,
                        const WabtTypeVector* actual,
                        const WabtTypeVector* expected,
                        const char* desc,
                        const char* index_kind) {
  if (actual->size == expected->size) {
    size_t i;
    for (i = 0; i < actual->size; ++i) {
      check_type_index(ctx, loc, actual->data[i], expected->data[i], desc, i,
                       index_kind);
    }
  } else {
    print_error(ctx, loc, "expected %" PRIzd " %ss, got %" PRIzd,
                expected->size, index_kind, actual->size);
  }
}

static void check_const_types(Context* ctx,
                              const WabtLocation* loc,
                              const WabtTypeVector* actual,
                              const WabtConstVector* expected,
                              const char* desc) {
  if (actual->size == expected->size) {
    size_t i;
    for (i = 0; i < actual->size; ++i) {
      check_type_index(ctx, loc, actual->data[i], expected->data[i].type, desc,
                       i, "result");
    }
  } else {
    print_error(ctx, loc, "expected %" PRIzd " results, got %" PRIzd,
                expected->size, actual->size);
  }
}

static void check_const_type(Context* ctx,
                             const WabtLocation* loc,
                             WabtType actual,
                             const WabtConstVector* expected,
                             const char* desc) {
  WabtTypeVector actual_types;
  WABT_ZERO_MEMORY(actual_types);
  actual_types.size = actual == WABT_TYPE_VOID ? 0 : 1;
  actual_types.data = &actual;
  check_const_types(ctx, loc, &actual_types, expected, desc);
}

static void check_assert_return_nan_type(Context* ctx,
                                         const WabtLocation* loc,
                                         WabtType actual,
                                         const char* desc) {
  /* when using assert_return_nan, the result can be either a f32 or f64 type
   * so we special case it here. */
  if (actual != WABT_TYPE_F32 && actual != WABT_TYPE_F64) {
    print_error(ctx, loc, "type mismatch at %s. got %s, expected f32 or f64",
                desc, wabt_get_type_name(actual));
  }
}

static void check_expr(Context* ctx, const WabtExpr* expr);

static void check_expr_list(Context* ctx,
                            const WabtLocation* loc,
                            const WabtExpr* first) {
  if (first) {
    const WabtExpr* expr;
    for (expr = first; expr; expr = expr->next)
      check_expr(ctx, expr);
  }
}

static void check_has_memory(Context* ctx,
                             const WabtLocation* loc,
                             WabtOpcode opcode) {
  if (ctx->current_module->memories.size == 0) {
    print_error(ctx, loc, "%s requires an imported or defined memory.",
                wabt_get_opcode_name(opcode));
  }
}

static void check_expr(Context* ctx, const WabtExpr* expr) {
  ctx->expr_loc = &expr->loc;

  switch (expr->type) {
    case WABT_EXPR_TYPE_BINARY:
      wabt_typechecker_on_binary(&ctx->typechecker, expr->binary.opcode);
      break;

    case WABT_EXPR_TYPE_BLOCK:
      wabt_typechecker_on_block(&ctx->typechecker, &expr->block.sig);
      check_expr_list(ctx, &expr->loc, expr->block.first);
      wabt_typechecker_on_end(&ctx->typechecker);
      break;

    case WABT_EXPR_TYPE_BR:
      wabt_typechecker_on_br(&ctx->typechecker, expr->br.var.index);
      break;

    case WABT_EXPR_TYPE_BR_IF:
      wabt_typechecker_on_br_if(&ctx->typechecker, expr->br_if.var.index);
      break;

    case WABT_EXPR_TYPE_BR_TABLE: {
      wabt_typechecker_begin_br_table(&ctx->typechecker);
      size_t i;
      for (i = 0; i < expr->br_table.targets.size; ++i) {
        wabt_typechecker_on_br_table_target(
            &ctx->typechecker, expr->br_table.targets.data[i].index);
      }
      wabt_typechecker_on_br_table_target(&ctx->typechecker,
                                          expr->br_table.default_target.index);
      wabt_typechecker_end_br_table(&ctx->typechecker);
      break;
    }

    case WABT_EXPR_TYPE_CALL: {
      const WabtFunc* callee;
      if (WABT_SUCCEEDED(check_func_var(ctx, &expr->call.var, &callee))) {
        wabt_typechecker_on_call(&ctx->typechecker,
                                 &callee->decl.sig.param_types,
                                 &callee->decl.sig.result_types);
      }
      break;
    }

    case WABT_EXPR_TYPE_CALL_INDIRECT: {
      const WabtFuncType* func_type;
      if (ctx->current_module->tables.size == 0) {
        print_error(ctx, &expr->loc,
                    "found call_indirect operator, but no table");
      }
      if (WABT_SUCCEEDED(
              check_func_type_var(ctx, &expr->call_indirect.var, &func_type))) {
        wabt_typechecker_on_call_indirect(&ctx->typechecker,
                                          &func_type->sig.param_types,
                                          &func_type->sig.result_types);
      }
      break;
    }

    case WABT_EXPR_TYPE_COMPARE:
      wabt_typechecker_on_compare(&ctx->typechecker, expr->compare.opcode);
      break;

    case WABT_EXPR_TYPE_CONST:
      wabt_typechecker_on_const(&ctx->typechecker, expr->const_.type);
      break;

    case WABT_EXPR_TYPE_CONVERT:
      wabt_typechecker_on_convert(&ctx->typechecker, expr->convert.opcode);
      break;

    case WABT_EXPR_TYPE_DROP:
      wabt_typechecker_on_drop(&ctx->typechecker);
      break;

    case WABT_EXPR_TYPE_GET_GLOBAL:
      wabt_typechecker_on_get_global(
          &ctx->typechecker,
          get_global_var_type_or_any(ctx, &expr->get_global.var));
      break;

    case WABT_EXPR_TYPE_GET_LOCAL:
      wabt_typechecker_on_get_local(
          &ctx->typechecker,
          get_local_var_type_or_any(ctx, &expr->get_local.var));
      break;

    case WABT_EXPR_TYPE_GROW_MEMORY:
      check_has_memory(ctx, &expr->loc, WABT_OPCODE_GROW_MEMORY);
      wabt_typechecker_on_grow_memory(&ctx->typechecker);
      break;

    case WABT_EXPR_TYPE_IF:
      wabt_typechecker_on_if(&ctx->typechecker, &expr->if_.true_.sig);
      check_expr_list(ctx, &expr->loc, expr->if_.true_.first);
      if (expr->if_.false_) {
        wabt_typechecker_on_else(&ctx->typechecker);
        check_expr_list(ctx, &expr->loc, expr->if_.false_);
      }
      wabt_typechecker_on_end(&ctx->typechecker);
      break;

    case WABT_EXPR_TYPE_LOAD:
      check_has_memory(ctx, &expr->loc, expr->load.opcode);
      check_align(ctx, &expr->loc, expr->load.align,
                  get_opcode_natural_alignment(expr->load.opcode));
      check_offset(ctx, &expr->loc, expr->load.offset);
      wabt_typechecker_on_load(&ctx->typechecker, expr->load.opcode);
      break;

    case WABT_EXPR_TYPE_LOOP:
      wabt_typechecker_on_loop(&ctx->typechecker, &expr->loop.sig);
      check_expr_list(ctx, &expr->loc, expr->loop.first);
      wabt_typechecker_on_end(&ctx->typechecker);
      break;

    case WABT_EXPR_TYPE_CURRENT_MEMORY:
      check_has_memory(ctx, &expr->loc, WABT_OPCODE_CURRENT_MEMORY);
      wabt_typechecker_on_current_memory(&ctx->typechecker);
      break;

    case WABT_EXPR_TYPE_NOP:
      break;

    case WABT_EXPR_TYPE_RETURN:
      wabt_typechecker_on_return(&ctx->typechecker);
      break;

    case WABT_EXPR_TYPE_SELECT:
      wabt_typechecker_on_select(&ctx->typechecker);
      break;

    case WABT_EXPR_TYPE_SET_GLOBAL:
      wabt_typechecker_on_set_global(
          &ctx->typechecker,
          get_global_var_type_or_any(ctx, &expr->set_global.var));
      break;

    case WABT_EXPR_TYPE_SET_LOCAL:
      wabt_typechecker_on_set_local(
          &ctx->typechecker,
          get_local_var_type_or_any(ctx, &expr->set_local.var));
      break;

    case WABT_EXPR_TYPE_STORE:
      check_has_memory(ctx, &expr->loc, expr->store.opcode);
      check_align(ctx, &expr->loc, expr->store.align,
                  get_opcode_natural_alignment(expr->store.opcode));
      check_offset(ctx, &expr->loc, expr->store.offset);
      wabt_typechecker_on_store(&ctx->typechecker, expr->store.opcode);
      break;

    case WABT_EXPR_TYPE_TEE_LOCAL:
      wabt_typechecker_on_tee_local(
          &ctx->typechecker,
          get_local_var_type_or_any(ctx, &expr->tee_local.var));
      break;

    case WABT_EXPR_TYPE_UNARY:
      wabt_typechecker_on_unary(&ctx->typechecker, expr->unary.opcode);
      break;

    case WABT_EXPR_TYPE_UNREACHABLE:
      wabt_typechecker_on_unreachable(&ctx->typechecker);
      break;
  }
}

static void check_func_signature_matches_func_type(
    Context* ctx,
    const WabtLocation* loc,
    const WabtFuncSignature* sig,
    const WabtFuncType* func_type) {
  check_types(ctx, loc, &sig->result_types, &func_type->sig.result_types,
              "function", "result");
  check_types(ctx, loc, &sig->param_types, &func_type->sig.param_types,
              "function", "argument");
}

static void check_func(Context* ctx,
                       const WabtLocation* loc,
                       const WabtFunc* func) {
  ctx->current_func = func;
  if (wabt_get_num_results(func) > 1) {
    print_error(ctx, loc, "multiple result values not currently supported.");
    /* don't run any other checks, the won't test the result_type properly */
    return;
  }
  if (wabt_decl_has_func_type(&func->decl)) {
    const WabtFuncType* func_type;
    if (WABT_SUCCEEDED(
            check_func_type_var(ctx, &func->decl.type_var, &func_type))) {
      check_func_signature_matches_func_type(ctx, loc, &func->decl.sig,
                                             func_type);
    }
  }

  ctx->expr_loc = loc;
  wabt_typechecker_begin_function(&ctx->typechecker,
                                  &func->decl.sig.result_types);
  check_expr_list(ctx, loc, func->first_expr);
  wabt_typechecker_end_function(&ctx->typechecker);
  ctx->current_func = nullptr;
}

static void print_const_expr_error(Context* ctx,
                                   const WabtLocation* loc,
                                   const char* desc) {
  print_error(ctx, loc,
              "invalid %s, must be a constant expression; either *.const or "
              "get_global.",
              desc);
}

static void check_const_init_expr(Context* ctx,
                                  const WabtLocation* loc,
                                  const WabtExpr* expr,
                                  WabtType expected_type,
                                  const char* desc) {
  WabtType type = WABT_TYPE_VOID;
  if (expr) {
    if (expr->next) {
      print_const_expr_error(ctx, loc, desc);
      return;
    }

    switch (expr->type) {
      case WABT_EXPR_TYPE_CONST:
        type = expr->const_.type;
        break;

      case WABT_EXPR_TYPE_GET_GLOBAL: {
        const WabtGlobal* ref_global = nullptr;
        int ref_global_index;
        if (WABT_FAILED(check_global_var(ctx, &expr->get_global.var,
                                         &ref_global, &ref_global_index))) {
          return;
        }

        type = ref_global->type;
        /* globals can only reference previously defined, internal globals */
        if (ref_global_index >= ctx->current_global_index) {
          print_error(ctx, loc,
                      "initializer expression can only reference a previously "
                      "defined global");
        } else if (ref_global_index >= ctx->num_imported_globals) {
          print_error(
              ctx, loc,
              "initializer expression can only reference an imported global");
        }

        if (ref_global->mutable_) {
          print_error(
              ctx, loc,
              "initializer expression cannot reference a mutable global");
        }
        break;
      }

      default:
        print_const_expr_error(ctx, loc, desc);
        return;
    }
  }

  check_type(ctx, expr ? &expr->loc : loc, type, expected_type, desc);
}

static void check_global(Context* ctx,
                         const WabtLocation* loc,
                         const WabtGlobal* global) {
  check_const_init_expr(ctx, loc, global->init_expr, global->type,
                        "global initializer expression");
}

static void check_limits(Context* ctx,
                         const WabtLocation* loc,
                         const WabtLimits* limits,
                         uint64_t absolute_max,
                         const char* desc) {
  if (limits->initial > absolute_max) {
    print_error(ctx, loc, "initial %s (%" PRIu64 ") must be <= (%" PRIu64 ")",
                desc, limits->initial, absolute_max);
  }

  if (limits->has_max) {
    if (limits->max > absolute_max) {
      print_error(ctx, loc, "max %s (%" PRIu64 ") must be <= (%" PRIu64 ")",
                  desc, limits->max, absolute_max);
    }

    if (limits->max < limits->initial) {
      print_error(ctx, loc,
                  "max %s (%" PRIu64 ") must be >= initial %s (%" PRIu64 ")",
                  desc, limits->max, desc, limits->initial);
    }
  }
}

static void check_table(Context* ctx,
                        const WabtLocation* loc,
                        const WabtTable* table) {
  if (ctx->current_table_index == 1)
    print_error(ctx, loc, "only one table allowed");
  check_limits(ctx, loc, &table->elem_limits, UINT32_MAX, "elems");
}

static void check_elem_segments(Context* ctx, const WabtModule* module) {
  WabtModuleField* field;
  for (field = module->first_field; field; field = field->next) {
    if (field->type != WABT_MODULE_FIELD_TYPE_ELEM_SEGMENT)
      continue;

    WabtElemSegment* elem_segment = &field->elem_segment;
    const WabtTable* table;
    if (!WABT_SUCCEEDED(
            check_table_var(ctx, &elem_segment->table_var, &table)))
      continue;

    size_t i;
    for (i = 0; i < elem_segment->vars.size; ++i) {
      if (!WABT_SUCCEEDED(
              check_func_var(ctx, &elem_segment->vars.data[i], nullptr)))
        continue;
    }

    check_const_init_expr(ctx, &field->loc, elem_segment->offset, WABT_TYPE_I32,
                          "elem segment offset");
  }
}

static void check_memory(Context* ctx,
                         const WabtLocation* loc,
                         const WabtMemory* memory) {
  if (ctx->current_memory_index == 1)
    print_error(ctx, loc, "only one memory block allowed");
  check_limits(ctx, loc, &memory->page_limits, WABT_MAX_PAGES, "pages");
}

static void check_data_segments(Context* ctx, const WabtModule* module) {
  WabtModuleField* field;
  for (field = module->first_field; field; field = field->next) {
    if (field->type != WABT_MODULE_FIELD_TYPE_DATA_SEGMENT)
      continue;

    WabtDataSegment* data_segment = &field->data_segment;
    const WabtMemory* memory;
    if (!WABT_SUCCEEDED(
            check_memory_var(ctx, &data_segment->memory_var, &memory)))
      continue;

    check_const_init_expr(ctx, &field->loc, data_segment->offset, WABT_TYPE_I32,
                          "data segment offset");
  }
}

static void check_import(Context* ctx,
                         const WabtLocation* loc,
                         const WabtImport* import) {
  switch (import->kind) {
    case WABT_EXTERNAL_KIND_FUNC:
      if (wabt_decl_has_func_type(&import->func.decl))
        check_func_type_var(ctx, &import->func.decl.type_var, nullptr);
      break;
    case WABT_EXTERNAL_KIND_TABLE:
      check_table(ctx, loc, &import->table);
      ctx->current_table_index++;
      break;
    case WABT_EXTERNAL_KIND_MEMORY:
      check_memory(ctx, loc, &import->memory);
      ctx->current_memory_index++;
      break;
    case WABT_EXTERNAL_KIND_GLOBAL:
      if (import->global.mutable_) {
        print_error(ctx, loc, "mutable globals cannot be imported");
      }
      ctx->num_imported_globals++;
      ctx->current_global_index++;
      break;
    case WABT_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
}

static void check_export(Context* ctx, const WabtExport* export_) {
  switch (export_->kind) {
    case WABT_EXTERNAL_KIND_FUNC:
      check_func_var(ctx, &export_->var, nullptr);
      break;
    case WABT_EXTERNAL_KIND_TABLE:
      check_table_var(ctx, &export_->var, nullptr);
      break;
    case WABT_EXTERNAL_KIND_MEMORY:
      check_memory_var(ctx, &export_->var, nullptr);
      break;
    case WABT_EXTERNAL_KIND_GLOBAL: {
      const WabtGlobal* global;
      if (WABT_SUCCEEDED(
              check_global_var(ctx, &export_->var, &global, nullptr))) {
        if (global->mutable_) {
          print_error(ctx, &export_->var.loc,
                      "mutable globals cannot be exported");
        }
      }
      break;
    }
    case WABT_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
}

static void on_duplicate_binding(WabtBindingHashEntry* a,
                                 WabtBindingHashEntry* b,
                                 void* user_data) {
  Context* ctx = (Context*)user_data;
  /* choose the location that is later in the file */
  WabtLocation* a_loc = &a->binding.loc;
  WabtLocation* b_loc = &b->binding.loc;
  WabtLocation* loc = a_loc->line > b_loc->line ? a_loc : b_loc;
  print_error(ctx, loc, "redefinition of export \"" PRIstringslice "\"",
              WABT_PRINTF_STRING_SLICE_ARG(a->binding.name));
}

static void check_duplicate_export_bindings(Context* ctx,
                                            const WabtModule* module) {
  wabt_find_duplicate_bindings(&module->export_bindings, on_duplicate_binding,
                               ctx);
}

static void check_module(Context* ctx, const WabtModule* module) {
  bool seen_start = false;

  ctx->current_module = module;
  ctx->current_table_index = 0;
  ctx->current_memory_index = 0;
  ctx->current_global_index = 0;
  ctx->num_imported_globals = 0;

  WabtModuleField* field;
  for (field = module->first_field; field; field = field->next) {
    switch (field->type) {
      case WABT_MODULE_FIELD_TYPE_FUNC:
        check_func(ctx, &field->loc, &field->func);
        break;

      case WABT_MODULE_FIELD_TYPE_GLOBAL:
        check_global(ctx, &field->loc, &field->global);
        ctx->current_global_index++;
        break;

      case WABT_MODULE_FIELD_TYPE_IMPORT:
        check_import(ctx, &field->loc, &field->import);
        break;

      case WABT_MODULE_FIELD_TYPE_EXPORT:
        check_export(ctx, &field->export_);
        break;

      case WABT_MODULE_FIELD_TYPE_TABLE:
        check_table(ctx, &field->loc, &field->table);
        ctx->current_table_index++;
        break;

      case WABT_MODULE_FIELD_TYPE_ELEM_SEGMENT:
        /* checked below */
        break;

      case WABT_MODULE_FIELD_TYPE_MEMORY:
        check_memory(ctx, &field->loc, &field->memory);
        ctx->current_memory_index++;
        break;

      case WABT_MODULE_FIELD_TYPE_DATA_SEGMENT:
        /* checked below */
        break;

      case WABT_MODULE_FIELD_TYPE_FUNC_TYPE:
        break;

      case WABT_MODULE_FIELD_TYPE_START: {
        if (seen_start) {
          print_error(ctx, &field->loc, "only one start function allowed");
        }

        const WabtFunc* start_func = nullptr;
        check_func_var(ctx, &field->start, &start_func);
        if (start_func) {
          if (wabt_get_num_params(start_func) != 0) {
            print_error(ctx, &field->loc, "start function must be nullary");
          }

          if (wabt_get_num_results(start_func) != 0) {
            print_error(ctx, &field->loc,
                        "start function must not return anything");
          }
        }
        seen_start = true;
        break;
      }
    }
  }

  check_elem_segments(ctx, module);
  check_data_segments(ctx, module);
  check_duplicate_export_bindings(ctx, module);
}

/* returns the result type of the invoked function, checked by the caller;
 * returning nullptr means that another error occured first, so the result type
 * should be ignored. */
static const WabtTypeVector* check_invoke(Context* ctx,
                                          const WabtAction* action) {
  const WabtActionInvoke* invoke = &action->invoke;
  const WabtModule* module =
      wabt_get_module_by_var(ctx->script, &action->module_var);
  if (!module) {
    print_error(ctx, &action->loc, "unknown module");
    return nullptr;
  }

  WabtExport* export_ = wabt_get_export_by_name(module, &invoke->name);
  if (!export_) {
    print_error(ctx, &action->loc,
                "unknown function export \"" PRIstringslice "\"",
                WABT_PRINTF_STRING_SLICE_ARG(invoke->name));
    return nullptr;
  }

  WabtFunc* func = wabt_get_func_by_var(module, &export_->var);
  if (!func) {
    /* this error will have already been reported, just skip it */
    return nullptr;
  }

  size_t actual_args = invoke->args.size;
  size_t expected_args = wabt_get_num_params(func);
  if (expected_args != actual_args) {
    print_error(ctx, &action->loc, "too %s parameters to function. got %" PRIzd
                                   ", expected %" PRIzd,
                actual_args > expected_args ? "many" : "few", actual_args,
                expected_args);
    return nullptr;
  }
  size_t i;
  for (i = 0; i < actual_args; ++i) {
    WabtConst* const_ = &invoke->args.data[i];
    check_type_index(ctx, &const_->loc, const_->type,
                     wabt_get_param_type(func, i), "invoke", i, "argument");
  }

  return &func->decl.sig.result_types;
}

static WabtResult check_get(Context* ctx,
                            const WabtAction* action,
                            WabtType* out_type) {
  const WabtActionGet* get = &action->get;
  const WabtModule* module =
      wabt_get_module_by_var(ctx->script, &action->module_var);
  if (!module) {
    print_error(ctx, &action->loc, "unknown module");
    return WABT_ERROR;
  }

  WabtExport* export_ = wabt_get_export_by_name(module, &get->name);
  if (!export_) {
    print_error(ctx, &action->loc,
                "unknown global export \"" PRIstringslice "\"",
                WABT_PRINTF_STRING_SLICE_ARG(get->name));
    return WABT_ERROR;
  }

  WabtGlobal* global = wabt_get_global_by_var(module, &export_->var);
  if (!global) {
    /* this error will have already been reported, just skip it */
    return WABT_ERROR;
  }

  *out_type = global->type;
  return WABT_OK;
}

static ActionResult check_action(Context* ctx, const WabtAction* action) {
  ActionResult result;
  WABT_ZERO_MEMORY(result);

  switch (action->type) {
    case WABT_ACTION_TYPE_INVOKE:
      result.types = check_invoke(ctx, action);
      result.kind =
          result.types ? ACTION_RESULT_KIND_TYPES : ACTION_RESULT_KIND_ERROR;
      break;

    case WABT_ACTION_TYPE_GET:
      if (WABT_SUCCEEDED(check_get(ctx, action, &result.type)))
        result.kind = ACTION_RESULT_KIND_TYPE;
      else
        result.kind = ACTION_RESULT_KIND_ERROR;
      break;
  }

  return result;
}

static void check_command(Context* ctx, const WabtCommand* command) {
  switch (command->type) {
    case WABT_COMMAND_TYPE_MODULE:
      check_module(ctx, &command->module);
      break;

    case WABT_COMMAND_TYPE_ACTION:
      /* ignore result type */
      check_action(ctx, &command->action);
      break;

    case WABT_COMMAND_TYPE_REGISTER:
    case WABT_COMMAND_TYPE_ASSERT_MALFORMED:
    case WABT_COMMAND_TYPE_ASSERT_INVALID:
    case WABT_COMMAND_TYPE_ASSERT_INVALID_NON_BINARY:
    case WABT_COMMAND_TYPE_ASSERT_UNLINKABLE:
    case WABT_COMMAND_TYPE_ASSERT_UNINSTANTIABLE:
      /* ignore */
      break;

    case WABT_COMMAND_TYPE_ASSERT_RETURN: {
      const WabtAction* action = &command->assert_return.action;
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

    case WABT_COMMAND_TYPE_ASSERT_RETURN_NAN: {
      const WabtAction* action = &command->assert_return_nan.action;
      ActionResult result = check_action(ctx, action);

      /* a valid result type will either be f32 or f64; convert a TYPES result
       * into a TYPE result, so it is easier to check below. WABT_TYPE_ANY is
       * used to specify a type that should not be checked (because an earlier
       * error occurred). */
      if (result.kind == ACTION_RESULT_KIND_TYPES) {
        if (result.types->size == 1) {
          result.kind = ACTION_RESULT_KIND_TYPE;
          result.type = result.types->data[0];
        } else {
          print_error(ctx, &action->loc, "expected 1 result, got %" PRIzd,
                      result.types->size);
          result.type = WABT_TYPE_ANY;
        }
      }

      if (result.kind == ACTION_RESULT_KIND_TYPE &&
          result.type != WABT_TYPE_ANY)
        check_assert_return_nan_type(ctx, &action->loc, result.type, "action");
      break;
    }

    case WABT_COMMAND_TYPE_ASSERT_TRAP:
    case WABT_COMMAND_TYPE_ASSERT_EXHAUSTION:
      /* ignore result type */
      check_action(ctx, &command->assert_trap.action);
      break;

    case WABT_NUM_COMMAND_TYPES:
      assert(0);
      break;
  }
}

static void wabt_destroy_context(Context* ctx) {
  wabt_destroy_typechecker(&ctx->typechecker);
}

WabtResult wabt_validate_script(WabtAstLexer* lexer,
                                const struct WabtScript* script,
                                WabtSourceErrorHandler* error_handler) {
  Context ctx;
  WABT_ZERO_MEMORY(ctx);
  ctx.lexer = lexer;
  ctx.error_handler = error_handler;
  ctx.result = WABT_OK;
  ctx.script = script;

  WabtTypeCheckerErrorHandler tc_error_handler;
  tc_error_handler.on_error = on_typechecker_error;
  tc_error_handler.user_data = &ctx;
  ctx.typechecker.error_handler = &tc_error_handler;

  size_t i;
  for (i = 0; i < script->commands.size; ++i)
    check_command(&ctx, &script->commands.data[i]);
  wabt_destroy_context(&ctx);
  return ctx.result;
}

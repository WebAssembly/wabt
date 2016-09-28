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

#include "wasm-binary-writer-spec.h"

#include <assert.h>

#include "wasm-ast.h"
#include "wasm-binary.h"
#include "wasm-binary-writer.h"
#include "wasm-config.h"
#include "wasm-writer.h"

#define DUMP_OCTETS_PER_LINE 16

#define CALLBACK0(ctx, member)                                     \
  do {                                                             \
    if (((ctx)->spec_options->member))                             \
      (ctx)->spec_options->member((ctx)->spec_options->user_data); \
  } while (0)

#define CALLBACK(ctx, member, ...)                                             \
  do {                                                                         \
    if (((ctx)->spec_options->member))                                         \
      (ctx)                                                                    \
          ->spec_options->member(__VA_ARGS__, (ctx)->spec_options->user_data); \
  } while (0)

typedef struct Context {
  WasmAllocator* allocator;
  WasmWriter* writer;
  size_t writer_offset;
  const WasmWriteBinaryOptions* options;
  const WasmWriteBinarySpecOptions* spec_options;
  WasmResult result;
} Context;

typedef struct ExprList {
  WasmExpr* first;
  WasmExpr* last;
} ExprList;

typedef struct ActionInfo {
  const WasmAction* action;
  int index;
  size_t num_results;
  union {
    /* when action->type == INVOKE; memory is shared with the callee's sig */
    const WasmTypeVector* result_types;
    /* when action->type == GET */
    WasmType result_type;
  };
} ActionInfo;

static void append_expr(ExprList* expr_list, WasmExpr* expr) {
  if (expr_list->last)
    expr_list->last->next = expr;
  else
    expr_list->first = expr;
  expr_list->last = expr;
}

static void append_const_expr(WasmAllocator* allocator,
                              ExprList* expr_list,
                              const WasmConst* const_) {
  WasmExpr* expr = wasm_new_const_expr(allocator);
  expr->const_ = *const_;
  append_expr(expr_list, expr);
}

static void append_i32_const_expr(WasmAllocator* allocator,
                                  ExprList* expr_list,
                                  uint32_t value) {
  WasmConst const_;
  const_.type = WASM_TYPE_I32;
  const_.u32 = value;
  append_const_expr(allocator, expr_list, &const_);
}

static void append_invoke_expr(WasmAllocator* allocator,
                               ExprList* expr_list,
                               const WasmActionInvoke* invoke,
                               int func_index) {
  size_t i;
  for (i = 0; i < invoke->args.size; ++i)
    append_const_expr(allocator, expr_list, &invoke->args.data[i]);

  WasmExpr* expr = wasm_new_call_expr(allocator);
  expr->call.var.type = WASM_VAR_TYPE_INDEX;
  expr->call.var.index = func_index;
  append_expr(expr_list, expr);
}

static void append_get_global_expr(WasmAllocator* allocator,
                                   ExprList* expr_list,
                                   const WasmActionGet* get,
                                   int global_index) {
  WasmExpr* expr = wasm_new_get_global_expr(allocator);
  expr->get_global.var.type = WASM_VAR_TYPE_INDEX;
  expr->get_global.var.index = global_index;
  append_expr(expr_list, expr);
}

static void append_eq_expr(WasmAllocator* allocator,
                           ExprList* expr_list,
                           WasmType type) {
  WasmExpr* expr = wasm_new_compare_expr(allocator);
  switch (type) {
    case WASM_TYPE_I32:
      expr->compare.opcode = WASM_OPCODE_I32_EQ;
      break;
    case WASM_TYPE_I64:
      expr->compare.opcode = WASM_OPCODE_I64_EQ;
      break;
    case WASM_TYPE_F32:
      expr->compare.opcode = WASM_OPCODE_F32_EQ;
      break;
    case WASM_TYPE_F64:
      expr->compare.opcode = WASM_OPCODE_F64_EQ;
      break;
    default:
      assert(0);
  }
  append_expr(expr_list, expr);
}

static void append_ne_expr(WasmAllocator* allocator,
                           ExprList* expr_list,
                           WasmType type) {
  WasmExpr* expr = wasm_new_compare_expr(allocator);
  switch (type) {
    case WASM_TYPE_I32:
      expr->compare.opcode = WASM_OPCODE_I32_NE;
      break;
    case WASM_TYPE_I64:
      expr->compare.opcode = WASM_OPCODE_I64_NE;
      break;
    case WASM_TYPE_F32:
      expr->compare.opcode = WASM_OPCODE_F32_NE;
      break;
    case WASM_TYPE_F64:
      expr->compare.opcode = WASM_OPCODE_F64_NE;
      break;
    default:
      assert(0);
  }
  append_expr(expr_list, expr);
}

static void append_tee_local_expr(WasmAllocator* allocator,
                                  ExprList* expr_list,
                                  int index) {
  WasmExpr* expr = wasm_new_tee_local_expr(allocator);
  expr->tee_local.var.type = WASM_VAR_TYPE_INDEX;
  expr->tee_local.var.index = index;
  append_expr(expr_list, expr);
}

static void append_get_local_expr(WasmAllocator* allocator,
                                  ExprList* expr_list,
                                  int index) {
  WasmExpr* expr = wasm_new_get_local_expr(allocator);
  expr->get_local.var.type = WASM_VAR_TYPE_INDEX;
  expr->get_local.var.index = index;
  append_expr(expr_list, expr);
}

static void append_reinterpret_expr(WasmAllocator* allocator,
                                    ExprList* expr_list,
                                    WasmType type) {
  WasmExpr* expr = wasm_new_convert_expr(allocator);
  switch (type) {
    case WASM_TYPE_F32:
      expr->convert.opcode = WASM_OPCODE_I32_REINTERPRET_F32;
      break;
    case WASM_TYPE_F64:
      expr->convert.opcode = WASM_OPCODE_I64_REINTERPRET_F64;
      break;
    default:
      assert(0);
      break;
  }
  append_expr(expr_list, expr);
}

static void append_return_expr(WasmAllocator* allocator, ExprList* expr_list) {
  WasmExpr* expr = wasm_new_return_expr(allocator);
  append_expr(expr_list, expr);
}

static WasmModuleField* append_module_field(
    WasmAllocator* allocator,
    WasmModule* module,
    WasmModuleFieldType module_field_type) {
  WasmModuleField* result = wasm_append_module_field(allocator, module);
  result->type = module_field_type;

  switch (module_field_type) {
    case WASM_MODULE_FIELD_TYPE_FUNC: {
      WasmFuncPtr* func_ptr = wasm_append_func_ptr(allocator, &module->funcs);
      *func_ptr = &result->func;
      break;
    }
    case WASM_MODULE_FIELD_TYPE_IMPORT: {
      WasmImportPtr* import_ptr =
          wasm_append_import_ptr(allocator, &module->imports);
      *import_ptr = &result->import;
      break;
    }
    case WASM_MODULE_FIELD_TYPE_EXPORT: {
      WasmExportPtr* export_ptr =
          wasm_append_export_ptr(allocator, &module->exports);
      *export_ptr = &result->export_;
      break;
    }
    case WASM_MODULE_FIELD_TYPE_FUNC_TYPE: {
      WasmFuncTypePtr* func_type_ptr =
          wasm_append_func_type_ptr(allocator, &module->func_types);
      *func_type_ptr = &result->func_type;
      break;
    }

    case WASM_MODULE_FIELD_TYPE_GLOBAL:
    case WASM_MODULE_FIELD_TYPE_TABLE:
    case WASM_MODULE_FIELD_TYPE_MEMORY:
    case WASM_MODULE_FIELD_TYPE_START:
    case WASM_MODULE_FIELD_TYPE_ELEM_SEGMENT:
    case WASM_MODULE_FIELD_TYPE_DATA_SEGMENT:
      /* not supported */
      assert(0);
      break;
  }

  return result;
}

static WasmStringSlice create_assert_func_name(WasmAllocator* allocator,
                                               const char* format,
                                               int format_index) {
  WasmStringSlice name;
  char buffer[256];
  int buffer_len = wasm_snprintf(buffer, 256, format, format_index);
  name.start = wasm_strndup(allocator, buffer, buffer_len);
  name.length = buffer_len;
  return name;
}

static WasmFunc* append_nullary_func(WasmAllocator* allocator,
                                     WasmModule* module,
                                     const WasmTypeVector* result_types,
                                     WasmStringSlice export_name) {
  WasmFuncType* func_type;
  WasmFuncSignature sig;
  WASM_ZERO_MEMORY(sig);
  /* OK to share memory w/ result_types, because we're just using it to see if
   * this signature already exists */
  sig.result_types = *result_types;
  int sig_index = wasm_get_func_type_index_by_sig(module, &sig);
  if (sig_index == -1) {
    WasmLocation loc;
    WASM_ZERO_MEMORY(loc);
    /* clone result_types so we don't share its memory */
    WASM_ZERO_MEMORY(sig.result_types);
    wasm_extend_types(allocator, &sig.result_types, result_types);
    func_type = wasm_append_implicit_func_type(allocator, &loc, module, &sig);
    sig_index = module->func_types.size - 1;
  } else {
    func_type = module->func_types.data[sig_index];
  }

  WasmModuleField* func_field =
      append_module_field(allocator, module, WASM_MODULE_FIELD_TYPE_FUNC);
  WasmFunc* func = &func_field->func;
  func->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_FUNC_TYPE;
  func->decl.type_var.type = WASM_VAR_TYPE_INDEX;
  func->decl.type_var.index = sig_index;
  func->decl.sig = func_type->sig;
  int func_index = module->funcs.size - 1;

  WasmModuleField* export_field =
      append_module_field(allocator, module, WASM_MODULE_FIELD_TYPE_EXPORT);
  WasmExport* export_ = &export_field->export_;
  export_->var.type = WASM_VAR_TYPE_INDEX;
  export_->var.index = func_index;
  export_->name = export_name;
  return module->funcs.data[func_index];
}

static WasmFunc* append_nullary_func_0(WasmAllocator* allocator,
                                       WasmModule* module,
                                       WasmStringSlice export_name) {
  WasmTypeVector types;
  WASM_ZERO_MEMORY(types);
  return append_nullary_func(allocator, module, &types, export_name);
}

static WasmFunc* append_nullary_func_1(WasmAllocator* allocator,
                                       WasmModule* module,
                                       WasmType type,
                                       WasmStringSlice export_name) {
  WasmTypeVector types;
  WASM_ZERO_MEMORY(types);
  types.size = 1;
  types.data = &type;
  return append_nullary_func(allocator, module, &types, export_name);
}

static ActionInfo get_action_info(const WasmModule* module,
                                  const WasmAction* action) {
  ActionInfo result;
  WASM_ZERO_MEMORY(result);

  switch (action->type) {
    case WASM_ACTION_TYPE_INVOKE: {
      WasmExport* export_ =
          wasm_get_export_by_name(module, &action->invoke.name);
      assert(export_);

      int func_index = wasm_get_func_index_by_var(module, &export_->var);
      assert(func_index >= 0 && (size_t)func_index < module->funcs.size);
      WasmFunc* callee = module->funcs.data[func_index];
      result.action = action;
      result.index = func_index;
      result.num_results = callee->decl.sig.result_types.size;
      result.result_types = &callee->decl.sig.result_types;
      break;
    }

    case WASM_ACTION_TYPE_GET: {
      WasmExport* export_ = wasm_get_export_by_name(module, &action->get.name);
      assert(export_);

      int global_index = wasm_get_global_index_by_var(module, &export_->var);
      assert(global_index >= 0 && (size_t)global_index < module->globals.size);
      WasmGlobal* global = module->globals.data[global_index];
      result.action = action;
      result.index = global_index;
      result.num_results = 1;
      result.result_type = global->type;
      break;
    }
  }
  return result;
}

static WasmType get_action_info_result_type(const ActionInfo* info) {
  assert(info->num_results == 1);

  switch (info->action->type) {
    case WASM_ACTION_TYPE_INVOKE:
      return info->result_types->data[0];

    case WASM_ACTION_TYPE_GET:
      return info->result_type;
  }
  WABT_UNREACHABLE;
}

static void append_action_expr(WasmAllocator* allocator,
                               WasmModule* module,
                               ExprList* expr_list,
                               const ActionInfo* info) {
  switch (info->action->type) {
    case WASM_ACTION_TYPE_INVOKE:
      append_invoke_expr(allocator, expr_list, &info->action->invoke,
                         info->index);
      break;

    case WASM_ACTION_TYPE_GET:
      append_get_global_expr(allocator, expr_list, &info->action->get,
                             info->index);
      break;
  }
}

static void write_module(Context* ctx, uint32_t index, WasmModule* module) {
  WasmResult result;
  WasmWriter* writer = NULL;
  CALLBACK(ctx, on_module_before_write, index, &writer);
  if (writer != NULL) {
    result =
        wasm_write_binary_module(ctx->allocator, writer, module, ctx->options);
  } else {
    result = WASM_ERROR;
  }
  if (WASM_FAILED(result))
    ctx->result = result;
  CALLBACK(ctx, on_module_end, index, result);
}

static void write_commands(Context* ctx, WasmScript* script) {
  uint32_t i;
  uint32_t num_modules = 0;
  WasmAllocator* allocator = script->allocator;
  WasmModule* last_module = NULL;
  uint32_t num_assert_funcs = 0;
  for (i = 0; i < script->commands.size; ++i) {
    WasmCommand* command = &script->commands.data[i];
    if (command->type == WASM_COMMAND_TYPE_MODULE) {
      if (last_module)
        write_module(ctx, num_modules, last_module);
      CALLBACK(ctx, on_module_begin, num_modules);
      last_module = &command->module;
      num_assert_funcs = 0;
      ++num_modules;
    } else {
      if (!last_module)
        continue;

      const char* format = NULL;
      WasmAction* action = NULL;
      switch (command->type) {
        case WASM_COMMAND_TYPE_ACTION:
          format = "$action_%d";
          action = &command->action;
          break;
        case WASM_COMMAND_TYPE_ASSERT_RETURN:
          format = "$assert_return_%d";
          action = &command->assert_return.action;
          break;
        case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
          format = "$assert_return_nan_%d";
          action = &command->assert_return_nan.action;
          break;
        case WASM_COMMAND_TYPE_ASSERT_TRAP:
          format = "$assert_trap_%d";
          action = &command->assert_trap.action;
          break;
        default:
          continue;
      }

      WasmStringSlice name =
          create_assert_func_name(allocator, format, num_assert_funcs);
      CALLBACK(ctx, on_command, num_assert_funcs, command->type, &name,
               &action->loc);

      ++num_assert_funcs;

      ActionInfo info = get_action_info(last_module, action);

      switch (command->type) {
        case WASM_COMMAND_TYPE_ACTION: {
          WasmFunc* caller =
              append_nullary_func_0(allocator, last_module, name);
          ExprList expr_list;
          WASM_ZERO_MEMORY(expr_list);
          append_action_expr(allocator, last_module, &expr_list, &info);
          append_return_expr(allocator, &expr_list);
          caller->first_expr = expr_list.first;
          break;
        }

        case WASM_COMMAND_TYPE_ASSERT_RETURN: {
          WasmFunc* caller = append_nullary_func_1(allocator, last_module,
                                                   WASM_TYPE_I32, name);
          ExprList expr_list;
          WASM_ZERO_MEMORY(expr_list);
          append_action_expr(allocator, last_module, &expr_list, &info);

          if (info.num_results == 1) {
            WasmType result_type = get_action_info_result_type(&info);
            const WasmConstVector* expected_consts =
                &command->assert_return.expected;
            if (expected_consts->size == 1) {
              WasmConst expected = expected_consts->data[0];
              if (expected.type == WASM_TYPE_F32) {
                append_reinterpret_expr(allocator, &expr_list, WASM_TYPE_F32);
                append_const_expr(allocator, &expr_list, &expected);
                append_reinterpret_expr(allocator, &expr_list, WASM_TYPE_F32);
                append_eq_expr(allocator, &expr_list, WASM_TYPE_I32);
              } else if (expected.type == WASM_TYPE_F64) {
                append_reinterpret_expr(allocator, &expr_list, WASM_TYPE_F64);
                append_const_expr(allocator, &expr_list, &expected);
                append_reinterpret_expr(allocator, &expr_list, WASM_TYPE_F64);
                append_eq_expr(allocator, &expr_list, WASM_TYPE_I64);
              } else {
                append_const_expr(allocator, &expr_list, &expected);
                append_eq_expr(allocator, &expr_list, result_type);
              }
            } else {
              /* function result count mismatch; just fail */
              append_i32_const_expr(allocator, &expr_list, 0);
              append_return_expr(allocator, &expr_list);
            }
          } else {
            /* 0 results or >1 results. If there are 0 results, we just assume
             * that everything was OK. We don't currenty support multiple
             * results, so consider that a failure. */
            append_i32_const_expr(allocator, &expr_list,
                                  info.num_results == 0 ? 1 : 0);
            append_return_expr(allocator, &expr_list);
          }

          caller->first_expr = expr_list.first;
          break;
        }

        case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN: {
          WasmFunc* caller = append_nullary_func_1(allocator, last_module,
                                                   WASM_TYPE_I32, name);
          ExprList expr_list;
          WASM_ZERO_MEMORY(expr_list);

          append_action_expr(allocator, last_module, &expr_list, &info);

          if (info.num_results == 1) {
            WasmType result_type = get_action_info_result_type(&info);
            wasm_append_type_value(allocator, &caller->local_types,
                                   &result_type);
            append_tee_local_expr(allocator, &expr_list, 0);
            append_get_local_expr(allocator, &expr_list, 0);
            /* x != x is true iff x is NaN */
            append_ne_expr(allocator, &expr_list, result_type);
          } else {
            /* assert_return_nan doesn't make sense w/ 0 or >1 results; just
             * fail */
            append_i32_const_expr(allocator, &expr_list, 0);
            append_return_expr(allocator, &expr_list);
          }

          caller->first_expr = expr_list.first;
          break;
        }

        case WASM_COMMAND_TYPE_ASSERT_TRAP: {
          WasmFunc* caller =
              append_nullary_func_0(allocator, last_module, name);
          ExprList expr_list;
          WASM_ZERO_MEMORY(expr_list);
          append_action_expr(allocator, last_module, &expr_list, &info);
          append_return_expr(allocator, &expr_list);
          caller->first_expr = expr_list.first;
          break;
        }

        default:
          assert(0);
      }
    }
  }
  if (last_module)
    write_module(ctx, num_modules, last_module);
}

WasmResult wasm_write_binary_spec_script(
    WasmAllocator* allocator,
    WasmScript* script,
    const WasmWriteBinaryOptions* options,
    const WasmWriteBinarySpecOptions* spec_options) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.options = options;
  ctx.spec_options = spec_options;
  ctx.result = WASM_OK;

  CALLBACK0(&ctx, on_script_begin);
  write_commands(&ctx, script);
  CALLBACK0(&ctx, on_script_end);

  return ctx.result;
}

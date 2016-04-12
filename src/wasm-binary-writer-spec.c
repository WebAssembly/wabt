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

#define ALLOC_FAILURE \
  fprintf(stderr, "%s:%d: allocation failed\n", __FILE__, __LINE__)

#define CHECK_ALLOC_(cond)      \
  do {                          \
    if (!(cond)) {               \
      ALLOC_FAILURE;            \
      ctx->result = WASM_ERROR; \
      return;                   \
    }                           \
  } while (0)

#define CHECK_ALLOC(e) CHECK_ALLOC_(WASM_SUCCEEDED(e))
#define CHECK_ALLOC_NULL(v) CHECK_ALLOC_((v) != NULL)

typedef struct WasmWriteSpecContext {
  WasmAllocator* allocator;
  WasmWriter* writer;
  size_t writer_offset;
  const WasmWriteBinaryOptions* options;
  const WasmWriteBinarySpecOptions* spec_options;
  WasmResult result;
} WasmWriteSpecContext;

static WasmBool is_nan_f32(uint32_t bits) {
  return (bits & 0x7f800000) == 0x7f800000 && (bits & 0x007fffff) != 0;
}

static WasmBool is_nan_f64(uint64_t bits) {
  return (bits & 0x7ff0000000000000LL) == 0x7ff0000000000000LL &&
         (bits & 0x000fffffffffffffLL) != 0;
}

static WasmExpr* create_const_expr(WasmAllocator* allocator,
                                   WasmConst* const_) {
  WasmExpr* expr = wasm_new_const_expr(allocator);
  if (!expr)
    return NULL;
  expr->const_ = *const_;
  return expr;
}

static WasmExpr* create_invoke_expr(WasmAllocator* allocator,
                                    WasmCommandInvoke* invoke,
                                    int func_index) {
  WasmExpr* expr = wasm_new_call_expr(allocator);
  if (!expr)
    return NULL;
  expr->call.var.type = WASM_VAR_TYPE_INDEX;
  expr->call.var.index = func_index;
  size_t i;
  for (i = 0; i < invoke->args.size; ++i) {
    WasmConst* arg = &invoke->args.data[i];
    WasmExprPtr* expr_ptr = wasm_append_expr_ptr(allocator, &expr->call.args);
    if (!expr_ptr)
      goto fail;
    *expr_ptr = create_const_expr(allocator, arg);
    if (!*expr_ptr)
      goto fail;
  }
  return expr;
fail:
  for (i = 0; i < invoke->args.size; ++i)
    wasm_destroy_expr_ptr(allocator, &expr->call.args.data[i]);
  wasm_free(allocator, expr);
  return NULL;
}

static WasmExpr* create_eq_expr(WasmAllocator* allocator,
                                WasmType type,
                                WasmExpr* left,
                                WasmExpr* right) {
  if (!left || !right) {
    wasm_destroy_expr_ptr(allocator, &left);
    wasm_destroy_expr_ptr(allocator, &right);
    return NULL;
  }

  WasmExpr* expr = wasm_new_compare_expr(allocator);
  if (!expr)
    return NULL;
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
  expr->compare.left = left;
  expr->compare.right = right;
  return expr;
}

static WasmExpr* create_ne_expr(WasmAllocator* allocator,
                                WasmType type,
                                WasmExpr* left,
                                WasmExpr* right) {
  if (!left || !right) {
    wasm_destroy_expr_ptr(allocator, &left);
    wasm_destroy_expr_ptr(allocator, &right);
    return NULL;
  }

  WasmExpr* expr = wasm_new_compare_expr(allocator);
  if (!expr)
    return NULL;
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
  expr->compare.left = left;
  expr->compare.right = right;
  return expr;
}

static WasmExpr* create_set_local_expr(WasmAllocator* allocator,
                                       int index,
                                       WasmExpr* value) {
  if (!value) {
    wasm_destroy_expr_ptr(allocator, &value);
    return NULL;
  }

  WasmExpr* expr = wasm_new_set_local_expr(allocator);
  if (!expr)
    return NULL;
  expr->set_local.var.type = WASM_VAR_TYPE_INDEX;
  expr->set_local.var.index = index;
  expr->set_local.expr = value;
  return expr;
}

static WasmExpr* create_get_local_expr(WasmAllocator* allocator, int index) {
  WasmExpr* expr = wasm_new_get_local_expr(allocator);
  if (!expr)
    return NULL;
  expr->get_local.var.type = WASM_VAR_TYPE_INDEX;
  expr->get_local.var.index = index;
  return expr;
}

static WasmExpr* create_reinterpret_expr(WasmAllocator* allocator,
                                         WasmType type,
                                         WasmExpr* expr) {
  if (!expr) {
    wasm_destroy_expr_ptr(allocator, &expr);
    return NULL;
  }

  WasmExpr* result = wasm_new_convert_expr(allocator);
  if (!result)
    return NULL;
  switch (type) {
    case WASM_TYPE_F32:
      result->convert.opcode = WASM_OPCODE_I32_REINTERPRET_F32;
      break;
    case WASM_TYPE_F64:
      result->convert.opcode = WASM_OPCODE_I64_REINTERPRET_F64;
      break;
    default:
      assert(0);
      break;
  }
  result->convert.expr = expr;
  return result;
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
      if (!func_ptr)
        goto fail;
      *func_ptr = &result->func;
      break;
    }
    case WASM_MODULE_FIELD_TYPE_IMPORT: {
      WasmImportPtr* import_ptr =
          wasm_append_import_ptr(allocator, &module->imports);
      if (!import_ptr)
        goto fail;
      *import_ptr = &result->import;
      break;
    }
    case WASM_MODULE_FIELD_TYPE_EXPORT: {
      WasmExportPtr* export_ptr =
          wasm_append_export_ptr(allocator, &module->exports);
      if (!export_ptr)
        goto fail;
      *export_ptr = &result->export_;
      break;
    }
    case WASM_MODULE_FIELD_TYPE_FUNC_TYPE: {
      WasmFuncTypePtr* func_type_ptr =
          wasm_append_func_type_ptr(allocator, &module->func_types);
      if (!func_type_ptr)
        goto fail;
      *func_type_ptr = &result->func_type;
      break;
    }

    case WASM_MODULE_FIELD_TYPE_TABLE:
    case WASM_MODULE_FIELD_TYPE_MEMORY:
    case WASM_MODULE_FIELD_TYPE_START:
    case WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY:
      /* not supported */
      assert(0);
      break;
  }

  return result;
fail:
  wasm_free(allocator, result);
  return NULL;
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
                                     WasmType result_type,
                                     WasmStringSlice export_name) {
  WasmModuleField* func_field =
      append_module_field(allocator, module, WASM_MODULE_FIELD_TYPE_FUNC);
  if (!func_field)
    return NULL;
  WasmFunc* func = &func_field->func;
  func->decl.flags = WASM_FUNC_DECLARATION_FLAG_HAS_SIGNATURE;
  func->decl.sig.result_type = result_type;
  int func_index = module->funcs.size - 1;

  WasmModuleField* export_field =
      append_module_field(allocator, module, WASM_MODULE_FIELD_TYPE_EXPORT);
  if (!export_field) {
    /* leave the func field, it will be cleaned up later */
    return NULL;
  }
  WasmExport* export = &export_field->export_;
  export->var.type = WASM_VAR_TYPE_INDEX;
  export->var.index = func_index;
  export->name = export_name;
  return module->funcs.data[func_index];
}

static void write_module(WasmWriteSpecContext* ctx,
                         uint32_t index,
                         WasmModule* module) {
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

static void write_commands(WasmWriteSpecContext* ctx, WasmScript* script) {
  uint32_t i;
  uint32_t num_modules = 0;
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
      const char* format = NULL;
      WasmCommandInvoke* invoke = NULL;
      switch (command->type) {
        case WASM_COMMAND_TYPE_INVOKE:
          format = "$invoke_%d";
          invoke = &command->invoke;
          break;
        case WASM_COMMAND_TYPE_ASSERT_RETURN:
          format = "$assert_return_%d";
          invoke = &command->assert_return.invoke;
          break;
        case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
          format = "$assert_return_nan_%d";
          invoke = &command->assert_return_nan.invoke;
          break;
        case WASM_COMMAND_TYPE_ASSERT_TRAP:
          format = "$assert_trap_%d";
          invoke = &command->assert_trap.invoke;
          break;
        default:
          continue;
      }

      WasmExport* export = wasm_get_export_by_name(last_module, &invoke->name);
      assert(export);
      int func_index = wasm_get_func_index_by_var(last_module, &export->var);
      assert(func_index >= 0 && (size_t)func_index < last_module->funcs.size);
      WasmFunc* callee = last_module->funcs.data[func_index];
      WasmType result_type = wasm_get_result_type(callee);
      /* these pointers will be invalidated later, so we can't use them */
      export = NULL;
      callee = NULL;

      WasmStringSlice name =
          create_assert_func_name(script->allocator, format, num_assert_funcs);

      CALLBACK(ctx, on_command, num_assert_funcs, command->type, &name,
               &invoke->loc);

      ++num_assert_funcs;

      WasmExprPtr* expr_ptr;
      switch (command->type) {
        case WASM_COMMAND_TYPE_INVOKE: {
          WasmFunc* caller = append_nullary_func(script->allocator, last_module,
                                                 result_type, name);
          CHECK_ALLOC_NULL(caller);
          expr_ptr = wasm_append_expr_ptr(script->allocator, &caller->exprs);
          CHECK_ALLOC_NULL(expr_ptr);
          *expr_ptr = create_invoke_expr(script->allocator, &command->invoke,
                                         func_index);
          CHECK_ALLOC_NULL(*expr_ptr);
          break;
        }

        case WASM_COMMAND_TYPE_ASSERT_RETURN: {
          WasmFunc* caller = append_nullary_func(script->allocator, last_module,
                                                 WASM_TYPE_I32, name);
          CHECK_ALLOC_NULL(caller);

          expr_ptr = wasm_append_expr_ptr(script->allocator, &caller->exprs);
          CHECK_ALLOC_NULL(expr_ptr);

          WasmExpr* invoke_expr = create_invoke_expr(
              script->allocator, &command->assert_return.invoke, func_index);
          CHECK_ALLOC_NULL(invoke_expr);

          if (result_type == WASM_TYPE_VOID) {
            /* The return type of the assert_return function is i32, but this
             invoked function has a return type of void, so we have nothing to
             compare to. Just return 1 to the caller, signifying everything is
             OK. */
            *expr_ptr = invoke_expr;
            WasmConst const_;
            const_.type = WASM_TYPE_I32;
            const_.u32 = 1;
            expr_ptr = wasm_append_expr_ptr(script->allocator, &caller->exprs);
            CHECK_ALLOC_NULL(expr_ptr);
            *expr_ptr = create_const_expr(script->allocator, &const_);
            CHECK_ALLOC_NULL(*expr_ptr);
          } else {
            WasmConst* expected = &command->assert_return.expected;
            WasmExpr* const_expr =
                create_const_expr(script->allocator, expected);
            CHECK_ALLOC_NULL(const_expr);

            if (expected->type == WASM_TYPE_F32 &&
                is_nan_f32(expected->f32_bits)) {
              *expr_ptr = create_eq_expr(
                  script->allocator, WASM_TYPE_I32,
                  create_reinterpret_expr(script->allocator, WASM_TYPE_F32,
                                          invoke_expr),
                  create_reinterpret_expr(script->allocator, WASM_TYPE_F32,
                                          const_expr));
              CHECK_ALLOC_NULL(*expr_ptr);
            } else if (expected->type == WASM_TYPE_F64 &&
                       is_nan_f64(expected->f64_bits)) {
              *expr_ptr = create_eq_expr(
                  script->allocator, WASM_TYPE_I64,
                  create_reinterpret_expr(script->allocator, WASM_TYPE_F64,
                                          invoke_expr),
                  create_reinterpret_expr(script->allocator, WASM_TYPE_F64,
                                          const_expr));
              CHECK_ALLOC_NULL(*expr_ptr);
            } else {
              *expr_ptr = create_eq_expr(script->allocator, result_type,
                                         invoke_expr, const_expr);
              CHECK_ALLOC_NULL(*expr_ptr);
            }
          }
          break;
        }

        case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN: {
          WasmFunc* caller = append_nullary_func(script->allocator, last_module,
                                                 WASM_TYPE_I32, name);
          CHECK_ALLOC_NULL(caller);
          CHECK_ALLOC(wasm_append_type_value(
              script->allocator, &caller->local_types, &result_type));
          expr_ptr = wasm_append_expr_ptr(script->allocator, &caller->exprs);
          CHECK_ALLOC_NULL(expr_ptr);
          *expr_ptr = create_set_local_expr(
              script->allocator, 0,
              create_invoke_expr(script->allocator,
                                 &command->assert_return_nan.invoke,
                                 func_index));
          CHECK_ALLOC_NULL(*expr_ptr);
          /* x != x is true iff x is NaN */
          expr_ptr = wasm_append_expr_ptr(script->allocator, &caller->exprs);
          CHECK_ALLOC_NULL(expr_ptr);
          *expr_ptr =
              create_ne_expr(script->allocator, result_type,
                             create_get_local_expr(script->allocator, 0),
                             create_get_local_expr(script->allocator, 0));
          CHECK_ALLOC_NULL(*expr_ptr);
          break;
        }

        case WASM_COMMAND_TYPE_ASSERT_TRAP: {
          WasmFunc* caller = append_nullary_func(script->allocator, last_module,
                                                 result_type, name);
          CHECK_ALLOC_NULL(caller);
          expr_ptr = wasm_append_expr_ptr(script->allocator, &caller->exprs);
          CHECK_ALLOC_NULL(expr_ptr);
          *expr_ptr = create_invoke_expr(script->allocator, &command->invoke,
                                         func_index);
          CHECK_ALLOC_NULL(*expr_ptr);
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
  WasmWriteSpecContext ctx;
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

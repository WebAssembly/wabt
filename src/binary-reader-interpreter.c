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

#include "binary-reader-interpreter.h"

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include "binary-reader.h"
#include "interpreter.h"
#include "type-checker.h"
#include "writer.h"

#define CHECK_RESULT(expr) \
  do {                     \
    if (WABT_FAILED(expr)) \
      return WABT_ERROR;   \
  } while (0)

#define CHECK_LOCAL(ctx, local_index)                                       \
  do {                                                                      \
    uint32_t max_local_index =                                              \
        (ctx)->current_func->defined.param_and_local_types.size;            \
    if ((local_index) >= max_local_index) {                                 \
      print_error((ctx), "invalid local_index: %d (max %d)", (local_index), \
                  max_local_index);                                         \
      return WABT_ERROR;                                                    \
    }                                                                       \
  } while (0)

#define CHECK_GLOBAL(ctx, global_index)                                       \
  do {                                                                        \
    uint32_t max_global_index = (ctx)->global_index_mapping.size;             \
    if ((global_index) >= max_global_index) {                                 \
      print_error((ctx), "invalid global_index: %d (max %d)", (global_index), \
                  max_global_index);                                          \
      return WABT_ERROR;                                                      \
    }                                                                         \
  } while (0)

typedef uint32_t Uint32;
WABT_DEFINE_VECTOR(uint32, Uint32);
WABT_DEFINE_VECTOR(uint32_vector, Uint32Vector);

typedef struct Label {
  uint32_t offset; /* branch location in the istream */
  uint32_t fixup_offset;
} Label;
WABT_DEFINE_VECTOR(label, Label);

typedef struct Context {
  WabtBinaryReader* reader;
  WabtBinaryErrorHandler* error_handler;
  WabtInterpreterEnvironment* env;
  WabtInterpreterModule* module;
  WabtInterpreterFunc* current_func;
  WabtTypeChecker typechecker;
  LabelVector label_stack;
  Uint32VectorVector func_fixups;
  Uint32VectorVector depth_fixups;
  WabtMemoryWriter istream_writer;
  uint32_t istream_offset;
  /* mappings from module index space to env index space; this won't just be a
   * translation, because imported values will be resolved as well */
  Uint32Vector sig_index_mapping;
  Uint32Vector func_index_mapping;
  Uint32Vector global_index_mapping;

  uint32_t num_func_imports;
  uint32_t num_global_imports;

  /* values cached in the Context so they can be shared between callbacks */
  WabtInterpreterTypedValue init_expr_value;
  uint32_t table_offset;
  WabtBool is_host_import;
  WabtInterpreterModule* host_import_module;
  uint32_t import_env_index;
} Context;

static Label* get_label(Context* ctx, uint32_t depth) {
  assert(depth < ctx->label_stack.size);
  return &ctx->label_stack.data[ctx->label_stack.size - depth - 1];
}

static Label* top_label(Context* ctx) {
  return get_label(ctx, 0);
}

static void handle_error(uint32_t offset, const char* message, Context* ctx) {
  if (ctx->error_handler->on_error) {
    ctx->error_handler->on_error(offset, message,
                                 ctx->error_handler->user_data);
  }
}

static void WABT_PRINTF_FORMAT(2, 3)
    print_error(Context* ctx, const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  handle_error(WABT_INVALID_OFFSET, buffer, ctx);
}

static void on_typechecker_error(const char* msg, void* user_data) {
  Context* ctx = user_data;
  print_error(ctx, "%s", msg);
}

static uint32_t translate_sig_index_to_env(Context* ctx, uint32_t sig_index) {
  assert(sig_index < ctx->sig_index_mapping.size);
  return ctx->sig_index_mapping.data[sig_index];
}

static WabtInterpreterFuncSignature* get_signature_by_env_index(
    Context* ctx,
    uint32_t sig_index) {
  assert(sig_index < ctx->env->sigs.size);
  return &ctx->env->sigs.data[sig_index];
}

static WabtInterpreterFuncSignature* get_signature_by_module_index(
    Context* ctx,
    uint32_t sig_index) {
  return get_signature_by_env_index(ctx,
                                    translate_sig_index_to_env(ctx, sig_index));
}

static uint32_t translate_func_index_to_env(Context* ctx, uint32_t func_index) {
  assert(func_index < ctx->func_index_mapping.size);
  return ctx->func_index_mapping.data[func_index];
}

static uint32_t translate_module_func_index_to_defined(Context* ctx,
                                                       uint32_t func_index) {
  assert(func_index >= ctx->num_func_imports);
  return func_index - ctx->num_func_imports;
}

static WabtInterpreterFunc* get_func_by_env_index(Context* ctx,
                                                  uint32_t func_index) {
  assert(func_index < ctx->env->funcs.size);
  return &ctx->env->funcs.data[func_index];
}

static WabtInterpreterFunc* get_func_by_module_index(Context* ctx,
                                                     uint32_t func_index) {
  return get_func_by_env_index(ctx,
                               translate_func_index_to_env(ctx, func_index));
}

static uint32_t translate_global_index_to_env(Context* ctx,
                                              uint32_t global_index) {
  assert(global_index < ctx->global_index_mapping.size);
  return ctx->global_index_mapping.data[global_index];
}

static WabtInterpreterGlobal* get_global_by_env_index(Context* ctx,
                                                      uint32_t global_index) {
  assert(global_index < ctx->env->globals.size);
  return &ctx->env->globals.data[global_index];
}

static WabtInterpreterGlobal* get_global_by_module_index(
    Context* ctx,
    uint32_t global_index) {
  return get_global_by_env_index(
      ctx, translate_global_index_to_env(ctx, global_index));
}

static WabtType get_global_type_by_module_index(Context* ctx,
                                                uint32_t global_index) {
  return get_global_by_module_index(ctx, global_index)->typed_value.type;
}

static WabtType get_local_type_by_index(WabtInterpreterFunc* func,
                                        uint32_t local_index) {
  assert(!func->is_host);
  assert(local_index < func->defined.param_and_local_types.size);
  return func->defined.param_and_local_types.data[local_index];
}

static uint32_t get_istream_offset(Context* ctx) {
  return ctx->istream_offset;
}

static WabtResult emit_data_at(Context* ctx,
                               size_t offset,
                               const void* data,
                               size_t size) {
  return ctx->istream_writer.base.write_data(
      offset, data, size, ctx->istream_writer.base.user_data);
}

static WabtResult emit_data(Context* ctx, const void* data, size_t size) {
  CHECK_RESULT(emit_data_at(ctx, ctx->istream_offset, data, size));
  ctx->istream_offset += size;
  return WABT_OK;
}

static WabtResult emit_opcode(Context* ctx, WabtOpcode opcode) {
  return emit_data(ctx, &opcode, sizeof(uint8_t));
}

static WabtResult emit_i8(Context* ctx, uint8_t value) {
  return emit_data(ctx, &value, sizeof(value));
}

static WabtResult emit_i32(Context* ctx, uint32_t value) {
  return emit_data(ctx, &value, sizeof(value));
}

static WabtResult emit_i64(Context* ctx, uint64_t value) {
  return emit_data(ctx, &value, sizeof(value));
}

static WabtResult emit_i32_at(Context* ctx, uint32_t offset, uint32_t value) {
  return emit_data_at(ctx, offset, &value, sizeof(value));
}

static WabtResult emit_drop_keep(Context* ctx, uint32_t drop, uint8_t keep) {
  assert(drop != UINT32_MAX);
  assert(keep <= 1);
  if (drop > 0) {
    if (drop == 1 && keep == 0) {
      CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_DROP));
    } else {
      CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_DROP_KEEP));
      CHECK_RESULT(emit_i32(ctx, drop));
      CHECK_RESULT(emit_i8(ctx, keep));
    }
  }
  return WABT_OK;
}

static WabtResult append_fixup(Context* ctx,
                               Uint32VectorVector* fixups_vector,
                               uint32_t index) {
  if (index >= fixups_vector->size)
    wabt_resize_uint32_vector_vector(fixups_vector, index + 1);
  Uint32Vector* fixups = &fixups_vector->data[index];
  uint32_t offset = get_istream_offset(ctx);
  wabt_append_uint32_value(fixups, &offset);
  return WABT_OK;
}

static WabtResult emit_br_offset(Context* ctx,
                                 uint32_t depth,
                                 uint32_t offset) {
  if (offset == WABT_INVALID_OFFSET) {
    /* depth_fixups stores the depth counting up from zero, where zero is the
     * top-level function scope. */
    depth =  ctx->label_stack.size - 1 - depth;
    CHECK_RESULT(
        append_fixup(ctx, &ctx->depth_fixups, depth));
  }
  CHECK_RESULT(emit_i32(ctx, offset));
  return WABT_OK;
}

static WabtResult get_br_drop_keep_count(Context* ctx,
                                         uint32_t depth,
                                         uint32_t* out_drop_count,
                                         uint32_t* out_keep_count) {
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(wabt_typechecker_get_label(&ctx->typechecker, depth, &label));
  *out_keep_count =
      label->label_type != WABT_LABEL_TYPE_LOOP ? label->sig.size : 0;
  if (wabt_typechecker_is_unreachable(&ctx->typechecker)) {
    *out_drop_count = 0;
  } else {
    *out_drop_count =
        (ctx->typechecker.type_stack.size - label->type_stack_limit) -
        *out_keep_count;
  }
  return WABT_OK;
}

static WabtResult get_return_drop_keep_count(Context* ctx,
                                             uint32_t* out_drop_count,
                                             uint32_t* out_keep_count) {
  if (WABT_FAILED(get_br_drop_keep_count(ctx, ctx->label_stack.size - 1,
                                         out_drop_count, out_keep_count))) {
    return WABT_ERROR;
  }

  *out_drop_count += ctx->current_func->defined.param_and_local_types.size;
  return WABT_OK;
}

static WabtResult emit_br(Context* ctx,
                          uint32_t depth,
                          uint32_t drop_count,
                          uint32_t keep_count) {
  CHECK_RESULT(emit_drop_keep(ctx, drop_count, keep_count));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_BR));
  CHECK_RESULT(emit_br_offset(ctx, depth, get_label(ctx, depth)->offset));
  return WABT_OK;
}

static WabtResult emit_br_table_offset(Context* ctx, uint32_t depth) {
  uint32_t drop_count, keep_count;
  CHECK_RESULT(get_br_drop_keep_count(ctx, depth, &drop_count, &keep_count));
  CHECK_RESULT(emit_br_offset(ctx, depth, get_label(ctx, depth)->offset));
  CHECK_RESULT(emit_i32(ctx, drop_count));
  CHECK_RESULT(emit_i8(ctx, keep_count));
  return WABT_OK;
}

static WabtResult fixup_top_label(Context* ctx) {
  uint32_t offset = get_istream_offset(ctx);
  uint32_t top = ctx->label_stack.size - 1;
  if (top >= ctx->depth_fixups.size) {
    /* nothing to fixup */
    return WABT_OK;
  }

  Uint32Vector* fixups = &ctx->depth_fixups.data[top];
  uint32_t i;
  for (i = 0; i < fixups->size; ++i)
    CHECK_RESULT(emit_i32_at(ctx, fixups->data[i], offset));
  /* reduce the size to 0 in case this gets reused. Keep the allocations for
   * later use */
  fixups->size = 0;
  return WABT_OK;
}

static WabtResult emit_func_offset(Context* ctx,
                                   WabtInterpreterFunc* func,
                                   uint32_t func_index) {
  if (func->defined.offset == WABT_INVALID_OFFSET) {
    uint32_t defined_index =
        translate_module_func_index_to_defined(ctx, func_index);
    CHECK_RESULT(append_fixup(ctx, &ctx->func_fixups, defined_index));
  }
  CHECK_RESULT(emit_i32(ctx, func->defined.offset));
  return WABT_OK;
}

static void on_error(WabtBinaryReaderContext* ctx, const char* message) {
  handle_error(ctx->offset, message, ctx->user_data);
}

static WabtResult on_signature_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wabt_resize_uint32_vector(&ctx->sig_index_mapping, count);
  uint32_t i;
  for (i = 0; i < count; ++i)
    ctx->sig_index_mapping.data[i] = ctx->env->sigs.size + i;
  wabt_resize_interpreter_func_signature_vector(&ctx->env->sigs,
                                                ctx->env->sigs.size + count);
  return WABT_OK;
}

static WabtResult on_signature(uint32_t index,
                               uint32_t param_count,
                               WabtType* param_types,
                               uint32_t result_count,
                               WabtType* result_types,
                               void* user_data) {
  Context* ctx = user_data;
  WabtInterpreterFuncSignature* sig = get_signature_by_module_index(ctx, index);

  wabt_reserve_types(&sig->param_types, param_count);
  sig->param_types.size = param_count;
  memcpy(sig->param_types.data, param_types, param_count * sizeof(WabtType));

  wabt_reserve_types(&sig->result_types, result_count);
  sig->result_types.size = result_count;
  memcpy(sig->result_types.data, result_types, result_count * sizeof(WabtType));
  return WABT_OK;
}

static WabtResult on_import_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wabt_new_interpreter_import_array(&ctx->module->defined.imports, count);
  return WABT_OK;
}

static WabtResult on_import(uint32_t index,
                            WabtStringSlice module_name,
                            WabtStringSlice field_name,
                            void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->defined.imports.size);
  WabtInterpreterImport* import = &ctx->module->defined.imports.data[index];
  import->module_name = wabt_dup_string_slice(module_name);
  import->field_name = wabt_dup_string_slice(field_name);
  int module_index = wabt_find_binding_index_by_name(
      &ctx->env->registered_module_bindings, &import->module_name);
  if (module_index < 0) {
    print_error(ctx, "unknown import module \"" PRIstringslice "\"",
                WABT_PRINTF_STRING_SLICE_ARG(import->module_name));
    return WABT_ERROR;
  }

  assert((size_t)module_index < ctx->env->modules.size);
  WabtInterpreterModule* module = &ctx->env->modules.data[module_index];
  if (module->is_host) {
    /* We don't yet know the kind of a host import module, so just assume it
     * exists for now. We'll fail later (in on_import_* below) if it doesn't
     * exist). */
    ctx->is_host_import = WABT_TRUE;
    ctx->host_import_module = module;
  } else {
    WabtInterpreterExport* export =
        wabt_get_interpreter_export_by_name(module, &import->field_name);
    if (!export) {
      print_error(ctx, "unknown module field \"" PRIstringslice "\"",
                  WABT_PRINTF_STRING_SLICE_ARG(import->field_name));
      return WABT_ERROR;
    }

    import->kind = export->kind;
    ctx->is_host_import = WABT_FALSE;
    ctx->import_env_index = export->index;
  }
  return WABT_OK;
}

static WabtResult check_import_kind(Context* ctx,
                                    WabtInterpreterImport* import,
                                    WabtExternalKind expected_kind) {
  if (import->kind != expected_kind) {
    print_error(ctx, "expected import \"" PRIstringslice "." PRIstringslice
                     "\" to have kind %s, not %s",
                WABT_PRINTF_STRING_SLICE_ARG(import->module_name),
                WABT_PRINTF_STRING_SLICE_ARG(import->field_name),
                wabt_get_kind_name(expected_kind),
                wabt_get_kind_name(import->kind));
    return WABT_ERROR;
  }
  return WABT_OK;
}

static WabtResult check_import_limits(Context* ctx,
                                      const WabtLimits* declared_limits,
                                      const WabtLimits* actual_limits) {
  if (actual_limits->initial < declared_limits->initial) {
    print_error(ctx,
                "actual size (%" PRIu64 ") smaller than declared (%" PRIu64 ")",
                actual_limits->initial, declared_limits->initial);
    return WABT_ERROR;
  }

  if (declared_limits->has_max) {
    if (!actual_limits->has_max) {
      print_error(ctx,
                  "max size (unspecified) larger than declared (%" PRIu64 ")",
                  declared_limits->max);
      return WABT_ERROR;
    } else if (actual_limits->max > declared_limits->max) {
      print_error(ctx,
                  "max size (%" PRIu64 ") larger than declared (%" PRIu64 ")",
                  actual_limits->max, declared_limits->max);
      return WABT_ERROR;
    }
  }

  return WABT_OK;
}

static WabtResult append_export(Context* ctx,
                                WabtInterpreterModule* module,
                                WabtExternalKind kind,
                                uint32_t item_index,
                                WabtStringSlice name) {
  if (wabt_find_binding_index_by_name(&module->export_bindings, &name) != -1) {
    print_error(ctx, "duplicate export \"" PRIstringslice "\"",
                WABT_PRINTF_STRING_SLICE_ARG(name));
    return WABT_ERROR;
  }

  WabtInterpreterExport* export =
      wabt_append_interpreter_export(&module->exports);
  export->name = wabt_dup_string_slice(name);
  export->kind = kind;
  export->index = item_index;

  WabtBinding* binding =
      wabt_insert_binding(&module->export_bindings, &export->name);
  binding->index = module->exports.size - 1;
  return WABT_OK;
}

static void on_host_import_print_error(const char* msg, void* user_data) {
  Context* ctx = user_data;
  print_error(ctx, "%s", msg);
}

static WabtPrintErrorCallback make_print_error_callback(Context* ctx) {
  WabtPrintErrorCallback result;
  result.print_error = on_host_import_print_error;
  result.user_data = ctx;
  return result;
}

static WabtResult on_import_func(uint32_t import_index,
                                 uint32_t func_index,
                                 uint32_t sig_index,
                                 void* user_data) {
  Context* ctx = user_data;
  assert(import_index < ctx->module->defined.imports.size);
  WabtInterpreterImport* import =
      &ctx->module->defined.imports.data[import_index];
  assert(sig_index < ctx->env->sigs.size);
  import->func.sig_index = translate_sig_index_to_env(ctx, sig_index);

  uint32_t func_env_index;
  if (ctx->is_host_import) {
    WabtInterpreterFunc* func = wabt_append_interpreter_func(&ctx->env->funcs);
    func->is_host = WABT_TRUE;
    func->sig_index = import->func.sig_index;
    func->host.module_name = import->module_name;
    func->host.field_name = import->field_name;

    WabtInterpreterHostImportDelegate* host_delegate =
        &ctx->host_import_module->host.import_delegate;
    WabtInterpreterFuncSignature* sig = &ctx->env->sigs.data[func->sig_index];
    CHECK_RESULT(host_delegate->import_func(import, func, sig,
                                            make_print_error_callback(ctx),
                                            host_delegate->user_data));
    assert(func->host.callback);

    func_env_index = ctx->env->funcs.size - 1;
    append_export(ctx, ctx->host_import_module, WABT_EXTERNAL_KIND_FUNC,
                  func_env_index, import->field_name);
  } else {
    CHECK_RESULT(check_import_kind(ctx, import, WABT_EXTERNAL_KIND_FUNC));
    assert(ctx->import_env_index < ctx->env->funcs.size);
    WabtInterpreterFunc* func = &ctx->env->funcs.data[ctx->import_env_index];
    if (!wabt_func_signatures_are_equal(ctx->env, import->func.sig_index,
                                        func->sig_index)) {
      print_error(ctx, "import signature mismatch");
      return WABT_ERROR;
    }

    func_env_index = ctx->import_env_index;
  }
  wabt_append_uint32_value(&ctx->func_index_mapping, &func_env_index);
  ctx->num_func_imports++;
  return WABT_OK;
}

static void init_table_func_indexes(Context* ctx, WabtInterpreterTable* table) {
  wabt_new_uint32_array(&table->func_indexes, table->limits.initial);
  size_t i;
  for (i = 0; i < table->func_indexes.size; ++i)
    table->func_indexes.data[i] = WABT_INVALID_INDEX;
}

static WabtResult on_import_table(uint32_t import_index,
                                  uint32_t table_index,
                                  WabtType elem_type,
                                  const WabtLimits* elem_limits,
                                  void* user_data) {
  Context* ctx = user_data;
  if (ctx->module->table_index != WABT_INVALID_INDEX) {
    print_error(ctx, "only one table allowed");
    return WABT_ERROR;
  }

  assert(import_index < ctx->module->defined.imports.size);
  WabtInterpreterImport* import =
      &ctx->module->defined.imports.data[import_index];

  if (ctx->is_host_import) {
    WabtInterpreterTable* table =
        wabt_append_interpreter_table(&ctx->env->tables);
    table->limits = *elem_limits;
    init_table_func_indexes(ctx, table);

    WabtInterpreterHostImportDelegate* host_delegate =
        &ctx->host_import_module->host.import_delegate;
    CHECK_RESULT(host_delegate->import_table(import, table,
                                             make_print_error_callback(ctx),
                                             host_delegate->user_data));

    CHECK_RESULT(check_import_limits(ctx, elem_limits, &table->limits));

    ctx->module->table_index = ctx->env->tables.size - 1;
    append_export(ctx, ctx->host_import_module, WABT_EXTERNAL_KIND_TABLE,
                  ctx->module->table_index, import->field_name);
  } else {
    CHECK_RESULT(check_import_kind(ctx, import, WABT_EXTERNAL_KIND_TABLE));
    assert(ctx->import_env_index < ctx->env->tables.size);
    WabtInterpreterTable* table = &ctx->env->tables.data[ctx->import_env_index];
    CHECK_RESULT(check_import_limits(ctx, elem_limits, &table->limits));

    import->table.limits = *elem_limits;
    ctx->module->table_index = ctx->import_env_index;
  }
  return WABT_OK;
}

static WabtResult on_import_memory(uint32_t import_index,
                                   uint32_t memory_index,
                                   const WabtLimits* page_limits,
                                   void* user_data) {
  Context* ctx = user_data;
  if (ctx->module->memory_index != WABT_INVALID_INDEX) {
    print_error(ctx, "only one memory allowed");
    return WABT_ERROR;
  }

  assert(import_index < ctx->module->defined.imports.size);
  WabtInterpreterImport* import =
      &ctx->module->defined.imports.data[import_index];

  if (ctx->is_host_import) {
    WabtInterpreterMemory* memory =
        wabt_append_interpreter_memory(&ctx->env->memories);

    WabtInterpreterHostImportDelegate* host_delegate =
        &ctx->host_import_module->host.import_delegate;
    CHECK_RESULT(host_delegate->import_memory(import, memory,
                                              make_print_error_callback(ctx),
                                              host_delegate->user_data));

    assert(memory->data);
    CHECK_RESULT(check_import_limits(ctx, page_limits, &memory->page_limits));

    ctx->module->memory_index = ctx->env->memories.size - 1;
    append_export(ctx, ctx->host_import_module, WABT_EXTERNAL_KIND_MEMORY,
                  ctx->module->memory_index, import->field_name);
  } else {
    CHECK_RESULT(check_import_kind(ctx, import, WABT_EXTERNAL_KIND_MEMORY));
    assert(ctx->import_env_index < ctx->env->memories.size);
    WabtInterpreterMemory* memory =
        &ctx->env->memories.data[ctx->import_env_index];
    CHECK_RESULT(check_import_limits(ctx, page_limits, &memory->page_limits));

    import->memory.limits = *page_limits;
    ctx->module->memory_index = ctx->import_env_index;
  }
  return WABT_OK;
}

static WabtResult on_import_global(uint32_t import_index,
                                   uint32_t global_index,
                                   WabtType type,
                                   WabtBool mutable,
                                   void* user_data) {
  Context* ctx = user_data;
  assert(import_index < ctx->module->defined.imports.size);
  WabtInterpreterImport* import =
      &ctx->module->defined.imports.data[import_index];

  uint32_t global_env_index = ctx->env->globals.size - 1;
  if (ctx->is_host_import) {
    WabtInterpreterGlobal* global =
        wabt_append_interpreter_global(&ctx->env->globals);
    global->typed_value.type = type;
    global->mutable_ = mutable;

    WabtInterpreterHostImportDelegate* host_delegate =
        &ctx->host_import_module->host.import_delegate;
    CHECK_RESULT(host_delegate->import_global(import, global,
                                              make_print_error_callback(ctx),
                                              host_delegate->user_data));

    global_env_index = ctx->env->globals.size - 1;
    append_export(ctx, ctx->host_import_module, WABT_EXTERNAL_KIND_GLOBAL,
                  global_env_index, import->field_name);
  } else {
    CHECK_RESULT(check_import_kind(ctx, import, WABT_EXTERNAL_KIND_GLOBAL));
    // TODO: check type and mutability
    import->global.type = type;
    import->global.mutable_ = mutable;
    global_env_index = ctx->import_env_index;
  }
  wabt_append_uint32_value(&ctx->global_index_mapping, &global_env_index);
  ctx->num_global_imports++;
  return WABT_OK;
}

static WabtResult on_function_signatures_count(uint32_t count,
                                               void* user_data) {
  Context* ctx = user_data;
  size_t old_size = ctx->func_index_mapping.size;
  wabt_resize_uint32_vector(&ctx->func_index_mapping, old_size + count);
  uint32_t i;
  for (i = 0; i < count; ++i)
    ctx->func_index_mapping.data[old_size + i] = ctx->env->funcs.size + i;
  wabt_resize_interpreter_func_vector(&ctx->env->funcs,
                                      ctx->env->funcs.size + count);
  wabt_resize_uint32_vector_vector(&ctx->func_fixups, count);
  return WABT_OK;
}

static WabtResult on_function_signature(uint32_t index,
                                        uint32_t sig_index,
                                        void* user_data) {
  Context* ctx = user_data;
  WabtInterpreterFunc* func = get_func_by_module_index(ctx, index);
  func->defined.offset = WABT_INVALID_OFFSET;
  func->sig_index = translate_sig_index_to_env(ctx, sig_index);
  return WABT_OK;
}

static WabtResult on_table(uint32_t index,
                           WabtType elem_type,
                           const WabtLimits* elem_limits,
                           void* user_data) {
  Context* ctx = user_data;
  if (ctx->module->table_index != WABT_INVALID_INDEX) {
    print_error(ctx, "only one table allowed");
    return WABT_ERROR;
  }
  WabtInterpreterTable* table =
      wabt_append_interpreter_table(&ctx->env->tables);
  table->limits = *elem_limits;
  init_table_func_indexes(ctx, table);
  ctx->module->table_index = ctx->env->tables.size - 1;
  return WABT_OK;
}

static WabtResult on_memory(uint32_t index,
                            const WabtLimits* page_limits,
                            void* user_data) {
  Context* ctx = user_data;
  if (ctx->module->memory_index != WABT_INVALID_INDEX) {
    print_error(ctx, "only one memory allowed");
    return WABT_ERROR;
  }
  WabtInterpreterMemory* memory =
      wabt_append_interpreter_memory(&ctx->env->memories);
  memory->page_limits = *page_limits;
  memory->byte_size = page_limits->initial * WABT_PAGE_SIZE;
  memory->data = wabt_alloc_zero(memory->byte_size);
  ctx->module->memory_index = ctx->env->memories.size - 1;
  return WABT_OK;
}

static WabtResult on_global_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  size_t old_size = ctx->global_index_mapping.size;
  wabt_resize_uint32_vector(&ctx->global_index_mapping, old_size + count);
  uint32_t i;
  for (i = 0; i < count; ++i)
    ctx->global_index_mapping.data[old_size + i] = ctx->env->globals.size + i;
  wabt_resize_interpreter_global_vector(&ctx->env->globals,
                                        ctx->env->globals.size + count);
  return WABT_OK;
}

static WabtResult begin_global(uint32_t index,
                               WabtType type,
                               WabtBool mutable_,
                               void* user_data) {
  Context* ctx = user_data;
  WabtInterpreterGlobal* global = get_global_by_module_index(ctx, index);
  global->typed_value.type = type;
  global->mutable_ = mutable_;
  return WABT_OK;
}

static WabtResult end_global_init_expr(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  WabtInterpreterGlobal* global = get_global_by_module_index(ctx, index);
  if (ctx->init_expr_value.type != global->typed_value.type) {
    print_error(ctx, "type mismatch in global, expected %s but got %s.",
                wabt_get_type_name(global->typed_value.type),
                wabt_get_type_name(ctx->init_expr_value.type));
    return WABT_ERROR;
  }
  global->typed_value = ctx->init_expr_value;
  return WABT_OK;
}

static WabtResult on_init_expr_f32_const_expr(uint32_t index,
                                              uint32_t value_bits,
                                              void* user_data) {
  Context* ctx = user_data;
  ctx->init_expr_value.type = WABT_TYPE_F32;
  ctx->init_expr_value.value.f32_bits = value_bits;
  return WABT_OK;
}

static WabtResult on_init_expr_f64_const_expr(uint32_t index,
                                              uint64_t value_bits,
                                              void* user_data) {
  Context* ctx = user_data;
  ctx->init_expr_value.type = WABT_TYPE_F64;
  ctx->init_expr_value.value.f64_bits = value_bits;
  return WABT_OK;
}

static WabtResult on_init_expr_get_global_expr(uint32_t index,
                                               uint32_t global_index,
                                               void* user_data) {
  Context* ctx = user_data;
  if (global_index >= ctx->num_global_imports) {
    print_error(ctx,
                "initializer expression can only reference an imported global");
    return WABT_ERROR;
  }
  WabtInterpreterGlobal* ref_global =
      get_global_by_module_index(ctx, global_index);
  if (ref_global->mutable_) {
    print_error(ctx,
                "initializer expression cannot reference a mutable global");
    return WABT_ERROR;
  }
  ctx->init_expr_value = ref_global->typed_value;
  return WABT_OK;
}

static WabtResult on_init_expr_i32_const_expr(uint32_t index,
                                              uint32_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  ctx->init_expr_value.type = WABT_TYPE_I32;
  ctx->init_expr_value.value.i32 = value;
  return WABT_OK;
}

static WabtResult on_init_expr_i64_const_expr(uint32_t index,
                                              uint64_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  ctx->init_expr_value.type = WABT_TYPE_I64;
  ctx->init_expr_value.value.i64 = value;
  return WABT_OK;
}

static WabtResult on_export(uint32_t index,
                            WabtExternalKind kind,
                            uint32_t item_index,
                            WabtStringSlice name,
                            void* user_data) {
  Context* ctx = user_data;
  switch (kind) {
    case WABT_EXTERNAL_KIND_FUNC:
      item_index = translate_func_index_to_env(ctx, item_index);
      break;

    case WABT_EXTERNAL_KIND_TABLE:
      item_index = ctx->module->table_index;
      break;

    case WABT_EXTERNAL_KIND_MEMORY:
      item_index = ctx->module->memory_index;
      break;

    case WABT_EXTERNAL_KIND_GLOBAL: {
      item_index = translate_global_index_to_env(ctx, item_index);
      WabtInterpreterGlobal* global = &ctx->env->globals.data[item_index];
      if (global->mutable_) {
        print_error(ctx, "mutable globals cannot be exported");
        return WABT_ERROR;
      }
      break;
    }

    case WABT_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
  return append_export(ctx, ctx->module, kind, item_index, name);
}

static WabtResult on_start_function(uint32_t func_index, void* user_data) {
  Context* ctx = user_data;
  uint32_t start_func_index = translate_func_index_to_env(ctx, func_index);
  WabtInterpreterFunc* start_func =
      get_func_by_env_index(ctx, start_func_index);
  WabtInterpreterFuncSignature* sig =
      get_signature_by_env_index(ctx, start_func->sig_index);
  if (sig->param_types.size != 0) {
    print_error(ctx, "start function must be nullary");
    return WABT_ERROR;
  }
  if (sig->result_types.size != 0) {
    print_error(ctx, "start function must not return anything");
    return WABT_ERROR;
  }
  ctx->module->defined.start_func_index = start_func_index;
  return WABT_OK;
}

static WabtResult end_elem_segment_init_expr(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  if (ctx->init_expr_value.type != WABT_TYPE_I32) {
    print_error(ctx, "type mismatch in elem segment, expected i32 but got %s",
                wabt_get_type_name(ctx->init_expr_value.type));
    return WABT_ERROR;
  }
  ctx->table_offset = ctx->init_expr_value.value.i32;
  return WABT_OK;
}

static WabtResult on_elem_segment_function_index_check(uint32_t index,
                                                       uint32_t func_index,
                                                       void* user_data) {
  Context* ctx = user_data;
  assert(ctx->module->table_index != WABT_INVALID_INDEX);
  WabtInterpreterTable* table =
      &ctx->env->tables.data[ctx->module->table_index];
  if (ctx->table_offset >= table->func_indexes.size) {
    print_error(ctx,
                "elem segment offset is out of bounds: %u >= max value %" PRIzd,
                ctx->table_offset, table->func_indexes.size);
    return WABT_ERROR;
  }

  uint32_t max_func_index = ctx->func_index_mapping.size;
  if (func_index >= max_func_index) {
    print_error(ctx, "invalid func_index: %d (max %d)", func_index,
                max_func_index);
    return WABT_ERROR;
  }

  table->func_indexes.data[ctx->table_offset++] =
      translate_func_index_to_env(ctx, func_index);
  return WABT_OK;
}

static WabtResult on_elem_segment_function_index(uint32_t index,
                                                 uint32_t func_index,
                                                 void* user_data) {
  Context* ctx = user_data;
  assert(ctx->module->table_index != WABT_INVALID_INDEX);
  WabtInterpreterTable* table =
      &ctx->env->tables.data[ctx->module->table_index];
  table->func_indexes.data[ctx->table_offset++] =
      translate_func_index_to_env(ctx, func_index);
  return WABT_OK;
}

static WabtResult on_data_segment_data_check(uint32_t index,
                                             const void* src_data,
                                             uint32_t size,
                                             void* user_data) {
  Context* ctx = user_data;
  assert(ctx->module->memory_index != WABT_INVALID_INDEX);
  WabtInterpreterMemory* memory =
      &ctx->env->memories.data[ctx->module->memory_index];
  if (ctx->init_expr_value.type != WABT_TYPE_I32) {
    print_error(ctx, "type mismatch in data segment, expected i32 but got %s",
                wabt_get_type_name(ctx->init_expr_value.type));
    return WABT_ERROR;
  }
  uint32_t address = ctx->init_expr_value.value.i32;
  uint64_t end_address = (uint64_t)address + (uint64_t)size;
  if (end_address > memory->byte_size) {
    print_error(ctx, "data segment is out of bounds: [%u, %" PRIu64
                     ") >= max value %u",
                address, end_address, memory->byte_size);
    return WABT_ERROR;
  }
  return WABT_OK;
}

static WabtResult on_data_segment_data(uint32_t index,
                                       const void* src_data,
                                       uint32_t size,
                                       void* user_data) {
  Context* ctx = user_data;
  assert(ctx->module->memory_index != WABT_INVALID_INDEX);
  WabtInterpreterMemory* memory =
      &ctx->env->memories.data[ctx->module->memory_index];
  uint32_t address = ctx->init_expr_value.value.i32;
  uint8_t* dst_data = memory->data;
  memcpy(&dst_data[address], src_data, size);
  return WABT_OK;
}

static void push_label(Context* ctx, uint32_t offset, uint32_t fixup_offset) {
  Label* label = wabt_append_label(&ctx->label_stack);
  label->offset = offset;
  label->fixup_offset = fixup_offset;
}

static void pop_label(Context* ctx) {
  ctx->label_stack.size--;
  /* reduce the depth_fixups stack as well, but it may be smaller than
   * label_stack so only do it conditionally. */
  if (ctx->depth_fixups.size > ctx->label_stack.size) {
    uint32_t from = ctx->label_stack.size;
    uint32_t to = ctx->depth_fixups.size;
    uint32_t i;
    for (i = from; i < to; ++i)
      wabt_destroy_uint32_vector(&ctx->depth_fixups.data[i]);
    ctx->depth_fixups.size = ctx->label_stack.size;
  }
}

static WabtResult begin_function_body(WabtBinaryReaderContext* context,
                                      uint32_t index) {
  Context* ctx = context->user_data;
  WabtInterpreterFunc* func = get_func_by_module_index(ctx, index);
  WabtInterpreterFuncSignature* sig =
      get_signature_by_env_index(ctx, func->sig_index);

  func->is_host = WABT_FALSE;
  func->defined.offset = get_istream_offset(ctx);
  func->defined.local_decl_count = 0;
  func->defined.local_count = 0;

  ctx->current_func = func;
  ctx->depth_fixups.size = 0;
  ctx->label_stack.size = 0;

  /* fixup function references */
  uint32_t defined_index = translate_module_func_index_to_defined(ctx, index);
  uint32_t i;
  Uint32Vector* fixups = &ctx->func_fixups.data[defined_index];
  for (i = 0; i < fixups->size; ++i)
    CHECK_RESULT(emit_i32_at(ctx, fixups->data[i], func->defined.offset));

  /* append param types */
  for (i = 0; i < sig->param_types.size; ++i) {
    wabt_append_type_value(&func->defined.param_and_local_types,
                           &sig->param_types.data[i]);
  }

  CHECK_RESULT(
      wabt_typechecker_begin_function(&ctx->typechecker, &sig->result_types));

  /* push implicit func label (equivalent to return) */
  push_label(ctx, WABT_INVALID_OFFSET, WABT_INVALID_OFFSET);
  return WABT_OK;
}

static WabtResult end_function_body(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  fixup_top_label(ctx);
  uint32_t drop_count, keep_count;
  CHECK_RESULT(get_return_drop_keep_count(ctx, &drop_count, &keep_count));
  CHECK_RESULT(wabt_typechecker_end_function(&ctx->typechecker));
  CHECK_RESULT(emit_drop_keep(ctx, drop_count, keep_count));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_RETURN));
  pop_label(ctx);
  ctx->current_func = NULL;
  return WABT_OK;
}

static WabtResult on_local_decl_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  WabtInterpreterFunc* func = ctx->current_func;
  func->defined.local_decl_count = count;
  return WABT_OK;
}

static WabtResult on_local_decl(uint32_t decl_index,
                                uint32_t count,
                                WabtType type,
                                void* user_data) {
  Context* ctx = user_data;
  WabtInterpreterFunc* func = ctx->current_func;
  func->defined.local_count += count;

  uint32_t i;
  for (i = 0; i < count; ++i) {
    wabt_append_type_value(&func->defined.param_and_local_types, &type);
  }

  if (decl_index == func->defined.local_decl_count - 1) {
    /* last local declaration, allocate space for all locals. */
    CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_ALLOCA));
    CHECK_RESULT(emit_i32(ctx, func->defined.local_count));
  }
  return WABT_OK;
}

static WabtResult check_has_memory(Context* ctx, WabtOpcode opcode) {
  if (ctx->module->memory_index == WABT_INVALID_INDEX) {
    print_error(ctx, "%s requires an imported or defined memory.",
                wabt_get_opcode_name(opcode));
    return WABT_ERROR;
  }
  return WABT_OK;
}

static WabtResult check_align(Context* ctx,
                              uint32_t alignment_log2,
                              uint32_t natural_alignment) {
  if (alignment_log2 >= 32 || (1U << alignment_log2) > natural_alignment) {
    print_error(ctx, "alignment must not be larger than natural alignment (%u)",
                natural_alignment);
    return WABT_ERROR;
  }
  return WABT_OK;
}

static WabtResult on_unary_expr(WabtOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(wabt_typechecker_on_unary(&ctx->typechecker, opcode));
  CHECK_RESULT(emit_opcode(ctx, opcode));
  return WABT_OK;
}

static WabtResult on_binary_expr(WabtOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(wabt_typechecker_on_binary(&ctx->typechecker, opcode));
  CHECK_RESULT(emit_opcode(ctx, opcode));
  return WABT_OK;
}

static WabtResult on_block_expr(uint32_t num_types,
                                WabtType* sig_types,
                                void* user_data) {
  Context* ctx = user_data;
  WabtTypeVector sig;
  sig.size = num_types;
  sig.data = sig_types;
  CHECK_RESULT(wabt_typechecker_on_block(&ctx->typechecker, &sig));
  push_label(ctx, WABT_INVALID_OFFSET, WABT_INVALID_OFFSET);
  return WABT_OK;
}

static WabtResult on_loop_expr(uint32_t num_types,
                               WabtType* sig_types,
                               void* user_data) {
  Context* ctx = user_data;
  WabtTypeVector sig;
  sig.size = num_types;
  sig.data = sig_types;
  CHECK_RESULT(wabt_typechecker_on_loop(&ctx->typechecker, &sig));
  push_label(ctx, get_istream_offset(ctx), WABT_INVALID_OFFSET);
  return WABT_OK;
}

static WabtResult on_if_expr(uint32_t num_types,
                             WabtType* sig_types,
                             void* user_data) {
  Context* ctx = user_data;
  WabtTypeVector sig;
  sig.size = num_types;
  sig.data = sig_types;
  CHECK_RESULT(wabt_typechecker_on_if(&ctx->typechecker, &sig));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_BR_UNLESS));
  uint32_t fixup_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WABT_INVALID_OFFSET));
  push_label(ctx, WABT_INVALID_OFFSET, fixup_offset);
  return WABT_OK;
}

static WabtResult on_else_expr(void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(wabt_typechecker_on_else(&ctx->typechecker));
  Label* label = top_label(ctx);
  uint32_t fixup_cond_offset = label->fixup_offset;
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_BR));
  label->fixup_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WABT_INVALID_OFFSET));
  CHECK_RESULT(emit_i32_at(ctx, fixup_cond_offset, get_istream_offset(ctx)));
  return WABT_OK;
}

static WabtResult on_end_expr(void* user_data) {
  Context* ctx = user_data;
  WabtTypeCheckerLabel* label;
  CHECK_RESULT(wabt_typechecker_get_label(&ctx->typechecker, 0, &label));
  WabtLabelType label_type = label->label_type;
  CHECK_RESULT(wabt_typechecker_on_end(&ctx->typechecker));
  if (label_type == WABT_LABEL_TYPE_IF || label_type == WABT_LABEL_TYPE_ELSE) {
    CHECK_RESULT(emit_i32_at(ctx, top_label(ctx)->fixup_offset,
                             get_istream_offset(ctx)));
  }
  fixup_top_label(ctx);
  pop_label(ctx);
  return WABT_OK;
}

static WabtResult on_br_expr(uint32_t depth, void* user_data) {
  Context* ctx = user_data;
  uint32_t drop_count, keep_count;
  CHECK_RESULT(get_br_drop_keep_count(ctx, depth, &drop_count, &keep_count));
  CHECK_RESULT(wabt_typechecker_on_br(&ctx->typechecker, depth));
  CHECK_RESULT(emit_br(ctx, depth, drop_count, keep_count));
  return WABT_OK;
}

static WabtResult on_br_if_expr(uint32_t depth, void* user_data) {
  Context* ctx = user_data;
  uint32_t drop_count, keep_count;
  CHECK_RESULT(wabt_typechecker_on_br_if(&ctx->typechecker, depth));
  CHECK_RESULT(get_br_drop_keep_count(ctx, depth, &drop_count, &keep_count));
  /* flip the br_if so if <cond> is true it can drop values from the stack */
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_BR_UNLESS));
  uint32_t fixup_br_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WABT_INVALID_OFFSET));
  CHECK_RESULT(emit_br(ctx, depth, drop_count, keep_count));
  CHECK_RESULT(emit_i32_at(ctx, fixup_br_offset, get_istream_offset(ctx)));
  return WABT_OK;
}

static WabtResult on_br_table_expr(WabtBinaryReaderContext* context,
                                   uint32_t num_targets,
                                   uint32_t* target_depths,
                                   uint32_t default_target_depth) {
  Context* ctx = context->user_data;
  CHECK_RESULT(wabt_typechecker_begin_br_table(&ctx->typechecker));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_BR_TABLE));
  CHECK_RESULT(emit_i32(ctx, num_targets));
  uint32_t fixup_table_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WABT_INVALID_OFFSET));
  /* not necessary for the interpreter, but it makes it easier to disassemble.
   * This opcode specifies how many bytes of data follow. */
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_DATA));
  CHECK_RESULT(emit_i32(ctx, (num_targets + 1) * WABT_TABLE_ENTRY_SIZE));
  CHECK_RESULT(emit_i32_at(ctx, fixup_table_offset, get_istream_offset(ctx)));

  uint32_t i;
  for (i = 0; i <= num_targets; ++i) {
    uint32_t depth = i != num_targets ? target_depths[i] : default_target_depth;
    CHECK_RESULT(wabt_typechecker_on_br_table_target(&ctx->typechecker, depth));
    CHECK_RESULT(emit_br_table_offset(ctx, depth));
  }

  CHECK_RESULT(wabt_typechecker_end_br_table(&ctx->typechecker));
  return WABT_OK;
}

static WabtResult on_call_expr(uint32_t func_index, void* user_data) {
  Context* ctx = user_data;
  WabtInterpreterFunc* func = get_func_by_module_index(ctx, func_index);
  WabtInterpreterFuncSignature* sig =
      get_signature_by_env_index(ctx, func->sig_index);
  CHECK_RESULT(wabt_typechecker_on_call(&ctx->typechecker, &sig->param_types,
                                        &sig->result_types));

  if (func->is_host) {
    CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_CALL_HOST));
    CHECK_RESULT(emit_i32(ctx, translate_func_index_to_env(ctx, func_index)));
  } else {
    CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_CALL));
    CHECK_RESULT(emit_func_offset(ctx, func, func_index));
  }

  return WABT_OK;
}

static WabtResult on_call_indirect_expr(uint32_t sig_index, void* user_data) {
  Context* ctx = user_data;
  if (ctx->module->table_index == WABT_INVALID_INDEX) {
    print_error(ctx, "found call_indirect operator, but no table");
    return WABT_ERROR;
  }
  WabtInterpreterFuncSignature* sig =
      get_signature_by_module_index(ctx, sig_index);
  CHECK_RESULT(wabt_typechecker_on_call_indirect(
      &ctx->typechecker, &sig->param_types, &sig->result_types));

  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_CALL_INDIRECT));
  CHECK_RESULT(emit_i32(ctx, ctx->module->table_index));
  CHECK_RESULT(emit_i32(ctx, translate_sig_index_to_env(ctx, sig_index)));
  return WABT_OK;
}

static WabtResult on_drop_expr(void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(wabt_typechecker_on_drop(&ctx->typechecker));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_DROP));
  return WABT_OK;
}

static WabtResult on_i32_const_expr(uint32_t value, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(wabt_typechecker_on_const(&ctx->typechecker, WABT_TYPE_I32));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_I32_CONST));
  CHECK_RESULT(emit_i32(ctx, value));
  return WABT_OK;
}

static WabtResult on_i64_const_expr(uint64_t value, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(wabt_typechecker_on_const(&ctx->typechecker, WABT_TYPE_I64));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_I64_CONST));
  CHECK_RESULT(emit_i64(ctx, value));
  return WABT_OK;
}

static WabtResult on_f32_const_expr(uint32_t value_bits, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(wabt_typechecker_on_const(&ctx->typechecker, WABT_TYPE_F32));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_F32_CONST));
  CHECK_RESULT(emit_i32(ctx, value_bits));
  return WABT_OK;
}

static WabtResult on_f64_const_expr(uint64_t value_bits, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(wabt_typechecker_on_const(&ctx->typechecker, WABT_TYPE_F64));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_F64_CONST));
  CHECK_RESULT(emit_i64(ctx, value_bits));
  return WABT_OK;
}

static WabtResult on_get_global_expr(uint32_t global_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_GLOBAL(ctx, global_index);
  WabtType type = get_global_type_by_module_index(ctx, global_index);
  CHECK_RESULT(wabt_typechecker_on_get_global(&ctx->typechecker, type));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_GET_GLOBAL));
  CHECK_RESULT(emit_i32(ctx, translate_global_index_to_env(ctx, global_index)));
  return WABT_OK;
}

static WabtResult on_set_global_expr(uint32_t global_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_GLOBAL(ctx, global_index);
  WabtInterpreterGlobal* global = get_global_by_module_index(ctx, global_index);
  if (global->mutable_ != WABT_TRUE) {
    print_error(ctx, "can't set_global on immutable global at index %u.",
                global_index);
    return WABT_ERROR;
  }
  CHECK_RESULT(wabt_typechecker_on_set_global(&ctx->typechecker,
                                              global->typed_value.type));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_SET_GLOBAL));
  CHECK_RESULT(emit_i32(ctx, translate_global_index_to_env(ctx, global_index)));
  return WABT_OK;
}

static uint32_t translate_local_index(Context* ctx, uint32_t local_index) {
  return ctx->typechecker.type_stack.size +
         ctx->current_func->defined.param_and_local_types.size - local_index;
}

static WabtResult on_get_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_LOCAL(ctx, local_index);
  WabtType type = get_local_type_by_index(ctx->current_func, local_index);
  /* Get the translated index before calling wabt_typechecker_on_get_local
   * because it will update the type stack size. We need the index to be
   * relative to the old stack size. */
  uint32_t translated_local_index = translate_local_index(ctx, local_index);
  CHECK_RESULT(wabt_typechecker_on_get_local(&ctx->typechecker, type));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_GET_LOCAL));
  CHECK_RESULT(emit_i32(ctx, translated_local_index));
  return WABT_OK;
}

static WabtResult on_set_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_LOCAL(ctx, local_index);
  WabtType type = get_local_type_by_index(ctx->current_func, local_index);
  CHECK_RESULT(wabt_typechecker_on_set_local(&ctx->typechecker, type));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_SET_LOCAL));
  CHECK_RESULT(emit_i32(ctx, translate_local_index(ctx, local_index)));
  return WABT_OK;
}

static WabtResult on_tee_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_LOCAL(ctx, local_index);
  WabtType type = get_local_type_by_index(ctx->current_func, local_index);
  CHECK_RESULT(wabt_typechecker_on_tee_local(&ctx->typechecker, type));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_TEE_LOCAL));
  CHECK_RESULT(emit_i32(ctx, translate_local_index(ctx, local_index)));
  return WABT_OK;
}

static WabtResult on_grow_memory_expr(void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(check_has_memory(ctx, WABT_OPCODE_GROW_MEMORY));
  CHECK_RESULT(wabt_typechecker_on_grow_memory(&ctx->typechecker));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_GROW_MEMORY));
  CHECK_RESULT(emit_i32(ctx, ctx->module->memory_index));
  return WABT_OK;
}

static WabtResult on_load_expr(WabtOpcode opcode,
                               uint32_t alignment_log2,
                               uint32_t offset,
                               void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(check_has_memory(ctx, opcode));
  CHECK_RESULT(
      check_align(ctx, alignment_log2, wabt_get_opcode_memory_size(opcode)));
  CHECK_RESULT(wabt_typechecker_on_load(&ctx->typechecker, opcode));
  CHECK_RESULT(emit_opcode(ctx, opcode));
  CHECK_RESULT(emit_i32(ctx, ctx->module->memory_index));
  CHECK_RESULT(emit_i32(ctx, offset));
  return WABT_OK;
}

static WabtResult on_store_expr(WabtOpcode opcode,
                                uint32_t alignment_log2,
                                uint32_t offset,
                                void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(check_has_memory(ctx, opcode));
  CHECK_RESULT(
      check_align(ctx, alignment_log2, wabt_get_opcode_memory_size(opcode)));
  CHECK_RESULT(wabt_typechecker_on_store(&ctx->typechecker, opcode));
  CHECK_RESULT(emit_opcode(ctx, opcode));
  CHECK_RESULT(emit_i32(ctx, ctx->module->memory_index));
  CHECK_RESULT(emit_i32(ctx, offset));
  return WABT_OK;
}

static WabtResult on_current_memory_expr(void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(check_has_memory(ctx, WABT_OPCODE_CURRENT_MEMORY));
  CHECK_RESULT(wabt_typechecker_on_current_memory(&ctx->typechecker));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_CURRENT_MEMORY));
  CHECK_RESULT(emit_i32(ctx, ctx->module->memory_index));
  return WABT_OK;
}

static WabtResult on_nop_expr(void* user_data) {
  return WABT_OK;
}

static WabtResult on_return_expr(void* user_data) {
  Context* ctx = user_data;
  uint32_t drop_count, keep_count;
  CHECK_RESULT(get_return_drop_keep_count(ctx, &drop_count, &keep_count));
  CHECK_RESULT(wabt_typechecker_on_return(&ctx->typechecker));
  CHECK_RESULT(emit_drop_keep(ctx, drop_count, keep_count));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_RETURN));
  return WABT_OK;
}

static WabtResult on_select_expr(void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(wabt_typechecker_on_select(&ctx->typechecker));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_SELECT));
  return WABT_OK;
}

static WabtResult on_unreachable_expr(void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(wabt_typechecker_on_unreachable(&ctx->typechecker));
  CHECK_RESULT(emit_opcode(ctx, WABT_OPCODE_UNREACHABLE));
  return WABT_OK;
}

static WabtBinaryReader s_binary_reader = {
    .user_data = NULL,
    .on_error = on_error,

    .on_signature_count = on_signature_count,
    .on_signature = on_signature,

    .on_import_count = on_import_count,
    .on_import = on_import,
    .on_import_func = on_import_func,
    .on_import_table = on_import_table,
    .on_import_memory = on_import_memory,
    .on_import_global = on_import_global,

    .on_function_signatures_count = on_function_signatures_count,
    .on_function_signature = on_function_signature,

    .on_table = on_table,

    .on_memory = on_memory,

    .on_global_count = on_global_count,
    .begin_global = begin_global,
    .end_global_init_expr = end_global_init_expr,

    .on_export = on_export,

    .on_start_function = on_start_function,

    .begin_function_body = begin_function_body,
    .on_local_decl_count = on_local_decl_count,
    .on_local_decl = on_local_decl,
    .on_binary_expr = on_binary_expr,
    .on_block_expr = on_block_expr,
    .on_br_expr = on_br_expr,
    .on_br_if_expr = on_br_if_expr,
    .on_br_table_expr = on_br_table_expr,
    .on_call_expr = on_call_expr,
    .on_call_indirect_expr = on_call_indirect_expr,
    .on_compare_expr = on_binary_expr,
    .on_convert_expr = on_unary_expr,
    .on_current_memory_expr = on_current_memory_expr,
    .on_drop_expr = on_drop_expr,
    .on_else_expr = on_else_expr,
    .on_end_expr = on_end_expr,
    .on_f32_const_expr = on_f32_const_expr,
    .on_f64_const_expr = on_f64_const_expr,
    .on_get_global_expr = on_get_global_expr,
    .on_get_local_expr = on_get_local_expr,
    .on_grow_memory_expr = on_grow_memory_expr,
    .on_i32_const_expr = on_i32_const_expr,
    .on_i64_const_expr = on_i64_const_expr,
    .on_if_expr = on_if_expr,
    .on_load_expr = on_load_expr,
    .on_loop_expr = on_loop_expr,
    .on_nop_expr = on_nop_expr,
    .on_return_expr = on_return_expr,
    .on_select_expr = on_select_expr,
    .on_set_global_expr = on_set_global_expr,
    .on_set_local_expr = on_set_local_expr,
    .on_store_expr = on_store_expr,
    .on_tee_local_expr = on_tee_local_expr,
    .on_unary_expr = on_unary_expr,
    .on_unreachable_expr = on_unreachable_expr,
    .end_function_body = end_function_body,

    .end_elem_segment_init_expr = end_elem_segment_init_expr,
    .on_elem_segment_function_index = on_elem_segment_function_index_check,

    .on_data_segment_data = on_data_segment_data_check,

    .on_init_expr_f32_const_expr = on_init_expr_f32_const_expr,
    .on_init_expr_f64_const_expr = on_init_expr_f64_const_expr,
    .on_init_expr_get_global_expr = on_init_expr_get_global_expr,
    .on_init_expr_i32_const_expr = on_init_expr_i32_const_expr,
    .on_init_expr_i64_const_expr = on_init_expr_i64_const_expr,
};

/* Second pass to assign data and elem segments after they are checked above. */
static WabtBinaryReader s_binary_reader_segments = {
    .user_data = NULL,
    .on_error = on_error,

    .end_elem_segment_init_expr = end_elem_segment_init_expr,
    .on_elem_segment_function_index = on_elem_segment_function_index,
    .on_data_segment_data = on_data_segment_data,

    .on_init_expr_f32_const_expr = on_init_expr_f32_const_expr,
    .on_init_expr_f64_const_expr = on_init_expr_f64_const_expr,
    .on_init_expr_get_global_expr = on_init_expr_get_global_expr,
    .on_init_expr_i32_const_expr = on_init_expr_i32_const_expr,
    .on_init_expr_i64_const_expr = on_init_expr_i64_const_expr,
};

static void destroy_context(Context* ctx) {
  wabt_destroy_label_vector(&ctx->label_stack);
  WABT_DESTROY_VECTOR_AND_ELEMENTS(ctx->depth_fixups, uint32_vector);
  WABT_DESTROY_VECTOR_AND_ELEMENTS(ctx->func_fixups, uint32_vector);
  wabt_destroy_uint32_vector(&ctx->sig_index_mapping);
  wabt_destroy_uint32_vector(&ctx->func_index_mapping);
  wabt_destroy_uint32_vector(&ctx->global_index_mapping);
  wabt_destroy_typechecker(&ctx->typechecker);
}

WabtResult wabt_read_binary_interpreter(WabtInterpreterEnvironment* env,
                                        const void* data,
                                        size_t size,
                                        const WabtReadBinaryOptions* options,
                                        WabtBinaryErrorHandler* error_handler,
                                        WabtInterpreterModule** out_module) {
  Context ctx;
  WabtBinaryReader reader;

  WabtInterpreterEnvironmentMark mark = wabt_mark_interpreter_environment(env);

  WabtInterpreterModule* module = wabt_append_interpreter_module(&env->modules);

  WABT_ZERO_MEMORY(ctx);
  WABT_ZERO_MEMORY(reader);

  ctx.reader = &reader;
  ctx.error_handler = error_handler;
  ctx.env = env;
  ctx.module = module;
  ctx.module->is_host = WABT_FALSE;
  ctx.module->table_index = WABT_INVALID_INDEX;
  ctx.module->memory_index = WABT_INVALID_INDEX;
  ctx.module->defined.start_func_index = WABT_INVALID_INDEX;
  ctx.module->defined.istream_start = env->istream.size;
  ctx.istream_offset = env->istream.size;
  CHECK_RESULT(
      wabt_init_mem_writer_existing(&ctx.istream_writer, &env->istream));

  WabtTypeCheckerErrorHandler tc_error_handler;
  tc_error_handler.on_error = on_typechecker_error;
  tc_error_handler.user_data = &ctx;
  ctx.typechecker.error_handler = &tc_error_handler;

  reader = s_binary_reader;
  reader.user_data = &ctx;

  const uint32_t num_function_passes = 1;
  WabtResult result =
      wabt_read_binary(data, size, &reader, num_function_passes, options);
  wabt_steal_mem_writer_output_buffer(&ctx.istream_writer, &env->istream);
  if (WABT_SUCCEEDED(result)) {
    /* Another pass on the read binary to assign data and elem segments. */
    reader = s_binary_reader_segments;
    reader.user_data = &ctx;
    result =
        wabt_read_binary(data, size, &reader, num_function_passes, options);
    assert(WABT_SUCCEEDED(result));

    env->istream.size = ctx.istream_offset;
    ctx.module->defined.istream_end = env->istream.size;
    *out_module = module;
  } else {
    wabt_reset_interpreter_environment_to_mark(env, mark);
    *out_module = NULL;
  }
  destroy_context(&ctx);
  return result;
}

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

#include "wasm-binary-reader-interpreter.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "wasm-allocator.h"
#include "wasm-binary-reader.h"
#include "wasm-interpreter.h"
#include "wasm-writer.h"

#define LOG 0

#if LOG
#define LOGF(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOGF(...) (void)0
#endif

#define CHECK_RESULT(expr) \
  do {                     \
    if (WASM_FAILED(expr)) \
      return WASM_ERROR;   \
  } while (0)

#define CHECK_DEPTH(ctx, depth)                                         \
  do {                                                                  \
    if ((depth) >= (ctx)->label_stack.size) {                           \
      print_error((ctx), "invalid depth: %d (max %" PRIzd ")", (depth), \
                  ((ctx)->label_stack.size));                           \
      return WASM_ERROR;                                                \
    }                                                                   \
  } while (0)

#define CHECK_LOCAL(ctx, local_index)                                       \
  do {                                                                      \
    uint32_t max_local_index =                                              \
        (ctx)->current_func->param_and_local_types.size;                    \
    if ((local_index) >= max_local_index) {                                 \
      print_error((ctx), "invalid local_index: %d (max %d)", (local_index), \
                  max_local_index);                                         \
      return WASM_ERROR;                                                    \
    }                                                                       \
  } while (0)

#define CHECK_GLOBAL(ctx, global_index)                                       \
  do {                                                                        \
    uint32_t max_global_index = (ctx)->module->globals.size;                  \
    if ((global_index) >= max_global_index) {                                 \
      print_error((ctx), "invalid global_index: %d (max %d)", (global_index), \
                  max_global_index);                                          \
      return WASM_ERROR;                                                      \
    }                                                                         \
  } while (0)

#define RETURN_IF_TOP_TYPE_IS_ANY(ctx) \
  if (top_type_is_any(ctx))            \
  return

#define RETURN_OK_IF_TOP_TYPE_IS_ANY(ctx) \
  if (top_type_is_any(ctx))               \
  return WASM_OK

#define WASM_TYPE_ANY WASM_NUM_TYPES

static const char* s_type_names[] = {
    "void", "i32", "i64", "f32", "f64", "any",
};
WASM_STATIC_ASSERT(WASM_ARRAY_SIZE(s_type_names) == WASM_NUM_TYPES + 1);

typedef uint32_t Uint32;
WASM_DEFINE_VECTOR(uint32, Uint32);
WASM_DEFINE_VECTOR(uint32_vector, Uint32Vector);

typedef enum LabelType {
  LABEL_TYPE_FUNC,
  LABEL_TYPE_BLOCK,
  LABEL_TYPE_LOOP,
  LABEL_TYPE_IF,
  LABEL_TYPE_ELSE,
} LabelType;

typedef struct Label {
  LabelType label_type;
  WasmTypeVector sig;
  uint32_t type_stack_limit;
  uint32_t offset; /* branch location in the istream */
  uint32_t fixup_offset;
} Label;
WASM_DEFINE_VECTOR(label, Label);

typedef struct Context {
  WasmAllocator* allocator;
  WasmBinaryReader* reader;
  WasmBinaryErrorHandler* error_handler;
  WasmAllocator* memory_allocator;
  WasmInterpreterModule* module;
  WasmInterpreterFunc* current_func;
  WasmTypeVector type_stack;
  LabelVector label_stack;
  Uint32VectorVector func_fixups;
  Uint32VectorVector depth_fixups;
  uint32_t depth;
  WasmMemoryWriter istream_writer;
  uint32_t istream_offset;
  WasmInterpreterTypedValue init_expr_value;
  uint32_t table_offset;
  uint32_t num_func_imports;
} Context;

static Label* get_label(Context* ctx, uint32_t depth) {
  assert(depth < ctx->label_stack.size);
  return &ctx->label_stack.data[depth];
}

static Label* top_label(Context* ctx) {
  return get_label(ctx, ctx->label_stack.size - 1);
}

static void handle_error(uint32_t offset, const char* message, Context* ctx) {
  if (ctx->error_handler->on_error) {
    ctx->error_handler->on_error(offset, message,
                                 ctx->error_handler->user_data);
  }
}

static void print_error(Context* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  handle_error(WASM_INVALID_OFFSET, buffer, ctx);
}

static WasmInterpreterFunc* get_func(Context* ctx, uint32_t func_index) {
  assert(func_index < ctx->module->funcs.size);
  return &ctx->module->funcs.data[func_index];
}

static WasmInterpreterFunc* get_defined_func(Context* ctx,
                                             uint32_t func_index) {
  /* all function imports are first, so skip over those */
  func_index += ctx->num_func_imports;
  assert(func_index < ctx->module->funcs.size);
  return &ctx->module->funcs.data[func_index];
}

static WasmInterpreterFuncSignature* get_signature(Context* ctx,
                                                   uint32_t sig_index) {
  assert(sig_index < ctx->module->sigs.size);
  return &ctx->module->sigs.data[sig_index];
}

static WasmInterpreterFuncSignature* get_func_signature(
    Context* ctx,
    WasmInterpreterFunc* func) {
  return get_signature(ctx, func->sig_index);
}

static WasmType get_local_type_by_index(WasmInterpreterFunc* func,
                                        uint32_t local_index) {
  assert(local_index < func->param_and_local_types.size);
  return func->param_and_local_types.data[local_index];
}

static WasmInterpreterGlobal* get_global_by_index(Context* ctx,
                                                  uint32_t global_index) {
  assert(global_index < ctx->module->globals.size);
  return &ctx->module->globals.data[global_index];
}

static WasmType get_global_type_by_index(Context* ctx, uint32_t global_index) {
  return get_global_by_index(ctx, global_index)->typed_value.type;
}

static uint32_t translate_local_index(Context* ctx, uint32_t local_index) {
  assert(local_index < ctx->type_stack.size);
  return ctx->type_stack.size - local_index;
}

static uint32_t get_istream_offset(Context* ctx) {
  return ctx->istream_offset;
}

static WasmResult emit_data_at(Context* ctx,
                               size_t offset,
                               const void* data,
                               size_t size) {
  return ctx->istream_writer.base.write_data(
      offset, data, size, ctx->istream_writer.base.user_data);
}

static WasmResult emit_data(Context* ctx, const void* data, size_t size) {
  CHECK_RESULT(emit_data_at(ctx, ctx->istream_offset, data, size));
  ctx->istream_offset += size;
  return WASM_OK;
}

static WasmResult emit_opcode(Context* ctx, WasmOpcode opcode) {
  return emit_data(ctx, &opcode, sizeof(uint8_t));
}

static WasmResult emit_i8(Context* ctx, uint8_t value) {
  return emit_data(ctx, &value, sizeof(value));
}

static WasmResult emit_i32(Context* ctx, uint32_t value) {
  return emit_data(ctx, &value, sizeof(value));
}

static WasmResult emit_i64(Context* ctx, uint64_t value) {
  return emit_data(ctx, &value, sizeof(value));
}

static WasmResult emit_i32_at(Context* ctx, uint32_t offset, uint32_t value) {
  return emit_data_at(ctx, offset, &value, sizeof(value));
}

static WasmResult emit_drop_keep(Context* ctx, uint32_t drop, uint8_t keep) {
  assert(drop != UINT32_MAX);
  assert(keep <= 1);
  if (drop > 0) {
    if (drop == 1 && keep == 0) {
      LOGF("%3" PRIzd ": drop\n", ctx->type_stack.size);
      CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DROP));
    } else {
      LOGF("%3" PRIzd ": drop_keep %u %u\n", ctx->type_stack.size, drop, keep);
      CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DROP_KEEP));
      CHECK_RESULT(emit_i32(ctx, drop));
      CHECK_RESULT(emit_i8(ctx, keep));
    }
  }
  return WASM_OK;
}

static WasmResult append_fixup(Context* ctx,
                               Uint32VectorVector* fixups_vector,
                               uint32_t index) {
  if (index >= fixups_vector->size)
    wasm_resize_uint32_vector_vector(ctx->allocator, fixups_vector, index + 1);
  Uint32Vector* fixups = &fixups_vector->data[index];
  uint32_t offset = get_istream_offset(ctx);
  wasm_append_uint32_value(ctx->allocator, fixups, &offset);
  return WASM_OK;
}

static WasmResult emit_br_offset(Context* ctx,
                                 uint32_t depth,
                                 uint32_t offset) {
  if (offset == WASM_INVALID_OFFSET)
    CHECK_RESULT(append_fixup(ctx, &ctx->depth_fixups, depth));
  CHECK_RESULT(emit_i32(ctx, offset));
  return WASM_OK;
}

static uint32_t get_label_br_arity(Label* label) {
  return label->label_type != LABEL_TYPE_LOOP ? label->sig.size : 0;
}

static WasmResult emit_br(Context* ctx, uint32_t depth) {
  Label* label = get_label(ctx, depth);
  uint32_t arity = get_label_br_arity(label);
  assert(ctx->type_stack.size >= label->type_stack_limit + arity);
  uint32_t drop_count =
      (ctx->type_stack.size - label->type_stack_limit) - arity;
  CHECK_RESULT(emit_drop_keep(ctx, drop_count, arity));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR));
  CHECK_RESULT(emit_br_offset(ctx, depth, label->offset));
  return WASM_OK;
}

static WasmResult emit_br_table_offset(Context* ctx, uint32_t depth) {
  Label* label = get_label(ctx, depth);
  uint32_t arity = get_label_br_arity(label);
  assert(ctx->type_stack.size >= label->type_stack_limit + arity);
  uint32_t drop_count =
      (ctx->type_stack.size - label->type_stack_limit) - arity;
  CHECK_RESULT(emit_br_offset(ctx, depth, label->offset));
  CHECK_RESULT(emit_i32(ctx, drop_count));
  CHECK_RESULT(emit_i8(ctx, arity));
  return WASM_OK;
}

static WasmResult fixup_top_label(Context* ctx, uint32_t offset) {
  uint32_t top = ctx->label_stack.size - 1;
  if (top >= ctx->depth_fixups.size) {
    /* nothing to fixup */
    return WASM_OK;
  }

  Uint32Vector* fixups = &ctx->depth_fixups.data[top];
  uint32_t i;
  for (i = 0; i < fixups->size; ++i)
    CHECK_RESULT(emit_i32_at(ctx, fixups->data[i], offset));
  /* reduce the size to 0 in case this gets reused. Keep the allocations for
   * later use */
  fixups->size = 0;
  return WASM_OK;
}

static WasmResult emit_func_offset(Context* ctx,
                                   WasmInterpreterFunc* func,
                                   uint32_t func_index) {
  if (func->offset == WASM_INVALID_OFFSET)
    CHECK_RESULT(append_fixup(ctx, &ctx->func_fixups, func_index));
  CHECK_RESULT(emit_i32(ctx, func->offset));
  return WASM_OK;
}

static void on_error(WasmBinaryReaderContext* ctx,
                     const char* message) {
  handle_error(ctx->offset, message, ctx->user_data);
}

static WasmResult on_signature_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_new_interpreter_func_signature_array(ctx->allocator, &ctx->module->sigs,
                                            count);
  return WASM_OK;
}

static WasmResult on_signature(uint32_t index,
                               uint32_t param_count,
                               WasmType* param_types,
                               uint32_t result_count,
                               WasmType* result_types,
                               void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterFuncSignature* sig = get_signature(ctx, index);

  wasm_reserve_types(ctx->allocator, &sig->param_types, param_count);
  sig->param_types.size = param_count;
  memcpy(sig->param_types.data, param_types, param_count * sizeof(WasmType));

  wasm_reserve_types(ctx->allocator, &sig->result_types, result_count);
  sig->result_types.size = result_count;
  memcpy(sig->result_types.data, result_types, result_count * sizeof(WasmType));
  return WASM_OK;
}

static WasmResult on_import_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_new_interpreter_import_array(ctx->allocator, &ctx->module->imports,
                                    count);
  return WASM_OK;
}

static WasmResult on_import(uint32_t index,
                            WasmStringSlice module_name,
                            WasmStringSlice field_name,
                            void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->imports.size);
  WasmInterpreterImport* import = &ctx->module->imports.data[index];
  import->module_name = wasm_dup_string_slice(ctx->allocator, module_name);
  import->field_name = wasm_dup_string_slice(ctx->allocator, field_name);
  return WASM_OK;
}

static WasmResult on_import_func(uint32_t index,
                                 uint32_t sig_index,
                                 void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->imports.size);
  WasmInterpreterImport* import = &ctx->module->imports.data[index];
  assert(sig_index < ctx->module->sigs.size);
  import->kind = WASM_EXTERNAL_KIND_FUNC;
  import->func.sig_index = sig_index;

  WasmInterpreterFunc* func =
      wasm_append_interpreter_func(ctx->allocator, &ctx->module->funcs);
  func->sig_index = sig_index;
  func->offset = WASM_INVALID_OFFSET;
  func->is_host = WASM_TRUE;
  func->import_index = index;

  ctx->num_func_imports++;
  return WASM_OK;
}

/* TODO(binji): implement import_table, import_memory, import_global */
static WasmResult on_import_table(uint32_t index,
                                  uint32_t elem_type,
                                  const WasmLimits* elem_limits,
                                  void* user_data) {
  /* TODO */
  return WASM_ERROR;
}

static WasmResult on_import_memory(uint32_t index,
                                   const WasmLimits* page_limits,
                                   void* user_data) {
  /* TODO */
  return WASM_ERROR;
}

static WasmResult on_import_global(uint32_t index,
                                   WasmType type,
                                   WasmBool mutable_,
                                   void* user_data) {
  /* TODO */
  return WASM_ERROR;
}

static WasmResult on_function_signatures_count(uint32_t count,
                                               void* user_data) {
  Context* ctx = user_data;
  wasm_resize_interpreter_func_vector(ctx->allocator, &ctx->module->funcs,
                                      ctx->module->funcs.size + count);
  wasm_resize_uint32_vector_vector(ctx->allocator, &ctx->func_fixups, count);
  return WASM_OK;
}

static WasmResult on_function_signature(uint32_t index,
                                        uint32_t sig_index,
                                        void* user_data) {
  Context* ctx = user_data;
  assert(sig_index < ctx->module->sigs.size);
  WasmInterpreterFunc* func = get_defined_func(ctx, index);
  func->offset = WASM_INVALID_OFFSET;
  func->sig_index = sig_index;
  return WASM_OK;
}

static WasmResult on_table(uint32_t index,
                           uint32_t elem_type,
                           const WasmLimits* elem_limits,
                           void* user_data) {
  Context* ctx = user_data;
  wasm_new_uint32_array(ctx->allocator, &ctx->module->func_table,
                        elem_limits->initial);
  return WASM_OK;
}

static WasmResult on_memory(uint32_t index,
                            const WasmLimits* page_limits,
                            void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterMemory* memory = &ctx->module->memory;
  memory->allocator = ctx->memory_allocator;
  memory->page_size = page_limits->initial;
  memory->byte_size = page_limits->initial * WASM_PAGE_SIZE;
  memory->max_page_size =
      page_limits->has_max ? page_limits->max : WASM_MAX_PAGES;
  memory->data = wasm_alloc_zero(ctx->memory_allocator, memory->byte_size,
                                 WASM_DEFAULT_ALIGN);
  return WASM_OK;
}

static WasmResult on_global_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_new_interpreter_global_array(ctx->allocator, &ctx->module->globals,
                                    count);
  return WASM_OK;
}

static WasmResult begin_global(uint32_t index,
                               WasmType type,
                               WasmBool mutable_,
                               void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->globals.size);
  WasmInterpreterGlobal* global = &ctx->module->globals.data[index];
  global->typed_value.type = type;
  global->mutable_ = mutable_;
  return WASM_OK;
}

static WasmResult end_global_init_expr(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->globals.size);
  WasmInterpreterGlobal* global = &ctx->module->globals.data[index];
  global->typed_value = ctx->init_expr_value;
  return WASM_OK;
}

static WasmResult on_init_expr_f32_const_expr(uint32_t index,
                                              uint32_t value_bits,
                                              void* user_data) {
  Context* ctx = user_data;
  ctx->init_expr_value.type = WASM_TYPE_F32;
  ctx->init_expr_value.value.f32_bits = value_bits;
  return WASM_OK;
}

static WasmResult on_init_expr_f64_const_expr(uint32_t index,
                                              uint64_t value_bits,
                                              void* user_data) {
  Context* ctx = user_data;
  ctx->init_expr_value.type = WASM_TYPE_F64;
  ctx->init_expr_value.value.f64_bits = value_bits;
  return WASM_OK;
}

static WasmResult on_init_expr_get_global_expr(uint32_t index,
                                               uint32_t global_index,
                                               void* user_data) {
  Context* ctx = user_data;
  assert(global_index < ctx->module->globals.size);
  WasmInterpreterGlobal* ref_global =
      &ctx->module->globals.data[global_index];
  ctx->init_expr_value = ref_global->typed_value;
  return WASM_OK;
}

static WasmResult on_init_expr_i32_const_expr(uint32_t index,
                                              uint32_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  ctx->init_expr_value.type = WASM_TYPE_I32;
  ctx->init_expr_value.value.i32 = value;
  return WASM_OK;
}

static WasmResult on_init_expr_i64_const_expr(uint32_t index,
                                              uint64_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  ctx->init_expr_value.type = WASM_TYPE_I64;
  ctx->init_expr_value.value.i64 = value;
  return WASM_OK;
}

static WasmResult on_export_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_new_interpreter_export_array(ctx->allocator, &ctx->module->exports,
                                    count);
  return WASM_OK;
}

static WasmResult on_export(uint32_t index,
                            WasmExternalKind kind,
                            uint32_t item_index,
                            WasmStringSlice name,
                            void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterExport* export = &ctx->module->exports.data[index];
  export->name = wasm_dup_string_slice(ctx->allocator, name);
  export->kind = kind;
  export->index = item_index;
  return WASM_OK;
}

static WasmResult on_start_function(uint32_t func_index, void* user_data) {
  Context* ctx = user_data;
  assert(func_index < ctx->module->funcs.size);
  ctx->module->start_func_index = func_index;
  return WASM_OK;
}

static WasmResult end_elem_segment_init_expr(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  assert(ctx->init_expr_value.type == WASM_TYPE_I32);
  ctx->table_offset = ctx->init_expr_value.value.i32;
  return WASM_OK;
}

static WasmResult on_elem_segment_function_index(uint32_t index,
                                                 uint32_t func_index,
                                                 void* user_data) {
  Context* ctx = user_data;
  assert(ctx->table_offset < ctx->module->func_table.size);
  assert(index < ctx->module->func_table.size);
  uint32_t* entry = &ctx->module->func_table.data[ctx->table_offset++];
  *entry = func_index;
  return WASM_OK;
}

static WasmResult on_data_segment_data(uint32_t index,
                                       const void* src_data,
                                       uint32_t size,
                                       void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterMemory* memory = &ctx->module->memory;
  assert(ctx->init_expr_value.type == WASM_TYPE_I32);
  uint32_t address = ctx->init_expr_value.value.i32;
  uint8_t* dst_data = memory->data;
  if ((uint64_t)address + (uint64_t)size > memory->byte_size)
    return WASM_ERROR;
  memcpy(&dst_data[address], src_data, size);
  return WASM_OK;
}

static WasmResult on_function_bodies_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  assert(ctx->num_func_imports + count == ctx->module->funcs.size);
  WASM_USE(ctx);
  return WASM_OK;
}

static uint32_t translate_depth(Context* ctx, uint32_t depth) {
  assert(depth < ctx->label_stack.size);
  return ctx->label_stack.size - 1 - depth;
}

static void push_label(Context* ctx,
                       LabelType label_type,
                       const WasmTypeVector* sig,
                       uint32_t offset,
                       uint32_t fixup_offset) {
  Label* label = wasm_append_label(ctx->allocator, &ctx->label_stack);
  label->label_type = label_type;
  wasm_extend_types(ctx->allocator, &label->sig, sig);
  label->type_stack_limit = ctx->type_stack.size;
  label->offset = offset;
  label->fixup_offset = fixup_offset;
  LOGF("   : +depth %" PRIzd "\n", ctx->label_stack.size - 1);
}

static void pop_label(Context* ctx) {
  LOGF("   : -depth %" PRIzd "\n", ctx->label_stack.size - 1);
  Label* label = top_label(ctx);
  wasm_destroy_type_vector(ctx->allocator, &label->sig);
  ctx->label_stack.size--;
  /* reduce the depth_fixups stack as well, but it may be smaller than
   * label_stack so only do it conditionally. */
  if (ctx->depth_fixups.size > ctx->label_stack.size) {
    uint32_t from = ctx->label_stack.size;
    uint32_t to = ctx->depth_fixups.size;
    uint32_t i;
    for (i = from; i < to; ++i)
      wasm_destroy_uint32_vector(ctx->allocator, &ctx->depth_fixups.data[i]);
    ctx->depth_fixups.size = ctx->label_stack.size;
  }
}

static WasmType top_type(Context* ctx) {
  Label* label = top_label(ctx);
  WASM_USE(label);
  assert(ctx->type_stack.size > label->type_stack_limit);
  return ctx->type_stack.data[ctx->type_stack.size - 1];
}

static WasmBool top_type_is_any(Context* ctx) {
  if (ctx->type_stack.size > ctx->current_func->param_and_local_types.size) {
    WasmType top_type = ctx->type_stack.data[ctx->type_stack.size - 1];
    if (top_type == WASM_TYPE_ANY)
      return WASM_TRUE;
  }
  return WASM_FALSE;
}

/* TODO(binji): share a lot of the type-checking code w/ wasm-ast-checker.c */
/* TODO(binji): flip actual + expected types, to match wasm-ast-checker.c */
static size_t type_stack_limit(Context* ctx) {
  return top_label(ctx)->type_stack_limit;
}

static WasmResult check_type_stack_limit(Context* ctx,
                                         size_t expected,
                                         const char* desc) {
  RETURN_OK_IF_TOP_TYPE_IS_ANY(ctx);
  size_t limit = type_stack_limit(ctx);
  size_t avail = ctx->type_stack.size - limit;
  if (expected > avail) {
    print_error(ctx, "type stack size too small at %s. got %" PRIzd
                     ", expected at least %" PRIzd,
                desc, avail, expected);
    return WASM_ERROR;
  }
  return WASM_OK;
}

static WasmResult check_type_stack_limit_exact(Context* ctx,
                                               size_t expected,
                                               const char* desc) {
  RETURN_OK_IF_TOP_TYPE_IS_ANY(ctx);
  size_t limit = type_stack_limit(ctx);
  size_t avail = ctx->type_stack.size - limit;
  if (expected != avail) {
    print_error(ctx, "type stack at end of %s is %" PRIzd ". expected %" PRIzd,
                desc, avail, expected);
    return WASM_ERROR;
  }
  return WASM_OK;
}

static void reset_type_stack_to_limit(Context* ctx) {
  ctx->type_stack.size = type_stack_limit(ctx);
}

static WasmResult check_type(Context* ctx,
                             WasmType expected,
                             WasmType actual,
                             const char* desc) {
  RETURN_OK_IF_TOP_TYPE_IS_ANY(ctx);
  if (expected != actual) {
    print_error(ctx, "type mismatch in %s, expected %s but got %s.", desc,
                s_type_names[expected], s_type_names[actual]);
    return WASM_ERROR;
  }
  return WASM_OK;
}

static WasmResult check_n_types(Context* ctx,
                                const WasmTypeVector* expected,
                                const char* desc) {
  RETURN_OK_IF_TOP_TYPE_IS_ANY(ctx);
  CHECK_RESULT(check_type_stack_limit(ctx, expected->size, desc));
  /* check the top of the type stack, with values pushed in reverse, against
   * the expected type list; for example, if:
   *   expected = [i32, f32, i32, f64]
   * then
   *   type_stack must be [ ..., f64, i32, f32, i32] */
  size_t i;
  for (i = 0; i < expected->size; ++i) {
    WasmType actual =
        ctx->type_stack.data[ctx->type_stack.size - expected->size + i];
    CHECK_RESULT(check_type(ctx, expected->data[expected->size - i - 1],
                            actual, desc));
  }
  return WASM_OK;
}

static WasmType pop_type(Context* ctx) {
  WasmType type = top_type(ctx);
  if (type != WASM_TYPE_ANY) {
    LOGF("%3" PRIzd "->%3" PRIzd ": pop  %s\n", ctx->type_stack.size,
         ctx->type_stack.size - 1, s_type_names[type]);
    ctx->type_stack.size--;
  }
  return type;
}

static void push_type(Context* ctx, WasmType type) {
  RETURN_IF_TOP_TYPE_IS_ANY(ctx);
  if (type != WASM_TYPE_VOID) {
    LOGF("%3" PRIzd "->%3" PRIzd ": push %s\n", ctx->type_stack.size,
         ctx->type_stack.size + 1, s_type_names[type]);
    wasm_append_type_value(ctx->allocator, &ctx->type_stack, &type);
  }
}

static void push_types(Context* ctx, const WasmTypeVector* types) {
  RETURN_IF_TOP_TYPE_IS_ANY(ctx);
  size_t i;
  for (i = 0; i < types->size; ++i)
    push_type(ctx, types->data[i]);
}

static WasmResult pop_and_check_1_type(Context* ctx,
                                       WasmType expected,
                                       const char* desc) {
  RETURN_OK_IF_TOP_TYPE_IS_ANY(ctx);
  if (WASM_SUCCEEDED(check_type_stack_limit(ctx, 1, desc))) {
    WasmType actual = pop_type(ctx);
    CHECK_RESULT(check_type(ctx, expected, actual, desc));
    return WASM_OK;
  }
  return WASM_ERROR;
}

static WasmResult pop_and_check_2_types(Context* ctx,
                                        WasmType expected1,
                                        WasmType expected2,
                                        const char* desc) {
  RETURN_OK_IF_TOP_TYPE_IS_ANY(ctx);
  if (WASM_SUCCEEDED(check_type_stack_limit(ctx, 2, desc))) {
    WasmType actual2 = pop_type(ctx);
    WasmType actual1 = pop_type(ctx);
    CHECK_RESULT(check_type(ctx, expected1, actual1, desc));
    CHECK_RESULT(check_type(ctx, expected2, actual2, desc));
    return WASM_OK;
  }
  return WASM_ERROR;
}

static WasmResult check_opcode1(Context* ctx, WasmOpcode opcode) {
  RETURN_OK_IF_TOP_TYPE_IS_ANY(ctx);
  CHECK_RESULT(pop_and_check_1_type(ctx, wasm_get_opcode_param_type_1(opcode),
                                    wasm_get_opcode_name(opcode)));
  push_type(ctx, wasm_get_opcode_result_type(opcode));
  return WASM_OK;
}

static WasmResult check_opcode2(Context* ctx, WasmOpcode opcode) {
  RETURN_OK_IF_TOP_TYPE_IS_ANY(ctx);
  CHECK_RESULT(pop_and_check_2_types(ctx, wasm_get_opcode_param_type_1(opcode),
                                     wasm_get_opcode_param_type_2(opcode),
                                     wasm_get_opcode_name(opcode)));
  push_type(ctx, wasm_get_opcode_result_type(opcode));
  return WASM_OK;
}

static WasmResult drop_types_for_return(Context* ctx, uint32_t arity) {
  RETURN_OK_IF_TOP_TYPE_IS_ANY(ctx);
  /* drop the locals and params, but keep the return value, if any.  */
  if (ctx->type_stack.size >= arity) {
    uint32_t drop_count = ctx->type_stack.size - arity;
    CHECK_RESULT(emit_drop_keep(ctx, drop_count, arity));
  } else {
    /* it is possible for the size of the type stack to be smaller than the
     * return arity if the last instruction of the function is
     * return. In that case the type stack should be empty */
    assert(ctx->type_stack.size == 0);
  }
  return WASM_OK;
}

static WasmResult begin_function_body(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterFunc* func = get_defined_func(ctx, index);
  WasmInterpreterFuncSignature* sig = get_signature(ctx, func->sig_index);

  func->offset = get_istream_offset(ctx);
  func->local_decl_count = 0;
  func->local_count = 0;

  ctx->current_func = func;
  ctx->depth_fixups.size = 0;
  ctx->type_stack.size = 0;
  ctx->label_stack.size = 0;
  ctx->depth = 0;

  /* fixup function references */
  uint32_t i;
  Uint32Vector* fixups = &ctx->func_fixups.data[index];
  for (i = 0; i < fixups->size; ++i)
    CHECK_RESULT(emit_i32_at(ctx, fixups->data[i], func->offset));

  /* append param types */
  for (i = 0; i < sig->param_types.size; ++i) {
    WasmType type = sig->param_types.data[i];
    wasm_append_type_value(ctx->allocator, &func->param_and_local_types, &type);
    wasm_append_type_value(ctx->allocator, &ctx->type_stack, &type);
  }

  /* push implicit func label (equivalent to return) */
  push_label(ctx, LABEL_TYPE_FUNC, &sig->result_types, WASM_INVALID_OFFSET,
             WASM_INVALID_OFFSET);
  return WASM_OK;
}

static WasmResult end_function_body(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  Label* label = top_label(ctx);
  if (!label || label->label_type != LABEL_TYPE_FUNC) {
    print_error(ctx, "unexpected function end");
    return WASM_ERROR;
  }

  CHECK_RESULT(check_n_types(ctx, &label->sig, "implicit return"));
  CHECK_RESULT(check_type_stack_limit_exact(ctx, label->sig.size, "func"));
  fixup_top_label(ctx, get_istream_offset(ctx));
  if (top_type_is_any(ctx)) {
    /* if the top type is any it means that this code is unreachable, at least
     * from the normal fallthrough, though it's possible that this code was
     * reached by branching to the implicit function label. If so, we have
     * already validated that the stack at that location, so we just need to
     * reset it to that state. */
    reset_type_stack_to_limit(ctx);
    push_types(ctx, &label->sig);
  }
  CHECK_RESULT(drop_types_for_return(ctx, label->sig.size));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_RETURN));
  pop_label(ctx);
  ctx->current_func = NULL;
  ctx->type_stack.size = 0;
  return WASM_OK;
}

static WasmResult on_local_decl_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterFunc* func = ctx->current_func;
  func->local_decl_count = count;
  return WASM_OK;
}

static WasmResult on_local_decl(uint32_t decl_index,
                                uint32_t count,
                                WasmType type,
                                void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": alloca\n", ctx->type_stack.size);
  WasmInterpreterFunc* func = ctx->current_func;
  func->local_count += count;

  uint32_t i;
  for (i = 0; i < count; ++i) {
    wasm_append_type_value(ctx->allocator, &func->param_and_local_types, &type);
    push_type(ctx, type);
  }

  if (decl_index == func->local_decl_count - 1) {
    /* last local declaration, allocate space for all locals. */
    CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_ALLOCA));
    CHECK_RESULT(emit_i32(ctx, func->local_count));
    /* fixup the function label's type_stack_limit to include these values. */
    Label* label = top_label(ctx);
    assert(label->label_type == LABEL_TYPE_FUNC);
    label->type_stack_limit += func->local_count;
  }
  return WASM_OK;
}

static WasmResult on_unary_expr(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(check_opcode1(ctx, opcode));
  CHECK_RESULT(emit_opcode(ctx, opcode));
  return WASM_OK;
}

static WasmResult on_binary_expr(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(check_opcode2(ctx, opcode));
  CHECK_RESULT(emit_opcode(ctx, opcode));
  return WASM_OK;
}

static WasmResult on_block_expr(uint32_t num_types,
                                WasmType* sig_types,
                                void* user_data) {
  Context* ctx = user_data;
  WasmTypeVector sig;
  sig.size = num_types;
  sig.data = sig_types;
  push_label(ctx, LABEL_TYPE_BLOCK, &sig, WASM_INVALID_OFFSET,
             WASM_INVALID_OFFSET);
  return WASM_OK;
}

static WasmResult on_loop_expr(uint32_t num_types,
                               WasmType* sig_types,
                               void* user_data) {
  Context* ctx = user_data;
  WasmTypeVector sig;
  sig.size = num_types;
  sig.data = sig_types;
  push_label(ctx, LABEL_TYPE_LOOP, &sig, get_istream_offset(ctx),
             WASM_INVALID_OFFSET);
  return WASM_OK;
}

static WasmResult on_if_expr(uint32_t num_types,
                             WasmType* sig_types,
                             void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(check_type_stack_limit(ctx, 1, "if"));
  CHECK_RESULT(pop_and_check_1_type(ctx, WASM_TYPE_I32, "if"));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_UNLESS));
  uint32_t fixup_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));

  WasmTypeVector sig;
  sig.size = num_types;
  sig.data = sig_types;
  push_label(ctx, LABEL_TYPE_IF, &sig, WASM_INVALID_OFFSET, fixup_offset);
  return WASM_OK;
}

static WasmResult on_else_expr(void* user_data) {
  Context* ctx = user_data;
  Label* label = top_label(ctx);
  if (!label || label->label_type != LABEL_TYPE_IF) {
    print_error(ctx, "unexpected else operator");
    return WASM_ERROR;
  }

  CHECK_RESULT(check_n_types(ctx, &label->sig, "if true branch"));

  label->label_type = LABEL_TYPE_ELSE;
  uint32_t fixup_cond_offset = label->fixup_offset;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR));
  label->fixup_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
  CHECK_RESULT(emit_i32_at(ctx, fixup_cond_offset, get_istream_offset(ctx)));
  /* reset the type stack for the other branch arm */
  ctx->type_stack.size = label->type_stack_limit;
  return WASM_OK;
}

static WasmResult on_end_expr(void* user_data) {
  Context* ctx = user_data;
  Label* label = top_label(ctx);
  if (!label) {
    print_error(ctx, "unexpected end operator");
    return WASM_ERROR;
  }

  const char* desc = NULL;
  switch (label->label_type) {
    case LABEL_TYPE_IF:
    case LABEL_TYPE_ELSE:
      desc = (label->label_type == LABEL_TYPE_IF) ? "if true branch"
                                                  : "if false branch";
      CHECK_RESULT(
          emit_i32_at(ctx, label->fixup_offset, get_istream_offset(ctx)));
      break;

    case LABEL_TYPE_BLOCK:
      desc = "block";
      break;

    case LABEL_TYPE_LOOP:
      desc = "loop";
      break;

    case LABEL_TYPE_FUNC:
      print_error(ctx, "unexpected end operator");
      return WASM_ERROR;
  }

  CHECK_RESULT(check_n_types(ctx, &label->sig, desc));
  CHECK_RESULT(check_type_stack_limit_exact(ctx, label->sig.size, desc));
  fixup_top_label(ctx, get_istream_offset(ctx));
  reset_type_stack_to_limit(ctx);
  push_types(ctx, &label->sig);
  pop_label(ctx);
  return WASM_OK;
}

static WasmResult on_br_expr(uint32_t depth, void* user_data) {
  Context* ctx = user_data;
  CHECK_DEPTH(ctx, depth);
  depth = translate_depth(ctx, depth);
  Label* label = get_label(ctx, depth);
  if (label->label_type != LABEL_TYPE_LOOP)
    CHECK_RESULT(check_n_types(ctx, &label->sig, "br"));
  CHECK_RESULT(emit_br(ctx, depth));
  reset_type_stack_to_limit(ctx);
  push_type(ctx, WASM_TYPE_ANY);
  return WASM_OK;
}

static WasmResult on_br_if_expr(uint32_t depth, void* user_data) {
  Context* ctx = user_data;
  CHECK_DEPTH(ctx, depth);
  depth = translate_depth(ctx, depth);
  CHECK_RESULT(pop_and_check_1_type(ctx, WASM_TYPE_I32, "br_if"));
  /* flip the br_if so if <cond> is true it can drop values from the stack */
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_UNLESS));
  uint32_t fixup_br_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
  CHECK_RESULT(emit_br(ctx, depth));
  CHECK_RESULT(emit_i32_at(ctx, fixup_br_offset, get_istream_offset(ctx)));
  return WASM_OK;
}

static WasmResult on_br_table_expr(uint32_t num_targets,
                                   uint32_t* target_depths,
                                   uint32_t default_target_depth,
                                   void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(pop_and_check_1_type(ctx, WASM_TYPE_I32, "br_table"));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_TABLE));
  CHECK_RESULT(emit_i32(ctx, num_targets));
  uint32_t fixup_table_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
  /* not necessary for the interpreter, but it makes it easier to disassemble.
   * This opcode specifies how many bytes of data follow. */
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DATA));
  CHECK_RESULT(emit_i32(ctx, (num_targets + 1) * WASM_TABLE_ENTRY_SIZE));
  CHECK_RESULT(emit_i32_at(ctx, fixup_table_offset, get_istream_offset(ctx)));

  uint32_t i;
  for (i = 0; i <= num_targets; ++i) {
    uint32_t depth = i != num_targets ? target_depths[i] : default_target_depth;
    depth = translate_depth(ctx, depth);
    Label* label = get_label(ctx, depth);
    CHECK_RESULT(check_n_types(ctx, &label->sig, "br_table"));
    CHECK_RESULT(emit_br_table_offset(ctx, depth));
  }

  reset_type_stack_to_limit(ctx);
  push_type(ctx, WASM_TYPE_ANY);
  return WASM_OK;
}

static WasmResult on_call_expr(uint32_t func_index, void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterFunc* func = get_func(ctx, func_index);
  WasmInterpreterFuncSignature* sig = get_signature(ctx, func->sig_index);
  CHECK_RESULT(check_type_stack_limit(ctx, sig->param_types.size, "call"));

  uint32_t i;
  for (i = sig->param_types.size; i > 0; --i) {
    WasmType arg = pop_type(ctx);
    CHECK_RESULT(check_type(ctx, sig->param_types.data[i - 1], arg, "call"));
  }

  if (func->is_host) {
    CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_HOST));
    CHECK_RESULT(emit_i32(ctx, func_index));
  } else {
    CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_FUNCTION));
    CHECK_RESULT(emit_func_offset(ctx, func, func_index));
  }
  push_types(ctx, &sig->result_types);

  return WASM_OK;
}

static WasmResult on_call_indirect_expr(uint32_t sig_index, void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterFuncSignature* sig = get_signature(ctx, sig_index);
  CHECK_RESULT(pop_and_check_1_type(ctx, WASM_TYPE_I32, "call_indirect"));
  CHECK_RESULT(
      check_type_stack_limit(ctx, sig->param_types.size, "call_indirect"));

  uint32_t i;
  for (i = sig->param_types.size; i > 0; --i) {
    WasmType arg = pop_type(ctx);
    CHECK_RESULT(
        check_type(ctx, sig->param_types.data[i - 1], arg, "call_indirect"));
  }

  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_INDIRECT));
  CHECK_RESULT(emit_i32(ctx, sig_index));
  push_types(ctx, &sig->result_types);
  return WASM_OK;
}

static WasmResult on_drop_expr(void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(check_type_stack_limit(ctx, 1, "drop"));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DROP));
  pop_type(ctx);
  return WASM_OK;
}

static WasmResult on_i32_const_expr(uint32_t value, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I32_CONST));
  CHECK_RESULT(emit_i32(ctx, value));
  push_type(ctx, WASM_TYPE_I32);
  return WASM_OK;
}

static WasmResult on_i64_const_expr(uint64_t value, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I64_CONST));
  CHECK_RESULT(emit_i64(ctx, value));
  push_type(ctx, WASM_TYPE_I64);
  return WASM_OK;
}

static WasmResult on_f32_const_expr(uint32_t value_bits, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_F32_CONST));
  CHECK_RESULT(emit_i32(ctx, value_bits));
  push_type(ctx, WASM_TYPE_F32);
  return WASM_OK;
}

static WasmResult on_f64_const_expr(uint64_t value_bits, void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_F64_CONST));
  CHECK_RESULT(emit_i64(ctx, value_bits));
  push_type(ctx, WASM_TYPE_F64);
  return WASM_OK;
}

static WasmResult on_get_global_expr(uint32_t global_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_GLOBAL(ctx, global_index);
  WasmType type = get_global_type_by_index(ctx, global_index);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_GET_GLOBAL));
  CHECK_RESULT(emit_i32(ctx, global_index));
  push_type(ctx, type);
  return WASM_OK;
}

static WasmResult on_set_global_expr(uint32_t global_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_GLOBAL(ctx, global_index);
  WasmInterpreterGlobal* global = get_global_by_index(ctx, global_index);
  if (global->mutable_ != WASM_TRUE) {
    print_error(ctx, "can't set_global on immutable global at index %u.",
                global_index);
    return WASM_ERROR;
  }
  CHECK_RESULT(
      pop_and_check_1_type(ctx, global->typed_value.type, "set_global"));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_SET_GLOBAL));
  CHECK_RESULT(emit_i32(ctx, global_index));
  return WASM_OK;
}

static WasmResult on_get_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_LOCAL(ctx, local_index);
  WasmType type = get_local_type_by_index(ctx->current_func, local_index);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_GET_LOCAL));
  CHECK_RESULT(emit_i32(ctx, translate_local_index(ctx, local_index)));
  push_type(ctx, type);
  return WASM_OK;
}

static WasmResult on_set_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_LOCAL(ctx, local_index);
  WasmType type = get_local_type_by_index(ctx->current_func, local_index);
  CHECK_RESULT(pop_and_check_1_type(ctx, type, "set_local"));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_SET_LOCAL));
  CHECK_RESULT(emit_i32(ctx, translate_local_index(ctx, local_index)));
  return WASM_OK;
}

static WasmResult on_tee_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_LOCAL(ctx, local_index);
  WasmType type = get_local_type_by_index(ctx->current_func, local_index);
  CHECK_RESULT(check_type_stack_limit(ctx, 1, "tee_local"));
  WasmType value = top_type(ctx);
  CHECK_RESULT(check_type(ctx, type, value, "tee_local"));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_TEE_LOCAL));
  CHECK_RESULT(emit_i32(ctx, translate_local_index(ctx, local_index)));
  return WASM_OK;
}

static WasmResult on_grow_memory_expr(void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(pop_and_check_1_type(ctx, WASM_TYPE_I32, "grow_memory"));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_GROW_MEMORY));
  push_type(ctx, WASM_TYPE_I32);
  return WASM_OK;
}

static WasmResult on_load_expr(WasmOpcode opcode,
                               uint32_t alignment_log2,
                               uint32_t offset,
                               void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(check_opcode1(ctx, opcode));
  CHECK_RESULT(emit_opcode(ctx, opcode));
  CHECK_RESULT(emit_i32(ctx, offset));
  return WASM_OK;
}

static WasmResult on_store_expr(WasmOpcode opcode,
                                uint32_t alignment_log2,
                                uint32_t offset,
                                void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(check_opcode2(ctx, opcode));
  CHECK_RESULT(emit_opcode(ctx, opcode));
  CHECK_RESULT(emit_i32(ctx, offset));
  return WASM_OK;
}

static WasmResult on_current_memory_expr(void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CURRENT_MEMORY));
  push_type(ctx, WASM_TYPE_I32);
  return WASM_OK;
}

static WasmResult on_nop_expr(void* user_data) {
  return WASM_OK;
}

static WasmResult on_return_expr(void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterFuncSignature* sig =
      get_func_signature(ctx, ctx->current_func);
  CHECK_RESULT(check_n_types(ctx, &sig->result_types, "return"));
  CHECK_RESULT(drop_types_for_return(ctx, sig->result_types.size));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_RETURN));
  reset_type_stack_to_limit(ctx);
  push_type(ctx, WASM_TYPE_ANY);
  return WASM_OK;
}

static WasmResult on_select_expr(void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(pop_and_check_1_type(ctx, WASM_TYPE_I32, "select"));
  CHECK_RESULT(check_type_stack_limit(ctx, 2, "select"));
  WasmType right = pop_type(ctx);
  WasmType left = pop_type(ctx);
  CHECK_RESULT(check_type(ctx, left, right, "select"));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_SELECT));
  push_type(ctx, left);
  return WASM_OK;
}

static WasmResult on_unreachable_expr(void* user_data) {
  Context* ctx = user_data;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_UNREACHABLE));
  reset_type_stack_to_limit(ctx);
  push_type(ctx, WASM_TYPE_ANY);
  return WASM_OK;
}

static WasmBinaryReader s_binary_reader = {
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

    .on_export_count = on_export_count,
    .on_export = on_export,

    .on_start_function = on_start_function,

    .on_function_bodies_count = on_function_bodies_count,
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
    .on_elem_segment_function_index = on_elem_segment_function_index,

    .on_data_segment_data = on_data_segment_data,

    .on_init_expr_f32_const_expr = on_init_expr_f32_const_expr,
    .on_init_expr_f64_const_expr = on_init_expr_f64_const_expr,
    .on_init_expr_get_global_expr = on_init_expr_get_global_expr,
    .on_init_expr_i32_const_expr = on_init_expr_i32_const_expr,
    .on_init_expr_i64_const_expr = on_init_expr_i64_const_expr,
};

static void destroy_context(Context* ctx) {
  wasm_destroy_type_vector(ctx->allocator, &ctx->type_stack);
  wasm_destroy_label_vector(ctx->allocator, &ctx->label_stack);
  WASM_DESTROY_VECTOR_AND_ELEMENTS(ctx->allocator, ctx->depth_fixups,
                                   uint32_vector);
  WASM_DESTROY_VECTOR_AND_ELEMENTS(ctx->allocator, ctx->func_fixups,
                                   uint32_vector);
}

WasmResult wasm_read_binary_interpreter(WasmAllocator* allocator,
                                        WasmAllocator* memory_allocator,
                                        const void* data,
                                        size_t size,
                                        const WasmReadBinaryOptions* options,
                                        WasmBinaryErrorHandler* error_handler,
                                        WasmInterpreterModule* out_module) {
  Context ctx;
  WasmBinaryReader reader;

  WASM_ZERO_MEMORY(ctx);
  WASM_ZERO_MEMORY(reader);

  ctx.allocator = allocator;
  ctx.reader = &reader;
  ctx.error_handler = error_handler;
  ctx.memory_allocator = memory_allocator;
  ctx.module = out_module;
  ctx.module->start_func_index = WASM_INVALID_FUNC_INDEX;
  CHECK_RESULT(wasm_init_mem_writer(allocator, &ctx.istream_writer));

  reader = s_binary_reader;
  reader.user_data = &ctx;

  const uint32_t num_function_passes = 1;
  WasmResult result = wasm_read_binary(allocator, data, size, &reader,
                                       num_function_passes, options);
  if (WASM_SUCCEEDED(result)) {
    wasm_steal_mem_writer_output_buffer(&ctx.istream_writer,
                                        &out_module->istream);
    out_module->istream.size = ctx.istream_offset;
  } else {
    wasm_close_mem_writer(&ctx.istream_writer);
  }
  destroy_context(&ctx);
  return result;
}

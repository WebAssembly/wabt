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

#define INVALID_FUNC_INDEX ((uint32_t)~0)

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
  do {                     \
    if (WASM_FAILED(expr)) \
      return WASM_ERROR;   \
  } while (0)

#define CHECK_DEPTH(ctx, depth)                                         \
  do {                                                                  \
    if ((depth) >= (ctx)->depth_stack.size) {                           \
      print_error((ctx), "invalid depth: %d (max %" PRIzd ")", (depth), \
                  ((ctx)->depth_stack.size));                           \
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

#define WASM_TYPE_ANY WASM_NUM_TYPES

static const char* s_type_names[] = {
    "void", "i32", "i64", "f32", "f64", "any",
};
WASM_STATIC_ASSERT(WASM_ARRAY_SIZE(s_type_names) == WASM_NUM_TYPES + 1);

/* TODO(binji): combine with the ones defined in wasm-check? */
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

#if LOG
#define V(rtype, type1, type2, mem_size, code, NAME, text) [code] = text,
static const char* s_opcode_name[] = {
  /* clang-format off */
  WASM_FOREACH_OPCODE(V)
  [WASM_OPCODE_ALLOCA] = "alloca",
  [WASM_OPCODE_DISCARD] = "discard",
  [WASM_OPCODE_DISCARD_KEEP] = "discard_keep",
  /* clang-format on */
};
#undef V
#endif

WASM_DEFINE_VECTOR(uint32, WasmUint32);
WASM_DEFINE_VECTOR(uint32_vector, WasmUint32Vector);

typedef struct WasmDepthNode {
  WasmType type;
  /* we store the value stack size at this depth so we know how many
   * values to discard if we break to this depth */
  uint32_t value_stack_size;
  uint32_t offset;
} WasmDepthNode;
WASM_DEFINE_VECTOR(depth_node, WasmDepthNode);

typedef struct WasmInterpreterExpr {
  WasmOpcode opcode;
  WasmType type;
  union {
    /* clang-format off */
    struct { uint32_t depth; } br, br_if;
    struct { uint32_t value_stack_size; } block, loop;
    struct { uint32_t num_targets, table_offset; } br_table;
    struct { uint32_t func_index; } call;
    struct { uint32_t import_index; } call_import;
    struct { uint32_t sig_index; } call_indirect;
    struct { uint32_t fixup_offset; } if_;
    struct {
      uint32_t fixup_nop_offset, fixup_cond_offset, fixup_true_offset;
      uint32_t value_stack_size;
    } if_else;
    struct { uint32_t mem_offset, alignment_log2; } load, store;
    struct { uint32_t local_index; } get_local, set_local;
    /* clang-format on */
  };
} WasmInterpreterExpr;

typedef struct WasmExprNode {
  WasmInterpreterExpr expr;
  uint32_t index;
  uint32_t total;
  uint32_t value_stack_size;
} WasmExprNode;
WASM_DEFINE_VECTOR(expr_node, WasmExprNode);

typedef struct WasmInterpreterFunc {
  uint32_t sig_index;
  uint32_t offset;
  uint32_t local_decl_count;
  uint32_t local_count;
  WasmTypeVector param_and_local_types;
} WasmInterpreterFunc;
WASM_DEFINE_ARRAY(interpreter_func, WasmInterpreterFunc);

typedef struct WasmContext {
  WasmAllocator* allocator;
  WasmBinaryErrorHandler* error_handler;
  WasmAllocator* memory_allocator;
  WasmInterpreterModule* module;
  WasmInterpreterFuncArray funcs;
  WasmInterpreterFunc* current_func;
  WasmExprNodeVector expr_stack;
  WasmDepthNodeVector depth_stack;
  WasmUint32VectorVector func_fixups;
  WasmUint32VectorVector depth_fixups;
  uint32_t value_stack_size;
  uint32_t depth;
  uint32_t start_func_index;
  WasmMemoryWriter istream_writer;
  uint32_t istream_offset;
  /* the last expression evaluated at the top-level of a func */
  WasmInterpreterExpr last_expr;
  WasmBool last_expr_was_discarded;
} WasmContext;

static WasmDepthNode* get_depth_node(WasmContext* ctx, uint32_t depth) {
  assert(depth < ctx->depth_stack.size);
  return &ctx->depth_stack.data[depth];
}

static WasmDepthNode* top_minus_nth_depth_node(WasmContext* ctx, uint32_t n) {
  return get_depth_node(ctx, ctx->depth_stack.size - n);
}

static WasmDepthNode* top_depth_node(WasmContext* ctx) {
  return top_minus_nth_depth_node(ctx, 1);
}

static uint32_t get_istream_offset(WasmContext* ctx) {
  return ctx->istream_offset;
}

static uint32_t get_value_count(WasmType result_type) {
  return (result_type == WASM_TYPE_VOID || result_type == WASM_TYPE_ANY) ? 0
                                                                         : 1;
}

static void on_error(uint32_t offset, const char* message, void* user_data);

static void print_error(WasmContext* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  on_error(WASM_INVALID_OFFSET, buffer, ctx);
}

static void adjust_value_stack(WasmContext* ctx, int32_t amount) {
  uint32_t old_size = ctx->value_stack_size;
  uint32_t new_size = old_size + (uint32_t)amount;
  assert((amount <= 0 && new_size <= old_size) ||
         (amount > 0 && new_size > old_size));
  WASM_USE(old_size);
  WASM_USE(new_size);
  ctx->value_stack_size += (uint32_t)amount;
#ifndef NDEBUG
  if (ctx->depth_stack.size > 0) {
    assert(ctx->value_stack_size >=
           ctx->depth_stack.data[ctx->depth_stack.size - 1].value_stack_size);
  } else {
    assert(ctx->value_stack_size >=
           ctx->current_func->param_and_local_types.size);
  }
#endif
}

static WasmResult type_mismatch(WasmContext* ctx,
                                WasmType expected_type,
                                WasmType type,
                                const char* desc) {
  print_error(ctx, "type mismatch%s, expected %s but got %s.", desc,
              s_type_names[expected_type], s_type_names[type]);
  return WASM_ERROR;
}

static WasmResult check_type(WasmContext* ctx,
                             WasmType expected_type,
                             WasmType type,
                             const char* desc) {
  if (expected_type == WASM_TYPE_ANY || type == WASM_TYPE_ANY ||
      expected_type == WASM_TYPE_VOID) {
    return WASM_OK;
  }
  if (expected_type == type)
    return WASM_OK;
  return type_mismatch(ctx, expected_type, type, desc);
}

static void unify_type(WasmType* dest_type, WasmType type) {
  if (*dest_type == WASM_TYPE_ANY)
    *dest_type = type;
  else if (type != WASM_TYPE_ANY && *dest_type != type)
    *dest_type = WASM_TYPE_VOID;
}

static WasmResult unify_and_check_type(WasmContext* ctx,
                                       WasmType* dest_type,
                                       WasmType type,
                                       const char* desc) {
  unify_type(dest_type, type);
  return check_type(ctx, *dest_type, type, desc);
}

static WasmResult emit_data_at(WasmContext* ctx,
                               size_t offset,
                               const void* data,
                               size_t size) {
  return ctx->istream_writer.base.write_data(
      offset, data, size, ctx->istream_writer.base.user_data);
}

static WasmResult emit_data(WasmContext* ctx, const void* data, size_t size) {
  CHECK_RESULT(emit_data_at(ctx, ctx->istream_offset, data, size));
  ctx->istream_offset += size;
  return WASM_OK;
}

static WasmResult emit_opcode(WasmContext* ctx, WasmOpcode opcode) {
  return emit_data(ctx, &opcode, sizeof(uint8_t));
}

static WasmResult emit_i8(WasmContext* ctx, uint8_t value) {
  return emit_data(ctx, &value, sizeof(value));
}

static WasmResult emit_i32(WasmContext* ctx, uint32_t value) {
  return emit_data(ctx, &value, sizeof(value));
}

static WasmResult emit_i64(WasmContext* ctx, uint64_t value) {
  return emit_data(ctx, &value, sizeof(value));
}

static WasmResult emit_opcode_at(WasmContext* ctx,
                                 uint32_t offset,
                                 WasmOpcode opcode) {
  return emit_data_at(ctx, offset, &opcode, sizeof(uint8_t));
}

static WasmResult emit_i32_at(WasmContext* ctx,
                              uint32_t offset,
                              uint32_t value) {
  return emit_data_at(ctx, offset, &value, sizeof(value));
}

static void unemit_discard(WasmContext* ctx) {
  assert(ctx->istream_offset > 0);
  assert(ctx->istream_offset <= ctx->istream_writer.buf.size);
  assert(((uint8_t*)ctx->istream_writer.buf.start)[ctx->istream_offset - 1] ==
         WASM_OPCODE_DISCARD);
  ctx->istream_offset--;
  ctx->value_stack_size++;
}

static WasmResult emit_discard(WasmContext* ctx) {
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DISCARD));
  adjust_value_stack(ctx, -1);
  return WASM_OK;
}

static WasmResult maybe_emit_discard(WasmContext* ctx,
                                     WasmType type,
                                     WasmBool* out_discarded) {
  WasmBool should_discard = get_value_count(type) != 0;
  if (out_discarded)
    *out_discarded = should_discard;
  if (should_discard)
    return emit_discard(ctx);
  return WASM_OK;
}

static WasmResult emit_discard_keep(WasmContext* ctx,
                                    uint32_t discard,
                                    uint8_t keep) {
  assert(discard != UINT32_MAX);
  assert(keep <= 1);
  if (discard > 0) {
    if (discard == 1 && keep == 0) {
      CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DISCARD));
    } else {
      CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DISCARD_KEEP));
      CHECK_RESULT(emit_i32(ctx, discard));
      CHECK_RESULT(emit_i8(ctx, keep));
    }
  }
  return WASM_OK;
}

static WasmResult emit_return(WasmContext* ctx, WasmType result_type) {
  uint32_t keep_count = get_value_count(result_type);
  uint32_t discard_count = ctx->value_stack_size - keep_count;
  CHECK_RESULT(emit_discard_keep(ctx, discard_count, keep_count));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_RETURN));
  return WASM_OK;
}

static WasmResult append_fixup(WasmContext* ctx,
                               WasmUint32VectorVector* fixups_vector,
                               uint32_t index) {
  if (index >= fixups_vector->size) {
    CHECK_ALLOC(ctx, wasm_resize_uint32_vector_vector(
                         ctx->allocator, fixups_vector, index + 1));
  }
  WasmUint32Vector* fixups = &fixups_vector->data[index];
  uint32_t offset = get_istream_offset(ctx);
  CHECK_ALLOC(ctx, wasm_append_uint32_value(ctx->allocator, fixups, &offset));
  return WASM_OK;
}

static WasmResult emit_br_offset(WasmContext* ctx,
                                 uint32_t depth,
                                 uint32_t offset) {
  if (offset == WASM_INVALID_OFFSET)
    CHECK_RESULT(append_fixup(ctx, &ctx->depth_fixups, depth));
  CHECK_RESULT(emit_i32(ctx, offset));
  return WASM_OK;
}

static WasmResult emit_br(WasmContext* ctx,
                          uint32_t depth,
                          WasmDepthNode* node) {
  WasmType expected_type = node->type;
  assert(ctx->value_stack_size >= node->value_stack_size);
  uint8_t keep_count = get_value_count(expected_type);
  uint32_t discard_count =
      (ctx->value_stack_size - node->value_stack_size) - keep_count;
  CHECK_RESULT(emit_discard_keep(ctx, discard_count, keep_count));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR));
  CHECK_RESULT(emit_br_offset(ctx, depth, node->offset));
  return WASM_OK;
}

static WasmResult emit_br_table_offset(WasmContext* ctx,
                                       uint32_t depth,
                                       WasmDepthNode* node) {
  uint32_t discard_count = ctx->value_stack_size - node->value_stack_size;
  CHECK_RESULT(emit_br_offset(ctx, depth, node->offset));
  CHECK_RESULT(emit_i32(ctx, discard_count));
  return WASM_OK;
}

static WasmResult emit_func_offset(WasmContext* ctx,
                                   WasmInterpreterFunc* func,
                                   uint32_t func_index) {
  if (func->offset == WASM_INVALID_OFFSET)
    CHECK_RESULT(append_fixup(ctx, &ctx->func_fixups, func_index));
  CHECK_RESULT(emit_i32(ctx, func->offset));
  return WASM_OK;
}

static WasmInterpreterFunc* get_func(WasmContext* ctx, uint32_t func_index) {
  assert(func_index < ctx->funcs.size);
  return &ctx->funcs.data[func_index];
}

static WasmInterpreterImport* get_import(WasmContext* ctx,
                                         uint32_t import_index) {
  assert(import_index < ctx->module->imports.size);
  return &ctx->module->imports.data[import_index];
}

static WasmInterpreterExport* get_export(WasmContext* ctx,
                                         uint32_t export_index) {
  assert(export_index < ctx->module->exports.size);
  return &ctx->module->exports.data[export_index];
}

static WasmInterpreterFuncSignature* get_signature(WasmContext* ctx,
                                                   uint32_t sig_index) {
  assert(sig_index < ctx->module->sigs.size);
  return &ctx->module->sigs.data[sig_index];
}

static WasmInterpreterFuncSignature* get_func_signature(
    WasmContext* ctx,
    WasmInterpreterFunc* func) {
  return get_signature(ctx, func->sig_index);
}

static WasmType get_local_index_type(WasmInterpreterFunc* func,
                                     uint32_t local_index) {
  assert(local_index < func->param_and_local_types.size);
  return func->param_and_local_types.data[local_index];
}

static WasmResult push_depth_with_offset(WasmContext* ctx,
                                         WasmType type,
                                         uint32_t offset) {
  WasmDepthNode* node =
      wasm_append_depth_node(ctx->allocator, &ctx->depth_stack);
  CHECK_ALLOC_NULL(ctx, node);
  node->type = type;
  node->value_stack_size = ctx->value_stack_size;
  node->offset = offset;
  LOGF("   (%d): push depth %" PRIzd ":%s\n", ctx->value_stack_size,
       ctx->depth_stack.size - 1, s_type_names[type]);
  return WASM_OK;
}

static WasmResult push_depth(WasmContext* ctx, WasmType type) {
  return push_depth_with_offset(ctx, type, WASM_INVALID_OFFSET);
}

static void pop_depth(WasmContext* ctx) {
  LOGF("   (%d): pop depth %" PRIzd "\n", ctx->value_stack_size,
       ctx->depth_stack.size - 1);
  assert(ctx->depth_stack.size > 0);
  ctx->depth_stack.size--;
  /* reduce the depth_fixups stack as well, but it may be smaller than
   * depth_stack so only do it conditionally. */
  if (ctx->depth_fixups.size > ctx->depth_stack.size) {
    uint32_t from = ctx->depth_stack.size;
    uint32_t to = ctx->depth_fixups.size;
    uint32_t i;
    for (i = from; i < to; ++i)
      wasm_destroy_uint32_vector(ctx->allocator, &ctx->depth_fixups.data[i]);
    ctx->depth_fixups.size = ctx->depth_stack.size;
  }
}

static uint32_t translate_depth(WasmContext* ctx, uint32_t depth) {
  assert(depth < ctx->depth_stack.size);
  return ctx->depth_stack.size - 1 - depth;
}

static WasmResult fixup_top_depth(WasmContext* ctx, uint32_t offset) {
  uint32_t top = ctx->depth_stack.size - 1;
  if (top >= ctx->depth_fixups.size) {
    /* nothing to fixup */
    return WASM_OK;
  }

  WasmUint32Vector* fixups = &ctx->depth_fixups.data[top];
  uint32_t i;
  for (i = 0; i < fixups->size; ++i)
    CHECK_RESULT(emit_i32_at(ctx, fixups->data[i], offset));
  /* reduce the size to 0 in case this gets reused. Keep the allocations for
   * later use */
  fixups->size = 0;
  return WASM_OK;
}

static uint32_t translate_local_index(WasmContext* ctx, uint32_t local_index) {
  assert(local_index < ctx->value_stack_size);
  return ctx->value_stack_size - local_index;
}

void on_error(uint32_t offset, const char* message, void* user_data) {
  WasmContext* ctx = user_data;
  if (ctx->error_handler->on_error) {
    ctx->error_handler->on_error(offset, message,
                                 ctx->error_handler->user_data);
  }
}

static WasmResult on_memory_initial_size_pages(uint32_t pages,
                                               void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterMemory* memory = &ctx->module->memory;
  memory->allocator = ctx->memory_allocator;
  memory->page_size = pages;
  memory->byte_size = pages * WASM_PAGE_SIZE;
  CHECK_ALLOC_NULL(
      ctx, memory->data = wasm_alloc_zero(
               ctx->memory_allocator, memory->byte_size, WASM_DEFAULT_ALIGN));
  return WASM_OK;
}

static WasmResult on_data_segment(uint32_t index,
                                  uint32_t address,
                                  const void* src_data,
                                  uint32_t size,
                                  void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterMemory* memory = &ctx->module->memory;
  uint8_t* dst_data = memory->data;
  memcpy(&dst_data[address], src_data, size);
  return WASM_OK;
}

static WasmResult on_signature_count(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_ALLOC(ctx, wasm_new_interpreter_func_signature_array(
                       ctx->allocator, &ctx->module->sigs, count));
  return WASM_OK;
}

static WasmResult on_signature(uint32_t index,
                               WasmType result_type,
                               uint32_t param_count,
                               WasmType* param_types,
                               void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterFuncSignature* sig = get_signature(ctx, index);
  sig->result_type = result_type;

  CHECK_ALLOC(
      ctx, wasm_reserve_types(ctx->allocator, &sig->param_types, param_count));
  sig->param_types.size = param_count;
  memcpy(sig->param_types.data, param_types, param_count * sizeof(WasmType));
  return WASM_OK;
}

static WasmResult on_import_count(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_ALLOC(ctx, wasm_new_interpreter_import_array(
                       ctx->allocator, &ctx->module->imports, count));
  return WASM_OK;
}

static WasmResult on_import(uint32_t index,
                            uint32_t sig_index,
                            WasmStringSlice module_name,
                            WasmStringSlice function_name,
                            void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterImport* import = &ctx->module->imports.data[index];
  CHECK_ALLOC_NULL_STR(ctx, import->module_name = wasm_dup_string_slice(
                                ctx->allocator, module_name));
  CHECK_ALLOC_NULL_STR(ctx, import->func_name = wasm_dup_string_slice(
                                ctx->allocator, function_name));
  assert(sig_index < ctx->module->sigs.size);
  import->sig_index = sig_index;
  return WASM_OK;
}

static WasmResult on_function_signatures_count(uint32_t count,
                                               void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_ALLOC(
      ctx, wasm_new_interpreter_func_array(ctx->allocator, &ctx->funcs, count));
  CHECK_ALLOC(ctx, wasm_resize_uint32_vector_vector(ctx->allocator,
                                                    &ctx->func_fixups, count));
  return WASM_OK;
}

static WasmResult on_function_signature(uint32_t index,
                                        uint32_t sig_index,
                                        void* user_data) {
  WasmContext* ctx = user_data;
  assert(sig_index < ctx->module->sigs.size);
  WasmInterpreterFunc* func = get_func(ctx, index);
  func->offset = WASM_INVALID_OFFSET;
  func->sig_index = sig_index;
  return WASM_OK;
}

static WasmResult on_function_bodies_count(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  assert(count == ctx->funcs.size);
  WASM_USE(ctx);
  return WASM_OK;
}

static WasmResult begin_function_body(uint32_t index, void* user_data) {
  LOGF("*** func %d ***\n", index);
  WasmContext* ctx = user_data;
  WasmInterpreterFunc* func = get_func(ctx, index);
  WasmInterpreterFuncSignature* sig = get_signature(ctx, func->sig_index);
  ctx->current_func = func;
  func->offset = get_istream_offset(ctx);

  /* fixup function references */
  uint32_t i;
  WasmUint32Vector* fixups = &ctx->func_fixups.data[index];
  for (i = 0; i < fixups->size; ++i)
    CHECK_RESULT(emit_i32_at(ctx, fixups->data[i], func->offset));

  /* append param types */
  for (i = 0; i < sig->param_types.size; ++i) {
    CHECK_RESULT(wasm_append_type_value(ctx->allocator,
                                        &func->param_and_local_types,
                                        &sig->param_types.data[i]));
  }

  ctx->value_stack_size = sig->param_types.size;
  WASM_ZERO_MEMORY(ctx->last_expr);
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
  assert(ctx->depth_stack.size == 0);
  WasmInterpreterFunc* func = ctx->current_func;
  WasmInterpreterFuncSignature* sig = get_signature(ctx, func->sig_index);
  if (ctx->last_expr.opcode != WASM_OPCODE_RETURN &&
      ctx->last_expr.opcode != WASM_OPCODE_UNREACHABLE) {
    if (sig->result_type != WASM_TYPE_VOID) {
      CHECK_RESULT(check_type(ctx, sig->result_type, ctx->last_expr.type,
                              " in function result"));
      if (ctx->last_expr_was_discarded)
        unemit_discard(ctx);
    }
    CHECK_RESULT(emit_return(ctx, sig->result_type));
  }
  ctx->current_func = NULL;
  ctx->value_stack_size = 0;
  return WASM_OK;
}

static WasmResult on_local_decl_count(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  ctx->current_func->local_decl_count = count;
  return WASM_OK;
}

static WasmResult on_local_decl(uint32_t decl_index,
                                uint32_t count,
                                WasmType type,
                                void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterFunc* func = ctx->current_func;
  func->local_count += count;

  uint32_t i;
  for (i = 0; i < count; ++i) {
    CHECK_RESULT(wasm_append_type_value(ctx->allocator,
                                        &func->param_and_local_types, &type));
  }

  if (decl_index == func->local_decl_count - 1) {
    /* last local declaration, allocate space for all locals. */
    CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_ALLOCA));
    CHECK_RESULT(emit_i32(ctx, func->local_count));
    adjust_value_stack(ctx, func->local_count);
  }
  return WASM_OK;
}

static WasmResult reduce(WasmContext* ctx, WasmInterpreterExpr* expr) {
  WasmBool done = WASM_FALSE;
  while (!done) {
    done = WASM_TRUE;

    if (ctx->expr_stack.size == 0) {
      LOGF("%3" PRIzd "(%d): reduce: <- %s:%s\n",
              ctx->expr_stack.size, ctx->value_stack_size,
              s_opcode_name[expr->opcode], s_type_names[expr->type]);

      /* discard all top-level values. The last one is the return value, which
       * we don't want to discard, but we won't know if this is the last
       * expression until we get the end_function_body message. So we'll always
       * write in a discard here, then remove it later if necessary. */
      CHECK_RESULT(
          maybe_emit_discard(ctx, expr->type, &ctx->last_expr_was_discarded));
      ctx->last_expr = *expr;
    } else {
      WasmExprNode* top = &ctx->expr_stack.data[ctx->expr_stack.size - 1];
      assert(top->index < top->total);

      LOGF("%3" PRIzd "(%d): reduce: %s(%d/%d) <- %s:%s\n",
              ctx->expr_stack.size, ctx->value_stack_size,
              s_opcode_name[top->expr.opcode], top->index, top->total,
              s_opcode_name[expr->opcode], s_type_names[expr->type]);
#if LOG
      if (top->expr.opcode == WASM_OPCODE_BR) {
        LOGF("      : br depth %u\n", top->expr.br.depth);
      }
#endif

      uint32_t cur_index = top->index++;
      WasmBool is_expr_done = top->index == top->total;
      int32_t result_count = 0;

      switch (top->expr.opcode) {
        /* handles all unary and binary operators */
        default:
          if (is_expr_done) {
            CHECK_RESULT(emit_opcode(ctx, top->expr.opcode));
            result_count = 1;
          } else {
            WasmType expected_type;
            if (cur_index == 0) {
              expected_type = s_opcode_type1[top->expr.opcode];
            } else if (cur_index == 1) {
              expected_type = s_opcode_type2[top->expr.opcode];
            } else {
              assert(0);
              break;
            }
            /* TODO use opcode name here */
            CHECK_RESULT(check_type(ctx, expected_type, expr->type, ""));
          }
          break;

        case WASM_OPCODE_BLOCK:
          if (is_expr_done) {
            WasmDepthNode* node = top_depth_node(ctx);
            unify_type(&top->expr.type, node->type);
            unify_type(&top->expr.type, expr->type);
            if (top->expr.type == WASM_TYPE_VOID)
              CHECK_RESULT(maybe_emit_discard(ctx, expr->type, NULL));
            CHECK_RESULT(fixup_top_depth(ctx, get_istream_offset(ctx)));
            pop_depth(ctx);
            result_count = get_value_count(top->expr.type);
          } else {
            CHECK_RESULT(maybe_emit_discard(ctx, expr->type, NULL));
          }
          break;

        case WASM_OPCODE_BR: {
          assert(cur_index == 0 && is_expr_done);
          uint32_t depth = top->expr.br.depth;
          WasmDepthNode* node = get_depth_node(ctx, depth);
          CHECK_RESULT(
              unify_and_check_type(ctx, &node->type, expr->type, " in br"));
          CHECK_RESULT(emit_br(ctx, depth, node));
          break;
        }

        case WASM_OPCODE_BR_IF: {
          uint32_t depth = top->expr.br.depth;
          WasmDepthNode* node = get_depth_node(ctx, depth);
          if (cur_index == 0) {
            CHECK_RESULT(unify_and_check_type(ctx, &node->type, expr->type,
                                              " in br_if"));
          } else {
            /* this actually flips the br_if so if <cond> is true it can
             * discard values from the stack, e.g.:
             *
             *   (br_if DEST <value> <cond>)
             *
             * becomes
             *
             *   <value>            value
             *   <cond>             value cond
             *   br_unless OVER     value
             *   discard_keep ...   value
             *   br DEST            value
             * OVER:
             *   discard
             *   ...
             */
            assert(cur_index == 1 && is_expr_done);
            CHECK_RESULT(
                check_type(ctx, WASM_TYPE_I32, expr->type, " in br_if"));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_UNLESS));
            uint32_t fixup_br_offset = get_istream_offset(ctx);
            CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
            /* adjust stack to account for br_unless consuming <cond> */
            adjust_value_stack(ctx, -get_value_count(expr->type));
            CHECK_RESULT(emit_br(ctx, depth, node));
            CHECK_RESULT(
                emit_i32_at(ctx, fixup_br_offset, get_istream_offset(ctx)));
            /* discard the value if the branch wasn't taken */
            CHECK_RESULT(maybe_emit_discard(ctx, node->type, NULL));
          }
          break;
        }

        case WASM_OPCODE_BR_TABLE: {
          assert(cur_index == 0 && is_expr_done);
          CHECK_RESULT(
              check_type(ctx, WASM_TYPE_I32, expr->type, " in br_table"));
          CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_TABLE));
          CHECK_RESULT(emit_i32(ctx, top->expr.br_table.num_targets));
          CHECK_RESULT(emit_i32(ctx, top->expr.br_table.table_offset));
          break;
        }

        case WASM_OPCODE_CALL_FUNCTION: {
          WasmInterpreterFunc* func = get_func(ctx, top->expr.call.func_index);
          WasmInterpreterFuncSignature* sig = get_func_signature(ctx, func);
          CHECK_RESULT(check_type(ctx, sig->param_types.data[cur_index],
                                  expr->type, " in call"));
          if (is_expr_done) {
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_FUNCTION));
            CHECK_RESULT(
                emit_func_offset(ctx, func, top->expr.call.func_index));
            result_count = get_value_count(sig->result_type);
          }
          break;
        }

        case WASM_OPCODE_CALL_IMPORT: {
          WasmInterpreterImport* import =
              get_import(ctx, top->expr.call_import.import_index);
          WasmInterpreterFuncSignature* sig =
              get_signature(ctx, import->sig_index);
          CHECK_RESULT(check_type(ctx, sig->param_types.data[cur_index],
                                  expr->type, " in call_import"));
          if (is_expr_done) {
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_IMPORT));
            CHECK_RESULT(emit_i32(ctx, top->expr.call_import.import_index));
            result_count = get_value_count(sig->result_type);
          }
          break;
        }

        case WASM_OPCODE_CALL_INDIRECT: {
          WasmInterpreterFuncSignature* sig =
              get_signature(ctx, top->expr.call_indirect.sig_index);
          if (cur_index == 0) {
            CHECK_RESULT(check_type(ctx, WASM_TYPE_I32, expr->type,
                                    " in call_indirect"));
          } else {
            CHECK_RESULT(check_type(ctx, sig->param_types.data[cur_index - 1],
                                    expr->type, " in call_indirect"));
          }
          if (is_expr_done) {
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_INDIRECT));
            CHECK_RESULT(emit_i32(ctx, top->expr.call_indirect.sig_index));
            /* the callee cleans up the params for us, but we have to clean up
             * the function table index */
            result_count = get_value_count(sig->result_type);
            CHECK_RESULT(emit_discard_keep(ctx, 1, result_count));
          }
          break;
        }

        case WASM_OPCODE_GROW_MEMORY:
          assert(cur_index == 0 && is_expr_done);
          CHECK_RESULT(
              check_type(ctx, WASM_TYPE_I32, expr->type, " in grow_memory"));
          CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_GROW_MEMORY));
          result_count = 1;
          break;

        case WASM_OPCODE_IF:
          if (cur_index == 0) {
            /* after cond */
            CHECK_RESULT(check_type(ctx, WASM_TYPE_I32, expr->type, " in if"));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_UNLESS));
            adjust_value_stack(ctx, -get_value_count(expr->type));
            top->expr.if_.fixup_offset = get_istream_offset(ctx);
            CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
          } else {
            /* after true */
            assert(cur_index == 1 && is_expr_done);
            /* discard the last value, if there is one; if is always void */
            CHECK_RESULT(maybe_emit_discard(ctx, expr->type, NULL));
            CHECK_RESULT(emit_i32_at(ctx, top->expr.if_.fixup_offset,
                                     get_istream_offset(ctx)));
            result_count = get_value_count(top->expr.type);
          }
          break;

        case WASM_OPCODE_IF_ELSE: {
          if (cur_index == 0) {
            /* after cond */
            CHECK_RESULT(
                check_type(ctx, WASM_TYPE_I32, expr->type, " in if_else"));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_UNLESS));
            adjust_value_stack(ctx, -get_value_count(expr->type));
            top->expr.if_else.fixup_cond_offset = get_istream_offset(ctx);
            top->expr.if_else.value_stack_size = ctx->value_stack_size;
            CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
          } else {
            WasmType prev_type = top->expr.type;
            CHECK_RESULT(unify_and_check_type(ctx, &top->expr.type, expr->type,
                                              " in if_else"));
            if (cur_index == 1) {
              /* after true */

              /* this NOP may or may not become a discard, depending on whether
               * the final type of this if_else has a value. */
              top->expr.if_else.fixup_nop_offset = get_istream_offset(ctx);
              CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_NOP));
              CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR));
              top->expr.if_else.fixup_true_offset = get_istream_offset(ctx);
              CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
              CHECK_RESULT(emit_i32_at(ctx, top->expr.if_else.fixup_cond_offset,
                                       get_istream_offset(ctx)));
              /* reset the value stack for the other branch arm */
              ctx->value_stack_size = top->expr.if_else.value_stack_size;
            } else {
              /* after false */
              assert(cur_index == 2 && is_expr_done);

              if (top->expr.type == WASM_TYPE_VOID) {
                CHECK_RESULT(maybe_emit_discard(ctx, expr->type, NULL));
                /* rewrite the nop above into a discard as well, if the
                 * previous type (i.e. the type returned by the true branch)
                 * had a value */
                if (get_value_count(prev_type)) {
                  emit_opcode_at(ctx, top->expr.if_else.fixup_nop_offset,
                                 WASM_OPCODE_DISCARD);
                }
              }

              CHECK_RESULT(emit_i32_at(ctx, top->expr.if_else.fixup_true_offset,
                                       get_istream_offset(ctx)));
              result_count = get_value_count(top->expr.type);
            }
          }
          break;
        }

        case WASM_OPCODE_I32_LOAD8_S:
        case WASM_OPCODE_I32_LOAD8_U:
        case WASM_OPCODE_I32_LOAD16_S:
        case WASM_OPCODE_I32_LOAD16_U:
        case WASM_OPCODE_I64_LOAD8_S:
        case WASM_OPCODE_I64_LOAD8_U:
        case WASM_OPCODE_I64_LOAD16_S:
        case WASM_OPCODE_I64_LOAD16_U:
        case WASM_OPCODE_I64_LOAD32_S:
        case WASM_OPCODE_I64_LOAD32_U:
        case WASM_OPCODE_I32_LOAD:
        case WASM_OPCODE_I64_LOAD:
        case WASM_OPCODE_F32_LOAD:
        case WASM_OPCODE_F64_LOAD:
          assert(cur_index == 0 && is_expr_done);
          /* TODO use opcode name here */
          CHECK_RESULT(check_type(ctx, WASM_TYPE_I32, expr->type, " in load"));
          CHECK_RESULT(emit_opcode(ctx, top->expr.opcode));
          CHECK_RESULT(emit_i32(ctx, top->expr.load.mem_offset));
          result_count = 1;
          break;

        case WASM_OPCODE_LOOP: {
          if (is_expr_done) {
            WasmDepthNode* node = top_minus_nth_depth_node(ctx, 2);
            unify_type(&top->expr.type, node->type);
            unify_type(&top->expr.type, expr->type);
          }
          if (top->expr.type == WASM_TYPE_VOID || !is_expr_done)
            CHECK_RESULT(maybe_emit_discard(ctx, expr->type, NULL));
          if (is_expr_done) {
            pop_depth(ctx); /* continue */
            CHECK_RESULT(fixup_top_depth(ctx, get_istream_offset(ctx)));
            pop_depth(ctx); /* exit */
            result_count = get_value_count(top->expr.type);
          }
          break;
        }

        case WASM_OPCODE_RETURN: {
          WasmInterpreterFuncSignature* sig =
              get_func_signature(ctx, ctx->current_func);
          CHECK_RESULT(
              check_type(ctx, sig->result_type, expr->type, " in return"));
          CHECK_RESULT(emit_return(ctx, sig->result_type));
          break;
        }

        case WASM_OPCODE_SELECT: {
          if (is_expr_done) {
            assert(cur_index == 2);
            CHECK_RESULT(
                check_type(ctx, WASM_TYPE_I32, expr->type, " in select"));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_SELECT));
            result_count = 1;
          } else {
            assert(cur_index < 2);
            CHECK_RESULT(unify_and_check_type(ctx, &top->expr.type, expr->type,
                                              " in select"));
          }
          break;
        }

        case WASM_OPCODE_SET_LOCAL: {
          assert(cur_index == 0 && is_expr_done);
          CHECK_RESULT(
              check_type(ctx, top->expr.type, expr->type, " in set_local"));
          CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_SET_LOCAL));
          uint32_t local_index =
              translate_local_index(ctx, top->expr.set_local.local_index);
          CHECK_RESULT(emit_i32(ctx, local_index));
          result_count = 1;
          break;
        }

        case WASM_OPCODE_I32_STORE8:
        case WASM_OPCODE_I32_STORE16:
        case WASM_OPCODE_I64_STORE8:
        case WASM_OPCODE_I64_STORE16:
        case WASM_OPCODE_I64_STORE32:
        case WASM_OPCODE_I32_STORE:
        case WASM_OPCODE_I64_STORE:
        case WASM_OPCODE_F32_STORE:
        case WASM_OPCODE_F64_STORE:
          if (cur_index == 0) {
            /* TODO use opcode name here */
            CHECK_RESULT(
                check_type(ctx, WASM_TYPE_I32, expr->type, " in store"));
          } else {
            assert(cur_index == 1 && is_expr_done);
            CHECK_RESULT(
                check_type(ctx, top->expr.type, expr->type, " in store"));
            CHECK_RESULT(emit_opcode(ctx, top->expr.opcode));
            CHECK_RESULT(emit_i32(ctx, top->expr.store.mem_offset));
            result_count = 1;
          }
          break;

        case WASM_OPCODE_F32_CONST:
        case WASM_OPCODE_F64_CONST:
        case WASM_OPCODE_GET_LOCAL:
        case WASM_OPCODE_I32_CONST:
        case WASM_OPCODE_I64_CONST:
        case WASM_OPCODE_MEMORY_SIZE:
        case WASM_OPCODE_NOP:
        case WASM_OPCODE_UNREACHABLE:
          assert(0);
          break;
      }

      if (is_expr_done) {
        /* "recurse" and reduce the current expr */
        ctx->value_stack_size = top->value_stack_size + result_count;
        expr = &top->expr;
        ctx->expr_stack.size--;
        done = WASM_FALSE;
      }
    }
  }

  return WASM_OK;
}

static WasmResult shift(WasmContext* ctx,
                        WasmInterpreterExpr* expr,
                        uint32_t count) {
  assert(count > 0);
  LOGF("%3" PRIzd "(%d): shift: %s:%s %u\n", ctx->expr_stack.size,
       ctx->value_stack_size, s_opcode_name[expr->opcode],
       s_type_names[expr->type], count);
  WasmExprNode* node = wasm_append_expr_node(ctx->allocator, &ctx->expr_stack);
  CHECK_ALLOC_NULL(ctx, node);
  node->expr = *expr;
  node->index = 0;
  node->total = count;
  node->value_stack_size = ctx->value_stack_size;
  return WASM_OK;
}

static WasmResult on_unary_expr(WasmOpcode opcode, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = s_opcode_rtype[opcode];
  expr.opcode = opcode;
  return shift(ctx, &expr, 1);
}

static WasmResult on_binary_expr(WasmOpcode opcode, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = s_opcode_rtype[opcode];
  expr.opcode = opcode;
  return shift(ctx, &expr, 2);
}

static WasmResult on_block_expr(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = count ? WASM_TYPE_ANY : WASM_TYPE_VOID;
  expr.opcode = WASM_OPCODE_BLOCK;
  expr.block.value_stack_size = ctx->value_stack_size;
  if (count > 0) {
    CHECK_RESULT(push_depth(ctx, expr.type));
    return shift(ctx, &expr, count);
  } else {
    return reduce(ctx, &expr);
  }
}

static WasmResult on_br_expr(uint32_t depth, void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_DEPTH(ctx, depth);
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_ANY;
  expr.opcode = WASM_OPCODE_BR;
  expr.br.depth = translate_depth(ctx, depth);
  return shift(ctx, &expr, 1);
}

static WasmResult on_br_if_expr(uint32_t depth, void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_DEPTH(ctx, depth);
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_VOID;
  expr.opcode = WASM_OPCODE_BR_IF;
  expr.br.depth = translate_depth(ctx, depth);
  return shift(ctx, &expr, 2);
}

static WasmResult on_br_table_expr(uint32_t num_targets,
                                   uint32_t* target_depths,
                                   uint32_t default_target_depth,
                                   void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_ANY;
  expr.opcode = WASM_OPCODE_BR_TABLE;
  expr.br_table.num_targets = num_targets;

  /* we need to parse the "key" expression before we can execute the br_table.
   * Rather than store the target_depths in an Expr, we just write them out
   * into the instruction stream and just jump over it. */
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR));
  uint32_t fixup_br_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));

  /* not necessary for the interpreter, but it makes it easier to disassemble.
   * This opcode specifies how many bytes of data follow. */
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DATA));
  CHECK_RESULT(emit_i32(ctx, (num_targets + 1) * sizeof(uint32_t) * 2));

  /* write the branch table as (offset, discard count) pairs */
  expr.br_table.table_offset = get_istream_offset(ctx);

  uint32_t i;
  for (i = 0; i <= num_targets; ++i) {
    uint32_t depth = i != num_targets ? target_depths[i] : default_target_depth;
    uint32_t translated_depth = translate_depth(ctx, depth);
    WasmDepthNode* node = get_depth_node(ctx, translated_depth);
    CHECK_RESULT(
        unify_and_check_type(ctx, &node->type, WASM_TYPE_VOID, " in br_table"));
    CHECK_RESULT(emit_br_table_offset(ctx, translated_depth, node));
  }

  CHECK_RESULT(emit_i32_at(ctx, fixup_br_offset, get_istream_offset(ctx)));
  return shift(ctx, &expr, 1);
}

static WasmResult on_call_expr(uint32_t func_index, void* user_data) {
  WasmContext* ctx = user_data;
  assert(func_index < ctx->funcs.size);
  WasmInterpreterFunc* func = get_func(ctx, func_index);
  WasmInterpreterFuncSignature* sig = get_func_signature(ctx, func);
  WasmInterpreterExpr expr;
  expr.type = sig->result_type;
  expr.opcode = WASM_OPCODE_CALL_FUNCTION;
  expr.call.func_index = func_index;
  if (sig->param_types.size > 0) {
    return shift(ctx, &expr, sig->param_types.size);
  } else {
    CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_FUNCTION));
    CHECK_RESULT(emit_func_offset(ctx, func, func_index));
    adjust_value_stack(ctx, get_value_count(sig->result_type));
    return reduce(ctx, &expr);
  }
}

static WasmResult on_call_import_expr(uint32_t import_index, void* user_data) {
  WasmContext* ctx = user_data;
  assert(import_index < ctx->module->imports.size);
  WasmInterpreterImport* import = get_import(ctx, import_index);
  WasmInterpreterFuncSignature* sig = get_signature(ctx, import->sig_index);
  WasmInterpreterExpr expr;
  expr.type = sig->result_type;
  expr.opcode = WASM_OPCODE_CALL_IMPORT;
  expr.call_import.import_index = import_index;
  if (sig->param_types.size > 0) {
    return shift(ctx, &expr, sig->param_types.size);
  } else {
    CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_IMPORT));
    CHECK_RESULT(emit_i32(ctx, import_index));
    adjust_value_stack(ctx, get_value_count(sig->result_type));
    return reduce(ctx, &expr);
  }
}

static WasmResult on_call_indirect_expr(uint32_t sig_index, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterFuncSignature* sig = get_signature(ctx, sig_index);
  WasmInterpreterExpr expr;
  expr.type = sig->result_type;
  expr.opcode = WASM_OPCODE_CALL_INDIRECT;
  expr.call_indirect.sig_index = sig_index;
  return shift(ctx, &expr, sig->param_types.size + 1);
}

static WasmResult on_i32_const_expr(uint32_t value, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_I32;
  expr.opcode = WASM_OPCODE_I32_CONST;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I32_CONST));
  CHECK_RESULT(emit_i32(ctx, value));
  adjust_value_stack(ctx, 1);
  return reduce(ctx, &expr);
}

static WasmResult on_i64_const_expr(uint64_t value, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_I64;
  expr.opcode = WASM_OPCODE_I64_CONST;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I64_CONST));
  CHECK_RESULT(emit_i64(ctx, value));
  adjust_value_stack(ctx, 1);
  return reduce(ctx, &expr);
}

static WasmResult on_f32_const_expr(uint32_t value_bits, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_F32;
  expr.opcode = WASM_OPCODE_F32_CONST;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_F32_CONST));
  CHECK_RESULT(emit_i32(ctx, value_bits));
  adjust_value_stack(ctx, 1);
  return reduce(ctx, &expr);
}

static WasmResult on_f64_const_expr(uint64_t value_bits, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_F64;
  expr.opcode = WASM_OPCODE_F64_CONST;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_F64_CONST));
  CHECK_RESULT(emit_i64(ctx, value_bits));
  adjust_value_stack(ctx, 1);
  return reduce(ctx, &expr);
}

static WasmResult on_get_local_expr(uint32_t local_index, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = get_local_index_type(ctx->current_func, local_index);
  expr.opcode = WASM_OPCODE_GET_LOCAL;
  expr.get_local.local_index = translate_local_index(ctx, local_index);
  CHECK_LOCAL(ctx, local_index);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_GET_LOCAL));
  CHECK_RESULT(emit_i32(ctx, expr.get_local.local_index));
  adjust_value_stack(ctx, 1);
  return reduce(ctx, &expr);
}

static WasmResult on_grow_memory_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_I32;
  expr.opcode = WASM_OPCODE_GROW_MEMORY;
  return shift(ctx, &expr, 1);
}

static WasmResult on_if_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_VOID;
  expr.opcode = WASM_OPCODE_IF;
  return shift(ctx, &expr, 2);
}

static WasmResult on_if_else_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_ANY;
  expr.opcode = WASM_OPCODE_IF_ELSE;
  return shift(ctx, &expr, 3);
}

static WasmResult on_load_expr(WasmOpcode opcode,
                               uint32_t alignment_log2,
                               uint32_t offset,
                               void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = s_opcode_rtype[opcode];
  expr.opcode = opcode;
  expr.load.mem_offset = offset;
  expr.load.alignment_log2 = alignment_log2;
  return shift(ctx, &expr, 1);
}

static WasmResult on_loop_expr(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = count ? WASM_TYPE_ANY : WASM_TYPE_VOID;
  expr.opcode = WASM_OPCODE_LOOP;
  expr.loop.value_stack_size = ctx->value_stack_size;
  if (count > 0) {
    CHECK_RESULT(push_depth(ctx, expr.type)); /* exit */
    CHECK_RESULT(push_depth_with_offset(
        ctx, WASM_TYPE_VOID, get_istream_offset(ctx))); /* continue */
    return shift(ctx, &expr, count);
  } else {
    return reduce(ctx, &expr);
  }
}

static WasmResult on_memory_size_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_I32;
  expr.opcode = WASM_OPCODE_MEMORY_SIZE;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_MEMORY_SIZE));
  adjust_value_stack(ctx, 1);
  return reduce(ctx, &expr);
}

static WasmResult on_nop_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_VOID;
  expr.opcode = WASM_OPCODE_NOP;
  return reduce(ctx, &expr);
}

static WasmResult on_return_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterFuncSignature* sig =
      get_func_signature(ctx, ctx->current_func);
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_ANY;
  expr.opcode = WASM_OPCODE_RETURN;
  uint32_t result_count = get_value_count(sig->result_type);
  if (result_count > 0) {
    return shift(ctx, &expr, result_count);
  } else {
    CHECK_RESULT(emit_return(ctx, sig->result_type));
    return reduce(ctx, &expr);
  }
}

static WasmResult on_select_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_ANY;
  expr.opcode = WASM_OPCODE_SELECT;
  return shift(ctx, &expr, 3);
}

static WasmResult on_set_local_expr(uint32_t local_index, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = get_local_index_type(ctx->current_func, local_index);
  expr.opcode = WASM_OPCODE_SET_LOCAL;
  expr.set_local.local_index = local_index;
  CHECK_LOCAL(ctx, local_index);
  return shift(ctx, &expr, 1);
}

static WasmResult on_store_expr(WasmOpcode opcode,
                                uint32_t alignment_log2,
                                uint32_t offset,
                                void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = s_opcode_rtype[opcode];
  expr.opcode = opcode;
  expr.store.mem_offset = offset;
  expr.store.alignment_log2 = alignment_log2;
  return shift(ctx, &expr, 2);
}

static WasmResult on_unreachable_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_ANY;
  expr.opcode = WASM_OPCODE_UNREACHABLE;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_UNREACHABLE));
  return reduce(ctx, &expr);
}

static WasmResult on_function_table_count(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_ALLOC(ctx, wasm_new_interpreter_func_table_entry_array(
                       ctx->allocator, &ctx->module->func_table, count));
  return WASM_OK;
}

static WasmResult on_function_table_entry(uint32_t index,
                                          uint32_t func_index,
                                          void* user_data) {
  WasmContext* ctx = user_data;
  assert(index < ctx->module->func_table.size);
  WasmInterpreterFuncTableEntry* entry = &ctx->module->func_table.data[index];
  WasmInterpreterFunc* func = get_func(ctx, func_index);
  entry->sig_index = func->sig_index;
  /* the function offset isn't known yet, so temporarily store the func index
   * in func_offset and resolve after the last function body */
  entry->func_offset = func_index;
  return WASM_OK;
}

static WasmResult on_start_function(uint32_t func_index, void* user_data) {
  WasmContext* ctx = user_data;
  /* can't get the function offset yet, because we haven't parsed the
   * functions. Just store the function index and resolve it later in
   * end_function_bodies_section. */
  assert(func_index < ctx->funcs.size);
  ctx->start_func_index = func_index;
  return WASM_OK;
}

static WasmResult on_export_count(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_ALLOC(ctx, wasm_new_interpreter_export_array(
                       ctx->allocator, &ctx->module->exports, count));
  return WASM_OK;
}

static WasmResult on_export(uint32_t index,
                            uint32_t func_index,
                            WasmStringSlice name,
                            void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExport* export = &ctx->module->exports.data[index];
  WasmInterpreterFunc* func = get_func(ctx, func_index);
  CHECK_ALLOC_NULL_STR(
      ctx, export->name = wasm_dup_string_slice(ctx->allocator, name));
  export->func_index = func_index;
  export->sig_index = func->sig_index;
  export->func_offset = WASM_INVALID_OFFSET;
  return WASM_OK;
}

static WasmResult end_function_bodies_section(void* user_data) {
  WasmContext* ctx = user_data;

  /* resolve the start function offset */
  if (ctx->start_func_index != INVALID_FUNC_INDEX) {
    WasmInterpreterFunc* func = get_func(ctx, ctx->start_func_index);
    assert(func->offset != WASM_INVALID_OFFSET);
    ctx->module->start_func_offset = func->offset;
  }

  /* resolve the export function offsets */
  uint32_t i;
  for (i = 0; i < ctx->module->exports.size; ++i) {
    WasmInterpreterExport* export = get_export(ctx, i);
    WasmInterpreterFunc* func = get_func(ctx, export->func_index);
    export->func_offset = func->offset;
  }

  /* resolve the function table entry offsets */
  for (i = 0; i < ctx->module->func_table.size; ++i) {
    WasmInterpreterFuncTableEntry* entry = &ctx->module->func_table.data[i];
    /* function index is stored in func_offset temporarily */
    WasmInterpreterFunc* func = get_func(ctx, entry->func_offset);
    entry->func_offset = func->offset;
  }

  return WASM_OK;
}

static WasmBinaryReader s_binary_reader = {
    .user_data = NULL,
    .on_error = &on_error,

    .on_memory_initial_size_pages = &on_memory_initial_size_pages,

    .on_data_segment = &on_data_segment,

    .on_signature_count = &on_signature_count,
    .on_signature = &on_signature,

    .on_import_count = &on_import_count,
    .on_import = &on_import,

    .on_function_signatures_count = &on_function_signatures_count,
    .on_function_signature = &on_function_signature,

    .on_function_bodies_count = &on_function_bodies_count,
    .begin_function_body = &begin_function_body,
    .on_local_decl_count = &on_local_decl_count,
    .on_local_decl = &on_local_decl,
    .on_binary_expr = &on_binary_expr,
    .on_block_expr = &on_block_expr,
    .on_br_expr = &on_br_expr,
    .on_br_if_expr = &on_br_if_expr,
    .on_br_table_expr = &on_br_table_expr,
    .on_call_expr = &on_call_expr,
    .on_call_import_expr = &on_call_import_expr,
    .on_call_indirect_expr = &on_call_indirect_expr,
    .on_compare_expr = &on_binary_expr,
    .on_i32_const_expr = &on_i32_const_expr,
    .on_i64_const_expr = &on_i64_const_expr,
    .on_f32_const_expr = &on_f32_const_expr,
    .on_f64_const_expr = &on_f64_const_expr,
    .on_convert_expr = &on_unary_expr,
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
    .end_function_bodies_section = &end_function_bodies_section,

    .on_function_table_count = &on_function_table_count,
    .on_function_table_entry = &on_function_table_entry,

    .on_start_function = &on_start_function,

    .on_export_count = &on_export_count,
    .on_export = &on_export,
};

static void wasm_destroy_interpreter_func(WasmAllocator* allocator,
                                          WasmInterpreterFunc* func) {
  wasm_destroy_type_vector(allocator, &func->param_and_local_types);
}

static void destroy_context(WasmContext* ctx) {
  wasm_destroy_expr_node_vector(ctx->allocator, &ctx->expr_stack);
  wasm_destroy_depth_node_vector(ctx->allocator, &ctx->depth_stack);
  WASM_DESTROY_ARRAY_AND_ELEMENTS(ctx->allocator, ctx->funcs, interpreter_func);
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
  WasmContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.error_handler = error_handler;
  ctx.memory_allocator = memory_allocator;
  ctx.module = out_module;
  ctx.start_func_index = INVALID_FUNC_INDEX;
  ctx.module->start_func_offset = WASM_INVALID_OFFSET;
  CHECK_RESULT(wasm_init_mem_writer(allocator, &ctx.istream_writer));

  WasmBinaryReader reader;
  WASM_ZERO_MEMORY(reader);
  reader = s_binary_reader;
  reader.user_data = &ctx;

  WasmResult result = wasm_read_binary(allocator, data, size, &reader, options);
  if (WASM_SUCCEEDED(result)) {
    wasm_steal_mem_writer_output_buffer(&ctx.istream_writer,
                                        &out_module->istream);
    out_module->istream.size = ctx.istream_offset;
  }
  destroy_context(&ctx);
  return result;
}

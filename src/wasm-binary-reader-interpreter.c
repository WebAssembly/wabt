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

#define INVALID_FUNC_INDEX ((uint32_t)~0)

#define CHECK_ALLOC_(ctx, cond)                                           \
  do {                                                                    \
    if (!(cond)) {                                                        \
      print_error((ctx), "%s:%d: allocation failed", __FILE__, __LINE__); \
      return WASM_ERROR;                                                  \
    }                                                                     \
  } while (0)

#define CHECK_ALLOC(ctx, e) CHECK_ALLOC_((ctx), (e) == WASM_OK)
#define CHECK_ALLOC_NULL(ctx, v) CHECK_ALLOC_((ctx), (v))
#define CHECK_ALLOC_NULL_STR(ctx, v) CHECK_ALLOC_((ctx), (v).start)

#define CHECK_RESULT(expr) \
  do {                     \
    if ((expr) != WASM_OK) \
      return WASM_ERROR;   \
  } while (0)

#define CHECK_DEPTH(ctx, depth)                                 \
  do {                                                          \
    if ((depth) >= (ctx)->depth_stack.size) {                   \
      print_error((ctx), "invalid depth: %d (max %d)", (depth), \
                  (int)((ctx)->depth_stack.size));              \
      return WASM_ERROR;                                        \
    }                                                           \
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
  WASM_FOREACH_OPCODE(V)
  [WASM_OPCODE_ALLOCA] = "alloca",
  [WASM_OPCODE_DISCARD] = "discard",
  [WASM_OPCODE_DISCARD_KEEP] = "discard_keep",
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
      uint32_t fixup_cond_offset, fixup_true_offset, value_stack_size;
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

typedef struct WasmReadInterpreterContext {
  WasmAllocator* allocator;
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
  int last_expr_was_discarded;
} WasmReadInterpreterContext;

static WasmDepthNode* get_depth_node(WasmReadInterpreterContext* ctx,
                                     uint32_t depth) {
  assert(depth < ctx->depth_stack.size);
  return &ctx->depth_stack.data[depth];
}

static WasmDepthNode* top_minus_nth_depth_node(WasmReadInterpreterContext* ctx,
                                               uint32_t n) {
  return get_depth_node(ctx, ctx->depth_stack.size - n);
}

static WasmDepthNode* top_depth_node(WasmReadInterpreterContext* ctx) {
  return top_minus_nth_depth_node(ctx, 1);
}

static uint32_t get_istream_offset(WasmReadInterpreterContext* ctx) {
  return ctx->istream_offset;
}

static uint32_t get_result_count(WasmType result_type) {
  return (result_type == WASM_TYPE_VOID || result_type == WASM_TYPE_ANY) ? 0
                                                                         : 1;
}

static void on_error(uint32_t offset, const char* message, void* user_data);

static void print_error(WasmReadInterpreterContext* ctx,
                        const char* format,
                        ...) {
  va_list args;
  va_list args_copy;
  va_start(args, format);
  va_copy(args_copy, args);

  char buffer[128];
  int len = wasm_vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  if (len + 1 > sizeof(buffer)) {
    char* buffer2 = alloca(len + 1);
    len = wasm_vsnprintf(buffer2, len + 1, format, args_copy);
    va_end(args_copy);
  }

  on_error(WASM_INVALID_OFFSET, buffer, ctx);
}

static void adjust_value_stack(WasmReadInterpreterContext* ctx,
                               int32_t amount) {
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

static void reset_value_stack(WasmReadInterpreterContext* ctx,
                              WasmInterpreterExpr* expr) {
  ctx->value_stack_size =
      expr->block.value_stack_size + get_result_count(expr->type);
}

static WasmResult type_mismatch(WasmReadInterpreterContext* ctx,
                                WasmType expected_type,
                                WasmType type,
                                const char* desc) {
  print_error(ctx, "type mismatch%s, expected %s but got %s.", desc,
              s_type_names[expected_type], s_type_names[type]);
  return WASM_ERROR;
}

static WasmResult check_type_exact(WasmReadInterpreterContext* ctx,
                                   WasmType expected_type,
                                   WasmType type,
                                   const char* desc) {
  if (expected_type == type)
    return WASM_OK;
  return type_mismatch(ctx, expected_type, type, desc);
}

static WasmResult check_type(WasmReadInterpreterContext* ctx,
                             WasmType expected_type,
                             WasmType type,
                             const char* desc) {
  if (expected_type == WASM_TYPE_ANY || type == WASM_TYPE_ANY ||
      expected_type == WASM_TYPE_VOID) {
    return WASM_OK;
  }
  return check_type_exact(ctx, expected_type, type, desc);
}

static void unify_type(WasmType* dest_type, WasmType type) {
  if (*dest_type == WASM_TYPE_ANY)
    *dest_type = type;
  else if (type != WASM_TYPE_ANY && *dest_type != type)
    *dest_type = WASM_TYPE_VOID;
}

static WasmResult unify_and_check_type(WasmReadInterpreterContext* ctx,
                                       WasmType* dest_type,
                                       WasmType type,
                                       const char* desc) {
  unify_type(dest_type, type);
  return check_type(ctx, *dest_type, type, desc);
}

static WasmResult unify_and_check_type_exact(WasmReadInterpreterContext* ctx,
                                             WasmType* dest_type,
                                             WasmType type,
                                             const char* desc) {
  unify_type(dest_type, type);
  return check_type_exact(ctx, *dest_type, type, desc);
}

static WasmResult emit_data_at(WasmReadInterpreterContext* ctx,
                               size_t offset,
                               const void* data,
                               size_t size) {
  return ctx->istream_writer.base.write_data(
      offset, data, size, ctx->istream_writer.base.user_data);
}

static WasmResult emit_data(WasmReadInterpreterContext* ctx,
                            const void* data,
                            size_t size) {
  CHECK_RESULT(emit_data_at(ctx, ctx->istream_offset, data, size));
  ctx->istream_offset += size;
  return WASM_OK;
}

static WasmResult emit_opcode(WasmReadInterpreterContext* ctx,
                              WasmOpcode opcode,
                              int32_t stack_adjustment) {
  CHECK_RESULT(emit_data(ctx, &opcode, sizeof(uint8_t)));
  adjust_value_stack(ctx, stack_adjustment);
  return WASM_OK;
}

static WasmResult emit_i8(WasmReadInterpreterContext* ctx, uint8_t value) {
  return emit_data(ctx, &value, sizeof(value));
}

static WasmResult emit_i32(WasmReadInterpreterContext* ctx, uint32_t value) {
  return emit_data(ctx, &value, sizeof(value));
}

static WasmResult emit_i64(WasmReadInterpreterContext* ctx, uint64_t value) {
  return emit_data(ctx, &value, sizeof(value));
}

static WasmResult emit_i32_at(WasmReadInterpreterContext* ctx,
                              uint32_t offset,
                              uint32_t value) {
  return emit_data_at(ctx, offset, &value, sizeof(value));
}

static void unemit_discard(WasmReadInterpreterContext* ctx) {
  assert(ctx->istream_offset > 0);
  assert(ctx->istream_offset <= ctx->istream_writer.buf.size);
  assert(((uint8_t*)ctx->istream_writer.buf.start)[ctx->istream_offset - 1] ==
         WASM_OPCODE_DISCARD);
  ctx->istream_offset--;
  ctx->value_stack_size++;
}

static WasmResult emit_discard(WasmReadInterpreterContext* ctx) {
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DISCARD, -1));
  return WASM_OK;
}

static WasmResult maybe_emit_discard(WasmReadInterpreterContext* ctx,
                                     WasmType type,
                                     int* out_discarded) {
  int should_discard = type != WASM_TYPE_VOID && type != WASM_TYPE_ANY;
  if (out_discarded)
    *out_discarded = should_discard;
  if (should_discard)
    return emit_discard(ctx);
  return WASM_OK;
}

static WasmResult emit_discard_keep(WasmReadInterpreterContext* ctx,
                                    uint32_t discard,
                                    uint8_t keep) {
  assert(discard != UINT32_MAX);
  assert(keep <= 1);
  if (discard > 0) {
    if (discard == 1 && keep == 0) {
      CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DISCARD, 0));
    } else {
      CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DISCARD_KEEP, 0));
      CHECK_RESULT(emit_i32(ctx, discard));
      CHECK_RESULT(emit_i8(ctx, keep));
    }
  }
  return WASM_OK;
}

static WasmResult emit_return(WasmReadInterpreterContext* ctx,
                              WasmType result_type) {
  uint32_t keep_count = get_result_count(result_type);
  uint32_t discard_count = ctx->value_stack_size - keep_count;
  CHECK_RESULT(emit_discard_keep(ctx, discard_count, keep_count));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_RETURN, 0));
  return WASM_OK;
}

static WasmResult append_fixup(WasmReadInterpreterContext* ctx,
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

static WasmResult emit_br_offset(WasmReadInterpreterContext* ctx,
                                 uint32_t depth,
                                 uint32_t offset) {
  if (offset == WASM_INVALID_OFFSET)
    CHECK_RESULT(append_fixup(ctx, &ctx->depth_fixups, depth));
  CHECK_RESULT(emit_i32(ctx, offset));
  return WASM_OK;
}

static WasmResult emit_br(WasmReadInterpreterContext* ctx,
                          uint32_t depth,
                          WasmDepthNode* node) {
  WasmType expected_type = node->type;
  assert(ctx->value_stack_size >= node->value_stack_size);
  uint32_t discard_count = ctx->value_stack_size - node->value_stack_size;
  uint8_t keep_count = get_result_count(expected_type);
  CHECK_RESULT(emit_discard_keep(ctx, discard_count, keep_count));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR, 0));
  CHECK_RESULT(emit_br_offset(ctx, depth, node->offset));
  return WASM_OK;
}

static WasmResult emit_br_table_offset(WasmReadInterpreterContext* ctx,
                                       uint32_t depth,
                                       WasmDepthNode* node,
                                       uint32_t discard_count) {
  discard_count = ctx->value_stack_size - node->value_stack_size;
  CHECK_RESULT(emit_br_offset(ctx, depth, node->offset));
  CHECK_RESULT(emit_i32(ctx, discard_count));
  return WASM_OK;
}

static WasmResult emit_func_offset(WasmReadInterpreterContext* ctx,
                                   WasmInterpreterFunc* func,
                                   uint32_t func_index) {
  if (func->offset == WASM_INVALID_OFFSET)
    CHECK_RESULT(append_fixup(ctx, &ctx->func_fixups, func_index));
  CHECK_RESULT(emit_i32(ctx, func->offset));
  return WASM_OK;
}

static WasmInterpreterFunc* get_func(WasmReadInterpreterContext* ctx,
                                     uint32_t func_index) {
  assert(func_index < ctx->funcs.size);
  return &ctx->funcs.data[func_index];
}

static WasmInterpreterImport* get_import(WasmReadInterpreterContext* ctx,
                                         uint32_t import_index) {
  assert(import_index < ctx->module->imports.size);
  return &ctx->module->imports.data[import_index];
}

static WasmInterpreterExport* get_export(WasmReadInterpreterContext* ctx,
                                         uint32_t export_index) {
  assert(export_index < ctx->module->exports.size);
  return &ctx->module->exports.data[export_index];
}

static WasmInterpreterFuncSignature* get_signature(
    WasmReadInterpreterContext* ctx,
    uint32_t sig_index) {
  assert(sig_index < ctx->module->sigs.size);
  return &ctx->module->sigs.data[sig_index];
}

static WasmInterpreterFuncSignature* get_func_signature(
    WasmReadInterpreterContext* ctx,
    WasmInterpreterFunc* func) {
  return get_signature(ctx, func->sig_index);
}

static WasmType get_local_index_type(WasmInterpreterFunc* func,
                                     uint32_t local_index) {
  assert(local_index < func->param_and_local_types.size);
  return func->param_and_local_types.data[local_index];
}

static WasmResult push_depth_with_offset(WasmReadInterpreterContext* ctx,
                                         WasmType type,
                                         uint32_t offset) {
  WasmDepthNode* node =
      wasm_append_depth_node(ctx->allocator, &ctx->depth_stack);
  CHECK_ALLOC_NULL(ctx, node);
  node->type = type;
  node->value_stack_size = ctx->value_stack_size;
  node->offset = offset;
#if LOG
  fprintf(stderr, "   (%d): push depth %" PRIzd ":%s\n", ctx->value_stack_size,
          ctx->depth_stack.size - 1, s_type_names[type]);
#endif
  return WASM_OK;
}

static WasmResult push_depth(WasmReadInterpreterContext* ctx, WasmType type) {
  return push_depth_with_offset(ctx, type, WASM_INVALID_OFFSET);
}

static void pop_depth(WasmReadInterpreterContext* ctx) {
#if LOG
  fprintf(stderr, "   (%d): pop depth %" PRIzd "\n", ctx->value_stack_size,
          ctx->depth_stack.size - 1);
#endif
  assert(ctx->depth_stack.size > 0);
  ctx->depth_stack.size--;
  /* reduce the depth_fixups stack as well, but it may be smaller than
   * depth_stack so only do it conditionally. */
  if (ctx->depth_fixups.size > ctx->depth_stack.size)
    ctx->depth_fixups.size = ctx->depth_stack.size;
}

static uint32_t translate_depth(WasmReadInterpreterContext* ctx,
                                uint32_t depth) {
  assert(depth < ctx->depth_stack.size);
  return ctx->depth_stack.size - 1 - depth;
}

static WasmResult fixup_top_depth(WasmReadInterpreterContext* ctx,
                                  uint32_t offset) {
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

static uint32_t translate_local_index(WasmReadInterpreterContext* ctx,
                                      uint32_t local_index) {
  assert(local_index < ctx->value_stack_size);
  return ctx->value_stack_size - local_index;
}

void on_error(uint32_t offset, const char* message, void* user_data) {
  if (offset == WASM_INVALID_OFFSET)
    fprintf(stderr, "error: %s\n", message);
  else
    fprintf(stderr, "error: @0x%08x: %s\n", offset, message);
}

static WasmResult on_memory_initial_size_pages(uint32_t pages,
                                               void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
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
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterMemory* memory = &ctx->module->memory;
  uint8_t* dst_data = memory->data;
  memcpy(&dst_data[address], src_data, size);
  return WASM_OK;
}

static WasmResult on_signature_count(uint32_t count, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  CHECK_ALLOC(ctx, wasm_new_interpreter_func_signature_array(
                       ctx->allocator, &ctx->module->sigs, count));
  return WASM_OK;
}

static WasmResult on_signature(uint32_t index,
                               WasmType result_type,
                               uint32_t param_count,
                               WasmType* param_types,
                               void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterFuncSignature* sig = get_signature(ctx, index);
  sig->result_type = result_type;

  CHECK_ALLOC(
      ctx, wasm_reserve_types(ctx->allocator, &sig->param_types, param_count));
  sig->param_types.size = param_count;
  memcpy(sig->param_types.data, param_types, param_count * sizeof(WasmType));
  return WASM_OK;
}

static WasmResult on_import_count(uint32_t count, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  CHECK_ALLOC(ctx, wasm_new_interpreter_import_array(
                       ctx->allocator, &ctx->module->imports, count));
  return WASM_OK;
}

static WasmResult on_import(uint32_t index,
                            uint32_t sig_index,
                            WasmStringSlice module_name,
                            WasmStringSlice function_name,
                            void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
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
  WasmReadInterpreterContext* ctx = user_data;
  CHECK_ALLOC(
      ctx, wasm_new_interpreter_func_array(ctx->allocator, &ctx->funcs, count));
  CHECK_ALLOC(ctx, wasm_resize_uint32_vector_vector(ctx->allocator,
                                                    &ctx->func_fixups, count));
  return WASM_OK;
}

static WasmResult on_function_signature(uint32_t index,
                                        uint32_t sig_index,
                                        void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  assert(sig_index < ctx->module->sigs.size);
  WasmInterpreterFunc* func = get_func(ctx, index);
  func->offset = WASM_INVALID_OFFSET;
  func->sig_index = sig_index;
  return WASM_OK;
}

static WasmResult on_function_bodies_count(uint32_t count, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  assert(count == ctx->funcs.size);
  WASM_USE(ctx);
  return WASM_OK;
}

static WasmResult begin_function_body(uint32_t index, void* user_data) {
#if LOG
  fprintf(stderr, "*** func %d ***\n", index);
#endif
  WasmReadInterpreterContext* ctx = user_data;
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
  WasmReadInterpreterContext* ctx = user_data;
  if (ctx->expr_stack.size != 0) {
    print_error(ctx, "expression stack not empty on function exit! %d items",
                (int)ctx->expr_stack.size);
    return WASM_ERROR;
  }
  WasmInterpreterFunc* func = ctx->current_func;
  WasmInterpreterFuncSignature* sig = get_signature(ctx, func->sig_index);
  if (ctx->last_expr.opcode != WASM_OPCODE_RETURN) {
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
  WasmReadInterpreterContext* ctx = user_data;
  ctx->current_func->local_decl_count = count;
  return WASM_OK;
}

static WasmResult on_local_decl(uint32_t decl_index,
                                uint32_t count,
                                WasmType type,
                                void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterFunc* func = ctx->current_func;
  func->local_count += count;

  uint32_t i;
  for (i = 0; i < count; ++i) {
    CHECK_RESULT(wasm_append_type_value(ctx->allocator,
                                        &func->param_and_local_types, &type));
  }

  if (decl_index == func->local_decl_count - 1) {
    /* last local declaration, allocate space for all locals. */
    CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_ALLOCA, func->local_count));
    CHECK_RESULT(emit_i32(ctx, func->local_count));
  }
  return WASM_OK;
}

static WasmResult reduce(WasmReadInterpreterContext* ctx,
                         WasmInterpreterExpr* expr) {
  int done = 0;
  while (!done) {
    done = 1;

    if (ctx->expr_stack.size == 0) {
#if LOG
      fprintf(stderr, "%3" PRIzd "(%d): reduce: <- %s:%s\n",
              ctx->expr_stack.size, ctx->value_stack_size,
              s_opcode_name[expr->opcode], s_type_names[expr->type]);
#endif

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

#if LOG
      fprintf(stderr, "%3" PRIzd "(%d): reduce: %s(%d/%d) <- %s:%s\n",
              ctx->expr_stack.size, ctx->value_stack_size,
              s_opcode_name[top->expr.opcode], top->index, top->total,
              s_opcode_name[expr->opcode], s_type_names[expr->type]);
#endif
#if LOG
      if (top->expr.opcode == WASM_OPCODE_BR) {
        fprintf(stderr, "      : br depth %u\n", top->expr.br.depth);
      }
#endif

      uint32_t cur_index = top->index++;
      int is_expr_done = top->index == top->total;

      switch (top->expr.opcode) {
        /* handles all unary and binary operators */
        default:
          if (is_expr_done) {
            CHECK_RESULT(emit_opcode(ctx, top->expr.opcode, 1 - top->total));
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
            CHECK_RESULT(check_type_exact(ctx, expected_type, expr->type, ""));
          }
          break;

        case WASM_OPCODE_BLOCK:
          if (is_expr_done) {
            WasmDepthNode* node = top_depth_node(ctx);
            unify_type(&top->expr.type, node->type);
            unify_type(&top->expr.type, expr->type);
          }
          if (top->expr.type == WASM_TYPE_VOID || !is_expr_done)
            CHECK_RESULT(maybe_emit_discard(ctx, expr->type, NULL));
          if (is_expr_done) {
            CHECK_RESULT(fixup_top_depth(ctx, get_istream_offset(ctx)));
            pop_depth(ctx);
            reset_value_stack(ctx, &top->expr);
          }
          break;

        case WASM_OPCODE_BR: {
          assert(cur_index == 0 && is_expr_done);
          uint32_t depth = top->expr.br.depth;
          WasmDepthNode* node = get_depth_node(ctx, depth);
          CHECK_RESULT(unify_and_check_type_exact(ctx, &node->type, expr->type,
                                                  " in br"));
          CHECK_RESULT(emit_br(ctx, depth, node));
          break;
        }

        case WASM_OPCODE_BR_IF: {
          uint32_t depth = top->expr.br.depth;
          WasmDepthNode* node = get_depth_node(ctx, depth);
          if (cur_index == 0) {
            CHECK_RESULT(unify_and_check_type_exact(ctx, &node->type,
                                                    expr->type, " in br_if"));
          } else {
            assert(cur_index == 1 && is_expr_done);
            CHECK_RESULT(
                check_type_exact(ctx, WASM_TYPE_I32, expr->type, " in br_if"));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I32_EQZ, 0));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_IF, -1));
            uint32_t fixup_br_offset = get_istream_offset(ctx);
            CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
            CHECK_RESULT(emit_br(ctx, depth, node));
            CHECK_RESULT(emit_i32_at(ctx, fixup_br_offset,
                                     get_istream_offset(ctx)));
          }
          break;
        }

        case WASM_OPCODE_BR_TABLE: {
          assert(cur_index == 0 && is_expr_done);
          CHECK_RESULT(
              check_type_exact(ctx, WASM_TYPE_I32, expr->type, " in br_table"));
          CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_TABLE, -1));
          CHECK_RESULT(emit_i32(ctx, top->expr.br_table.num_targets));
          CHECK_RESULT(emit_i32(ctx, top->expr.br_table.table_offset));
          break;
        }

        case WASM_OPCODE_CALL_FUNCTION: {
          WasmInterpreterFunc* func = get_func(ctx, top->expr.call.func_index);
          WasmInterpreterFuncSignature* sig = get_func_signature(ctx, func);
          CHECK_RESULT(check_type_exact(ctx, sig->param_types.data[cur_index],
                                        expr->type, " in call"));
          if (is_expr_done) {
            int32_t num_results = get_result_count(sig->result_type);
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_FUNCTION,
                                     num_results - top->total));
            CHECK_RESULT(
                emit_func_offset(ctx, func, top->expr.call.func_index));
          }
          break;
        }

        case WASM_OPCODE_CALL_IMPORT: {
          WasmInterpreterImport* import =
              get_import(ctx, top->expr.call_import.import_index);
          WasmInterpreterFuncSignature* sig =
              get_signature(ctx, import->sig_index);
          CHECK_RESULT(check_type_exact(ctx, sig->param_types.data[cur_index],
                                        expr->type, " in call_import"));
          if (is_expr_done) {
            int32_t num_results = get_result_count(sig->result_type);
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_IMPORT,
                                     num_results - top->total));
            CHECK_RESULT(emit_i32(ctx, top->expr.call_import.import_index));
          }
          break;
        }

        case WASM_OPCODE_CALL_INDIRECT: {
          WasmInterpreterFuncSignature* sig =
              get_signature(ctx, top->expr.call_indirect.sig_index);
          if (cur_index == 0) {
            CHECK_RESULT(check_type_exact(ctx, WASM_TYPE_I32, expr->type,
                                    " in call_indirect"));
          } else {
            CHECK_RESULT(check_type_exact(ctx,
                                          sig->param_types.data[cur_index - 1],
                                          expr->type, " in call_indirect"));
          }
          if (is_expr_done) {
            int32_t num_results = get_result_count(sig->result_type);
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_INDIRECT,
                                     num_results - top->total));
            CHECK_RESULT(emit_i32(ctx, top->expr.call_indirect.sig_index));
            /* the callee cleans up the params for us, but we have to clean up
             * the function table index */
            CHECK_RESULT(emit_discard_keep(ctx, 1, num_results));
          }
          break;
        }

        case WASM_OPCODE_GROW_MEMORY:
          assert(cur_index == 0 && is_expr_done);
          CHECK_RESULT(check_type_exact(ctx, WASM_TYPE_I32, expr->type,
                                        " in grow_memory"));
          CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_GROW_MEMORY, 0));
          break;

        case WASM_OPCODE_IF:
          if (cur_index == 0) {
            /* after cond */
            CHECK_RESULT(
                check_type_exact(ctx, WASM_TYPE_I32, expr->type, " in if"));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I32_EQZ, 0));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_IF, -1));
            top->expr.if_.fixup_offset = get_istream_offset(ctx);
            CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
          } else {
            /* after true */
            assert(cur_index == 1 && is_expr_done);
            /* discard the last value, if there is one; if is always void */
            CHECK_RESULT(maybe_emit_discard(ctx, expr->type, NULL));
            CHECK_RESULT(emit_i32_at(ctx, top->expr.if_.fixup_offset,
                                     get_istream_offset(ctx)));
          }
          break;

        case WASM_OPCODE_IF_ELSE: {
          if (cur_index == 0) {
            /* after cond */
            CHECK_RESULT(check_type_exact(ctx, WASM_TYPE_I32, expr->type,
                                          " in if_else"));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I32_EQZ, 0));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_IF, -1));
            top->expr.if_else.fixup_cond_offset = get_istream_offset(ctx);
            top->expr.if_else.value_stack_size = ctx->value_stack_size;
            CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
          } else {
            CHECK_RESULT(unify_and_check_type(ctx, &top->expr.type, expr->type,
                                              " in if_else"));
            if (top->expr.type == WASM_TYPE_VOID)
              CHECK_RESULT(maybe_emit_discard(ctx, expr->type, NULL));

            if (cur_index == 1) {
              /* after true */
              CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR, 0));
              top->expr.if_else.fixup_true_offset = get_istream_offset(ctx);
              CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
              CHECK_RESULT(emit_i32_at(ctx, top->expr.if_else.fixup_cond_offset,
                                       get_istream_offset(ctx)));
              /* reset the value stack for the other branch arm */
              ctx->value_stack_size = top->expr.if_else.value_stack_size;
            } else {
              /* after false */
              assert(cur_index == 2 && is_expr_done);
              CHECK_RESULT(emit_i32_at(ctx, top->expr.if_else.fixup_true_offset,
                                       get_istream_offset(ctx)));

              /* weird case: if the true branch's type is not VOID or ANY, and
               * the false branch's type is any, we need to adjust the stack up
               * to match the other branch.
               *
               * A reduced expression's type is only ANY if it requires
               * non-local control flow. In that case, we will not have
               * adjusted the stack for that value, but the only normal control
               * flow that will produce a value is from the true branch. */
              if (top->expr.type != WASM_TYPE_VOID &&
                  top->expr.type != WASM_TYPE_ANY &&
                  expr->type == WASM_TYPE_ANY) {
                adjust_value_stack(ctx, 1);
              }
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
          CHECK_RESULT(
              check_type_exact(ctx, WASM_TYPE_I32, expr->type, " in load"));
          CHECK_RESULT(emit_opcode(ctx, top->expr.opcode, 0));
          CHECK_RESULT(emit_i32(ctx, top->expr.load.mem_offset));
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
            reset_value_stack(ctx, &top->expr);
          }
          break;
        }

        case WASM_OPCODE_RETURN: {
          WasmInterpreterFuncSignature* sig =
              get_func_signature(ctx, ctx->current_func);
          CHECK_RESULT(check_type_exact(ctx, sig->result_type, expr->type,
                                        " in return"));
          CHECK_RESULT(emit_return(ctx, sig->result_type));
          adjust_value_stack(ctx, -get_result_count(sig->result_type));
          break;
        }

        case WASM_OPCODE_SELECT: {
          if (is_expr_done) {
            assert(cur_index == 2);
            CHECK_RESULT(
                check_type_exact(ctx, WASM_TYPE_I32, expr->type, " in select"));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_SELECT, -2));
          } else {
            assert(cur_index < 2);
            CHECK_RESULT(unify_and_check_type_exact(ctx, &top->expr.type,
                                                    expr->type, " in select"));
          }
          break;
        }

        case WASM_OPCODE_SET_LOCAL: {
          assert(cur_index == 0 && is_expr_done);
          CHECK_RESULT(check_type_exact(ctx, top->expr.type, expr->type,
                                        " in set_local"));
          CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_SET_LOCAL, 0));
          uint32_t local_index =
              translate_local_index(ctx, top->expr.set_local.local_index);
          CHECK_RESULT(emit_i32(ctx, local_index));
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
                check_type_exact(ctx, WASM_TYPE_I32, expr->type, " in store"));
          } else {
            assert(cur_index == 1 && is_expr_done);
            CHECK_RESULT(
                check_type_exact(ctx, top->expr.type, expr->type, " in store"));
            CHECK_RESULT(emit_opcode(ctx, top->expr.opcode, -1));
            CHECK_RESULT(emit_i32(ctx, top->expr.store.mem_offset));
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
        expr = &top->expr;
        ctx->expr_stack.size--;
        done = 0;
      }
    }
  }

  return WASM_OK;
}

static WasmResult shift(WasmReadInterpreterContext* ctx,
                        WasmInterpreterExpr* expr,
                        uint32_t count) {
  if (count > 0) {
#if LOG
    fprintf(stderr, "%3" PRIzd "(%d): shift: %s:%s %u\n", ctx->expr_stack.size,
            ctx->value_stack_size, s_opcode_name[expr->opcode],
            s_type_names[expr->type], count);
#endif
    WasmExprNode* node =
        wasm_append_expr_node(ctx->allocator, &ctx->expr_stack);
    CHECK_ALLOC_NULL(ctx, node);
    node->expr = *expr;
    node->index = 0;
    node->total = count;
    return WASM_OK;
  } else {
    adjust_value_stack(ctx, get_result_count(expr->type));
    return reduce(ctx, expr);
  }
}

static WasmResult on_unary_expr(WasmOpcode opcode, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = s_opcode_rtype[opcode];
  expr.opcode = opcode;
  return shift(ctx, &expr, 1);
}

static WasmResult on_binary_expr(WasmOpcode opcode, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = s_opcode_rtype[opcode];
  expr.opcode = opcode;
  return shift(ctx, &expr, 2);
}

static WasmResult on_block_expr(uint32_t count, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = count ? WASM_TYPE_ANY : WASM_TYPE_VOID;
  expr.opcode = WASM_OPCODE_BLOCK;
  expr.block.value_stack_size = ctx->value_stack_size;
  CHECK_RESULT(push_depth(ctx, expr.type));
  return shift(ctx, &expr, count);
}

static WasmResult on_br_expr(uint32_t depth, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  CHECK_DEPTH(ctx, depth);
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_ANY;
  expr.opcode = WASM_OPCODE_BR;
  expr.br.depth = translate_depth(ctx, depth);
  return shift(ctx, &expr, 1);
}

static WasmResult on_br_if_expr(uint32_t depth, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
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
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_ANY;
  expr.opcode = WASM_OPCODE_BR_TABLE;
  expr.br_table.num_targets = num_targets;

  /* we need to parse the "key" expression before we can execute the br_table.
   * Rather than store the target_depths in an Expr, we just write them out
   * into the instruction stream and just jump over it. */
  uint32_t fixup_br_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR, 0));
  CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));

  /* write the branch table as (offset, discard count) pairs */
  expr.br_table.table_offset = get_istream_offset(ctx);

  WasmDepthNode* node;
  uint32_t discard_count;
  uint32_t i;
  for (i = 0; i < num_targets; ++i) {
    uint32_t depth = translate_depth(ctx, target_depths[i]);
    node = get_depth_node(ctx, depth);
    discard_count = ctx->value_stack_size - node->value_stack_size;
    CHECK_RESULT(unify_and_check_type_exact(ctx, &node->type, WASM_TYPE_VOID,
                                            " in br_table"));
    CHECK_RESULT(emit_br_table_offset(ctx, depth, node, discard_count));
  }
  /* write default target */
  node = get_depth_node(ctx, translate_depth(ctx, default_target_depth));
  discard_count = ctx->value_stack_size - node->value_stack_size;
  CHECK_RESULT(unify_and_check_type_exact(ctx, &node->type, WASM_TYPE_VOID,
                                          " in br_table"));
  CHECK_RESULT(
      emit_br_table_offset(ctx, default_target_depth, node, discard_count));

  CHECK_RESULT(emit_i32_at(ctx, fixup_br_offset, get_istream_offset(ctx)));
  return shift(ctx, &expr, 1);
}

static WasmResult on_call_expr(uint32_t func_index, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  assert(func_index < ctx->funcs.size);
  WasmInterpreterFunc* func = get_func(ctx, func_index);
  WasmInterpreterFuncSignature* sig = get_func_signature(ctx, func);
  WasmInterpreterExpr expr;
  expr.type = sig->result_type;
  expr.opcode = WASM_OPCODE_CALL_FUNCTION;
  expr.call.func_index = func_index;
  return shift(ctx, &expr, sig->param_types.size);
}

static WasmResult on_call_import_expr(uint32_t import_index, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  assert(import_index < ctx->module->imports.size);
  WasmInterpreterImport* import = get_import(ctx, import_index);
  WasmInterpreterFuncSignature* sig = get_signature(ctx, import->sig_index);
  WasmInterpreterExpr expr;
  expr.type = sig->result_type;
  expr.opcode = WASM_OPCODE_CALL_IMPORT;
  expr.call_import.import_index = import_index;
  return shift(ctx, &expr, sig->param_types.size);
}

static WasmResult on_call_indirect_expr(uint32_t sig_index, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterFuncSignature* sig = get_signature(ctx, sig_index);
  WasmInterpreterExpr expr;
  expr.type = sig->result_type;
  expr.opcode = WASM_OPCODE_CALL_INDIRECT;
  expr.call_indirect.sig_index = sig_index;
  return shift(ctx, &expr, sig->param_types.size + 1);
}

static WasmResult on_i32_const_expr(uint32_t value, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_I32;
  expr.opcode = WASM_OPCODE_I32_CONST;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I32_CONST, 1));
  CHECK_RESULT(emit_i32(ctx, value));
  return reduce(ctx, &expr);
}

static WasmResult on_i64_const_expr(uint64_t value, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_I64;
  expr.opcode = WASM_OPCODE_I64_CONST;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I64_CONST, 1));
  CHECK_RESULT(emit_i64(ctx, value));
  return reduce(ctx, &expr);
}

static WasmResult on_f32_const_expr(uint32_t value_bits, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_F32;
  expr.opcode = WASM_OPCODE_F32_CONST;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_F32_CONST, 1));
  CHECK_RESULT(emit_i32(ctx, value_bits));
  return reduce(ctx, &expr);
}

static WasmResult on_f64_const_expr(uint64_t value_bits, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_F64;
  expr.opcode = WASM_OPCODE_F64_CONST;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_F64_CONST, 1));
  CHECK_RESULT(emit_i64(ctx, value_bits));
  return reduce(ctx, &expr);
}

static WasmResult on_get_local_expr(uint32_t local_index, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = get_local_index_type(ctx->current_func, local_index);
  expr.opcode = WASM_OPCODE_GET_LOCAL;
  expr.get_local.local_index = translate_local_index(ctx, local_index);
  CHECK_LOCAL(ctx, local_index);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_GET_LOCAL, 1));
  CHECK_RESULT(emit_i32(ctx, expr.get_local.local_index));
  return reduce(ctx, &expr);
}

static WasmResult on_grow_memory_expr(void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_I32;
  expr.opcode = WASM_OPCODE_GROW_MEMORY;
  return shift(ctx, &expr, 1);
}

static WasmResult on_if_expr(void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_VOID;
  expr.opcode = WASM_OPCODE_IF;
  return shift(ctx, &expr, 2);
}

static WasmResult on_if_else_expr(void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_ANY;
  expr.opcode = WASM_OPCODE_IF_ELSE;
  return shift(ctx, &expr, 3);
}

static WasmResult on_load_expr(WasmOpcode opcode,
                               uint32_t alignment_log2,
                               uint32_t offset,
                               void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = s_opcode_rtype[opcode];
  expr.opcode = opcode;
  expr.load.mem_offset = offset;
  expr.load.alignment_log2 = alignment_log2;
  return shift(ctx, &expr, 1);
}

static WasmResult on_loop_expr(uint32_t count, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = count ? WASM_TYPE_ANY : WASM_TYPE_VOID;
  expr.opcode = WASM_OPCODE_LOOP;
  expr.loop.value_stack_size = ctx->value_stack_size;
  CHECK_RESULT(push_depth(ctx, expr.type)); /* exit */
  CHECK_RESULT(push_depth_with_offset(ctx, WASM_TYPE_VOID,
                                      get_istream_offset(ctx))); /* continue */
  return shift(ctx, &expr, count);
}

static WasmResult on_memory_size_expr(void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_I32;
  expr.opcode = WASM_OPCODE_MEMORY_SIZE;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_MEMORY_SIZE, 0));
  return reduce(ctx, &expr);
}

static WasmResult on_nop_expr(void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_VOID;
  expr.opcode = WASM_OPCODE_NOP;
  return reduce(ctx, &expr);
}

static WasmResult on_return_expr(void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterFuncSignature* sig =
      get_func_signature(ctx, ctx->current_func);
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_ANY;
  expr.opcode = WASM_OPCODE_RETURN;
  return shift(ctx, &expr, get_result_count(sig->result_type));
}

static WasmResult on_select_expr(void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_ANY;
  expr.opcode = WASM_OPCODE_SELECT;
  return shift(ctx, &expr, 3);
}

static WasmResult on_set_local_expr(uint32_t local_index, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
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
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = s_opcode_rtype[opcode];
  expr.opcode = opcode;
  expr.store.mem_offset = offset;
  expr.store.alignment_log2 = alignment_log2;
  return shift(ctx, &expr, 2);
}

static WasmResult on_unreachable_expr(void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.type = WASM_TYPE_ANY;
  expr.opcode = WASM_OPCODE_UNREACHABLE;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_UNREACHABLE, 0));
  return reduce(ctx, &expr);
}

static WasmResult on_function_table_count(uint32_t count, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  CHECK_ALLOC(ctx, wasm_new_interpreter_func_table_entry_array(
                       ctx->allocator, &ctx->module->func_table, count));
  return WASM_OK;
}

static WasmResult on_function_table_entry(uint32_t index,
                                          uint32_t func_index,
                                          void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  assert(index < ctx->module->func_table.size);
  WasmInterpreterFuncTableEntry* entry = &ctx->module->func_table.data[index];
  WasmInterpreterFunc* func = get_func(ctx, func_index);
  entry->sig_index = func->sig_index;
  assert(func->offset != WASM_INVALID_OFFSET);
  entry->func_offset = func->offset;
  return WASM_OK;
}

static WasmResult on_start_function(uint32_t func_index, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  /* can't get the function offset yet, because we haven't parsed the
   * functions. Just store the function index and resolve it later in
   * end_function_bodies_section. */
  assert(func_index < ctx->funcs.size);
  ctx->start_func_index = func_index;
  return WASM_OK;
}

static WasmResult on_export_count(uint32_t count, void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
  CHECK_ALLOC(ctx, wasm_new_interpreter_export_array(
                       ctx->allocator, &ctx->module->exports, count));
  return WASM_OK;
}

static WasmResult on_export(uint32_t index,
                            uint32_t func_index,
                            WasmStringSlice name,
                            void* user_data) {
  WasmReadInterpreterContext* ctx = user_data;
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
  WasmReadInterpreterContext* ctx = user_data;

  if (ctx->start_func_index != INVALID_FUNC_INDEX) {
    WasmInterpreterFunc* func = get_func(ctx, ctx->start_func_index);
    assert(func->offset != WASM_INVALID_OFFSET);
    ctx->module->start_func_offset = func->offset;
  }

  uint32_t i;
  for (i = 0; i < ctx->module->exports.size; ++i) {
    WasmInterpreterExport* export = get_export(ctx, i);
    WasmInterpreterFunc* func = get_func(ctx, export->func_index);
    export->func_offset = func->offset;
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

static void destroy_context(WasmReadInterpreterContext* ctx) {
  wasm_destroy_expr_node_vector(ctx->allocator, &ctx->expr_stack);
  wasm_destroy_depth_node_vector(ctx->allocator, &ctx->depth_stack);
  WASM_DESTROY_VECTOR_AND_ELEMENTS(ctx->allocator, ctx->depth_fixups,
                                   uint32_vector);
  WASM_DESTROY_VECTOR_AND_ELEMENTS(ctx->allocator, ctx->func_fixups,
                                   uint32_vector);
}

WasmResult wasm_read_binary_interpreter(
    struct WasmAllocator* allocator,
    struct WasmAllocator* memory_allocator,
    const void* data,
    size_t size,
    struct WasmReadBinaryOptions* options,
    struct WasmInterpreterModule* out_module) {
  WasmReadInterpreterContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
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
  if (result == WASM_OK) {
    wasm_steal_mem_writer_output_buffer(&ctx.istream_writer,
                                        &out_module->istream);
    out_module->istream.size = ctx.istream_offset;
  }
  destroy_context(&ctx);
  return result;
}

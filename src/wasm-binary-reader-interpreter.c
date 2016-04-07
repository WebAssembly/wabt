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

#define LOG 1

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
    "void", "i32", "i64", "f32", "f64", "any", "union", "optional"
};
WASM_STATIC_ASSERT(WASM_ARRAY_SIZE(s_type_names) == WASM_NUM_TYPES + 3);

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

/*
 * The expression stack_elements count on the WasmDepthNode and
 * WasmInterpreterExpr is only needed due to the single pass code generation in
 * this compiler and is not a fundamental part of the wasm type system, so it is
 * kept separate from the WasmExprType. This compiler generates code not knowing
 * the final type of some expressions, such as blocks, and speculatively chooses
 * the number of stack elements to hold the result values when encountering the
 * first reachable exit expression. This single pass strategy might not even be
 * possible with post-order encoding, and not relevant to compilers that
 * generate code in a later pass.
 */

typedef struct WasmDepthNode {
  WasmExprType type;
  uint32_t stack_elements;
  /* we store the value stack size at this depth so we know how many
   * values to discard if we break to this depth */
  uint32_t value_stack_size;
  uint32_t offset;
} WasmDepthNode;
WASM_DEFINE_VECTOR(depth_node, WasmDepthNode);

typedef struct WasmInterpreterExpr {
  WasmOpcode opcode;
  WasmExprType type;
  uint32_t stack_elements;
  union {
    /* clang-format off */
    struct { uint32_t depth; } br, br_if;
    struct { uint32_t value_stack_size; int rtn_first; } block;
    struct { uint32_t value_stack_size; } loop;
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
  uint32_t last_expr_stack_elements;
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

static void on_error(uint32_t offset, const char* message, void* user_data);

static void print_error(WasmContext* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  on_error(WASM_INVALID_OFFSET, buffer, ctx);
}

static void init_type_expr(WasmContext* ctx, WasmExprType* type, WasmType type1) {
  if (type1 == WASM_TYPE_ANY) {
    type->empty = 1;
    type->size = 0;
  } else if (type1 == WASM_TYPE_VOID) {
    type->empty = 0;
    type->size = 0;
  } else {
    type->empty = 0;
    type->size = 1;
    type->types[0] = type1;
  }
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

static void expr_type_string(char* buffer, int size, WasmExprType* type) {
  if (type->empty) {
    strncpy(buffer, "empty", size);
    return;
  }
  int pos = 0;
  pos += wasm_snprintf(buffer + pos, size - pos, "(");
  for (size_t i = 0; i < type->size; i++) {
    WasmType type1 = type->types[i];
    fprintf(stderr, " type1 %d\n", type1);
    pos += wasm_snprintf(buffer + pos, size - pos, "%s%s",
                         i == 0 ? "" : " ",
                         s_type_names[type1]);
  }
  pos += wasm_snprintf(buffer + pos, size - pos, ")");
}

static WasmResult type_mismatch(WasmContext* ctx,
                                WasmExprType* expected_type,
                                WasmExprType* type,
                                const char* desc) {
  char expected_type_buffer[256], type_buffer[256];
  expr_type_string(expected_type_buffer, sizeof(expected_type_buffer), expected_type);
  expr_type_string(type_buffer,  sizeof(type_buffer), type);
  print_error(ctx, "type mismatch%s, expected %s but got %s.", desc,
              expected_type_buffer, type_buffer);
  return WASM_ERROR;
}

static WasmResult check_type(WasmContext* ctx,
                             WasmExprType* expected_type,
                             WasmExprType* type,
                             const char* desc) {
  if (type->empty == 1) {
    assert(type->size == 0);
    return WASM_OK;
  }

  // Accept an empty type for the target only when the actual value is
  // also empty.
  assert(expected_type->empty == 0);

  if (expected_type->size == 0)
    return WASM_OK;

  if (expected_type->size > type->size)
    return type_mismatch(ctx, expected_type, type, desc);

  for (size_t i = 0; i < expected_type->size; i++) {
    WasmType expected = expected_type->types[i];
    WasmType actual = type->types[i];
    assert(expected != WASM_TYPE_VOID);
    assert(expected != WASM_TYPE_ANY);
    assert(expected != WASM_TYPE_UNION);
    assert(expected != WASM_TYPE_OPTIONAL);
    assert(actual != WASM_TYPE_ANY);
    assert(actual != WASM_TYPE_VOID);
    if (actual == WASM_TYPE_UNION ||
	actual == WASM_TYPE_OPTIONAL ||
	actual != expected)
      return type_mismatch(ctx, expected_type, type, desc);
  }

  return WASM_OK;
}

/* Common case of a single static expected value. */
static WasmResult check_type1(WasmContext* ctx,
                              WasmType expected_type1,
                              WasmExprType* type,
                              const char* desc) {
  assert(expected_type1 != WASM_TYPE_ANY);
  assert(expected_type1 != WASM_TYPE_VOID);
  assert(expected_type1 != WASM_TYPE_UNION);
  assert(expected_type1 != WASM_TYPE_OPTIONAL);
  // dtc could do better
  WasmExprType expected_type;
  init_type_expr(ctx, &expected_type, expected_type1);
  return check_type(ctx, &expected_type, type, desc);
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

static WasmResult emit_i32_at(WasmContext* ctx,
                              uint32_t offset,
                              uint32_t value) {
  return emit_data_at(ctx, offset, &value, sizeof(value));
}

static void unemit_discard(WasmContext* ctx) {
  assert(ctx->istream_offset > 0);
  assert(ctx->istream_offset <= ctx->istream_writer.buf.size);
  if (ctx->last_expr_stack_elements == 1) {
    assert(((uint8_t*)ctx->istream_writer.buf.start)[ctx->istream_offset - 1] == WASM_OPCODE_DISCARD);
    ctx->istream_offset--;
    ctx->value_stack_size++;
  } else {
    assert(((uint8_t*)ctx->istream_writer.buf.start)[ctx->istream_offset - 6] == WASM_OPCODE_DISCARD_KEEP);
    assert(*(uint32_t*)((uint8_t*)ctx->istream_writer.buf.start + ctx->istream_offset - 5) == ctx->last_expr_stack_elements);
    assert(((uint8_t*)ctx->istream_writer.buf.start)[ctx->istream_offset - 1] == 0);
    ctx->istream_offset -= 6;
    ctx->value_stack_size += ctx->last_expr_stack_elements;
  }
}

static WasmResult emit_discard_keep(WasmContext* ctx,
                                    uint32_t discard,
                                    uint8_t keep) {
  assert(discard != UINT32_MAX);
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

static WasmResult emit_return(WasmContext* ctx, WasmExprType* result_type) {
  uint32_t keep_count = result_type->size;
  uint32_t discard_count = ctx->value_stack_size - keep_count;
  CHECK_RESULT(emit_discard_keep(ctx, discard_count, keep_count));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_RETURN));
  return WASM_OK;
}

/*
 * This is used to determine the actual type of the union
 * of multiple exits from a block or if_else etc.
 *
 * The number of values returned must be the same for all exits. Some
 * values stack discarding or filling at each exit is possible to
 * achieve this. It would be invalid to consume a filled value but if
 * not consumed then they can be empty stack slots until discarded.
 * Thus the union can not decrease the number of values, although some
 * may be flagged as empty. This creates an asymmetry, one of the
 * types is the existing type setting the result size, and here this
 * is the dest_type.
 *
 * If the first exit has the shortest number of values, and thus the
 * same as the final number of values then good code can be emitted.
 *
 * An interesting case occurs when the first set of types are both empty. The
 * caller could not match the stacks in this case. If these are for separate
 * exits then the mismatch values stack in the unreachable path does not matter,
 * but when used concurrently such as for the select operator the caller needs
 * to catch this case.
 */
static void type_union(WasmExprType* dest_type, WasmExprType* type) {
#if LOG
  char dest_type_buffer[256], type_buffer[256];
  expr_type_string(dest_type_buffer, sizeof(dest_type_buffer), dest_type);
  expr_type_string(type_buffer, sizeof(type_buffer), type);
  fprintf(stderr, "type_union %s, %s\n", dest_type_buffer, type_buffer);
#endif

  if (dest_type->empty) {
    *dest_type = *type;
    return;
  }

  if (type->empty)
    return;

  size_t new_size = type->size > dest_type->size ? type->size : dest_type->size;

  for (size_t i = 0; i < new_size; i++) {
    /* Should not see unreachable types here, double check. */
    assert(i >= dest_type->size || dest_type->types[i] != WASM_TYPE_ANY);
    assert(i >= type->size || type->types[i] != WASM_TYPE_ANY);
    if (i >= dest_type->size || i >= type->size) {
      /* These values can not be consumed, might be missing. */
      dest_type->types[i] = WASM_TYPE_OPTIONAL;
    } else if (dest_type->types[i] == WASM_TYPE_OPTIONAL ||
	       type->types[i] == WASM_TYPE_OPTIONAL) {
      /* These values can not be consumed, might be missing. */
      dest_type->types[i] = WASM_TYPE_OPTIONAL;
    } else if (dest_type->types[i] != type->types[i]) {
      /* These values can not be consumed, not the same type. */
      dest_type->types[i] = WASM_TYPE_UNION;
    }
  }

  dest_type->size = new_size;

#if LOG
  expr_type_string(dest_type_buffer, sizeof(dest_type_buffer), dest_type);
  fprintf(stderr, " => %s\n", dest_type_buffer);
#endif
}

/*
 * Consume a single expression result value, discarding excess values.  It is a
 * validation error if a value is not available which can occur if either the
 * expression returned no values, or the expression returned a 'filled' stack
 * element not intended to be consumed.
 *
 * If the type is empty then the caller will still assume there is a single
 * value on the stack to consume so fill the stack with a value, but skip
 * pushing a value to the actual stack as this is unreachable code.
 */
static WasmResult consume_values(WasmContext* ctx,
                                 WasmExprType* type,
                                 uint32_t stack_elements,
                                 uint32_t count,
                                 WasmOpcode opcode) {
  if (type->empty) {
    adjust_value_stack(ctx, count);
    return WASM_OK;
  }

  assert(stack_elements <= type->size);

  if (count > stack_elements) {
#if LOG
    char type_buffer[256];
    expr_type_string(type_buffer,  sizeof(type_buffer), type);
    print_error(ctx, "type mismatch in %s, expected %u values but got type %s.",
                s_opcode_name[opcode], count, type_buffer);
#endif
    return WASM_ERROR;
  }

  for (size_t i = 0; i < count; i++) {
    assert(type->types[i] != WASM_TYPE_ANY);
    if (type->types[i] == WASM_TYPE_UNION ||
	type->types[i] == WASM_TYPE_OPTIONAL) {
#if LOG
      char type_buffer[256];
      expr_type_string(type_buffer,  sizeof(type_buffer), type);
      print_error(ctx, "type mismatch in %s, expected %d values but got type %s.",
                  s_opcode_name[opcode], count, type_buffer);
#endif
      return WASM_ERROR;
    }
  }

  uint32_t discard_count = stack_elements - count;
  CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
  adjust_value_stack(ctx, -discard_count);

  return WASM_OK;
}

/*
 * Balance the number of values of the stack for the exit expected type,
 * discarding or filling as necessary.
 */
static WasmResult balance_value_stack(WasmContext* ctx,
                                      uint32_t target_stack_elements,
                                      uint32_t source_stack_elements) {
  int discard_count = source_stack_elements - target_stack_elements;

  if (discard_count > 0) {
    CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
    adjust_value_stack(ctx, -discard_count);
  } else if (discard_count < 0) {
    CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_ALLOCA));
    CHECK_RESULT(emit_i32(ctx, -discard_count));
    adjust_value_stack(ctx, -discard_count);
  }

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
  assert(ctx->value_stack_size >= node->value_stack_size);
  uint8_t keep_count = node->stack_elements;
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
                                         WasmExprType* type,
                                         uint32_t offset) {
  WasmDepthNode* node =
      wasm_append_depth_node(ctx->allocator, &ctx->depth_stack);
  CHECK_ALLOC_NULL(ctx, node);
  node->type = *type;
  node->stack_elements = 0;
  node->value_stack_size = ctx->value_stack_size;
  node->offset = offset;
#if LOG
  char type_buffer[256];
  expr_type_string(type_buffer, sizeof(type_buffer), type);
  fprintf(stderr, "   (%d): push depth %" PRIzd ":%s\n", ctx->value_stack_size,
          ctx->depth_stack.size - 1, type_buffer);
#endif
  return WASM_OK;
}

static WasmResult push_depth(WasmContext* ctx, WasmExprType* type) {
  return push_depth_with_offset(ctx, type, WASM_INVALID_OFFSET);
}

static void pop_depth(WasmContext* ctx) {
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
  if (offset == WASM_INVALID_OFFSET)
    fprintf(stderr, "error: %s\n", message);
  else
    fprintf(stderr, "error: @0x%08x: %s\n", offset, message);
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
                               uint32_t param_count,
                               WasmType* param_types,
                               uint32_t result_count,
                               WasmType* result_types,
                               void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterFuncSignature* sig = get_signature(ctx, index);

  CHECK_ALLOC(
      ctx, wasm_reserve_types(ctx->allocator, &sig->param_types, param_count));
  sig->param_types.size = param_count;
  memcpy(sig->param_types.data, param_types, param_count * sizeof(WasmType));

  WASM_ZERO_MEMORY(sig->result_type);
  sig->result_type.size = result_count;
  memcpy(sig->result_type.types, result_types, result_count * sizeof(WasmType));

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
#if LOG
  fprintf(stderr, "*** func %d ***\n", index);
#endif
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
  ctx->last_expr_stack_elements = 0;
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
    if (sig->result_type.size > 0) {
      CHECK_RESULT(check_type(ctx, &sig->result_type, &ctx->last_expr.type,
                              " in function result"));
      if (ctx->last_expr_stack_elements)
        unemit_discard(ctx);
      CHECK_RESULT(consume_values(ctx, &ctx->last_expr.type,
                                  ctx->last_expr_stack_elements,
                                  sig->result_type.size,
                                  WASM_OPCODE_RETURN));
    }
    assert(ctx->value_stack_size == sig->param_types.size + func->local_count + sig->result_type.size);
    CHECK_RESULT(emit_return(ctx, &sig->result_type));
    adjust_value_stack(ctx, -sig->result_type.size);
  }
  assert(ctx->value_stack_size == sig->param_types.size + func->local_count);
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
#if LOG
      char type_buffer[256];
      expr_type_string(type_buffer, sizeof(type_buffer), &expr->type);
      fprintf(stderr, "%3" PRIzd "(%d): reduce: <- %s:%s:%d\n",
              ctx->expr_stack.size, ctx->value_stack_size,
              s_opcode_name[expr->opcode], type_buffer,
              expr->stack_elements);
#endif

      /*
       * This is a function top level expression, but it is not known if this is
       * the last expression of the function. The result values of all top level
       * expressions are discard, except for the last expression. This is
       * resolved by speculatively discarding all expression values and at the
       * end of the function body reverting the last discard operation if the
       * values are needed.
       */

      int discard_count = expr->stack_elements;
      CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
      adjust_value_stack(ctx, -discard_count);
      ctx->last_expr = *expr;
      ctx->last_expr_stack_elements = discard_count;
    } else {
      WasmExprNode* top = &ctx->expr_stack.data[ctx->expr_stack.size - 1];
      assert(top->index < top->total);

#if LOG
      char type_buffer[256];
      expr_type_string(type_buffer, sizeof(type_buffer), &expr->type);
      fprintf(stderr, "%3" PRIzd "(%d): reduce: %s(%d/%d) <- %s:%s:%d\n",
              ctx->expr_stack.size, ctx->value_stack_size,
              s_opcode_name[top->expr.opcode], top->index, top->total,
              s_opcode_name[expr->opcode], type_buffer,
              expr->stack_elements);
#endif
#if LOG
      if (top->expr.opcode == WASM_OPCODE_BR) {
        fprintf(stderr, "      : br depth %u\n", top->expr.br.depth);
      }
#endif

      uint32_t cur_index = top->index++;
      WasmBool is_expr_done = top->index == top->total;
      int32_t result_count = 0;

      switch (top->expr.opcode) {
        /* handles all unary and binary operators */
        default:
          CHECK_RESULT(consume_values(ctx, &expr->type,
                                      expr->stack_elements,
                                      1, top->expr.opcode));

          WasmType expected_type;
          if (cur_index == 0) {
            expected_type = s_opcode_type1[top->expr.opcode];
          } else if (cur_index == 1) {
            expected_type = s_opcode_type2[top->expr.opcode];
          } else {
            assert(0);
            break;
          }
          /* TODO use opcode name */
          CHECK_RESULT(check_type1(ctx, expected_type, &expr->type,
                                   " in unary or binary op"));

          if (is_expr_done) {
            CHECK_RESULT(emit_opcode(ctx, top->expr.opcode));
            adjust_value_stack(ctx, -cur_index);
            result_count = 1;
          }
          break;

        case WASM_OPCODE_BLOCK:
          if (top->expr.block.rtn_first) {
            if (cur_index == 0) {
              WasmDepthNode* node = top_depth_node(ctx);
              int was_empty = node->type.empty;
              type_union(&node->type, &expr->type);
              /* Set the result stack elements on the first reachable exit. */
              if (was_empty && !node->type.empty)
                node->stack_elements = expr->stack_elements;
            } else {
              int discard_count = expr->stack_elements;
              CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
              adjust_value_stack(ctx, -discard_count);
            }
            if (is_expr_done) {
              WasmDepthNode* node = top_depth_node(ctx);
              top->expr.type = node->type;
              top->expr.stack_elements = node->stack_elements;
            }
          } else {
            if (is_expr_done) {
              /* Merge the type from the depth node. */
              WasmDepthNode* node = top_depth_node(ctx);
              int was_empty = node->type.empty;
              type_union(&node->type, &expr->type);
              /* Set the result stack elements on the first reachable exit. */
              if (was_empty && !node->type.empty)
                node->stack_elements = expr->stack_elements;
              top->expr.type = node->type;
              top->expr.stack_elements = node->stack_elements;
              balance_value_stack(ctx, top->expr.stack_elements,
                                  expr->stack_elements);
            } else {
              int discard_count = expr->stack_elements;
              CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
              adjust_value_stack(ctx, -discard_count);
            }
          }
          if (is_expr_done) {
            CHECK_RESULT(fixup_top_depth(ctx, get_istream_offset(ctx)));
            pop_depth(ctx);
            result_count = top->expr.stack_elements;
          }
          break;

        case WASM_OPCODE_BR: {
          assert(cur_index == 0 && is_expr_done);
          uint32_t depth = top->expr.br.depth;
          WasmDepthNode* node = get_depth_node(ctx, depth);
          int was_empty = node->type.empty;
          type_union(&node->type, &expr->type);
          assert(!node->type.empty || expr->type.empty);
          /* Set the result stack elements on the first reachable exit. */
          if (was_empty && !node->type.empty)
            node->stack_elements = expr->stack_elements;
          balance_value_stack(ctx, node->stack_elements,
                              expr->stack_elements);
          CHECK_RESULT(emit_br(ctx, depth, node));
          adjust_value_stack(ctx, -node->stack_elements);
          result_count = 0;
          break;
        }

        case WASM_OPCODE_BR_IF: {
          uint32_t depth = top->expr.br.depth;
          WasmDepthNode* node = get_depth_node(ctx, depth);
          if (cur_index == 0) {
            int was_empty = node->type.empty;
            type_union(&node->type, &expr->type);
            assert(!node->type.empty || expr->type.empty);
            /* Set the result stack elements on the first reachable exit. */
            if (was_empty && !node->type.empty)
              node->stack_elements = expr->stack_elements;
            balance_value_stack(ctx, node->stack_elements,
                                expr->stack_elements);
          } else {
            /* this actually flips the br_if so if <cond> is true it can
             * discard values from the stack, e.g.:
             *
             *   (br_if DEST <value> <cond>)
             *
             * becomes
             *
             *   <value>            values
             *   <cond>             values cond
             *   br_unless OVER     values
             *   discard_keep ...   values
             *   br DEST            values
             * OVER:
             *   discard
             *   ...
             */
            assert(cur_index == 1 && is_expr_done);
            CHECK_RESULT(consume_values(ctx, &expr->type,
                                        expr->stack_elements,
                                        1, top->expr.opcode));
            CHECK_RESULT(check_type1(ctx, WASM_TYPE_I32, &expr->type,
                                     " in br_if"));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_UNLESS));
            uint32_t fixup_br_offset = get_istream_offset(ctx);
            CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
            /* adjust stack to account for br_unless consuming <cond> */
            adjust_value_stack(ctx, -1);
            CHECK_RESULT(emit_br(ctx, depth, node));
            CHECK_RESULT(emit_i32_at(ctx, fixup_br_offset,
                                     get_istream_offset(ctx)));
            /* Discard the values if the branch wasn't taken. */
            int discard_count = node->stack_elements;
            CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
            adjust_value_stack(ctx, -discard_count);
            result_count = 0;
          }
          break;
        }

        case WASM_OPCODE_BR_TABLE: {
          assert(cur_index == 0 && is_expr_done);
          CHECK_RESULT(consume_values(ctx, &expr->type,
                                      expr->stack_elements,
                                      1, top->expr.opcode));
          CHECK_RESULT(check_type1(ctx, WASM_TYPE_I32, &expr->type,
                                   " in br_table"));
          CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_TABLE));
          CHECK_RESULT(emit_i32(ctx, top->expr.br_table.num_targets));
          CHECK_RESULT(emit_i32(ctx, top->expr.br_table.table_offset));
          /* adjust stack to account for br_table consuming <cond> */
          adjust_value_stack(ctx, -1);
          break;
        }

        case WASM_OPCODE_CALL_FUNCTION: {
          WasmInterpreterFunc* func = get_func(ctx, top->expr.call.func_index);
          WasmInterpreterFuncSignature* sig = get_func_signature(ctx, func);
          CHECK_RESULT(consume_values(ctx, &expr->type,
                                      expr->stack_elements,
                                      1, top->expr.opcode));
          CHECK_RESULT(check_type1(ctx, sig->param_types.data[cur_index],
                                   &expr->type, " in call"));
          if (is_expr_done) {
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_FUNCTION));
            CHECK_RESULT(emit_func_offset(ctx, func, top->expr.call.func_index));
            result_count = sig->result_type.size;
            adjust_value_stack(ctx, result_count - sig->param_types.size);
          }
          break;
        }

        case WASM_OPCODE_MV_CALL_FUNCTION: {
          assert(cur_index == 0 && is_expr_done);
          WasmInterpreterFunc* func = get_func(ctx, top->expr.call.func_index);
          WasmInterpreterFuncSignature* sig = get_func_signature(ctx, func);
          if (expr->type.empty) {
            /* Fill */
            adjust_value_stack(ctx, sig->param_types.size);
          } else {
            /*
             * Require a fixed number of values.
             */
            for (size_t i = 0; i < sig->param_types.size; i++) {
              assert(expr->type.types[i] != WASM_TYPE_ANY);
              if (expr->type.types[i] != sig->param_types.data[cur_index]) {
                print_error(ctx, "type mismatch in mv_call, expected %s but got %s.",
                            s_type_names[sig->param_types.data[cur_index]],
                            s_type_names[expr->type.types[i]]);
                return WASM_ERROR;
              }
            }
            assert(expr->stack_elements == expr->type.size);
          }
          CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_FUNCTION));
          CHECK_RESULT(emit_func_offset(ctx, func, top->expr.call.func_index));
          result_count = sig->result_type.size;
          adjust_value_stack(ctx, result_count - sig->param_types.size);
          break;
        }

        case WASM_OPCODE_CALL_IMPORT: {
          WasmInterpreterImport* import =
              get_import(ctx, top->expr.call_import.import_index);
          WasmInterpreterFuncSignature* sig =
              get_signature(ctx, import->sig_index);
          CHECK_RESULT(consume_values(ctx, &expr->type,
                                      expr->stack_elements,
                                      1, top->expr.opcode));
          CHECK_RESULT(check_type1(ctx, sig->param_types.data[cur_index],
                                   &expr->type, " in call_import"));
          if (is_expr_done) {
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_IMPORT));
            CHECK_RESULT(emit_i32(ctx, top->expr.call_import.import_index));
            result_count = sig->result_type.size;
            adjust_value_stack(ctx, result_count - sig->param_types.size);
          }
          break;
        }

        case WASM_OPCODE_CALL_INDIRECT: {
          WasmInterpreterFuncSignature* sig =
              get_signature(ctx, top->expr.call_indirect.sig_index);
          CHECK_RESULT(consume_values(ctx, &expr->type,
                                      expr->stack_elements,
                                      1, top->expr.opcode));
          if (cur_index == 0) {
            CHECK_RESULT(check_type1(ctx, WASM_TYPE_I32, &expr->type,
                                    " in call_indirect"));
          } else {
            CHECK_RESULT(check_type1(ctx, sig->param_types.data[cur_index - 1],
                                     &expr->type, " in call_indirect"));
          }
          if (is_expr_done) {
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_INDIRECT));
            CHECK_RESULT(emit_i32(ctx, top->expr.call_indirect.sig_index));
            /* the callee cleans up the params for us, but we have to clean up
             * the function table index */
            result_count = sig->result_type.size;
            CHECK_RESULT(emit_discard_keep(ctx, 1, result_count));
            adjust_value_stack(ctx, result_count - sig->param_types.size - 1);
          }
          break;
        }

        case WASM_OPCODE_VALUES: {
          /*
           * If any argument is the unreachable type then the result is the
           * unreachable type, and all values are discarded.
           */
          if (top->expr.type.empty) {
            int discard_count = expr->stack_elements;
            CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
            adjust_value_stack(ctx, -discard_count);
          } else if (expr->type.empty) {
            int discard_count = top->expr.stack_elements + expr->stack_elements;
            CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
            adjust_value_stack(ctx, -discard_count);
            top->expr.type.empty = 1;
            top->expr.type.size = 0;
            top->expr.stack_elements = 0;
          } else {
            CHECK_RESULT(consume_values(ctx, &expr->type,
                                        expr->stack_elements,
                                        1, top->expr.opcode));
            assert(top->expr.type.size == cur_index);
            assert(expr->type.size >= 1);
            top->expr.type.types[cur_index] = expr->type.types[0];
            top->expr.type.size = cur_index + 1;
            top->expr.stack_elements = cur_index + 1;
          }
          if (is_expr_done)
            result_count = top->expr.stack_elements;
          break;
        }

        case WASM_OPCODE_CONC_VALUES: {
          /*
           * If any argument is the unreachable type then the result is the
           * unreachable type, and all values are discarded.
           */
          if (top->expr.type.empty) {
            int discard_count = expr->stack_elements;
            CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
            adjust_value_stack(ctx, -discard_count);
          } else if (expr->type.empty) {
            int discard_count = top->expr.stack_elements + expr->stack_elements;
            CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
            adjust_value_stack(ctx, -discard_count);
            top->expr.type.empty = 1;
            top->expr.type.size = 0;
            top->expr.stack_elements = 0;
          } else {
            /*
             * Require a fixed number of values.
             */
            for (size_t i = 0; i < expr->type.size; i++) {
              assert(expr->type.types[i] != WASM_TYPE_ANY);
              if (expr->type.types[i] == WASM_TYPE_OPTIONAL) {
#if LOG
                char type_buffer[256];
                expr_type_string(type_buffer,  sizeof(type_buffer), &expr->type);
                print_error(ctx, "type mismatch in %s, expected a fixed number of values but got type %s.",
                            s_opcode_name[WASM_OPCODE_CONC_VALUES], type_buffer);
#endif
                return WASM_ERROR;
              }
              top->expr.type.types[top->expr.type.size++] = expr->type.types[i];
            }
            assert(expr->stack_elements == expr->type.size);
            top->expr.stack_elements = top->expr.type.size;
          }
          if (is_expr_done)
            result_count = top->expr.stack_elements;
          break;
        }

        case WASM_OPCODE_GROW_MEMORY:
          assert(cur_index == 0 && is_expr_done);
          CHECK_RESULT(consume_values(ctx, &expr->type,
                                      expr->stack_elements,
                                      1, top->expr.opcode));
          CHECK_RESULT(check_type1(ctx, WASM_TYPE_I32, &expr->type,
                                   " in grow_memory"));
          CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_GROW_MEMORY));
          result_count = 1;
          break;

        case WASM_OPCODE_IF:
          if (cur_index == 0) {
            /* after cond */
            CHECK_RESULT(consume_values(ctx, &expr->type,
                                        expr->stack_elements,
                                        1, top->expr.opcode));
            CHECK_RESULT(check_type1(ctx, WASM_TYPE_I32, &expr->type, " in if"));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_UNLESS));
            /* adjust stack to account for br_unless consuming <cond> */
            adjust_value_stack(ctx, -1);
            top->expr.if_.fixup_offset = get_istream_offset(ctx);
            CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
          } else {
            /* after true */
            assert(cur_index == 1 && is_expr_done);
            /* discard the last values, if any; if is always void */
            int discard_count = expr->stack_elements;
            CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
            adjust_value_stack(ctx, -discard_count);
            /* Backpatch the branch over the true expression. */
            CHECK_RESULT(emit_i32_at(ctx, top->expr.if_.fixup_offset,
                                     get_istream_offset(ctx)));
            result_count = 0;
          }
          break;

        case WASM_OPCODE_IF_ELSE: {
          if (cur_index == 0) {
            /* after cond */
            CHECK_RESULT(consume_values(ctx, &expr->type,
                                        expr->stack_elements,
                                        1, top->expr.opcode));
            CHECK_RESULT(
                check_type1(ctx, WASM_TYPE_I32, &expr->type, " in if_else"));
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_UNLESS));
            /* adjust stack to account for br_unless consuming <cond> */
            adjust_value_stack(ctx, -1);
            top->expr.if_else.fixup_cond_offset = get_istream_offset(ctx);
            top->expr.if_else.value_stack_size = ctx->value_stack_size;
            CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
          } else {
            if (cur_index == 1) {
              /* after true */
              assert(top->expr.type.empty);
              top->expr.type = expr->type;
              /* Use this first exit path to set the number of result stack elements. */
              if (!top->expr.type.empty)
                top->expr.stack_elements = expr->stack_elements;
              CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR));
              top->expr.if_else.fixup_true_offset = get_istream_offset(ctx);
              CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
              CHECK_RESULT(emit_i32_at(ctx, top->expr.if_else.fixup_cond_offset,
                                       get_istream_offset(ctx)));
              /* reset the value stack for the other branch arm */
              adjust_value_stack(ctx, -expr->stack_elements);
              assert(ctx->value_stack_size == top->expr.if_else.value_stack_size);
              fprintf(stderr, "p0: %d %d\n", top->expr.stack_elements, expr->stack_elements);
            } else {
              /* after false */
              assert(cur_index == 2 && is_expr_done);
              int was_empty = top->expr.type.empty;
              type_union(&top->expr.type, &expr->type);
              assert(!top->expr.type.empty || expr->type.empty);
              /* Set the result stack elements on the first reachable exit. */
              fprintf(stderr, "p1: %d %d\n", top->expr.stack_elements, expr->stack_elements);
              if (was_empty && !top->expr.type.empty)
                top->expr.stack_elements = expr->stack_elements;
              fprintf(stderr, "p2: %d %d\n", top->expr.stack_elements, expr->stack_elements);
              balance_value_stack(ctx, top->expr.stack_elements,
                                  expr->stack_elements);
              fprintf(stderr, "p3: %d %d\n", top->expr.stack_elements, expr->stack_elements);
              CHECK_RESULT(emit_i32_at(ctx, top->expr.if_else.fixup_true_offset,
                                       get_istream_offset(ctx)));
              result_count = top->expr.stack_elements;
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
          CHECK_RESULT(consume_values(ctx, &expr->type,
                                      expr->stack_elements,
                                      1, top->expr.opcode));
          /* TODO use opcode name here */
          CHECK_RESULT(check_type1(ctx, WASM_TYPE_I32, &expr->type, " in load"));
          CHECK_RESULT(emit_opcode(ctx, top->expr.opcode));
          CHECK_RESULT(emit_i32(ctx, top->expr.load.mem_offset));
          result_count = 1;
          break;

        case WASM_OPCODE_LOOP: {
          if (is_expr_done) {
            WasmDepthNode* node = top_minus_nth_depth_node(ctx, 2);
            int was_empty = node->type.empty;
            type_union(&node->type, &expr->type);
            /* Set the result stack elements on the first reachable exit. */
            if (was_empty && !node->type.empty)
              node->stack_elements = expr->stack_elements;
            top->expr.type = node->type;
            top->expr.stack_elements = node->stack_elements;
            balance_value_stack(ctx, top->expr.stack_elements,
                                expr->stack_elements);
          } else {
            int discard_count = expr->stack_elements;
            CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
            adjust_value_stack(ctx, -discard_count);
          }
          if (is_expr_done) {
            pop_depth(ctx); /* continue */
            CHECK_RESULT(fixup_top_depth(ctx, get_istream_offset(ctx)));
            pop_depth(ctx); /* exit */
            result_count = top->expr.stack_elements;
          }
          break;
        }

        case WASM_OPCODE_RETURN: {
          assert(is_expr_done);
          WasmInterpreterFuncSignature* sig =
              get_func_signature(ctx, ctx->current_func);
          CHECK_RESULT(consume_values(ctx, &expr->type,
                                      expr->stack_elements,
                                      sig->result_type.size,
                                      top->expr.opcode));
          CHECK_RESULT(check_type(ctx, &sig->result_type, &expr->type, " in return"));
          CHECK_RESULT(emit_return(ctx, &sig->result_type));
          adjust_value_stack(ctx, -sig->result_type.size);
          result_count = 0;
          break;
        }

        case WASM_OPCODE_SELECT: {
          if (is_expr_done) {
            assert(cur_index == 2);
            CHECK_RESULT(consume_values(ctx, &expr->type,
                                        expr->stack_elements,
                                        1, top->expr.opcode));
            CHECK_RESULT(check_type1(ctx, WASM_TYPE_I32, &expr->type,
                                     " in select"));
            result_count = top->expr.stack_elements;
            CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_SELECT));
            CHECK_RESULT(emit_i32(ctx, result_count));
            /* Adjust stack to account for select consuming the arguments. */
            adjust_value_stack(ctx, -(result_count + 1));
          } else if (cur_index == 0) {
            top->expr.type = expr->type;
            /* Set the number of result stack elements using the first result expression. */
            top->expr.stack_elements = expr->stack_elements;
          } else {
            assert(cur_index == 1);
            if (top->expr.type.empty || expr->type.empty) {
              /* Discard the values here in this case. */
              int discard_count = top->expr.stack_elements + expr->stack_elements;
	      CHECK_RESULT(emit_discard_keep(ctx, discard_count, 0));
	      adjust_value_stack(ctx, -discard_count);
              /* Result type is empty if either are empty. */
              top->expr.type.empty = 1;
              top->expr.type.size = 0;
              top->expr.stack_elements = 0;
            } else {
              type_union(&top->expr.type, &expr->type);
              assert(!top->expr.type.empty || expr->type.empty);
              balance_value_stack(ctx, top->expr.stack_elements,
                                  expr->stack_elements);
            }
          }
          break;
        }

        case WASM_OPCODE_SET_LOCAL: {
          assert(cur_index == 0 && is_expr_done);
          CHECK_RESULT(consume_values(ctx, &expr->type,
                                      expr->stack_elements,
                                      1, top->expr.opcode));
          CHECK_RESULT(check_type(ctx, &top->expr.type, &expr->type, " in set_local"));
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
          CHECK_RESULT(consume_values(ctx, &expr->type,
                                      expr->stack_elements,
                                      1, top->expr.opcode));
          if (cur_index == 0) {
            /* TODO use opcode name here */
            CHECK_RESULT(
                check_type1(ctx, WASM_TYPE_I32, &expr->type, " in store"));
          } else {
            assert(cur_index == 1 && is_expr_done);
            CHECK_RESULT(check_type(ctx, &top->expr.type, &expr->type, " in store"));
            CHECK_RESULT(emit_opcode(ctx, top->expr.opcode));
            CHECK_RESULT(emit_i32(ctx, top->expr.store.mem_offset));
            result_count = 1;
            adjust_value_stack(ctx, -1);
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
        assert(ctx->value_stack_size == top->value_stack_size + result_count);
        //ctx->value_stack_size = top->value_stack_size + result_count;
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
#if LOG
  char type_buffer[256];
  expr_type_string(type_buffer, sizeof(type_buffer), &expr->type);
  fprintf(stderr, "%3" PRIzd "(%d): shift: %s:%s %u\n", ctx->expr_stack.size,
          ctx->value_stack_size, s_opcode_name[expr->opcode],
          type_buffer, count);
#endif
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
  init_type_expr(ctx, &expr.type, s_opcode_rtype[opcode]);
  expr.stack_elements = 1;
  expr.opcode = opcode;
  return shift(ctx, &expr, 1);
}

static WasmResult on_binary_expr(WasmOpcode opcode, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, s_opcode_rtype[opcode]);
  expr.stack_elements = 1;
  expr.opcode = opcode;
  return shift(ctx, &expr, 2);
}

static WasmResult on_block_expr(int rtn_first, uint32_t count,
                                void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.opcode = WASM_OPCODE_BLOCK;
  expr.block.value_stack_size = ctx->value_stack_size;
  expr.block.rtn_first = rtn_first;
  expr.stack_elements = 0;
  if (count > 0) {
    init_type_expr(ctx, &expr.type, WASM_TYPE_ANY);
    CHECK_RESULT(push_depth(ctx, &expr.type));
    return shift(ctx, &expr, count);
  } else {
    init_type_expr(ctx, &expr.type, WASM_TYPE_VOID);
    return reduce(ctx, &expr);
  }
}

static WasmResult on_br_expr(uint32_t depth, void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_DEPTH(ctx, depth);
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_ANY);
  expr.stack_elements = 0;
  expr.opcode = WASM_OPCODE_BR;
  expr.br.depth = translate_depth(ctx, depth);
  return shift(ctx, &expr, 1);
}

static WasmResult on_br_if_expr(uint32_t depth, void* user_data) {
  WasmContext* ctx = user_data;
  CHECK_DEPTH(ctx, depth);
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_VOID);
  expr.stack_elements = 0;
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
  init_type_expr(ctx, &expr.type, WASM_TYPE_ANY);
  expr.stack_elements = 0;
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
    WasmExprType zero_values_type;
    init_type_expr(ctx, &zero_values_type, WASM_TYPE_VOID);
    type_union(&node->type, &zero_values_type);
    assert(!node->type.empty);
    /* No need to set the result stack elements here as they are either set or
     * already zero. */
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
  expr.stack_elements = expr.type.size;
  expr.opcode = WASM_OPCODE_CALL_FUNCTION;
  expr.call.func_index = func_index;
  if (sig->param_types.size > 0) {
    return shift(ctx, &expr, sig->param_types.size);
  } else {
    CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_FUNCTION));
    CHECK_RESULT(emit_func_offset(ctx, func, func_index));
    adjust_value_stack(ctx, sig->result_type.size);
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
  expr.stack_elements = expr.type.size;
  expr.opcode = WASM_OPCODE_CALL_IMPORT;
  expr.call_import.import_index = import_index;
  if (sig->param_types.size > 0) {
    return shift(ctx, &expr, sig->param_types.size);
  } else {
    CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_IMPORT));
    CHECK_RESULT(emit_i32(ctx, import_index));
    adjust_value_stack(ctx, sig->result_type.size);
    return reduce(ctx, &expr);
  }
}

static WasmResult on_call_indirect_expr(uint32_t sig_index, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterFuncSignature* sig = get_signature(ctx, sig_index);
  WasmInterpreterExpr expr;
  expr.type = sig->result_type;
  expr.stack_elements = expr.type.size;
  expr.opcode = WASM_OPCODE_CALL_INDIRECT;
  expr.call_indirect.sig_index = sig_index;
  return shift(ctx, &expr, sig->param_types.size + 1);
}

static WasmResult on_values_expr(uint32_t values_count, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_VOID);
  expr.stack_elements = 0;
  expr.opcode = WASM_OPCODE_VALUES;
  if (values_count > 0) {
    return shift(ctx, &expr, values_count);
  } else {
    return reduce(ctx, &expr);
  }
}

static WasmResult on_conc_values_expr(uint32_t values_count, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_VOID);
  expr.stack_elements = 0;
  expr.opcode = WASM_OPCODE_CONC_VALUES;
  if (values_count > 0) {
    return shift(ctx, &expr, values_count);
  } else {
    return reduce(ctx, &expr);
  }
}

static WasmResult on_mv_call_expr(uint32_t func_index, void* user_data) {
  WasmContext* ctx = user_data;
  assert(func_index < ctx->funcs.size);
  WasmInterpreterFunc* func = get_func(ctx, func_index);
  WasmInterpreterFuncSignature* sig = get_func_signature(ctx, func);
  WasmInterpreterExpr expr;
  expr.type = sig->result_type;
  expr.stack_elements = expr.type.size;
  expr.opcode = WASM_OPCODE_MV_CALL_FUNCTION;
  expr.call.func_index = func_index;
  return shift(ctx, &expr, 1);
}

static WasmResult on_i32_const_expr(uint32_t value, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_I32);
  expr.stack_elements = 1;
  expr.opcode = WASM_OPCODE_I32_CONST;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I32_CONST));
  CHECK_RESULT(emit_i32(ctx, value));
  adjust_value_stack(ctx, 1);
  return reduce(ctx, &expr);
}

static WasmResult on_i64_const_expr(uint64_t value, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_I64);
  expr.stack_elements = 1;
  expr.opcode = WASM_OPCODE_I64_CONST;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I64_CONST));
  CHECK_RESULT(emit_i64(ctx, value));
  adjust_value_stack(ctx, 1);
  return reduce(ctx, &expr);
}

static WasmResult on_f32_const_expr(uint32_t value_bits, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_F32);
  expr.stack_elements = 1;
  expr.opcode = WASM_OPCODE_F32_CONST;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_F32_CONST));
  CHECK_RESULT(emit_i32(ctx, value_bits));
  adjust_value_stack(ctx, 1);
  return reduce(ctx, &expr);
}

static WasmResult on_f64_const_expr(uint64_t value_bits, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_F64);
  expr.stack_elements = 1;
  expr.opcode = WASM_OPCODE_F64_CONST;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_F64_CONST));
  CHECK_RESULT(emit_i64(ctx, value_bits));
  adjust_value_stack(ctx, 1);
  return reduce(ctx, &expr);
}

static WasmResult on_get_local_expr(uint32_t local_index, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, get_local_index_type(ctx->current_func,
                                                       local_index));
  expr.stack_elements = 1;
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
  init_type_expr(ctx, &expr.type, WASM_TYPE_I32);
  expr.stack_elements = 1;
  expr.opcode = WASM_OPCODE_GROW_MEMORY;
  return shift(ctx, &expr, 1);
}

static WasmResult on_if_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_VOID);
  expr.stack_elements = 0;
  expr.opcode = WASM_OPCODE_IF;
  return shift(ctx, &expr, 2);
}

static WasmResult on_if_else_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_ANY);
  expr.stack_elements = 0;
  expr.opcode = WASM_OPCODE_IF_ELSE;
  return shift(ctx, &expr, 3);
}

static WasmResult on_load_expr(WasmOpcode opcode,
                               uint32_t alignment_log2,
                               uint32_t offset,
                               void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, s_opcode_rtype[opcode]);
  expr.stack_elements = 1;
  expr.opcode = opcode;
  expr.load.mem_offset = offset;
  expr.load.alignment_log2 = alignment_log2;
  return shift(ctx, &expr, 1);
}

static WasmResult on_loop_expr(uint32_t count, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  expr.opcode = WASM_OPCODE_LOOP;
  expr.loop.value_stack_size = ctx->value_stack_size;
  expr.stack_elements = 0;
  if (count > 0) {
    init_type_expr(ctx, &expr.type, WASM_TYPE_ANY);
    CHECK_RESULT(push_depth(ctx, &expr.type)); /* exit */
    WasmExprType zero_values_type;
    init_type_expr(ctx, &zero_values_type, WASM_TYPE_VOID);
    CHECK_RESULT(push_depth_with_offset(
        ctx, &zero_values_type, get_istream_offset(ctx))); /* continue */
    return shift(ctx, &expr, count);
  } else {
    init_type_expr(ctx, &expr.type, WASM_TYPE_VOID);
    return reduce(ctx, &expr);
  }
}

static WasmResult on_memory_size_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_I32);
  expr.stack_elements = 1;
  expr.opcode = WASM_OPCODE_MEMORY_SIZE;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_MEMORY_SIZE));
  adjust_value_stack(ctx, 1);
  return reduce(ctx, &expr);
}

static WasmResult on_nop_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_VOID);
  expr.stack_elements = 0;
  expr.opcode = WASM_OPCODE_NOP;
  return reduce(ctx, &expr);
}

static WasmResult on_return_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_ANY);
  expr.stack_elements = 0;
  expr.opcode = WASM_OPCODE_RETURN;
  return shift(ctx, &expr, 1);
}

static WasmResult on_select_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_ANY);
  expr.stack_elements = 0;
  expr.opcode = WASM_OPCODE_SELECT;
  return shift(ctx, &expr, 3);
}

static WasmResult on_set_local_expr(uint32_t local_index, void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, get_local_index_type(ctx->current_func, local_index));
  expr.stack_elements = 1;
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
  init_type_expr(ctx, &expr.type, s_opcode_rtype[opcode]);
  expr.stack_elements = 1;
  expr.opcode = opcode;
  expr.store.mem_offset = offset;
  expr.store.alignment_log2 = alignment_log2;
  return shift(ctx, &expr, 2);
}

static WasmResult on_unreachable_expr(void* user_data) {
  WasmContext* ctx = user_data;
  WasmInterpreterExpr expr;
  init_type_expr(ctx, &expr.type, WASM_TYPE_ANY);
  expr.stack_elements = 0;
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
    .on_values_expr = &on_values_expr,
    .on_conc_values_expr = &on_conc_values_expr,
    .on_mv_call_expr = &on_mv_call_expr,
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
                                        WasmInterpreterModule* out_module) {
  WasmContext ctx;
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
  if (WASM_SUCCEEDED(result)) {
    wasm_steal_mem_writer_output_buffer(&ctx.istream_writer,
                                        &out_module->istream);
    out_module->istream.size = ctx.istream_offset;
  }
  destroy_context(&ctx);
  return result;
}

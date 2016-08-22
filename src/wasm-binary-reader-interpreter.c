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

#define CHECK_RESULT(expr) \
  do {                     \
    if (WASM_FAILED(expr)) \
      return WASM_ERROR;   \
  } while (0)

#define CHECK_DEPTH(ctx, depth)                                         \
  do {                                                                  \
    if ((depth) >= (ctx)->typecheck_label_stack.size) {                 \
      print_error((ctx), "invalid depth: %d (max %" PRIzd ")", (depth), \
                  ((ctx)->typecheck_label_stack.size));                 \
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

static const char* s_label_type_name[] = {
    "func", "block", "loop", "if", "else",
};

/* used for the typecheck pass */
typedef struct TypecheckLabel {
  LabelType label_type;
  WasmType type;
  uint32_t expr_stack_size;
  uint32_t expr_index;      /* the index of the starting op (block/loop/if) */
  uint32_t last_expr_index; /* last index of the true branch in an if */
} TypecheckLabel;
WASM_DEFINE_VECTOR(typecheck_label, TypecheckLabel);

/* used for the emit pass */
typedef struct EmitLabel {
  LabelType label_type;
  uint32_t offset; /* branch location in the istream */
  uint32_t fixup_offset;
  uint32_t value_stack_size;
  WasmBool has_value;
} EmitLabel;
WASM_DEFINE_VECTOR(emit_label, EmitLabel);

typedef struct ExprNode {
  uint32_t index;
  WasmType type;
} ExprNode;
WASM_DEFINE_VECTOR(expr_node, ExprNode);

typedef struct InterpreterFunc {
  uint32_t sig_index;
  uint32_t offset;
  uint32_t local_decl_count;
  uint32_t local_count;
  WasmTypeVector param_and_local_types;
} InterpreterFunc;
WASM_DEFINE_ARRAY(interpreter_func, InterpreterFunc);

typedef struct Context {
  WasmAllocator* allocator;
  WasmBinaryReader* reader;
  WasmBinaryErrorHandler* error_handler;
  WasmAllocator* memory_allocator;
  WasmInterpreterModule* module;
  InterpreterFuncArray funcs;
  InterpreterFunc* current_func;
  ExprNodeVector expr_stack;
  TypecheckLabelVector typecheck_label_stack;
  EmitLabelVector emit_label_stack;
  Uint32VectorVector func_fixups;
  Uint32VectorVector depth_fixups;
  Uint32Vector discarded_exprs; /* bitset */
  uint32_t value_stack_size;
  uint32_t depth;
  uint32_t expr_count;
  uint32_t start_func_index;
  WasmMemoryWriter istream_writer;
  uint32_t istream_offset;
} Context;

static TypecheckLabel* get_typecheck_label(Context* ctx, uint32_t depth) {
  assert(depth < ctx->typecheck_label_stack.size);
  return &ctx->typecheck_label_stack.data[depth];
}

static TypecheckLabel* top_typecheck_label(Context* ctx) {
  return get_typecheck_label(ctx, ctx->typecheck_label_stack.size - 1);
}

static uint32_t get_value_count(WasmType result_type) {
  return (result_type == WASM_TYPE_VOID || result_type == WASM_TYPE_ANY) ? 0
                                                                         : 1;
}

static WasmBool is_expr_discarded(Context* ctx, uint32_t expr_index) {
  uint32_t word_index = expr_index >> 5;
  if (word_index >= ctx->discarded_exprs.size)
    return WASM_FALSE;

  uint32_t bit_index = expr_index & 31;
  assert(word_index < ctx->discarded_exprs.size);
  return (ctx->discarded_exprs.data[word_index] & (1U << bit_index))
             ? WASM_TRUE
             : WASM_FALSE;
}

static void on_error(uint32_t offset, const char* message, void* user_data);

static void print_error(Context* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  on_error(WASM_INVALID_OFFSET, buffer, ctx);
}

static InterpreterFunc* get_func(Context* ctx, uint32_t func_index) {
  assert(func_index < ctx->funcs.size);
  return &ctx->funcs.data[func_index];
}

static WasmInterpreterImport* get_import(Context* ctx, uint32_t import_index) {
  assert(import_index < ctx->module->imports.size);
  return &ctx->module->imports.data[import_index];
}

static WasmInterpreterExport* get_export(Context* ctx, uint32_t export_index) {
  assert(export_index < ctx->module->exports.size);
  return &ctx->module->exports.data[export_index];
}

static WasmInterpreterFuncSignature* get_signature(Context* ctx,
                                                   uint32_t sig_index) {
  assert(sig_index < ctx->module->sigs.size);
  return &ctx->module->sigs.data[sig_index];
}

static WasmInterpreterFuncSignature* get_func_signature(Context* ctx,
                                                        InterpreterFunc* func) {
  return get_signature(ctx, func->sig_index);
}

static WasmType get_local_index_type(InterpreterFunc* func,
                                     uint32_t local_index) {
  assert(local_index < func->param_and_local_types.size);
  return func->param_and_local_types.data[local_index];
}

static uint32_t translate_depth(Context* ctx, size_t size, uint32_t depth) {
  assert(depth < size);
  return size - 1 - depth;
}

static uint32_t translate_local_index(Context* ctx, uint32_t local_index) {
  assert(local_index < ctx->value_stack_size);
  return ctx->value_stack_size - local_index;
}

void on_error(uint32_t offset, const char* message, void* user_data) {
  Context* ctx = user_data;
  if (ctx->error_handler->on_error) {
    ctx->error_handler->on_error(offset, message,
                                 ctx->error_handler->user_data);
  }
}

static WasmResult on_memory_initial_size_pages(uint32_t pages,
                                               void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterMemory* memory = &ctx->module->memory;
  memory->allocator = ctx->memory_allocator;
  memory->page_size = pages;
  memory->byte_size = pages * WASM_PAGE_SIZE;
  memory->data = wasm_alloc_zero(ctx->memory_allocator, memory->byte_size,
                                 WASM_DEFAULT_ALIGN);
  return WASM_OK;
}

static WasmResult on_data_segment(uint32_t index,
                                  uint32_t address,
                                  const void* src_data,
                                  uint32_t size,
                                  void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterMemory* memory = &ctx->module->memory;
  uint8_t* dst_data = memory->data;
  if ((uint64_t)address + (uint64_t)size > memory->byte_size)
    return WASM_ERROR;
  memcpy(&dst_data[address], src_data, size);
  return WASM_OK;
}

static WasmResult on_signature_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_new_interpreter_func_signature_array(ctx->allocator, &ctx->module->sigs,
                                            count);
  return WASM_OK;
}

static WasmResult on_signature(uint32_t index,
                               WasmType result_type,
                               uint32_t param_count,
                               WasmType* param_types,
                               void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterFuncSignature* sig = get_signature(ctx, index);
  sig->result_type = result_type;

  wasm_reserve_types(ctx->allocator, &sig->param_types, param_count);
  sig->param_types.size = param_count;
  memcpy(sig->param_types.data, param_types, param_count * sizeof(WasmType));
  return WASM_OK;
}

static WasmResult on_import_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_new_interpreter_import_array(ctx->allocator, &ctx->module->imports,
                                    count);
  return WASM_OK;
}

static WasmResult trapping_import_callback(
    WasmInterpreterModule* module,
    WasmInterpreterImport* import,
    uint32_t num_args,
    WasmInterpreterTypedValue* args,
    WasmInterpreterTypedValue* out_result,
    void* user_data) {
  return WASM_ERROR;
}

static WasmResult on_import(uint32_t index,
                            uint32_t sig_index,
                            WasmStringSlice module_name,
                            WasmStringSlice function_name,
                            void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterImport* import = &ctx->module->imports.data[index];
  import->module_name = wasm_dup_string_slice(ctx->allocator, module_name);
  import->func_name = wasm_dup_string_slice(ctx->allocator, function_name);
  assert(sig_index < ctx->module->sigs.size);
  import->sig_index = sig_index;
  import->callback = trapping_import_callback;
  import->user_data = NULL;
  return WASM_OK;
}

static WasmResult on_function_signatures_count(uint32_t count,
                                               void* user_data) {
  Context* ctx = user_data;
  wasm_new_interpreter_func_array(ctx->allocator, &ctx->funcs, count);
  wasm_resize_uint32_vector_vector(ctx->allocator, &ctx->func_fixups, count);
  return WASM_OK;
}

static WasmResult on_function_signature(uint32_t index,
                                        uint32_t sig_index,
                                        void* user_data) {
  Context* ctx = user_data;
  assert(sig_index < ctx->module->sigs.size);
  InterpreterFunc* func = get_func(ctx, index);
  func->offset = WASM_INVALID_OFFSET;
  func->sig_index = sig_index;
  return WASM_OK;
}

static WasmResult on_function_bodies_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  assert(count == ctx->funcs.size);
  WASM_USE(ctx);
  return WASM_OK;
}

/* defined below so they can reference s_binary_reader{,_emit} */
static WasmResult begin_function_body_pass(uint32_t index,
                                           uint32_t pass,
                                           void* user_data);
static WasmResult end_function_body_pass(uint32_t index,
                                         uint32_t pass,
                                         void* user_data);

/* Code generation happens per function in two passes. First, the function is
 * type-checked, bottom up. While this is done, a bitset ("discarded_exprs") is
 * maintained.
 *
 * There are three different but related meanings for a bit being set in
 * |discarded_exprs|:
 *
 * 1) if the operator is a block, loop or if: the bit is set if the block does
 * not use its value. This is used when emitting a br to determine whether its
 * value should be discarded.
 *
 * 2) if the operator is br_if: the bit is set if the br_if has a value. This
 * is used to determine whether the br_if value needs to be discarded if the
 * branch is not taken.
 *
 * 3) otherwise: the bit is set if the expression is subsequently discarded
 * (e.g. a set_local whose value is not used.)
 *
 * The second pass is the emit pass, where instructions are written to an
 * instruction stream, to be executed by the stack machine interpreter.
 *
 * Two passes are needed because you can't determine whether a value is used in
 * one pass. It would be possible to generate code in one pass, but it would
 * use a very large stack for blocks with many discarded expressions.
 */

/*** typecheck pass ***********************************************************/

static WasmResult type_mismatch(Context* ctx,
                                WasmType expected_type,
                                WasmType type,
                                const char* desc) {
  print_error(ctx, "type mismatch in %s, expected %s but got %s.", desc,
              s_type_names[expected_type], s_type_names[type]);
  return WASM_ERROR;
}

static WasmResult check_type(Context* ctx,
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

static WasmResult unify_and_check_type(Context* ctx,
                                       WasmType* dest_type,
                                       WasmType type,
                                       const char* desc) {
  unify_type(dest_type, type);
  return check_type(ctx, *dest_type, type, desc);
}

static uint32_t translate_typecheck_depth(Context* ctx, uint32_t depth) {
  return translate_depth(ctx, ctx->typecheck_label_stack.size, depth);
}

static void push_typecheck_label(Context* ctx,
                                 LabelType label_type,
                                 WasmType type) {
  TypecheckLabel* label =
      wasm_append_typecheck_label(ctx->allocator, &ctx->typecheck_label_stack);
  label->label_type = label_type;
  label->type = type;
  label->expr_stack_size = ctx->expr_stack.size;
  label->expr_index = ctx->expr_count;
  LOGF("   : +depth %" PRIzd ":%s\n", ctx->typecheck_label_stack.size - 1,
       s_type_names[type]);
}

static void pop_typecheck_label(Context* ctx) {
  LOGF("   : -depth %" PRIzd "\n", ctx->typecheck_label_stack.size - 1);
  assert(ctx->typecheck_label_stack.size > 0);
  ctx->typecheck_label_stack.size--;
}

static void push_expr(Context* ctx, WasmType type, WasmOpcode opcode) {
  LOGF("%3" PRIzd ": push %s:%s (#%u)\n", ctx->expr_stack.size,
       s_opcode_name[opcode], s_type_names[type], ctx->expr_count);
  ExprNode* expr;
  expr = wasm_append_expr_node(ctx->allocator, &ctx->expr_stack);
  expr->index = ctx->expr_count;
  expr->type = type;
  ctx->expr_count++;
}

static WasmType pop_expr(Context* ctx) {
  assert(ctx->expr_stack.size > 0);
  ExprNode* expr = &ctx->expr_stack.data[ctx->expr_stack.size - 1];

  LOGF("%3" PRIzd ": pop  %s (#%u)\n", ctx->expr_stack.size,
       s_type_names[expr->type], expr->index);

  WasmType type = expr->type;
  ctx->expr_stack.size--;
  return type;
}

static WasmType pop_expr_if(Context* ctx, WasmBool cond) {
  if (cond)
    return pop_expr(ctx);
  return WASM_TYPE_VOID;
}

static void set_expr_discarded(Context* ctx, uint32_t expr_index) {
  LOGF("   : set_expr_discarded #%u\n", expr_index);
  uint32_t word_index = expr_index >> 5;
  size_t new_size = word_index + 1;
  if (new_size > ctx->discarded_exprs.capacity)
    wasm_reserve_uint32s(ctx->allocator, &ctx->discarded_exprs, new_size);
  size_t old_size = ctx->discarded_exprs.size;
  if (new_size > old_size) {
    memset(&ctx->discarded_exprs.data[old_size], 0,
           (new_size - old_size) * sizeof(uint32_t));
    ctx->discarded_exprs.size = new_size;
  }

  uint32_t bit_index = expr_index & 31;
  ctx->discarded_exprs.data[word_index] |= 1U << bit_index;
}

static void set_expr_discarded_unless_void(Context* ctx,
                                                 uint32_t expr_index,
                                                 WasmType type) {
  if (type != WASM_TYPE_VOID)
    set_expr_discarded(ctx, expr_index);
}

static WasmResult begin_function_body(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  InterpreterFunc* func = get_func(ctx, index);
  WasmInterpreterFuncSignature* sig = get_signature(ctx, func->sig_index);

  ctx->current_func = func;
  ctx->expr_stack.size = 0;
  ctx->typecheck_label_stack.size = 0;
  ctx->discarded_exprs.size = 0;
  ctx->depth = 0;
  ctx->expr_count = 0;

  /* append param types */
  uint32_t i;
  for (i = 0; i < sig->param_types.size; ++i) {
    wasm_append_type_value(ctx->allocator, &func->param_and_local_types,
                           &sig->param_types.data[i]);
  }

  /* push implicit func label (equivalent to return) */
  push_typecheck_label(ctx, LABEL_TYPE_FUNC, sig->result_type);

  return WASM_OK;
}

static WasmResult end_function_body(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  uint32_t discard_max = 0;

  pop_typecheck_label(ctx);

  WasmInterpreterFuncSignature* sig =
      get_func_signature(ctx, ctx->current_func);
  if (sig->result_type == WASM_TYPE_VOID) {
    /* discard all expr */
    discard_max = ctx->expr_stack.size;
  } else if (ctx->expr_stack.size > 0) {
    /* discard everything except the last expr */
    discard_max = ctx->expr_stack.size - 1;
  } else {
    return WASM_ERROR;
  }

  uint32_t i;
  for (i = 0; i < discard_max; ++i) {
    ExprNode* expr = &ctx->expr_stack.data[i];
    set_expr_discarded_unless_void(ctx, expr->index, expr->type);
  }

  ctx->current_func = NULL;
  return WASM_OK;
}

static WasmResult on_local_decl(uint32_t decl_index,
                                uint32_t count,
                                WasmType type,
                                void* user_data) {
  Context* ctx = user_data;
  InterpreterFunc* func = ctx->current_func;

  uint32_t i;
  for (i = 0; i < count; ++i)
    wasm_append_type_value(ctx->allocator, &func->param_and_local_types, &type);
  return WASM_OK;
}

static WasmResult on_unary_expr(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  WasmType value = pop_expr(ctx);
  CHECK_RESULT(
      check_type(ctx, s_opcode_type1[opcode], value, s_opcode_name[opcode]));
  push_expr(ctx, s_opcode_rtype[opcode], opcode);
  return WASM_OK;
}

static WasmResult on_binary_expr(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  WasmType right = pop_expr(ctx);
  WasmType left = pop_expr(ctx);
  /* TODO use opcode name here */
  CHECK_RESULT(
      check_type(ctx, s_opcode_type1[opcode], left, s_opcode_name[opcode]));
  CHECK_RESULT(
      check_type(ctx, s_opcode_type2[opcode], right, s_opcode_name[opcode]));
  push_expr(ctx, s_opcode_rtype[opcode], opcode);
  return WASM_OK;
}

static WasmResult on_block_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": block (#%u)\n", ctx->expr_stack.size, ctx->expr_count);
  push_typecheck_label(ctx, LABEL_TYPE_BLOCK, WASM_TYPE_ANY);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_loop_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": loop (#%u)\n", ctx->expr_stack.size, ctx->expr_count);
  push_typecheck_label(ctx, LABEL_TYPE_LOOP, WASM_TYPE_VOID);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_if_expr(void* user_data) {
  Context* ctx = user_data;
  WasmType cond = pop_expr(ctx);
  CHECK_RESULT(check_type(ctx, WASM_TYPE_I32, cond, "if"));
  LOGF("%3" PRIzd ": if (#%u)\n", ctx->expr_stack.size, ctx->expr_count);
  push_typecheck_label(ctx, LABEL_TYPE_IF, WASM_TYPE_ANY);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_else_expr(void* user_data) {
  Context* ctx = user_data;
  TypecheckLabel* label = top_typecheck_label(ctx);
  if (!label || label->label_type != LABEL_TYPE_IF) {
    print_error(ctx, "unexpected else operator");
    return WASM_ERROR;
  }

  LOGF("%3" PRIzd ": else (#%u)\n", ctx->expr_stack.size, ctx->expr_count);

  if (label->expr_stack_size < ctx->expr_stack.size) {
    ExprNode* last_expr = &ctx->expr_stack.data[ctx->expr_stack.size - 1];
    label->last_expr_index = last_expr->index;
    CHECK_RESULT(unify_and_check_type(ctx, &label->type, last_expr->type,
                                      "if true branch"));

    /* discard everything except the last expr; we don't know if we'll need to
     * discard that one until after we process the false branch */
    uint32_t i;
    for (i = label->expr_stack_size; i < ctx->expr_stack.size - 1; ++i) {
      ExprNode* expr = &ctx->expr_stack.data[i];
      set_expr_discarded_unless_void(ctx, expr->index, expr->type);
    }
  }

  label->label_type = LABEL_TYPE_ELSE;
  ctx->expr_stack.size = label->expr_stack_size;
  return WASM_OK;
}

static WasmResult on_end_expr(void* user_data) {
  Context* ctx = user_data;
  TypecheckLabel* label = top_typecheck_label(ctx);
  if (!label) {
    print_error(ctx, "unexpected end operator");
    return WASM_ERROR;
  }

  if (label->label_type == LABEL_TYPE_IF)
    label->type = WASM_TYPE_VOID;

  if (label->expr_stack_size < ctx->expr_stack.size) {
    ExprNode* last_expr = &ctx->expr_stack.data[ctx->expr_stack.size - 1];
    WasmType old_type = label->type;
    CHECK_RESULT(unify_and_check_type(ctx, &label->type, last_expr->type,
                                      s_label_type_name[label->label_type]));
    if (label->label_type == LABEL_TYPE_ELSE &&
        get_value_count(old_type) > get_value_count(label->type)) {
      /* we unified the types between the true and false branches; the true
       * branch had a value and the false branch didn't, so we need to discard
       * the true branch's value */
      set_expr_discarded(ctx, label->last_expr_index);
    }

    /* discard everything except the last expr */
    uint32_t i;
    for (i = label->expr_stack_size; i < ctx->expr_stack.size - 1; ++i) {
      ExprNode* expr = &ctx->expr_stack.data[i];
      set_expr_discarded_unless_void(ctx, expr->index, expr->type);
    }

    if (label->type == WASM_TYPE_VOID) {
      /* discard the last child expr if this block/if/loop is void */
      set_expr_discarded_unless_void(ctx, last_expr->index, last_expr->type);
    }
  } else {
    /* the block/loop/if was empty */
    label->type = WASM_TYPE_VOID;
  }

  if (label->type == WASM_TYPE_VOID) {
    /* the "discarded" bit is overloaded for the initial block/loop/if
     * operators; in that case it means that there is no value expected to be
     * returned, so if a br operator yields a value, it should be immediately
     * discarded */
    set_expr_discarded(ctx, label->expr_index);
  }

  ctx->expr_stack.size = label->expr_stack_size;
  pop_typecheck_label(ctx);
  push_expr(ctx, label->type, WASM_OPCODE_END);
  return WASM_OK;
}

static WasmResult on_br_expr(uint8_t arity, uint32_t depth, void* user_data) {
  Context* ctx = user_data;
  WasmType value = pop_expr_if(ctx, arity == 1);
  CHECK_DEPTH(ctx, depth);
  depth = translate_typecheck_depth(ctx, depth);
  TypecheckLabel* label = get_typecheck_label(ctx, depth);
  CHECK_RESULT(unify_and_check_type(ctx, &label->type, value, "br"));
  push_expr(ctx, WASM_TYPE_ANY, WASM_OPCODE_BR);
  return WASM_OK;
}

static WasmResult on_br_if_expr(uint8_t arity,
                                uint32_t depth,
                                void* user_data) {
  Context* ctx = user_data;
  WasmType cond = pop_expr(ctx);
  WasmType value = pop_expr_if(ctx, arity == 1);
  CHECK_DEPTH(ctx, depth);
  depth = translate_typecheck_depth(ctx, depth);
  TypecheckLabel* label = get_typecheck_label(ctx, depth);
  CHECK_RESULT(unify_and_check_type(ctx, &label->type, value, "br_if"));
  CHECK_RESULT(check_type(ctx, WASM_TYPE_I32, cond, "br_if"));
  /* the "discarded" bit is overloaded for br_if; if set, the br_if value is
   * non-void; i.e. should be discarded if the branch is not taken */
  set_expr_discarded_unless_void(ctx, ctx->expr_count, value);
  push_expr(ctx, WASM_TYPE_VOID, WASM_OPCODE_BR_IF);
  return WASM_OK;
}

static WasmResult on_br_table_expr(uint8_t arity,
                                   uint32_t num_targets,
                                   uint32_t* target_depths,
                                   uint32_t default_target_depth,
                                   void* user_data) {
  Context* ctx = user_data;
  WasmType key = pop_expr(ctx);
  WasmType value = pop_expr_if(ctx, arity == 1);
  CHECK_RESULT(check_type(ctx, WASM_TYPE_I32, key, "br_table"));

  uint32_t i;
  for (i = 0; i <= num_targets; ++i) {
    uint32_t depth = i != num_targets ? target_depths[i] : default_target_depth;
    uint32_t translated_depth = translate_typecheck_depth(ctx, depth);
    TypecheckLabel* label = get_typecheck_label(ctx, translated_depth);
    CHECK_RESULT(unify_and_check_type(ctx, &label->type, value, "br_table"));
  }

  push_expr(ctx, WASM_TYPE_ANY, WASM_OPCODE_BR_TABLE);
  return WASM_OK;
}

static WasmResult on_call_expr(uint32_t func_index, void* user_data) {
  Context* ctx = user_data;
  assert(func_index < ctx->funcs.size);
  InterpreterFunc* func = get_func(ctx, func_index);
  WasmInterpreterFuncSignature* sig = get_func_signature(ctx, func);

  uint32_t i;
  for (i = sig->param_types.size; i > 0; --i) {
    WasmType arg = pop_expr(ctx);
    CHECK_RESULT(check_type(ctx, sig->param_types.data[i - 1], arg, "call"));
  }

  push_expr(ctx, sig->result_type, WASM_OPCODE_CALL_FUNCTION);
  return WASM_OK;
}

static WasmResult on_call_import_expr(uint32_t import_index, void* user_data) {
  Context* ctx = user_data;
  assert(import_index < ctx->module->imports.size);
  WasmInterpreterImport* import = get_import(ctx, import_index);
  WasmInterpreterFuncSignature* sig = get_signature(ctx, import->sig_index);

  uint32_t i;
  for (i = sig->param_types.size; i > 0; --i) {
    WasmType arg = pop_expr(ctx);
    CHECK_RESULT(
        check_type(ctx, sig->param_types.data[i - 1], arg, "call_import"));
  }

  push_expr(ctx, sig->result_type, WASM_OPCODE_CALL_IMPORT);
  return WASM_OK;
}

static WasmResult on_call_indirect_expr(uint32_t sig_index, void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterFuncSignature* sig = get_signature(ctx, sig_index);

  uint32_t i;
  for (i = sig->param_types.size; i > 0; --i) {
    WasmType arg = pop_expr(ctx);
    CHECK_RESULT(
        check_type(ctx, sig->param_types.data[i - 1], arg, "call_indirect"));
  }

  WasmType entry_index = pop_expr(ctx);
  CHECK_RESULT(check_type(ctx, WASM_TYPE_I32, entry_index, "call_indirect"));
  push_expr(ctx, sig->result_type, WASM_OPCODE_CALL_INDIRECT);
  return WASM_OK;
}

static WasmResult on_drop_expr(void* user_data) {
  Context* ctx = user_data;
  pop_expr(ctx);
  return WASM_OK;
}

static WasmResult on_i32_const_expr(uint32_t value, void* user_data) {
  Context* ctx = user_data;
  push_expr(ctx, WASM_TYPE_I32, WASM_OPCODE_I32_CONST);
  return WASM_OK;
}

static WasmResult on_i64_const_expr(uint64_t value, void* user_data) {
  Context* ctx = user_data;
  push_expr(ctx, WASM_TYPE_I64, WASM_OPCODE_I64_CONST);
  return WASM_OK;
}

static WasmResult on_f32_const_expr(uint32_t value_bits, void* user_data) {
  Context* ctx = user_data;
  push_expr(ctx, WASM_TYPE_F32, WASM_OPCODE_F32_CONST);
  return WASM_OK;
}

static WasmResult on_f64_const_expr(uint64_t value_bits, void* user_data) {
  Context* ctx = user_data;
  push_expr(ctx, WASM_TYPE_F64, WASM_OPCODE_F64_CONST);
  return WASM_OK;
}

static WasmResult on_get_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_LOCAL(ctx, local_index);
  WasmType type = get_local_index_type(ctx->current_func, local_index);
  push_expr(ctx, type, WASM_OPCODE_GET_LOCAL);
  return WASM_OK;
}

static WasmResult on_set_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_LOCAL(ctx, local_index);
  WasmType type = get_local_index_type(ctx->current_func, local_index);
  WasmType value = pop_expr(ctx);
  CHECK_RESULT(check_type(ctx, type, value, "set_local"));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_tee_local_expr(uint32_t local_index, void* user_data) {
  Context* ctx = user_data;
  CHECK_LOCAL(ctx, local_index);
  WasmType type = get_local_index_type(ctx->current_func, local_index);
  WasmType value = pop_expr(ctx);
  CHECK_RESULT(check_type(ctx, type, value, "tee_local"));
  push_expr(ctx, type, WASM_OPCODE_TEE_LOCAL);
  return WASM_OK;
}

static WasmResult on_grow_memory_expr(void* user_data) {
  Context* ctx = user_data;
  WasmType value = pop_expr(ctx);
  CHECK_RESULT(check_type(ctx, WASM_TYPE_I32, value, "grow_memory"));
  push_expr(ctx, WASM_TYPE_I32, WASM_OPCODE_GROW_MEMORY);
  return WASM_OK;
}

static WasmResult on_load_expr(WasmOpcode opcode,
                               uint32_t alignment_log2,
                               uint32_t offset,
                               void* user_data) {
  Context* ctx = user_data;
  WasmType addr = pop_expr(ctx);
  CHECK_RESULT(check_type(ctx, WASM_TYPE_I32, addr, s_opcode_name[opcode]));
  push_expr(ctx, s_opcode_rtype[opcode], opcode);
  return WASM_OK;
}

static WasmResult on_store_expr(WasmOpcode opcode,
                                uint32_t alignment_log2,
                                uint32_t offset,
                                void* user_data) {
  Context* ctx = user_data;
  WasmType value = pop_expr(ctx);
  WasmType addr = pop_expr(ctx);
  WasmType type = s_opcode_rtype[opcode];
  CHECK_RESULT(check_type(ctx, WASM_TYPE_I32, addr, s_opcode_name[opcode]));
  CHECK_RESULT(check_type(ctx, type, value, s_opcode_name[opcode]));
  push_expr(ctx, type, opcode);
  return WASM_OK;
}

static WasmResult on_current_memory_expr(void* user_data) {
  Context* ctx = user_data;
  push_expr(ctx, WASM_TYPE_I32, WASM_OPCODE_CURRENT_MEMORY);
  return WASM_OK;
}

static WasmResult on_nop_expr(void* user_data) {
  Context* ctx = user_data;
  push_expr(ctx, WASM_TYPE_VOID, WASM_OPCODE_NOP);
  return WASM_OK;
}

static WasmResult on_return_expr(void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterFuncSignature* sig =
      get_func_signature(ctx, ctx->current_func);
  if (get_value_count(sig->result_type)) {
    WasmType value = pop_expr(ctx);
    CHECK_RESULT(check_type(ctx, sig->result_type, value, "return"));
  }
  push_expr(ctx, WASM_TYPE_ANY, WASM_OPCODE_RETURN);
  return WASM_OK;
}

static WasmResult on_select_expr(void* user_data) {
  Context* ctx = user_data;
  WasmType cond = pop_expr(ctx);
  WasmType right = pop_expr(ctx);
  WasmType left = pop_expr(ctx);
  WasmType type = WASM_TYPE_ANY;
  CHECK_RESULT(unify_and_check_type(ctx, &type, left, "select"));
  CHECK_RESULT(unify_and_check_type(ctx, &type, right, "select"));
  CHECK_RESULT(check_type(ctx, WASM_TYPE_I32, cond, "select"));
  push_expr(ctx, type, WASM_OPCODE_SELECT);
  return WASM_OK;
}

static WasmResult on_unreachable_expr(void* user_data) {
  Context* ctx = user_data;
  push_expr(ctx, WASM_TYPE_ANY, WASM_OPCODE_UNREACHABLE);
  return WASM_OK;
}

/*** emit pass ****************************************************************/

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

static void adjust_value_stack(Context* ctx, int32_t amount) {
  uint32_t old_size = ctx->value_stack_size;
  uint32_t new_size = old_size + (uint32_t)amount;
  assert((amount <= 0 && new_size <= old_size) ||
         (amount > 0 && new_size > old_size));
  WASM_USE(old_size);
  WASM_USE(new_size);
  ctx->value_stack_size += (uint32_t)amount;
#ifndef NDEBUG
  if (ctx->emit_label_stack.size > 0) {
    assert(ctx->value_stack_size >=
           ctx->emit_label_stack.data[ctx->emit_label_stack.size - 1]
               .value_stack_size);
  } else {
    assert(ctx->value_stack_size >=
           ctx->current_func->param_and_local_types.size);
  }
#endif
}

static void adjust_value_stack_for_nonlocal_continuation(Context* ctx) {
  /* adjust stack up; these operations type-check as ANY, so they can be used in
   * any operation. No value will actually be pushed, and the expressions that
   * use the result won't ever be executed. But it will make the stack the
   * "normal" size, so we won't have to special case it anywhere else. */
  adjust_value_stack(ctx, is_expr_discarded(ctx, ctx->expr_count) ? 0 : 1);
}

static EmitLabel* get_emit_label(Context* ctx, uint32_t depth) {
  assert(depth < ctx->emit_label_stack.size);
  return &ctx->emit_label_stack.data[depth];
}

static EmitLabel* top_emit_label(Context* ctx) {
  return get_emit_label(ctx, ctx->emit_label_stack.size - 1);
}

static uint32_t translate_emit_depth(Context* ctx, uint32_t depth) {
  return translate_depth(ctx, ctx->emit_label_stack.size, depth);
}

static void push_emit_label(Context* ctx,
                            LabelType label_type,
                            uint32_t offset,
                            uint32_t fixup_offset,
                            WasmBool has_value) {
  EmitLabel* label =
      wasm_append_emit_label(ctx->allocator, &ctx->emit_label_stack);
  label->label_type = label_type;
  label->offset = offset;
  label->value_stack_size = ctx->value_stack_size;
  label->has_value = has_value;
  label->fixup_offset = fixup_offset;
  LOGF("   : +depth %" PRIzd "\n", ctx->emit_label_stack.size - 1);
}

static void pop_emit_label(Context* ctx) {
  LOGF("   : -depth %" PRIzd "\n", ctx->emit_label_stack.size - 1);
  assert(ctx->emit_label_stack.size > 0);
  ctx->emit_label_stack.size--;
  /* reduce the depth_fixups stack as well, but it may be smaller than
   * emit_label_stack so only do it conditionally. */
  if (ctx->depth_fixups.size > ctx->emit_label_stack.size) {
    uint32_t from = ctx->emit_label_stack.size;
    uint32_t to = ctx->depth_fixups.size;
    uint32_t i;
    for (i = from; i < to; ++i)
      wasm_destroy_uint32_vector(ctx->allocator, &ctx->depth_fixups.data[i]);
    ctx->depth_fixups.size = ctx->emit_label_stack.size;
  }
}

static WasmResult fixup_top_emit_label(Context* ctx, uint32_t offset) {
  uint32_t top = ctx->emit_label_stack.size - 1;
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

static WasmResult emit_discard(Context* ctx) {
  LOGF("%3" PRIzd ": discard\n", ctx->value_stack_size);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DISCARD));
  adjust_value_stack(ctx, -1);
  return WASM_OK;
}

static WasmResult maybe_emit_discard(Context* ctx, uint32_t expr_index) {
  WasmBool should_discard = is_expr_discarded(ctx, expr_index);
  if (should_discard)
    return emit_discard(ctx);
  return WASM_OK;
}

static WasmResult emit_discard_keep(Context* ctx,
                                    uint32_t discard,
                                    uint8_t keep) {
  assert(discard != UINT32_MAX);
  assert(keep <= 1);
  if (discard > 0) {
    if (discard == 1 && keep == 0) {
      LOGF("%3" PRIzd ": discard\n", ctx->value_stack_size);
      CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DISCARD));
    } else {
      LOGF("%3" PRIzd ": discard_keep %u %u\n", ctx->value_stack_size, discard,
           keep);
      CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DISCARD_KEEP));
      CHECK_RESULT(emit_i32(ctx, discard));
      CHECK_RESULT(emit_i8(ctx, keep));
    }
  }
  return WASM_OK;
}

static WasmResult emit_return(Context* ctx, WasmType result_type) {
  uint32_t keep_count = get_value_count(result_type);
  uint32_t discard_count = ctx->value_stack_size - keep_count;
  CHECK_RESULT(emit_discard_keep(ctx, discard_count, keep_count));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_RETURN));
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

static WasmResult emit_br(Context* ctx, uint32_t depth) {
  EmitLabel* label = get_emit_label(ctx, depth);
  assert(ctx->value_stack_size >= label->value_stack_size);
  uint8_t keep_count = label->has_value ? 1 : 0;
  uint32_t discard_count =
      (ctx->value_stack_size - label->value_stack_size) - keep_count;
  CHECK_RESULT(emit_discard_keep(ctx, discard_count, keep_count));
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR));
  CHECK_RESULT(emit_br_offset(ctx, depth, label->offset));
  return WASM_OK;
}

static WasmResult emit_br_table_offset(Context* ctx, uint32_t depth) {
  EmitLabel* label = get_emit_label(ctx, depth);
  uint8_t keep_count = label->has_value ? 1 : 0;
  uint32_t discard_count =
      (ctx->value_stack_size - label->value_stack_size) - keep_count;
  CHECK_RESULT(emit_br_offset(ctx, depth, label->offset));
  CHECK_RESULT(emit_i32(ctx, discard_count));
  CHECK_RESULT(emit_i8(ctx, keep_count));
  return WASM_OK;
}

static WasmResult emit_func_offset(Context* ctx,
                                   InterpreterFunc* func,
                                   uint32_t func_index) {
  if (func->offset == WASM_INVALID_OFFSET)
    CHECK_RESULT(append_fixup(ctx, &ctx->func_fixups, func_index));
  CHECK_RESULT(emit_i32(ctx, func->offset));
  return WASM_OK;
}

static WasmResult begin_emit_function_body(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  InterpreterFunc* func = get_func(ctx, index);
  WasmInterpreterFuncSignature* sig = get_signature(ctx, func->sig_index);

  func->offset = get_istream_offset(ctx);
  func->local_decl_count = 0;
  func->local_count = 0;

  ctx->current_func = func;
  ctx->emit_label_stack.size = 0;
  ctx->depth_fixups.size = 0;
  ctx->value_stack_size = sig->param_types.size;
  ctx->depth = 0;
  ctx->expr_count = 0;

  /* fixup function references */
  uint32_t i;
  Uint32Vector* fixups = &ctx->func_fixups.data[index];
  for (i = 0; i < fixups->size; ++i)
    CHECK_RESULT(emit_i32_at(ctx, fixups->data[i], func->offset));

  push_emit_label(ctx, LABEL_TYPE_FUNC, WASM_INVALID_OFFSET, 0,
                  sig->result_type != WASM_TYPE_VOID);

  return WASM_OK;
}

static WasmResult end_emit_function_body(uint32_t index, void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterFuncSignature* sig =
      get_func_signature(ctx, ctx->current_func);
  fixup_top_emit_label(ctx, get_istream_offset(ctx));
  EmitLabel* label = top_emit_label(ctx);
  ctx->value_stack_size = label->value_stack_size;
  adjust_value_stack(ctx, label->has_value ? 1 : 0);
  CHECK_RESULT(emit_return(ctx, sig->result_type));
  pop_emit_label(ctx);
  ctx->current_func = NULL;
  ctx->value_stack_size = 0;
  return WASM_OK;
}

static WasmResult on_emit_local_decl_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  ctx->current_func->local_decl_count = count;
  return WASM_OK;
}

static WasmResult on_emit_local_decl(uint32_t decl_index,
                                     uint32_t count,
                                     WasmType type,
                                     void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_ALLOCA]);
  InterpreterFunc* func = ctx->current_func;
  func->local_count += count;

  if (decl_index == func->local_decl_count - 1) {
    /* last local declaration, allocate space for all locals. */
    CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_ALLOCA));
    CHECK_RESULT(emit_i32(ctx, func->local_count));
    adjust_value_stack(ctx, func->local_count);

    /* adjust the "return" label to include the locals in the value stack; this
     * way they'll be discarded when the function returns. */
    EmitLabel* label = top_emit_label(ctx);
    assert(label->label_type == LABEL_TYPE_FUNC);
    label->value_stack_size += func->local_count;
  }
  return WASM_OK;
}

static WasmResult on_emit_unary_expr(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size, s_opcode_name[opcode]);
  CHECK_RESULT(emit_opcode(ctx, opcode));
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_binary_expr(WasmOpcode opcode, void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size, s_opcode_name[opcode]);
  adjust_value_stack(ctx, -1);
  CHECK_RESULT(emit_opcode(ctx, opcode));
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_block_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_BLOCK]);
  push_emit_label(ctx, LABEL_TYPE_BLOCK, WASM_INVALID_OFFSET, 0,
                  !is_expr_discarded(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_loop_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_LOOP]);
  push_emit_label(ctx, LABEL_TYPE_LOOP, get_istream_offset(ctx), 0, WASM_FALSE);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_if_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_IF]);

  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_UNLESS));
  adjust_value_stack(ctx, -1);
  uint32_t fixup_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));

  push_emit_label(ctx, LABEL_TYPE_IF, WASM_INVALID_OFFSET, fixup_offset,
                  !is_expr_discarded(ctx, ctx->expr_count));

  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_else_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_ELSE]);
  EmitLabel* label = top_emit_label(ctx);
  assert(label->label_type == LABEL_TYPE_IF);
  label->label_type = LABEL_TYPE_ELSE;
  uint32_t fixup_cond_offset = label->fixup_offset;
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR));
  label->fixup_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
  CHECK_RESULT(emit_i32_at(ctx, fixup_cond_offset, get_istream_offset(ctx)));
  /* reset the value stack for the other branch arm */
  ctx->value_stack_size = label->value_stack_size;
  return WASM_OK;
}

static WasmResult on_emit_end_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_END]);
  EmitLabel* label = top_emit_label(ctx);
  switch (label->label_type) {
    case LABEL_TYPE_LOOP:
      break;

    case LABEL_TYPE_IF:
    case LABEL_TYPE_ELSE: {
      uint32_t fixup_true_offset = label->fixup_offset;
      CHECK_RESULT(
          emit_i32_at(ctx, fixup_true_offset, get_istream_offset(ctx)));
      break;
    }

    default:
      break;
  }
  fixup_top_emit_label(ctx, get_istream_offset(ctx));
  ctx->value_stack_size = label->value_stack_size;
  adjust_value_stack(ctx, label->has_value ? 1 : 0);
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  pop_emit_label(ctx);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_br_expr(uint8_t arity,
                                  uint32_t depth,
                                  void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_BR]);
  CHECK_RESULT(emit_br(ctx, translate_emit_depth(ctx, depth)));
  adjust_value_stack_for_nonlocal_continuation(ctx);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_br_if_expr(uint8_t arity,
                                     uint32_t depth,
                                     void* user_data) {
  /* flip the br_if so if <cond> is true it can discard values from the stack */
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_BR_IF]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_UNLESS));
  adjust_value_stack(ctx, -1); /* account for br_unless consuming <cond> */
  uint32_t fixup_br_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
  CHECK_RESULT(emit_br(ctx, translate_emit_depth(ctx, depth)));
  CHECK_RESULT(emit_i32_at(ctx, fixup_br_offset, get_istream_offset(ctx)));
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_br_table_expr(uint8_t arity,
                                        uint32_t num_targets,
                                        uint32_t* target_depths,
                                        uint32_t default_target_depth,
                                        void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_BR_TABLE]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_BR_TABLE));
  CHECK_RESULT(emit_i32(ctx, num_targets));
  uint32_t fixup_table_offset = get_istream_offset(ctx);
  CHECK_RESULT(emit_i32(ctx, WASM_INVALID_OFFSET));
  adjust_value_stack(ctx, -1);

  /* not necessary for the interpreter, but it makes it easier to disassemble.
   * This opcode specifies how many bytes of data follow. */
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_DATA));
  CHECK_RESULT(emit_i32(ctx, (num_targets + 1) * WASM_TABLE_ENTRY_SIZE));
  CHECK_RESULT(emit_i32_at(ctx, fixup_table_offset, get_istream_offset(ctx)));

  /* write the branch table as (offset, discard count) pairs */
  uint32_t i;
  for (i = 0; i <= num_targets; ++i) {
    uint32_t depth = i != num_targets ? target_depths[i] : default_target_depth;
    CHECK_RESULT(emit_br_table_offset(ctx, translate_emit_depth(ctx, depth)));
  }

  adjust_value_stack_for_nonlocal_continuation(ctx);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_call_expr(uint32_t func_index, void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_CALL_FUNCTION]);
  InterpreterFunc* func = get_func(ctx, func_index);
  WasmInterpreterFuncSignature* sig = get_func_signature(ctx, func);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_FUNCTION));
  CHECK_RESULT(emit_func_offset(ctx, func, func_index));
  adjust_value_stack(ctx,
                     get_value_count(sig->result_type) - sig->param_types.size);
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_call_import_expr(uint32_t import_index,
                                           void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_CALL_IMPORT]);
  WasmInterpreterImport* import = get_import(ctx, import_index);
  WasmInterpreterFuncSignature* sig = get_signature(ctx, import->sig_index);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_IMPORT));
  CHECK_RESULT(emit_i32(ctx, import_index));
  adjust_value_stack(ctx,
                     get_value_count(sig->result_type) - sig->param_types.size);
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_call_indirect_expr(uint32_t sig_index,
                                             void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_CALL_INDIRECT]);
  WasmInterpreterFuncSignature* sig = get_signature(ctx, sig_index);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CALL_INDIRECT));
  CHECK_RESULT(emit_i32(ctx, sig_index));
  uint32_t result_count = get_value_count(sig->result_type);
  /* the callee cleans up the params for us, but we have to clean up the
   * function table index */
  adjust_value_stack(ctx, result_count - sig->param_types.size);
  CHECK_RESULT(emit_discard_keep(ctx, 1, result_count));
  adjust_value_stack(ctx, -1);
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_drop_expr(void *user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_DROP]);
  CHECK_RESULT(emit_discard(ctx));
  return WASM_OK;
}

static WasmResult on_emit_i32_const_expr(uint32_t value, void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_I32_CONST]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I32_CONST));
  CHECK_RESULT(emit_i32(ctx, value));
  adjust_value_stack(ctx, 1);
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_i64_const_expr(uint64_t value, void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_I64_CONST]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_I64_CONST));
  CHECK_RESULT(emit_i64(ctx, value));
  adjust_value_stack(ctx, 1);
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_f32_const_expr(uint32_t value_bits, void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_F32_CONST]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_F32_CONST));
  CHECK_RESULT(emit_i32(ctx, value_bits));
  adjust_value_stack(ctx, 1);
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_f64_const_expr(uint64_t value_bits, void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_F64_CONST]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_F64_CONST));
  CHECK_RESULT(emit_i64(ctx, value_bits));
  adjust_value_stack(ctx, 1);
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_get_local_expr(uint32_t local_index,
                                         void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_GET_LOCAL]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_GET_LOCAL));
  CHECK_RESULT(emit_i32(ctx, translate_local_index(ctx, local_index)));
  adjust_value_stack(ctx, 1);
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_set_local_expr(uint32_t local_index,
                                         void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_SET_LOCAL]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_SET_LOCAL));
  CHECK_RESULT(emit_i32(ctx, translate_local_index(ctx, local_index)));
  adjust_value_stack(ctx, -1);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_tee_local_expr(uint32_t local_index,
                                         void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_TEE_LOCAL]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_TEE_LOCAL));
  CHECK_RESULT(emit_i32(ctx, translate_local_index(ctx, local_index)));
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_grow_memory_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_GROW_MEMORY]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_GROW_MEMORY));
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_load_expr(WasmOpcode opcode,
                                    uint32_t alignment_log2,
                                    uint32_t offset,
                                    void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size, s_opcode_name[opcode]);
  CHECK_RESULT(emit_opcode(ctx, opcode));
  CHECK_RESULT(emit_i32(ctx, offset));
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_store_expr(WasmOpcode opcode,
                                     uint32_t alignment_log2,
                                     uint32_t offset,
                                     void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size, s_opcode_name[opcode]);
  CHECK_RESULT(emit_opcode(ctx, opcode));
  CHECK_RESULT(emit_i32(ctx, offset));
  adjust_value_stack(ctx, -1);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_current_memory_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_CURRENT_MEMORY]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_CURRENT_MEMORY));
  adjust_value_stack(ctx, 1);
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_nop_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_NOP]);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_return_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_RETURN]);
  WasmInterpreterFuncSignature* sig =
      get_func_signature(ctx, ctx->current_func);
  CHECK_RESULT(emit_return(ctx, sig->result_type));
  adjust_value_stack_for_nonlocal_continuation(ctx);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_select_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_SELECT]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_SELECT));
  CHECK_RESULT(maybe_emit_discard(ctx, ctx->expr_count));
  adjust_value_stack(ctx, -2);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_emit_unreachable_expr(void* user_data) {
  Context* ctx = user_data;
  LOGF("%3" PRIzd ": %s\n", ctx->value_stack_size,
       s_opcode_name[WASM_OPCODE_UNREACHABLE]);
  CHECK_RESULT(emit_opcode(ctx, WASM_OPCODE_UNREACHABLE));
  adjust_value_stack_for_nonlocal_continuation(ctx);
  ctx->expr_count++;
  return WASM_OK;
}

static WasmResult on_function_table_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_new_interpreter_func_table_entry_array(ctx->allocator,
                                              &ctx->module->func_table, count);
  return WASM_OK;
}

static WasmResult on_function_table_entry(uint32_t index,
                                          uint32_t func_index,
                                          void* user_data) {
  Context* ctx = user_data;
  assert(index < ctx->module->func_table.size);
  WasmInterpreterFuncTableEntry* entry = &ctx->module->func_table.data[index];
  InterpreterFunc* func = get_func(ctx, func_index);
  entry->sig_index = func->sig_index;
  /* the function offset isn't known yet, so temporarily store the func index
   * in func_offset and resolve after the last function body */
  entry->func_offset = func_index;
  return WASM_OK;
}

static WasmResult on_start_function(uint32_t func_index, void* user_data) {
  Context* ctx = user_data;
  /* can't get the function offset yet, because we haven't parsed the
   * functions. Just store the function index and resolve it later in
   * end_function_bodies_section. */
  assert(func_index < ctx->funcs.size);
  ctx->start_func_index = func_index;
  return WASM_OK;
}

static WasmResult on_export_count(uint32_t count, void* user_data) {
  Context* ctx = user_data;
  wasm_new_interpreter_export_array(ctx->allocator, &ctx->module->exports,
                                    count);
  return WASM_OK;
}

static WasmResult on_export(uint32_t index,
                            uint32_t func_index,
                            WasmStringSlice name,
                            void* user_data) {
  Context* ctx = user_data;
  WasmInterpreterExport* export = &ctx->module->exports.data[index];
  InterpreterFunc* func = get_func(ctx, func_index);
  export->name = wasm_dup_string_slice(ctx->allocator, name);
  export->func_index = func_index;
  export->sig_index = func->sig_index;
  export->func_offset = WASM_INVALID_OFFSET;
  return WASM_OK;
}

static WasmResult end_function_bodies_section(void* user_data) {
  Context* ctx = user_data;

  /* resolve the start function offset */
  if (ctx->start_func_index != INVALID_FUNC_INDEX) {
    InterpreterFunc* func = get_func(ctx, ctx->start_func_index);
    assert(func->offset != WASM_INVALID_OFFSET);
    ctx->module->start_func_offset = func->offset;
  }

  /* resolve the export function offsets */
  uint32_t i;
  for (i = 0; i < ctx->module->exports.size; ++i) {
    WasmInterpreterExport* export = get_export(ctx, i);
    InterpreterFunc* func = get_func(ctx, export->func_index);
    export->func_offset = func->offset;
  }

  /* resolve the function table entry offsets */
  for (i = 0; i < ctx->module->func_table.size; ++i) {
    WasmInterpreterFuncTableEntry* entry = &ctx->module->func_table.data[i];
    /* function index is stored in func_offset temporarily */
    InterpreterFunc* func = get_func(ctx, entry->func_offset);
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
    .begin_function_body_pass = &begin_function_body_pass,
    .begin_function_body = &begin_function_body,
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
    .on_convert_expr = &on_unary_expr,
    .on_current_memory_expr = &on_current_memory_expr,
    .on_drop_expr = &on_drop_expr,
    .on_else_expr = &on_else_expr,
    .on_end_expr = &on_end_expr,
    .on_f32_const_expr = &on_f32_const_expr,
    .on_f64_const_expr = &on_f64_const_expr,
    .on_get_local_expr = &on_get_local_expr,
    .on_grow_memory_expr = &on_grow_memory_expr,
    .on_i32_const_expr = &on_i32_const_expr,
    .on_i64_const_expr = &on_i64_const_expr,
    .on_if_expr = &on_if_expr,
    .on_load_expr = &on_load_expr,
    .on_loop_expr = &on_loop_expr,
    .on_nop_expr = &on_nop_expr,
    .on_return_expr = &on_return_expr,
    .on_select_expr = &on_select_expr,
    .on_set_local_expr = &on_set_local_expr,
    .on_store_expr = &on_store_expr,
    .on_tee_local_expr = &on_tee_local_expr,
    .on_unary_expr = &on_unary_expr,
    .on_unreachable_expr = &on_unreachable_expr,
    .end_function_body = &end_function_body,
    .end_function_bodies_section = &end_function_bodies_section,
    .end_function_body_pass = &end_function_body_pass,

    .on_function_table_count = &on_function_table_count,
    .on_function_table_entry = &on_function_table_entry,

    .on_start_function = &on_start_function,

    .on_export_count = &on_export_count,
    .on_export = &on_export,
};

static WasmBinaryReader s_binary_reader_emit = {
    .on_error = &on_error,
    .begin_function_body_pass = &begin_function_body_pass,
    .begin_function_body = &begin_emit_function_body,
    .on_local_decl_count = &on_emit_local_decl_count,
    .on_local_decl = &on_emit_local_decl,
    .on_binary_expr = &on_emit_binary_expr,
    .on_block_expr = &on_emit_block_expr,
    .on_br_expr = &on_emit_br_expr,
    .on_br_if_expr = &on_emit_br_if_expr,
    .on_br_table_expr = &on_emit_br_table_expr,
    .on_call_expr = &on_emit_call_expr,
    .on_call_import_expr = &on_emit_call_import_expr,
    .on_call_indirect_expr = &on_emit_call_indirect_expr,
    .on_compare_expr = &on_emit_binary_expr,
    .on_convert_expr = &on_emit_unary_expr,
    .on_current_memory_expr = &on_emit_current_memory_expr,
    .on_drop_expr = &on_emit_drop_expr,
    .on_else_expr = &on_emit_else_expr,
    .on_end_expr = &on_emit_end_expr,
    .on_f32_const_expr = &on_emit_f32_const_expr,
    .on_f64_const_expr = &on_emit_f64_const_expr,
    .on_get_local_expr = &on_emit_get_local_expr,
    .on_grow_memory_expr = &on_emit_grow_memory_expr,
    .on_i32_const_expr = &on_emit_i32_const_expr,
    .on_i64_const_expr = &on_emit_i64_const_expr,
    .on_if_expr = &on_emit_if_expr,
    .on_load_expr = &on_emit_load_expr,
    .on_loop_expr = &on_emit_loop_expr,
    .on_nop_expr = &on_emit_nop_expr,
    .on_return_expr = &on_emit_return_expr,
    .on_select_expr = &on_emit_select_expr,
    .on_set_local_expr = &on_emit_set_local_expr,
    .on_store_expr = &on_emit_store_expr,
    .on_tee_local_expr = &on_emit_tee_local_expr,
    .on_unary_expr = &on_emit_unary_expr,
    .on_unreachable_expr = &on_emit_unreachable_expr,
    .end_function_body = &end_emit_function_body,
    .end_function_body_pass = &end_function_body_pass,
};

static WasmResult begin_function_body_pass(uint32_t index,
                                           uint32_t pass,
                                           void* user_data) {
  LOGF("*** func %d pass %d ***\n", index, pass);
  Context* ctx = user_data;
  assert(pass < 2);
  *ctx->reader = pass == 0 ? s_binary_reader : s_binary_reader_emit;
  ctx->reader->user_data = user_data;
  return WASM_OK;
}

static WasmResult end_function_body_pass(uint32_t index,
                                         uint32_t pass,
                                         void* user_data) {
  Context* ctx = user_data;
  /* reset the reader to its original callbacks */
  if (pass == 1)
    *ctx->reader = s_binary_reader;
  ctx->reader->user_data = user_data;
  return WASM_OK;
}

static void wasm_destroy_interpreter_func(WasmAllocator* allocator,
                                          InterpreterFunc* func) {
  wasm_destroy_type_vector(allocator, &func->param_and_local_types);
}

static void destroy_context(Context* ctx) {
  wasm_destroy_uint32_vector(ctx->allocator, &ctx->discarded_exprs);
  wasm_destroy_expr_node_vector(ctx->allocator, &ctx->expr_stack);
  wasm_destroy_emit_label_vector(ctx->allocator, &ctx->emit_label_stack);
  wasm_destroy_typecheck_label_vector(ctx->allocator,
                                      &ctx->typecheck_label_stack);
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
  Context ctx;
  WasmBinaryReader reader;

  WASM_ZERO_MEMORY(ctx);
  WASM_ZERO_MEMORY(reader);

  ctx.allocator = allocator;
  ctx.reader = &reader;
  ctx.error_handler = error_handler;
  ctx.memory_allocator = memory_allocator;
  ctx.module = out_module;
  ctx.start_func_index = INVALID_FUNC_INDEX;
  ctx.module->start_func_offset = WASM_INVALID_OFFSET;
  CHECK_RESULT(wasm_init_mem_writer(allocator, &ctx.istream_writer));

  reader = s_binary_reader;
  reader.user_data = &ctx;

  const uint32_t num_function_passes = 2;
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

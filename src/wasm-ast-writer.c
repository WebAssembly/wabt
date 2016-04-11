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

#include "wasm-ast-writer.h"

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include "wasm-ast.h"
#include "wasm-common.h"
#include "wasm-literal.h"
#include "wasm-writer.h"

#define INDENT_SIZE 2
#define NO_FORCE_NEWLINE 0
#define FORCE_NEWLINE 1

#define V(rtype, type1, type2, mem_size, code, NAME, text) [code] = text,
static const char* s_opcode_name[] = {WASM_FOREACH_OPCODE(V)};
#undef V

#define ALLOC_FAILURE \
  fprintf(stderr, "%s:%d: allocation failed\n", __FILE__, __LINE__)

#define CHECK_ALLOC_(ctx, cond) \
  do {                          \
    if (!(cond)) {              \
      ALLOC_FAILURE;            \
      ctx->result = WASM_ERROR; \
      return;                   \
    }                           \
  } while (0)

#define CHECK_ALLOC(ctx, e) CHECK_ALLOC_(ctx, WASM_SUCCEEDED(e))

static const uint8_t s_is_char_escaped[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

typedef enum WasmNextChar {
  WASM_NEXT_CHAR_NONE,
  WASM_NEXT_CHAR_SPACE,
  WASM_NEXT_CHAR_NEWLINE,
  WASM_NEXT_CHAR_FORCE_NEWLINE,
} WasmNextChar;

typedef struct WasmContext {
  WasmAllocator* allocator;
  WasmWriter* writer;
  size_t offset;
  WasmResult result;
  int indent;
  WasmNextChar next_char;
  int depth;
  WasmStringSliceVector index_to_name;
} WasmContext;

static void indent(WasmContext* ctx) {
  ctx->indent += INDENT_SIZE;
}

static void dedent(WasmContext* ctx) {
  ctx->indent -= INDENT_SIZE;
  assert(ctx->indent >= 0);
}

static void out_data_at(WasmContext* ctx,
                        size_t offset,
                        const void* src,
                        size_t size) {
  if (WASM_FAILED(ctx->result))
    return;
  if (ctx->writer->write_data) {
    ctx->result =
        ctx->writer->write_data(offset, src, size, ctx->writer->user_data);
  }
}

static void out_data(WasmContext* ctx, const void* src, size_t size) {
  out_data_at(ctx, ctx->offset, src, size);
  ctx->offset += size;
}

static void out_indent(WasmContext* ctx) {
  static char s_indent[] =
      "                                                                       "
      "                                                                       ";
  static size_t s_indent_len = sizeof(s_indent) - 1;
  size_t indent = ctx->indent;
  while (indent > s_indent_len) {
    out_data(ctx, s_indent, s_indent_len);
    indent -= s_indent_len;
  }
  if (indent > 0) {
    out_data(ctx, s_indent, indent);
  }
}

static void out_next_char(WasmContext* ctx) {
  switch (ctx->next_char) {
    case WASM_NEXT_CHAR_SPACE:
      out_data(ctx, " ", 1);
      break;
    case WASM_NEXT_CHAR_NEWLINE:
    case WASM_NEXT_CHAR_FORCE_NEWLINE:
      out_data(ctx, "\n", 1);
      out_indent(ctx);
      break;

    default:
    case WASM_NEXT_CHAR_NONE:
      break;
  }
  ctx->next_char = WASM_NEXT_CHAR_NONE;
}

static void out_data_with_next_char(WasmContext* ctx,
                                    const void* src,
                                    size_t size) {
  out_next_char(ctx);
  out_data(ctx, src, size);
}

static void out_printf(WasmContext* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  /* default to following space */
  out_data_with_next_char(ctx, buffer, length);
  ctx->next_char = WASM_NEXT_CHAR_SPACE;
}

static void out_putc(WasmContext* ctx, char c) {
  out_data(ctx, &c, 1);
}

static void out_puts(WasmContext* ctx, const char* s, WasmNextChar next_char) {
  size_t len = strlen(s);
  out_data_with_next_char(ctx, s, len);
  ctx->next_char = next_char;
}

static void out_puts_space(WasmContext* ctx, const char* s) {
  out_puts(ctx, s, WASM_NEXT_CHAR_SPACE);
}

static void out_newline(WasmContext* ctx, WasmBool force) {
  if (ctx->next_char == WASM_NEXT_CHAR_FORCE_NEWLINE)
    out_next_char(ctx);
  ctx->next_char =
      force ? WASM_NEXT_CHAR_FORCE_NEWLINE : WASM_NEXT_CHAR_NEWLINE;
}

static void out_open(WasmContext* ctx,
                     const char* name,
                     WasmNextChar next_char) {
  out_puts(ctx, "(", WASM_NEXT_CHAR_NONE);
  out_puts(ctx, name, next_char);
  indent(ctx);
}

static void out_open_newline(WasmContext* ctx, const char* name) {
  out_open(ctx, name, WASM_NEXT_CHAR_NEWLINE);
}

static void out_open_space(WasmContext* ctx, const char* name) {
  out_open(ctx, name, WASM_NEXT_CHAR_SPACE);
}

static void out_close(WasmContext* ctx, WasmNextChar next_char) {
  if (ctx->next_char != WASM_NEXT_CHAR_FORCE_NEWLINE)
    ctx->next_char = WASM_NEXT_CHAR_NONE;
  dedent(ctx);
  out_puts(ctx, ")", next_char);
}

static void out_close_newline(WasmContext* ctx) {
  out_close(ctx, WASM_NEXT_CHAR_NEWLINE);
}

static void out_close_space(WasmContext* ctx) {
  out_close(ctx, WASM_NEXT_CHAR_SPACE);
}

static void out_string_slice(WasmContext* ctx,
                             const WasmStringSlice* str,
                             WasmNextChar next_char) {
  out_printf(ctx, PRIstringslice, WASM_PRINTF_STRING_SLICE_ARG(*str));
  ctx->next_char = next_char;
}

static void out_string_slice_opt(WasmContext* ctx,
                                 const WasmStringSlice* str,
                                 WasmNextChar next_char) {
  if (str->start)
    out_string_slice(ctx, str, next_char);
}

static void out_string_slice_or_index(WasmContext* ctx,
                                      const WasmStringSlice* str,
                                      uint32_t index,
                                      WasmNextChar next_char) {
  if (str->start)
    out_string_slice(ctx, str, next_char);
  else
    out_printf(ctx, "(;%u;)", index);
}

static void out_quoted_data(WasmContext* ctx, const void* data, size_t length) {
  const uint8_t* u8_data = data;
  static const char s_hexdigits[] = "0123456789abcdef";
  out_next_char(ctx);
  out_putc(ctx, '\"');
  size_t i;
  for (i = 0; i < length; ++i) {
    uint8_t c = u8_data[i];
    if (s_is_char_escaped[c]) {
      out_putc(ctx, '\\');
      out_putc(ctx, s_hexdigits[c >> 4]);
      out_putc(ctx, s_hexdigits[c & 0xf]);
    } else {
      out_putc(ctx, c);
    }
  }
  out_putc(ctx, '\"');
  ctx->next_char = WASM_NEXT_CHAR_SPACE;
}

static void out_quoted_string_slice(WasmContext* ctx,
                                    const WasmStringSlice* str,
                                    WasmNextChar next_char) {
  out_quoted_data(ctx, str->start, str->length);
  ctx->next_char = next_char;
}

static void out_var(WasmContext* ctx,
                    const WasmVar* var,
                    WasmNextChar next_char) {
  if (var->type == WASM_VAR_TYPE_INDEX) {
    out_printf(ctx, "%d", var->index);
    ctx->next_char = next_char;
  } else {
    out_string_slice(ctx, &var->name, next_char);
  }
}

static void out_br_var(WasmContext* ctx,
                       const WasmVar* var,
                       WasmNextChar next_char) {
  if (var->type == WASM_VAR_TYPE_INDEX) {
    out_printf(ctx, "%d (;@%d;)", var->index, ctx->depth - var->index - 1);
    ctx->next_char = next_char;
  } else {
    out_string_slice(ctx, &var->name, next_char);
  }
}

static void out_type(WasmContext* ctx, WasmType type, WasmNextChar next_char) {
  static const char* s_types[] = {NULL, "i32", "i64", "f32", "f64"};
  out_puts(ctx, s_types[type], next_char);
}

static void out_func_sig_space(WasmContext* ctx,
                               const WasmFuncSignature* func_sig) {
  if (func_sig->param_types.size) {
    size_t i;
    out_open_space(ctx, "param");
    for (i = 0; i < func_sig->param_types.size; ++i) {
      out_type(ctx, func_sig->param_types.data[i], WASM_NEXT_CHAR_SPACE);
    }
    out_close_space(ctx);
  }

  if (func_sig->result_type != WASM_TYPE_VOID) {
    out_open_space(ctx, "result");
    out_type(ctx, func_sig->result_type, WASM_NEXT_CHAR_NONE);
    out_close_space(ctx);
  }
}

static void write_exprs(WasmContext* ctx, const WasmExprPtrVector* exprs);

static void write_expr(WasmContext* ctx, const WasmExpr* expr);

static void write_block(WasmContext* ctx,
                        const WasmBlock* block,
                        const char* text) {
  out_open_space(ctx, text);
  out_string_slice_opt(ctx, &block->label, WASM_NEXT_CHAR_SPACE);
  out_printf(ctx, " ;; exit = @%d", ctx->depth);
  out_newline(ctx, FORCE_NEWLINE);
  ctx->depth++;
  write_exprs(ctx, &block->exprs);
  ctx->depth--;
  out_close_newline(ctx);
}

/* TODO(binji): remove all this if-block stuff when we switch to postorder */
typedef enum WriteIfBranchType {
  WASM_IF_BRANCH_TYPE_ONE_EXPRESSION,
  WASM_IF_BRANCH_TYPE_BLOCK_EXPRESSION,
} WriteIfBranchType;

static WriteIfBranchType get_if_branch_type(const WasmExpr* expr) {
  return expr->type == WASM_EXPR_TYPE_BLOCK
             ? WASM_IF_BRANCH_TYPE_BLOCK_EXPRESSION
             : WASM_IF_BRANCH_TYPE_ONE_EXPRESSION;
}

static void write_if_branch(WasmContext* ctx,
                            WriteIfBranchType type,
                            const WasmExpr* expr,
                            const char* text) {
  switch (type) {
    case WASM_IF_BRANCH_TYPE_ONE_EXPRESSION:
      out_printf(ctx, ";; exit = @%d", ctx->depth);
      out_newline(ctx, FORCE_NEWLINE);
      ctx->depth++;
      write_expr(ctx, expr);
      ctx->depth--;
      break;
    case WASM_IF_BRANCH_TYPE_BLOCK_EXPRESSION:
      if (expr->type == WASM_EXPR_TYPE_BLOCK) {
        write_block(ctx, &expr->block, text);
      } else {
        out_open_space(ctx, text);
        out_printf(ctx, " ;; exit = @%d", ctx->depth);
        out_newline(ctx, FORCE_NEWLINE);
        ctx->depth++;
        write_expr(ctx, expr);
        ctx->depth--;
        out_close_newline(ctx);
      }
      break;
    default:
      assert(0);
      break;
  }
}

static void write_const(WasmContext* ctx, const WasmConst* const_) {
  switch (const_->type) {
    case WASM_TYPE_I32:
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_I32_CONST]);
      out_printf(ctx, "%d", (int32_t)const_->u32);
      out_close_newline(ctx);
      break;

    case WASM_TYPE_I64:
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_I64_CONST]);
      out_printf(ctx, "%" PRId64, (int64_t)const_->u64);
      out_close_newline(ctx);
      break;

    case WASM_TYPE_F32: {
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_F32_CONST]);
      char buffer[128];
      wasm_write_float_hex(buffer, 128, const_->f32_bits);
      out_puts_space(ctx, buffer);
      out_close_newline(ctx);
      break;
    }

    case WASM_TYPE_F64: {
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_F64_CONST]);
      char buffer[128];
      wasm_write_double_hex(buffer, 128, const_->f64_bits);
      out_puts_space(ctx, buffer);
      out_close_newline(ctx);
      break;
    }

    default:
      assert(0);
      break;
  }
}

static void write_expr(WasmContext* ctx, const WasmExpr* expr) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      out_open_newline(ctx, s_opcode_name[expr->binary.opcode]);
      write_expr(ctx, expr->binary.left);
      write_expr(ctx, expr->binary.right);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_BLOCK:
      write_block(ctx, &expr->block, s_opcode_name[WASM_OPCODE_BLOCK]);
      break;

    case WASM_EXPR_TYPE_BR:
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_BR]);
      out_br_var(ctx, &expr->br.var, WASM_NEXT_CHAR_NEWLINE);
      if (expr->br.expr && expr->br.expr->type != WASM_EXPR_TYPE_NOP)
        write_expr(ctx, expr->br.expr);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_BR_IF:
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_BR_IF]);
      out_br_var(ctx, &expr->br_if.var, WASM_NEXT_CHAR_NEWLINE);
      if (expr->br_if.expr && expr->br_if.expr->type != WASM_EXPR_TYPE_NOP)
        write_expr(ctx, expr->br_if.expr);
      write_expr(ctx, expr->br_if.cond);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_BR_TABLE: {
      out_open_newline(ctx, s_opcode_name[WASM_OPCODE_BR_TABLE]);
      size_t i;
      for (i = 0; i < expr->br_table.targets.size; ++i)
        out_br_var(ctx, &expr->br_table.targets.data[i], WASM_NEXT_CHAR_SPACE);
      out_br_var(ctx, &expr->br_table.default_target, WASM_NEXT_CHAR_NEWLINE);
      write_expr(ctx, expr->br_table.expr);
      out_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_CALL: {
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_CALL_FUNCTION]);
      out_var(ctx, &expr->call.var, WASM_NEXT_CHAR_NEWLINE);
      size_t i;
      for (i = 0; i < expr->call.args.size; ++i)
        write_expr(ctx, expr->call.args.data[i]);
      out_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_CALL_IMPORT: {
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_CALL_IMPORT]);
      out_var(ctx, &expr->call.var, WASM_NEXT_CHAR_NEWLINE);
      size_t i;
      for (i = 0; i < expr->call.args.size; ++i)
        write_expr(ctx, expr->call.args.data[i]);
      out_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_CALL_INDIRECT: {
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_CALL_INDIRECT]);
      out_var(ctx, &expr->call_indirect.var, WASM_NEXT_CHAR_NEWLINE);
      write_expr(ctx, expr->call_indirect.expr);
      size_t i;
      for (i = 0; i < expr->call_indirect.args.size; ++i)
        write_expr(ctx, expr->call_indirect.args.data[i]);
      out_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_COMPARE:
      out_open_newline(ctx, s_opcode_name[expr->compare.opcode]);
      write_expr(ctx, expr->compare.left);
      write_expr(ctx, expr->compare.right);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_CONST:
      write_const(ctx, &expr->const_);
      break;

    case WASM_EXPR_TYPE_CONVERT:
      out_open_newline(ctx, s_opcode_name[expr->convert.opcode]);
      write_expr(ctx, expr->convert.expr);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_GET_LOCAL:
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_GET_LOCAL]);
      out_var(ctx, &expr->get_local.var, WASM_NEXT_CHAR_NONE);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_GROW_MEMORY:
      out_open_newline(ctx, s_opcode_name[WASM_OPCODE_GROW_MEMORY]);
      write_expr(ctx, expr->grow_memory.expr);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_IF: {
      out_open_newline(ctx, s_opcode_name[WASM_OPCODE_IF]);
      write_expr(ctx, expr->if_.cond);
      WriteIfBranchType true_type = get_if_branch_type(expr->if_.true_);
      write_if_branch(ctx, true_type, expr->if_.true_, "then");
      out_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_IF_ELSE: {
      out_open_newline(ctx, s_opcode_name[WASM_OPCODE_IF]);
      write_expr(ctx, expr->if_else.cond);

      /* silly dance to make sure that we don't use mismatching format for
       * if/then/else. We're allowed to do the short if form:
       *   (if (cond) (true) (false))
       * only when true and false are one non-block expression. Otherwise we
       * need to use the full
       *   (if (then (true1) (true2)) (else (false1) (false2)))
       * form. */
      WriteIfBranchType true_type = get_if_branch_type(expr->if_else.true_);
      WriteIfBranchType false_type = get_if_branch_type(expr->if_else.false_);
      if (true_type != false_type)
        true_type = false_type = WASM_IF_BRANCH_TYPE_BLOCK_EXPRESSION;

      write_if_branch(ctx, true_type, expr->if_else.true_, "then");
      write_if_branch(ctx, false_type, expr->if_else.false_, "else");
      out_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_LOAD:
      out_open_space(ctx, s_opcode_name[expr->load.opcode]);
      if (expr->load.offset)
        out_printf(ctx, "offset=%" PRIu64, expr->load.offset);
      if (!wasm_is_naturally_aligned(expr->load.opcode, expr->load.align))
        out_printf(ctx, "align=%u", expr->load.align);
      out_newline(ctx, NO_FORCE_NEWLINE);
      write_expr(ctx, expr->load.addr);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_LOOP:
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_LOOP]);
      out_string_slice_opt(ctx, &expr->loop.inner, WASM_NEXT_CHAR_SPACE);
      out_string_slice_opt(ctx, &expr->loop.outer, WASM_NEXT_CHAR_SPACE);
      out_printf(ctx, " ;; exit = @%d, cont = @%d", ctx->depth, ctx->depth + 1);
      out_newline(ctx, FORCE_NEWLINE);
      ctx->depth += 2;
      write_exprs(ctx, &expr->loop.exprs);
      ctx->depth -= 2;
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_MEMORY_SIZE:
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_MEMORY_SIZE]);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_NOP:
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_NOP]);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_RETURN:
      out_open_newline(ctx, s_opcode_name[WASM_OPCODE_RETURN]);
      if (expr->return_.expr)
        write_expr(ctx, expr->return_.expr);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_SELECT:
      out_open_newline(ctx, s_opcode_name[WASM_OPCODE_SELECT]);
      write_expr(ctx, expr->select.true_);
      write_expr(ctx, expr->select.false_);
      write_expr(ctx, expr->select.cond);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_SET_LOCAL:
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_SET_LOCAL]);
      out_var(ctx, &expr->set_local.var, WASM_NEXT_CHAR_NEWLINE);
      write_expr(ctx, expr->set_local.expr);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_STORE:
      out_open_space(ctx, s_opcode_name[expr->store.opcode]);
      if (expr->store.offset)
        out_printf(ctx, "offset=%" PRIu64, expr->store.offset);
      if (!wasm_is_naturally_aligned(expr->store.opcode, expr->store.align))
        out_printf(ctx, "align=%u", expr->store.align);
      out_newline(ctx, NO_FORCE_NEWLINE);
      write_expr(ctx, expr->store.addr);
      write_expr(ctx, expr->store.value);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_UNARY:
      out_open_newline(ctx, s_opcode_name[expr->unary.opcode]);
      write_expr(ctx, expr->unary.expr);
      out_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_UNREACHABLE:
      out_open_space(ctx, s_opcode_name[WASM_OPCODE_UNREACHABLE]);
      out_close_newline(ctx);
      break;

    default:
      fprintf(stderr, "bad expr type: %d\n", expr->type);
      assert(0);
      break;
  }
}

static void write_exprs(WasmContext* ctx, const WasmExprPtrVector* exprs) {
  size_t i;
  for (i = 0; i < exprs->size; ++i)
    write_expr(ctx, exprs->data[i]);
}

static void write_type_bindings(WasmContext* ctx,
                                const char* prefix,
                                const WasmFunc* func,
                                const WasmTypeVector* types,
                                const WasmBindingHash* bindings) {
  CHECK_ALLOC(ctx, wasm_make_type_binding_reverse_mapping(
                       ctx->allocator, types, bindings, &ctx->index_to_name));

  /* named params/locals must be specified by themselves, but nameless
   * params/locals can be compressed, e.g.:
   *   (param $foo i32)
   *   (param i32 i64 f32)
   */
  WasmBool is_open = WASM_FALSE;
  size_t i;
  for (i = 0; i < types->size; ++i) {
    if (!is_open) {
      out_open_space(ctx, prefix);
      is_open = WASM_TRUE;
    }

    const WasmStringSlice* name = &ctx->index_to_name.data[i];
    WasmBool has_name = name->start != NULL;
    if (has_name)
      out_string_slice(ctx, name, WASM_NEXT_CHAR_SPACE);
    out_type(ctx, types->data[i], WASM_NEXT_CHAR_SPACE);
    if (has_name) {
      out_close_space(ctx);
      is_open = WASM_FALSE;
    }
  }
  if (is_open)
    out_close_space(ctx);
}

static void write_func(WasmContext* ctx,
                       const WasmModule* module,
                       int func_index,
                       const WasmFunc* func) {
  out_open_space(ctx, "func");
  out_string_slice_or_index(ctx, &func->name, func_index, WASM_NEXT_CHAR_SPACE);
  if (wasm_decl_has_func_type(&func->decl)) {
    out_open_space(ctx, "type");
    out_var(ctx, &func->decl.type_var, WASM_NEXT_CHAR_NONE);
    out_close_space(ctx);
  }
  if (wasm_decl_has_signature(&func->decl)) {
    write_type_bindings(ctx, "param", func, &func->decl.sig.param_types,
                        &func->param_bindings);
    if (wasm_get_result_type(func) != WASM_TYPE_VOID) {
      out_open_space(ctx, "result");
      out_type(ctx, wasm_get_result_type(func), WASM_NEXT_CHAR_NONE);
      out_close_space(ctx);
    }
  }
  out_newline(ctx, NO_FORCE_NEWLINE);
  if (func->local_types.size) {
    write_type_bindings(ctx, "local", func, &func->local_types,
                        &func->local_bindings);
  }
  out_newline(ctx, NO_FORCE_NEWLINE);
  write_exprs(ctx, &func->exprs);
  out_close_newline(ctx);
}

static void write_import(WasmContext* ctx,
                         int import_index,
                         const WasmImport* import) {
  out_open_space(ctx, "import");
  out_string_slice_or_index(ctx, &import->name, import_index,
                            WASM_NEXT_CHAR_SPACE);
  out_quoted_string_slice(ctx, &import->module_name, WASM_NEXT_CHAR_SPACE);
  out_quoted_string_slice(ctx, &import->func_name, WASM_NEXT_CHAR_SPACE);
  if (wasm_decl_has_func_type(&import->decl)) {
    out_open_space(ctx, "type");
    out_var(ctx, &import->decl.type_var, WASM_NEXT_CHAR_NONE);
    out_close_space(ctx);
  } else {
    out_func_sig_space(ctx, &import->decl.sig);
  }
  out_close_newline(ctx);
}

static void write_export(WasmContext* ctx,
                         int export_index,
                         const WasmExport* export) {
  out_open_space(ctx, "export");
  out_quoted_string_slice(ctx, &export->name, WASM_NEXT_CHAR_SPACE);
  out_var(ctx, &export->var, WASM_NEXT_CHAR_SPACE);
  out_close_newline(ctx);
}

static void write_export_memory(WasmContext* ctx,
                                const WasmExportMemory* export) {
  out_open_space(ctx, "export");
  out_quoted_string_slice(ctx, &export->name, WASM_NEXT_CHAR_SPACE);
  out_puts_space(ctx, "memory");
  out_close_newline(ctx);
}

static void write_table(WasmContext* ctx, const WasmVarVector* table) {
  out_open_space(ctx, "table");
  size_t i;
  for (i = 0; i < table->size; ++i)
    out_var(ctx, &table->data[i], WASM_NEXT_CHAR_SPACE);
  out_close_newline(ctx);
}

static void write_segment(WasmContext* ctx, const WasmSegment* segment) {
  out_open_space(ctx, "segment");
  out_printf(ctx, "%u", segment->addr);
  out_quoted_data(ctx, segment->data, segment->size);
  out_close_newline(ctx);
}

static void write_memory(WasmContext* ctx, const WasmMemory* memory) {
  out_open_space(ctx, "memory");
  out_printf(ctx, "%u", memory->initial_pages);
  if (memory->initial_pages != memory->max_pages)
    out_printf(ctx, "%u", memory->max_pages);
  out_newline(ctx, NO_FORCE_NEWLINE);
  size_t i;
  for (i = 0; i < memory->segments.size; ++i) {
    const WasmSegment* segment = &memory->segments.data[i];
    write_segment(ctx, segment);
  }
  out_close_newline(ctx);
}

static void write_func_type(WasmContext* ctx,
                            int func_type_index,
                            const WasmFuncType* func_type) {
  out_open_space(ctx, "type");
  out_string_slice_or_index(ctx, &func_type->name, func_type_index,
                            WASM_NEXT_CHAR_SPACE);
  out_open_space(ctx, "func");
  out_func_sig_space(ctx, &func_type->sig);
  out_close_space(ctx);
  out_close_newline(ctx);
}

static void write_start_function(WasmContext* ctx, const WasmVar* start) {
  out_open_space(ctx, "start");
  out_var(ctx, start, WASM_NEXT_CHAR_NONE);
  out_close_newline(ctx);
}

static void write_module(WasmContext* ctx, const WasmModule* module) {
  out_open_newline(ctx, "module");
  const WasmModuleField* field;
  int func_index = 0;
  int import_index = 0;
  int export_index = 0;
  int func_type_index = 0;
  for (field = module->first_field; field != NULL; field = field->next) {
    switch (field->type) {
      case WASM_MODULE_FIELD_TYPE_FUNC:
        write_func(ctx, module, func_index++, &field->func);
        break;
      case WASM_MODULE_FIELD_TYPE_IMPORT:
        write_import(ctx, import_index++, &field->import);
        break;
      case WASM_MODULE_FIELD_TYPE_EXPORT:
        write_export(ctx, export_index++, &field->export_);
        break;
      case WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY:
        write_export_memory(ctx, &field->export_memory);
        break;
      case WASM_MODULE_FIELD_TYPE_TABLE:
        write_table(ctx, &field->table);
        break;
      case WASM_MODULE_FIELD_TYPE_MEMORY:
        write_memory(ctx, &field->memory);
        break;
      case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
        write_func_type(ctx, func_type_index++, &field->func_type);
        break;
      case WASM_MODULE_FIELD_TYPE_START:
        write_start_function(ctx, &field->start);
        break;
    }
  }
  out_close_newline(ctx);
  /* force the newline to be written */
  out_next_char(ctx);
}

WasmResult wasm_write_ast(WasmAllocator* allocator,
                          WasmWriter* writer,
                          const WasmModule* module) {
  WasmContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.result = WASM_OK;
  ctx.writer = writer;
  write_module(&ctx, module);
  /* the memory for the actual string slice is shared with the module, so we
   * only need to free the vector */
  wasm_destroy_string_slice_vector(allocator, &ctx.index_to_name);
  return ctx.result;
}

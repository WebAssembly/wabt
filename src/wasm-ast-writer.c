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
#include "wasm-stream.h"
#include "wasm-writer.h"

#define INDENT_SIZE 2
#define NO_FORCE_NEWLINE 0
#define FORCE_NEWLINE 1

#define V(rtype, type1, type2, mem_size, code, NAME, text) [code] = text,
static const char* s_opcode_name[] = {WASM_FOREACH_OPCODE(V)};
#undef V

#define ALLOC_FAILURE \
  fprintf(stderr, "%s:%d: allocation failed\n", __FILE__, __LINE__)

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

typedef enum NextChar {
  NEXT_CHAR_NONE,
  NEXT_CHAR_SPACE,
  NEXT_CHAR_NEWLINE,
  NEXT_CHAR_FORCE_NEWLINE,
} NextChar;

typedef struct Context {
  WasmAllocator* allocator;
  WasmStream stream;
  WasmResult result;
  int indent;
  NextChar next_char;
  int depth;
  WasmStringSliceVector index_to_name;
} Context;

static void indent(Context* ctx) {
  ctx->indent += INDENT_SIZE;
}

static void dedent(Context* ctx) {
  ctx->indent -= INDENT_SIZE;
  assert(ctx->indent >= 0);
}

static void write_indent(Context* ctx) {
  static char s_indent[] =
      "                                                                       "
      "                                                                       ";
  static size_t s_indent_len = sizeof(s_indent) - 1;
  size_t indent = ctx->indent;
  while (indent > s_indent_len) {
    wasm_write_data(&ctx->stream, s_indent, s_indent_len, NULL);
    indent -= s_indent_len;
  }
  if (indent > 0) {
    wasm_write_data(&ctx->stream, s_indent, indent, NULL);
  }
}

static void write_next_char(Context* ctx) {
  switch (ctx->next_char) {
    case NEXT_CHAR_SPACE:
      wasm_write_data(&ctx->stream, " ", 1, NULL);
      break;
    case NEXT_CHAR_NEWLINE:
    case NEXT_CHAR_FORCE_NEWLINE:
      wasm_write_data(&ctx->stream, "\n", 1, NULL);
      write_indent(ctx);
      break;

    default:
    case NEXT_CHAR_NONE:
      break;
  }
  ctx->next_char = NEXT_CHAR_NONE;
}

static void write_data_with_next_char(Context* ctx,
                                      const void* src,
                                      size_t size) {
  write_next_char(ctx);
  wasm_write_data(&ctx->stream, src, size, NULL);
}

static void WASM_PRINTF_FORMAT(2, 3)
    writef(Context* ctx, const char* format, ...) {
  WASM_SNPRINTF_ALLOCA(buffer, length, format);
  /* default to following space */
  write_data_with_next_char(ctx, buffer, length);
  ctx->next_char = NEXT_CHAR_SPACE;
}

static void write_putc(Context* ctx, char c) {
  wasm_write_data(&ctx->stream, &c, 1, NULL);
}

static void write_puts(Context* ctx, const char* s, NextChar next_char) {
  size_t len = strlen(s);
  write_data_with_next_char(ctx, s, len);
  ctx->next_char = next_char;
}

static void write_puts_space(Context* ctx, const char* s) {
  write_puts(ctx, s, NEXT_CHAR_SPACE);
}

static void write_newline(Context* ctx, WasmBool force) {
  if (ctx->next_char == NEXT_CHAR_FORCE_NEWLINE)
    write_next_char(ctx);
  ctx->next_char = force ? NEXT_CHAR_FORCE_NEWLINE : NEXT_CHAR_NEWLINE;
}

static void write_open(Context* ctx, const char* name, NextChar next_char) {
  write_puts(ctx, "(", NEXT_CHAR_NONE);
  write_puts(ctx, name, next_char);
  indent(ctx);
}

static void write_open_newline(Context* ctx, const char* name) {
  write_open(ctx, name, NEXT_CHAR_NEWLINE);
}

static void write_open_space(Context* ctx, const char* name) {
  write_open(ctx, name, NEXT_CHAR_SPACE);
}

static void write_close(Context* ctx, NextChar next_char) {
  if (ctx->next_char != NEXT_CHAR_FORCE_NEWLINE)
    ctx->next_char = NEXT_CHAR_NONE;
  dedent(ctx);
  write_puts(ctx, ")", next_char);
}

static void write_close_newline(Context* ctx) {
  write_close(ctx, NEXT_CHAR_NEWLINE);
}

static void write_close_space(Context* ctx) {
  write_close(ctx, NEXT_CHAR_SPACE);
}

static void write_string_slice(Context* ctx,
                               const WasmStringSlice* str,
                               NextChar next_char) {
  writef(ctx, PRIstringslice, WASM_PRINTF_STRING_SLICE_ARG(*str));
  ctx->next_char = next_char;
}

static WasmBool write_string_slice_opt(Context* ctx,
                                       const WasmStringSlice* str,
                                       NextChar next_char) {
  if (str->start)
    write_string_slice(ctx, str, next_char);
  return str->start ? WASM_TRUE : WASM_FALSE;
}

static void write_string_slice_or_index(Context* ctx,
                                        const WasmStringSlice* str,
                                        uint32_t index,
                                        NextChar next_char) {
  if (str->start)
    write_string_slice(ctx, str, next_char);
  else
    writef(ctx, "(;%u;)", index);
}

static void write_quoted_data(Context* ctx, const void* data, size_t length) {
  const uint8_t* u8_data = data;
  static const char s_hexdigits[] = "0123456789abcdef";
  write_next_char(ctx);
  write_putc(ctx, '\"');
  size_t i;
  for (i = 0; i < length; ++i) {
    uint8_t c = u8_data[i];
    if (s_is_char_escaped[c]) {
      write_putc(ctx, '\\');
      write_putc(ctx, s_hexdigits[c >> 4]);
      write_putc(ctx, s_hexdigits[c & 0xf]);
    } else {
      write_putc(ctx, c);
    }
  }
  write_putc(ctx, '\"');
  ctx->next_char = NEXT_CHAR_SPACE;
}

static void write_quoted_string_slice(Context* ctx,
                                      const WasmStringSlice* str,
                                      NextChar next_char) {
  write_quoted_data(ctx, str->start, str->length);
  ctx->next_char = next_char;
}

static void write_var(Context* ctx, const WasmVar* var, NextChar next_char) {
  if (var->type == WASM_VAR_TYPE_INDEX) {
    writef(ctx, "%" PRId64, var->index);
    ctx->next_char = next_char;
  } else {
    write_string_slice(ctx, &var->name, next_char);
  }
}

static void write_br_var(Context* ctx, const WasmVar* var, NextChar next_char) {
  if (var->type == WASM_VAR_TYPE_INDEX) {
    writef(ctx, "%" PRId64 " (;@%" PRId64 ";)", var->index,
           ctx->depth - var->index - 1);
    ctx->next_char = next_char;
  } else {
    write_string_slice(ctx, &var->name, next_char);
  }
}

static void write_type(Context* ctx, WasmType type, NextChar next_char) {
  static const char* s_types[] = {NULL, "i32", "i64", "f32", "f64"};
  write_puts(ctx, s_types[type], next_char);
}

static void write_func_sig_space(Context* ctx,
                                 const WasmFuncSignature* func_sig) {
  if (func_sig->param_types.size) {
    size_t i;
    write_open_space(ctx, "param");
    for (i = 0; i < func_sig->param_types.size; ++i) {
      write_type(ctx, func_sig->param_types.data[i], NEXT_CHAR_SPACE);
    }
    write_close_space(ctx);
  }

  if (func_sig->result_type != WASM_TYPE_VOID) {
    write_open_space(ctx, "result");
    write_type(ctx, func_sig->result_type, NEXT_CHAR_NONE);
    write_close_space(ctx);
  }
}

static void write_expr_list(Context* ctx, const WasmExpr* first);

static void write_expr(Context* ctx, const WasmExpr* expr);

static void write_block(Context* ctx,
                        const WasmBlock* block,
                        const char* text) {
  write_open_space(ctx, text);
  if (!write_string_slice_opt(ctx, &block->label, NEXT_CHAR_SPACE))
    writef(ctx, " ;; exit = @%d", ctx->depth);
  write_newline(ctx, FORCE_NEWLINE);
  ctx->depth++;
  write_expr_list(ctx, block->first);
  ctx->depth--;
  write_close_newline(ctx);
}

static void write_const(Context* ctx, const WasmConst* const_) {
  switch (const_->type) {
    case WASM_TYPE_I32:
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_I32_CONST]);
      writef(ctx, "%d", (int32_t)const_->u32);
      write_close_newline(ctx);
      break;

    case WASM_TYPE_I64:
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_I64_CONST]);
      writef(ctx, "%" PRId64, (int64_t)const_->u64);
      write_close_newline(ctx);
      break;

    case WASM_TYPE_F32: {
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_F32_CONST]);
      char buffer[128];
      wasm_write_float_hex(buffer, 128, const_->f32_bits);
      write_puts_space(ctx, buffer);
      write_close_newline(ctx);
      break;
    }

    case WASM_TYPE_F64: {
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_F64_CONST]);
      char buffer[128];
      wasm_write_double_hex(buffer, 128, const_->f64_bits);
      write_puts_space(ctx, buffer);
      write_close_newline(ctx);
      break;
    }

    default:
      assert(0);
      break;
  }
}

static void write_expr(Context* ctx, const WasmExpr* expr) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      write_open_newline(ctx, s_opcode_name[expr->binary.opcode]);
      write_expr(ctx, expr->binary.left);
      write_expr(ctx, expr->binary.right);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_BLOCK:
      write_block(ctx, &expr->block, s_opcode_name[WASM_OPCODE_BLOCK]);
      break;

    case WASM_EXPR_TYPE_BR:
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_BR]);
      write_br_var(ctx, &expr->br.var, NEXT_CHAR_NEWLINE);
      if (expr->br.expr)
        write_expr(ctx, expr->br.expr);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_BR_IF:
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_BR_IF]);
      write_br_var(ctx, &expr->br_if.var, NEXT_CHAR_NEWLINE);
      if (expr->br_if.expr)
        write_expr(ctx, expr->br_if.expr);
      write_expr(ctx, expr->br_if.cond);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_BR_TABLE: {
      write_open_newline(ctx, s_opcode_name[WASM_OPCODE_BR_TABLE]);
      size_t i;
      for (i = 0; i < expr->br_table.targets.size; ++i)
        write_br_var(ctx, &expr->br_table.targets.data[i], NEXT_CHAR_SPACE);
      write_br_var(ctx, &expr->br_table.default_target, NEXT_CHAR_NEWLINE);
      if (expr->br_table.expr)
        write_expr(ctx, expr->br_table.expr);
      write_expr(ctx, expr->br_table.key);
      write_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_CALL: {
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_CALL_FUNCTION]);
      write_var(ctx, &expr->call.var, NEXT_CHAR_NEWLINE);
      write_expr_list(ctx, expr->call.first_arg);
      write_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_CALL_IMPORT: {
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_CALL_IMPORT]);
      write_var(ctx, &expr->call.var, NEXT_CHAR_NEWLINE);
      write_expr_list(ctx, expr->call.first_arg);
      write_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_CALL_INDIRECT: {
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_CALL_INDIRECT]);
      write_var(ctx, &expr->call_indirect.var, NEXT_CHAR_NEWLINE);
      write_expr(ctx, expr->call_indirect.expr);
      write_expr_list(ctx, expr->call_indirect.first_arg);
      write_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_COMPARE:
      write_open_newline(ctx, s_opcode_name[expr->compare.opcode]);
      write_expr(ctx, expr->compare.left);
      write_expr(ctx, expr->compare.right);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_CONST:
      write_const(ctx, &expr->const_);
      break;

    case WASM_EXPR_TYPE_CONVERT:
      write_open_newline(ctx, s_opcode_name[expr->convert.opcode]);
      write_expr(ctx, expr->convert.expr);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_GET_LOCAL:
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_GET_LOCAL]);
      write_var(ctx, &expr->get_local.var, NEXT_CHAR_NONE);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_GROW_MEMORY:
      write_open_newline(ctx, s_opcode_name[WASM_OPCODE_GROW_MEMORY]);
      write_expr(ctx, expr->grow_memory.expr);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_IF: {
      write_open_newline(ctx, s_opcode_name[WASM_OPCODE_IF]);
      write_expr(ctx, expr->if_.cond);
      write_block(ctx, &expr->if_.true_, "then");
      write_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_IF_ELSE: {
      write_open_newline(ctx, s_opcode_name[WASM_OPCODE_IF]);
      write_expr(ctx, expr->if_else.cond);
      write_block(ctx, &expr->if_else.true_, "then");
      write_block(ctx, &expr->if_else.false_, "else");
      write_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_LOAD:
      write_open_space(ctx, s_opcode_name[expr->load.opcode]);
      if (expr->load.offset)
        writef(ctx, "offset=%" PRIu64, expr->load.offset);
      if (!wasm_is_naturally_aligned(expr->load.opcode, expr->load.align))
        writef(ctx, "align=%u", expr->load.align);
      write_newline(ctx, NO_FORCE_NEWLINE);
      write_expr(ctx, expr->load.addr);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_LOOP: {
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_LOOP]);
      WasmBool has_outer_name =
          write_string_slice_opt(ctx, &expr->loop.outer, NEXT_CHAR_SPACE);
      WasmBool has_inner_name =
          write_string_slice_opt(ctx, &expr->loop.inner, NEXT_CHAR_SPACE);
      if (!has_outer_name || !has_inner_name) {
        writef(ctx, " ;;");
        if (!has_outer_name)
          writef(ctx, "exit = @%d", ctx->depth);
        if (!has_inner_name) {
          writef(ctx, "cont = @%d", ctx->depth + 1);
        }
      }
      write_newline(ctx, FORCE_NEWLINE);
      ctx->depth += 2;
      write_expr_list(ctx, expr->loop.first);
      ctx->depth -= 2;
      write_close_newline(ctx);
      break;
    }

    case WASM_EXPR_TYPE_CURRENT_MEMORY:
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_CURRENT_MEMORY]);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_NOP:
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_NOP]);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_RETURN:
      write_open_newline(ctx, s_opcode_name[WASM_OPCODE_RETURN]);
      if (expr->return_.expr)
        write_expr(ctx, expr->return_.expr);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_SELECT:
      write_open_newline(ctx, s_opcode_name[WASM_OPCODE_SELECT]);
      write_expr(ctx, expr->select.true_);
      write_expr(ctx, expr->select.false_);
      write_expr(ctx, expr->select.cond);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_SET_LOCAL:
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_SET_LOCAL]);
      write_var(ctx, &expr->set_local.var, NEXT_CHAR_NEWLINE);
      write_expr(ctx, expr->set_local.expr);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_STORE:
      write_open_space(ctx, s_opcode_name[expr->store.opcode]);
      if (expr->store.offset)
        writef(ctx, "offset=%" PRIu64, expr->store.offset);
      if (!wasm_is_naturally_aligned(expr->store.opcode, expr->store.align))
        writef(ctx, "align=%u", expr->store.align);
      write_newline(ctx, NO_FORCE_NEWLINE);
      write_expr(ctx, expr->store.addr);
      write_expr(ctx, expr->store.value);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_UNARY:
      write_open_newline(ctx, s_opcode_name[expr->unary.opcode]);
      write_expr(ctx, expr->unary.expr);
      write_close_newline(ctx);
      break;

    case WASM_EXPR_TYPE_UNREACHABLE:
      write_open_space(ctx, s_opcode_name[WASM_OPCODE_UNREACHABLE]);
      write_close_newline(ctx);
      break;

    default:
      fprintf(stderr, "bad expr type: %d\n", expr->type);
      assert(0);
      break;
  }
}

static void write_expr_list(Context* ctx, const WasmExpr* first) {
  const WasmExpr* expr;
  for (expr = first; expr; expr = expr->next)
    write_expr(ctx, expr);
}

static void write_type_bindings(Context* ctx,
                                const char* prefix,
                                const WasmFunc* func,
                                const WasmTypeVector* types,
                                const WasmBindingHash* bindings) {
  wasm_make_type_binding_reverse_mapping(ctx->allocator, types, bindings,
                                         &ctx->index_to_name);

  /* named params/locals must be specified by themselves, but nameless
   * params/locals can be compressed, e.g.:
   *   (param $foo i32)
   *   (param i32 i64 f32)
   */
  WasmBool is_open = WASM_FALSE;
  size_t i;
  for (i = 0; i < types->size; ++i) {
    if (!is_open) {
      write_open_space(ctx, prefix);
      is_open = WASM_TRUE;
    }

    const WasmStringSlice* name = &ctx->index_to_name.data[i];
    WasmBool has_name = name->start != NULL;
    if (has_name)
      write_string_slice(ctx, name, NEXT_CHAR_SPACE);
    write_type(ctx, types->data[i], NEXT_CHAR_SPACE);
    if (has_name) {
      write_close_space(ctx);
      is_open = WASM_FALSE;
    }
  }
  if (is_open)
    write_close_space(ctx);
}

static void write_func(Context* ctx,
                       const WasmModule* module,
                       int func_index,
                       const WasmFunc* func) {
  write_open_space(ctx, "func");
  write_string_slice_or_index(ctx, &func->name, func_index, NEXT_CHAR_SPACE);
  if (wasm_decl_has_func_type(&func->decl)) {
    write_open_space(ctx, "type");
    write_var(ctx, &func->decl.type_var, NEXT_CHAR_NONE);
    write_close_space(ctx);
  }
  write_type_bindings(ctx, "param", func, &func->decl.sig.param_types,
                      &func->param_bindings);
  if (wasm_get_result_type(module, func) != WASM_TYPE_VOID) {
    write_open_space(ctx, "result");
    write_type(ctx, wasm_get_result_type(module, func), NEXT_CHAR_NONE);
    write_close_space(ctx);
  }
  write_newline(ctx, NO_FORCE_NEWLINE);
  if (func->local_types.size) {
    write_type_bindings(ctx, "local", func, &func->local_types,
                        &func->local_bindings);
  }
  write_newline(ctx, NO_FORCE_NEWLINE);
  ctx->depth = 1; /* for the implicit "return" label */
  write_expr_list(ctx, func->first_expr);
  write_close_newline(ctx);
}

static void write_import(Context* ctx,
                         int import_index,
                         const WasmImport* import) {
  write_open_space(ctx, "import");
  write_string_slice_or_index(ctx, &import->name, import_index,
                              NEXT_CHAR_SPACE);
  write_quoted_string_slice(ctx, &import->module_name, NEXT_CHAR_SPACE);
  write_quoted_string_slice(ctx, &import->func_name, NEXT_CHAR_SPACE);
  if (wasm_decl_has_func_type(&import->decl)) {
    write_open_space(ctx, "type");
    write_var(ctx, &import->decl.type_var, NEXT_CHAR_NONE);
    write_close_space(ctx);
  } else {
    write_func_sig_space(ctx, &import->decl.sig);
  }
  write_close_newline(ctx);
}

static void write_export(Context* ctx,
                         int export_index,
                         const WasmExport* export) {
  write_open_space(ctx, "export");
  write_quoted_string_slice(ctx, &export->name, NEXT_CHAR_SPACE);
  write_var(ctx, &export->var, NEXT_CHAR_SPACE);
  write_close_newline(ctx);
}

static void write_export_memory(Context* ctx, const WasmExportMemory* export) {
  write_open_space(ctx, "export");
  write_quoted_string_slice(ctx, &export->name, NEXT_CHAR_SPACE);
  write_puts_space(ctx, "memory");
  write_close_newline(ctx);
}

static void write_table(Context* ctx, const WasmVarVector* table) {
  write_open_space(ctx, "table");
  size_t i;
  for (i = 0; i < table->size; ++i)
    write_var(ctx, &table->data[i], NEXT_CHAR_SPACE);
  write_close_newline(ctx);
}

static void write_segment(Context* ctx, const WasmSegment* segment) {
  write_open_space(ctx, "segment");
  writef(ctx, "%u", segment->addr);
  write_quoted_data(ctx, segment->data, segment->size);
  write_close_newline(ctx);
}

static void write_memory(Context* ctx, const WasmMemory* memory) {
  write_open_space(ctx, "memory");
  writef(ctx, "%" PRIu64, memory->initial_pages);
  if (memory->initial_pages != memory->max_pages)
    writef(ctx, "%" PRIu64, memory->max_pages);
  write_newline(ctx, NO_FORCE_NEWLINE);
  size_t i;
  for (i = 0; i < memory->segments.size; ++i) {
    const WasmSegment* segment = &memory->segments.data[i];
    write_segment(ctx, segment);
  }
  write_close_newline(ctx);
}

static void write_func_type(Context* ctx,
                            int func_type_index,
                            const WasmFuncType* func_type) {
  write_open_space(ctx, "type");
  write_string_slice_or_index(ctx, &func_type->name, func_type_index,
                              NEXT_CHAR_SPACE);
  write_open_space(ctx, "func");
  write_func_sig_space(ctx, &func_type->sig);
  write_close_space(ctx);
  write_close_newline(ctx);
}

static void write_start_function(Context* ctx, const WasmVar* start) {
  write_open_space(ctx, "start");
  write_var(ctx, start, NEXT_CHAR_NONE);
  write_close_newline(ctx);
}

static void write_module(Context* ctx, const WasmModule* module) {
  write_open_newline(ctx, "module");
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
  write_close_newline(ctx);
  /* force the newline to be written */
  write_next_char(ctx);
}

WasmResult wasm_write_ast(WasmAllocator* allocator,
                          WasmWriter* writer,
                          const WasmModule* module) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.result = WASM_OK;
  wasm_init_stream(&ctx.stream, writer, NULL);
  write_module(&ctx, module);
  /* the memory for the actual string slice is shared with the module, so we
   * only need to free the vector */
  wasm_destroy_string_slice_vector(allocator, &ctx.index_to_name);
  return ctx.result;
}

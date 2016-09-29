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

  int func_index;
  int global_index;
  int export_index;
  int table_index;
  int memory_index;
  int func_type_index;
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

static void write_puts_newline(Context* ctx, const char* s) {
  write_puts(ctx, s, NEXT_CHAR_NEWLINE);
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

static void write_types(Context* ctx,
                        const WasmTypeVector* types,
                        const char* name) {
  if (types->size) {
    size_t i;
    if (name)
      write_open_space(ctx, name);
    for (i = 0; i < types->size; ++i)
      write_type(ctx, types->data[i], NEXT_CHAR_SPACE);
    if (name)
      write_close_space(ctx);
  }
}

static void write_func_sig_space(Context* ctx,
                                 const WasmFuncSignature* func_sig) {
  write_types(ctx, &func_sig->param_types, "param");
  write_types(ctx, &func_sig->result_types, "result");
}

static void write_expr_list(Context* ctx, const WasmExpr* first);

static void write_expr(Context* ctx, const WasmExpr* expr);

static void write_begin_block(Context* ctx,
                              const WasmBlock* block,
                              const char* text) {
  if (text)
    write_puts_space(ctx, text);
  WasmBool has_label =
      write_string_slice_opt(ctx, &block->label, NEXT_CHAR_SPACE);
  write_types(ctx, &block->sig, NULL);
  if (!has_label)
    writef(ctx, " ;; exit = @%d", ctx->depth);
  write_newline(ctx, FORCE_NEWLINE);
  ctx->depth++;
  indent(ctx);
}

static void write_end_block(Context* ctx, const char* text) {
  dedent(ctx);
  ctx->depth--;
  if (text)
    write_puts_newline(ctx, text);
}

static void write_block(Context* ctx,
                        const WasmBlock* block,
                        const char* start_text,
                        const char* end_text) {
  write_begin_block(ctx, block, start_text);
  write_expr_list(ctx, block->first);
  write_end_block(ctx, end_text);
}

static void write_const(Context* ctx, const WasmConst* const_) {
  switch (const_->type) {
    case WASM_TYPE_I32:
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_I32_CONST]);
      writef(ctx, "%d", (int32_t)const_->u32);
      write_newline(ctx, NO_FORCE_NEWLINE);
      break;

    case WASM_TYPE_I64:
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_I64_CONST]);
      writef(ctx, "%" PRId64, (int64_t)const_->u64);
      write_newline(ctx, NO_FORCE_NEWLINE);
      break;

    case WASM_TYPE_F32: {
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_F32_CONST]);
      char buffer[128];
      wasm_write_float_hex(buffer, 128, const_->f32_bits);
      write_puts_space(ctx, buffer);
      write_newline(ctx, NO_FORCE_NEWLINE);
      break;
    }

    case WASM_TYPE_F64: {
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_F64_CONST]);
      char buffer[128];
      wasm_write_double_hex(buffer, 128, const_->f64_bits);
      write_puts_space(ctx, buffer);
      write_newline(ctx, NO_FORCE_NEWLINE);
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
      write_puts_newline(ctx, s_opcode_name[expr->binary.opcode]);
      break;

    case WASM_EXPR_TYPE_BLOCK:
      write_block(ctx, &expr->block, s_opcode_name[WASM_OPCODE_BLOCK],
                  s_opcode_name[WASM_OPCODE_END]);
      break;

    case WASM_EXPR_TYPE_BR:
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_BR]);
      write_br_var(ctx, &expr->br.var, NEXT_CHAR_NEWLINE);
      break;

    case WASM_EXPR_TYPE_BR_IF:
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_BR_IF]);
      write_br_var(ctx, &expr->br_if.var, NEXT_CHAR_NEWLINE);
      break;

    case WASM_EXPR_TYPE_BR_TABLE: {
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_BR_TABLE]);
      size_t i;
      for (i = 0; i < expr->br_table.targets.size; ++i)
        write_br_var(ctx, &expr->br_table.targets.data[i], NEXT_CHAR_SPACE);
      write_br_var(ctx, &expr->br_table.default_target, NEXT_CHAR_NEWLINE);
      break;
    }

    case WASM_EXPR_TYPE_CALL:
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_CALL_FUNCTION]);
      write_var(ctx, &expr->call.var, NEXT_CHAR_NEWLINE);
      break;

    case WASM_EXPR_TYPE_CALL_INDIRECT:
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_CALL_INDIRECT]);
      write_var(ctx, &expr->call_indirect.var, NEXT_CHAR_NEWLINE);
      break;

    case WASM_EXPR_TYPE_COMPARE:
      write_puts_newline(ctx, s_opcode_name[expr->compare.opcode]);
      break;

    case WASM_EXPR_TYPE_CONST:
      write_const(ctx, &expr->const_);
      break;

    case WASM_EXPR_TYPE_CONVERT:
      write_puts_newline(ctx, s_opcode_name[expr->convert.opcode]);
      break;

    case WASM_EXPR_TYPE_DROP:
      write_puts_newline(ctx, s_opcode_name[WASM_OPCODE_DROP]);
      break;

    case WASM_EXPR_TYPE_GET_GLOBAL:
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_GET_GLOBAL]);
      write_var(ctx, &expr->get_global.var, NEXT_CHAR_NEWLINE);
      break;

    case WASM_EXPR_TYPE_GET_LOCAL:
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_GET_LOCAL]);
      write_var(ctx, &expr->get_local.var, NEXT_CHAR_NEWLINE);
      break;

    case WASM_EXPR_TYPE_GROW_MEMORY:
      write_puts_newline(ctx, s_opcode_name[WASM_OPCODE_GROW_MEMORY]);
      break;

    case WASM_EXPR_TYPE_IF:
      write_begin_block(ctx, &expr->if_.true_, s_opcode_name[WASM_OPCODE_IF]);
      write_expr_list(ctx, expr->if_.true_.first);
      if (expr->if_.false_) {
        dedent(ctx);
        write_puts_space(ctx, s_opcode_name[WASM_OPCODE_ELSE]);
        indent(ctx);
        write_newline(ctx, FORCE_NEWLINE);
        write_expr_list(ctx, expr->if_.false_);
      }
      write_end_block(ctx, s_opcode_name[WASM_OPCODE_END]);
      break;

    case WASM_EXPR_TYPE_LOAD:
      write_puts_space(ctx, s_opcode_name[expr->load.opcode]);
      if (expr->load.offset)
        writef(ctx, "offset=%" PRIu64, expr->load.offset);
      if (!wasm_is_naturally_aligned(expr->load.opcode, expr->load.align))
        writef(ctx, "align=%u", expr->load.align);
      write_newline(ctx, NO_FORCE_NEWLINE);
      break;

    case WASM_EXPR_TYPE_LOOP:
      write_block(ctx, &expr->loop, s_opcode_name[WASM_OPCODE_LOOP],
                  s_opcode_name[WASM_OPCODE_END]);
      break;

    case WASM_EXPR_TYPE_CURRENT_MEMORY:
      write_puts_newline(ctx, s_opcode_name[WASM_OPCODE_CURRENT_MEMORY]);
      break;

    case WASM_EXPR_TYPE_NOP:
      write_puts_newline(ctx, s_opcode_name[WASM_OPCODE_NOP]);
      break;

    case WASM_EXPR_TYPE_RETURN:
      write_puts_newline(ctx, s_opcode_name[WASM_OPCODE_RETURN]);
      break;

    case WASM_EXPR_TYPE_SELECT:
      write_puts_newline(ctx, s_opcode_name[WASM_OPCODE_SELECT]);
      break;

    case WASM_EXPR_TYPE_SET_GLOBAL:
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_SET_GLOBAL]);
      write_var(ctx, &expr->set_global.var, NEXT_CHAR_NEWLINE);
      break;

    case WASM_EXPR_TYPE_SET_LOCAL:
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_SET_LOCAL]);
      write_var(ctx, &expr->set_local.var, NEXT_CHAR_NEWLINE);
      break;

    case WASM_EXPR_TYPE_STORE:
      write_puts_space(ctx, s_opcode_name[expr->store.opcode]);
      if (expr->store.offset)
        writef(ctx, "offset=%" PRIu64, expr->store.offset);
      if (!wasm_is_naturally_aligned(expr->store.opcode, expr->store.align))
        writef(ctx, "align=%u", expr->store.align);
      write_newline(ctx, NO_FORCE_NEWLINE);
      break;

    case WASM_EXPR_TYPE_TEE_LOCAL:
      write_puts_space(ctx, s_opcode_name[WASM_OPCODE_TEE_LOCAL]);
      write_var(ctx, &expr->tee_local.var, NEXT_CHAR_NEWLINE);
      break;

    case WASM_EXPR_TYPE_UNARY:
      write_puts_newline(ctx, s_opcode_name[expr->unary.opcode]);
      break;

    case WASM_EXPR_TYPE_UNREACHABLE:
      write_puts_newline(ctx, s_opcode_name[WASM_OPCODE_UNREACHABLE]);
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

static void write_init_expr(Context* ctx, const WasmExpr* expr) {
  if (expr) {
    write_puts(ctx, "(", NEXT_CHAR_NONE);
    write_expr(ctx, expr);
    /* clear the next char, so we don't write a newline after the expr */
    ctx->next_char = NEXT_CHAR_NONE;
    write_puts(ctx, ")", NEXT_CHAR_SPACE);
  }
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
                       const WasmFunc* func) {
  write_open_space(ctx, "func");
  write_string_slice_or_index(ctx, &func->name, ctx->func_index,
                              NEXT_CHAR_SPACE);
  if (wasm_decl_has_func_type(&func->decl)) {
    write_open_space(ctx, "type");
    write_var(ctx, &func->decl.type_var, NEXT_CHAR_NONE);
    write_close_space(ctx);
  }
  write_type_bindings(ctx, "param", func, &func->decl.sig.param_types,
                      &func->param_bindings);
  write_types(ctx, &func->decl.sig.result_types, "result");
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

static void write_begin_global(Context* ctx, const WasmGlobal* global) {
  write_open_space(ctx, "global");
  write_string_slice_or_index(ctx, &global->name, ctx->global_index++,
                              NEXT_CHAR_SPACE);
  if (global->mutable_) {
    write_open_space(ctx, "mut");
    write_type(ctx, global->type, NEXT_CHAR_SPACE);
    write_close_space(ctx);
  } else {
    write_type(ctx, global->type, NEXT_CHAR_SPACE);
  }
}

static void write_global(Context* ctx, const WasmGlobal* global) {
  write_begin_global(ctx, global);
  write_init_expr(ctx, global->init_expr);
  write_close_newline(ctx);
}

static void write_limits(Context* ctx, const WasmLimits* limits) {
  writef(ctx, "%" PRIu64, limits->initial);
  if (limits->has_max)
    writef(ctx, "%" PRIu64, limits->max);
}

static void write_table(Context* ctx, const WasmTable* table) {
  write_open_space(ctx, "table");
  write_string_slice_or_index(ctx, &table->name, ctx->table_index++,
                              NEXT_CHAR_SPACE);
  write_limits(ctx, &table->elem_limits);
  write_puts_space(ctx, "anyfunc");
  write_close_newline(ctx);
}

static void write_elem_segment(Context* ctx, const WasmElemSegment* segment) {
  write_open_space(ctx, "elem");
  write_init_expr(ctx, segment->offset);
  size_t i;
  for (i = 0; i < segment->vars.size; ++i)
    write_var(ctx, &segment->vars.data[i], NEXT_CHAR_SPACE);
  write_close_newline(ctx);
}

static void write_memory(Context* ctx, const WasmMemory* memory) {
  write_open_space(ctx, "memory");
  write_string_slice_or_index(ctx, &memory->name, ctx->memory_index++,
                              NEXT_CHAR_SPACE);
  write_limits(ctx, &memory->page_limits);
  write_close_newline(ctx);
}

static void write_data_segment(Context* ctx, const WasmDataSegment* segment) {
  write_open_space(ctx, "data");
  write_init_expr(ctx, segment->offset);
  write_quoted_data(ctx, segment->data, segment->size);
  write_close_newline(ctx);
}

static void write_import(Context* ctx, const WasmImport* import) {
  write_open_space(ctx, "import");
  uint32_t index = 0;
  switch (import->kind) {
    case WASM_EXTERNAL_KIND_FUNC:
      index = ctx->func_index;
      break;
    case WASM_EXTERNAL_KIND_TABLE:
      index = ctx->table_index;
      break;
    case WASM_EXTERNAL_KIND_MEMORY:
      index = ctx->memory_index;
      break;
    case WASM_EXTERNAL_KIND_GLOBAL:
      index = ctx->global_index;
      break;
    case WASM_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
  write_quoted_string_slice(ctx, &import->module_name, NEXT_CHAR_SPACE);
  write_quoted_string_slice(ctx, &import->field_name, NEXT_CHAR_SPACE);
  switch (import->kind) {
    case WASM_EXTERNAL_KIND_FUNC:
      write_open_space(ctx, "func");
      write_string_slice_or_index(ctx, &import->func.name, index,
                                  NEXT_CHAR_SPACE);
      if (wasm_decl_has_func_type(&import->func.decl)) {
        write_open_space(ctx, "type");
        write_var(ctx, &import->func.decl.type_var, NEXT_CHAR_NONE);
        write_close_space(ctx);
      } else {
        write_func_sig_space(ctx, &import->func.decl.sig);
      }
      write_close_space(ctx);
      break;

    case WASM_EXTERNAL_KIND_TABLE:
      write_table(ctx, &import->table);
      break;

    case WASM_EXTERNAL_KIND_MEMORY:
      write_memory(ctx, &import->memory);
      break;

    case WASM_EXTERNAL_KIND_GLOBAL:
      write_begin_global(ctx, &import->global);
      write_close_space(ctx);
      break;

    case WASM_NUM_EXTERNAL_KINDS:
      assert(0);
      break;
  }
  write_close_newline(ctx);
}

static void write_export(Context* ctx, const WasmExport* export) {
  static const char* s_kind_names[] = {"func", "table", "memory", "global"};
  WASM_STATIC_ASSERT(WASM_ARRAY_SIZE(s_kind_names) == WASM_NUM_EXTERNAL_KINDS);
  write_open_space(ctx, "export");
  write_quoted_string_slice(ctx, &export->name, NEXT_CHAR_SPACE);
  assert(export->kind < WASM_ARRAY_SIZE(s_kind_names));
  write_open_space(ctx, s_kind_names[export->kind]);
  write_var(ctx, &export->var, NEXT_CHAR_SPACE);
  write_close_space(ctx);
  write_close_newline(ctx);
}

static void write_func_type(Context* ctx, const WasmFuncType* func_type) {
  write_open_space(ctx, "type");
  write_string_slice_or_index(ctx, &func_type->name, ctx->func_type_index++,
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
  for (field = module->first_field; field != NULL; field = field->next) {
    switch (field->type) {
      case WASM_MODULE_FIELD_TYPE_FUNC:
        write_func(ctx, module, &field->func);
        break;
      case WASM_MODULE_FIELD_TYPE_GLOBAL:
        write_global(ctx, &field->global);
        break;
      case WASM_MODULE_FIELD_TYPE_IMPORT:
        write_import(ctx, &field->import);
        break;
      case WASM_MODULE_FIELD_TYPE_EXPORT:
        write_export(ctx, &field->export_);
        break;
      case WASM_MODULE_FIELD_TYPE_TABLE:
        write_table(ctx, &field->table);
        break;
      case WASM_MODULE_FIELD_TYPE_ELEM_SEGMENT:
        write_elem_segment(ctx, &field->elem_segment);
        break;
      case WASM_MODULE_FIELD_TYPE_MEMORY:
        write_memory(ctx, &field->memory);
        break;
      case WASM_MODULE_FIELD_TYPE_DATA_SEGMENT:
        write_data_segment(ctx, &field->data_segment);
        break;
      case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
        write_func_type(ctx, &field->func_type);
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

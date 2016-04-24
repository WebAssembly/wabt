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

#include "wasm-binary-writer.h"
#include "wasm-config.h"

#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "wasm-ast.h"
#include "wasm-binary.h"
#include "wasm-stream.h"
#include "wasm-writer.h"

#define PRINT_HEADER_NO_INDEX -1
#define MAX_U32_LEB128_BYTES 5
#define MAX_U64_LEB128_BYTES 10

#define ALLOC_FAILURE \
  fprintf(stderr, "%s:%d: allocation failed\n", __FILE__, __LINE__)

#define CHECK_ALLOC_(cond)             \
  do {                                 \
    if (!(cond)) {                     \
      ALLOC_FAILURE;                   \
      ctx->stream.result = WASM_ERROR; \
      return;                          \
    }                                  \
  } while (0)

#define CHECK_ALLOC(e) CHECK_ALLOC_(WASM_SUCCEEDED(e))
#define CHECK_ALLOC_NULL(v) CHECK_ALLOC_((v) != NULL)

WASM_STATIC_ASSERT(WASM_TYPE_VOID == 0);
WASM_STATIC_ASSERT(WASM_TYPE_I32 == 1);
WASM_STATIC_ASSERT(WASM_TYPE_I64 == 2);
WASM_STATIC_ASSERT(WASM_TYPE_F32 == 3);
WASM_STATIC_ASSERT(WASM_TYPE_F64 == 4);

static const char* s_type_names[] = {
    "WASM_TYPE_VOID", "WASM_TYPE_I32", "WASM_TYPE_I64",
    "WASM_TYPE_F32",  "WASM_TYPE_F64",
};
WASM_STATIC_ASSERT(WASM_ARRAY_SIZE(s_type_names) == WASM_NUM_TYPES);

#define V(rtype, type1, type2, mem_size, code, NAME, text) \
  [code] = "OPCODE_" #NAME,
static const char* s_opcode_name[] = {WASM_FOREACH_OPCODE(V)};
#undef V

typedef struct WasmLabelNode {
  const WasmLabel* label;
  int depth;
  struct WasmLabelNode* next;
} WasmLabelNode;

typedef struct WasmContext {
  WasmAllocator* allocator;
  WasmStream stream;
  WasmStream* log_stream;
  const WasmWriteBinaryOptions* options;
  WasmLabelNode* top_label;
  int max_depth;

  size_t last_section_offset;
  size_t last_section_leb_size_guess;

  int* import_sig_indexes;
  int* func_sig_indexes;
  uint32_t* remapped_locals;         /* from unpacked -> packed index */
  uint32_t* reverse_remapped_locals; /* from packed -> unpacked index */
} WasmContext;

WASM_DEFINE_VECTOR(func_signature, WasmFuncSignature);

static void destroy_func_signature_vector_and_elements(
    WasmAllocator* allocator,
    WasmFuncSignatureVector* sigs) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, *sigs, func_signature);
}

static void write_header(WasmContext* ctx, const char* name, int index) {
  if (ctx->log_stream) {
    if (index == PRINT_HEADER_NO_INDEX) {
      wasm_writef(ctx->log_stream, "; %s\n", name);
    } else {
      wasm_writef(ctx->log_stream, "; %s %d\n", name, index);
    }
  }
}

#define LEB128_LOOP_UNTIL(end_cond) \
  do {                              \
    uint8_t byte = value & 0x7f;    \
    value >>= 7;                    \
    if (end_cond) {                 \
      data[i++] = byte;             \
      break;                        \
    } else {                        \
      data[i++] = byte | 0x80;      \
    }                               \
  } while (1)

/* returns the length of the leb128 */
static uint32_t write_u32_leb128_at(WasmStream* stream,
                                    uint32_t offset,
                                    uint32_t value,
                                    const char* desc) {
  uint8_t data[MAX_U32_LEB128_BYTES];
  uint32_t i = 0;
  LEB128_LOOP_UNTIL(value == 0);
  uint32_t length = i;
  wasm_write_data_at(stream, offset, data, length, WASM_DONT_PRINT_CHARS, desc);
  return length;
}

static uint32_t write_fixed_u32_leb128_at(WasmStream* stream,
                                          uint32_t offset,
                                          uint32_t value,
                                          const char* desc) {
  uint8_t data[MAX_U32_LEB128_BYTES];
  data[0] = (value & 0x7f) | 0x80;
  data[1] = ((value >> 7) & 0x7f) | 0x80;
  data[2] = ((value >> 14) & 0x7f) | 0x80;
  data[3] = ((value >> 21) & 0x7f) | 0x80;
  data[4] = ((value >> 28) & 0x0f);
  wasm_write_data_at(stream, offset, data, MAX_U32_LEB128_BYTES,
                     WASM_DONT_PRINT_CHARS, desc);
  return MAX_U32_LEB128_BYTES;
}

static void write_u32_leb128(WasmStream* stream,
                             uint32_t value,
                             const char* desc) {
  uint32_t length = write_u32_leb128_at(stream, stream->offset, value, desc);
  stream->offset += length;
}

static void write_i32_leb128(WasmStream* stream,
                             int32_t value,
                             const char* desc) {
  uint8_t data[MAX_U32_LEB128_BYTES];
  uint32_t i = 0;
  if (value < 0)
    LEB128_LOOP_UNTIL(value == -1 && (byte & 0x40));
  else
    LEB128_LOOP_UNTIL(value == 0 && !(byte & 0x40));

  uint32_t length = i;
  wasm_write_data_at(stream, stream->offset, data, length,
                     WASM_DONT_PRINT_CHARS, desc);
  stream->offset += length;
}

static void write_i64_leb128(WasmStream* stream,
                             int64_t value,
                             const char* desc) {
  uint8_t data[MAX_U64_LEB128_BYTES];
  uint32_t i = 0;
  if (value < 0)
    LEB128_LOOP_UNTIL(value == -1 && (byte & 0x40));
  else
    LEB128_LOOP_UNTIL(value == 0 && !(byte & 0x40));

  int length = i;
  wasm_write_data_at(stream, stream->offset, data, length,
                     WASM_DONT_PRINT_CHARS, desc);
  stream->offset += length;
}

#undef LEB128_LOOP_UNTIL

static uint32_t size_u32_leb128(uint32_t value) {
  uint32_t size = 0;
  do {
    value >>= 7;
    size++;
  } while (value != 0);
  return size;
}

/* returns offset of leb128 */
static uint32_t write_u32_leb128_space(WasmContext* ctx,
                                       uint32_t leb_size_guess,
                                       const char* desc) {
  assert(leb_size_guess <= MAX_U32_LEB128_BYTES);
  uint8_t data[MAX_U32_LEB128_BYTES] = {0};
  uint32_t result = ctx->stream.offset;
  uint32_t bytes_to_write =
      ctx->options->canonicalize_lebs ? leb_size_guess : MAX_U32_LEB128_BYTES;
  wasm_write_data(&ctx->stream, data, bytes_to_write, desc);
  return result;
}

static void write_fixup_u32_leb128_size(WasmContext* ctx,
                                        uint32_t offset,
                                        uint32_t leb_size_guess,
                                        const char* desc) {
  if (ctx->options->canonicalize_lebs) {
    uint32_t size = ctx->stream.offset - offset - leb_size_guess;
    uint32_t leb_size = size_u32_leb128(size);
    if (leb_size != leb_size_guess) {
      uint32_t src_offset = offset + leb_size_guess;
      uint32_t dst_offset = offset + leb_size;
      wasm_move_data(&ctx->stream, dst_offset, src_offset, size);
    }
    write_u32_leb128_at(&ctx->stream, offset, size, desc);
    ctx->stream.offset += leb_size - leb_size_guess;
  } else {
    uint32_t size = ctx->stream.offset - offset - MAX_U32_LEB128_BYTES;
    write_fixed_u32_leb128_at(&ctx->stream, offset, size, desc);
  }
}

static void write_str(WasmStream* stream,
                      const char* s,
                      size_t length,
                      WasmPrintChars print_chars,
                      const char* desc) {
  write_u32_leb128(stream, length, "string length");
  wasm_write_data_at(stream, stream->offset, s, length, print_chars, desc);
  stream->offset += length;
}

static void write_opcode(WasmStream* stream, uint8_t opcode) {
  wasm_write_u8(stream, opcode, s_opcode_name[opcode]);
}

static void begin_section(WasmContext* ctx,
                          const char* name,
                          size_t leb_size_guess) {
  assert(ctx->last_section_leb_size_guess == 0);
  char desc[100];
  wasm_snprintf(desc, sizeof(desc), "section \"%s\"", name);
  write_header(ctx, desc, PRINT_HEADER_NO_INDEX);
  ctx->last_section_offset =
      write_u32_leb128_space(ctx, leb_size_guess, "section size (guess)");
  ctx->last_section_leb_size_guess = leb_size_guess;
  wasm_snprintf(desc, sizeof(desc), "section id: \"%s\"", name);
  write_str(&ctx->stream, name, strlen(name), WASM_DONT_PRINT_CHARS, desc);
}

static void end_section(WasmContext* ctx) {
  assert(ctx->last_section_leb_size_guess != 0);
  write_fixup_u32_leb128_size(ctx, ctx->last_section_offset,
                              ctx->last_section_leb_size_guess,
                              "FIXUP section size");
  ctx->last_section_leb_size_guess = 0;
}

static WasmLabelNode* find_label_by_name(WasmLabelNode* top_label,
                                         const WasmStringSlice* name) {
  WasmLabelNode* node = top_label;
  while (node) {
    if (node->label && wasm_string_slices_are_equal(node->label, name))
      return node;
    node = node->next;
  }
  return NULL;
}

static WasmLabelNode* find_label_by_var(WasmLabelNode* top_label,
                                        const WasmVar* var) {
  if (var->type == WASM_VAR_TYPE_NAME)
    return find_label_by_name(top_label, &var->name);

  WasmLabelNode* node = top_label;
  int i = 0;
  while (node && i != var->index) {
    node = node->next;
    i++;
  }
  return node;
}

static void push_unused_label(WasmContext* ctx,
                              WasmLabelNode* node,
                              const WasmLabel* label) {
  assert(label);
  node->label = label;
  node->next = ctx->top_label;
  node->depth = ctx->max_depth;
  ctx->top_label = node;
}

static void pop_unused_label(WasmContext* ctx, const WasmLabel* label) {
  if (ctx->top_label && ctx->top_label->label == label)
    ctx->top_label = ctx->top_label->next;
}

static void push_label(WasmContext* ctx,
                       WasmLabelNode* node,
                       const WasmLabel* label) {
  push_unused_label(ctx, node, label);
  ctx->max_depth++;
}

static void pop_label(WasmContext* ctx, const WasmLabel* label) {
  ctx->max_depth--;
  pop_unused_label(ctx, label);
}

static int find_func_signature(const WasmFuncSignatureVector* sigs,
                               const WasmType result_type,
                               const WasmTypeVector* param_types) {
  size_t i;
  for (i = 0; i < sigs->size; ++i) {
    const WasmFuncSignature* sig2 = &sigs->data[i];
    if (sig2->result_type != result_type)
      continue;
    if (sig2->param_types.size != param_types->size)
      continue;
    size_t j;
    for (j = 0; j < param_types->size; ++j) {
      if (sig2->param_types.data[j] != param_types->data[j])
        break;
    }
    if (j == param_types->size)
      return i;
  }
  return -1;
}

static void get_or_create_func_signature(WasmContext* ctx,
                                         WasmFuncSignatureVector* sigs,
                                         WasmType result_type,
                                         const WasmTypeVector* param_types,
                                         int* out_index) {
  int index = find_func_signature(sigs, result_type, param_types);
  if (index == -1) {
    index = sigs->size;
    WasmFuncSignature* sig = wasm_append_func_signature(ctx->allocator, sigs);
    CHECK_ALLOC_NULL(sig);
    sig->result_type = result_type;
    WASM_ZERO_MEMORY(sig->param_types);
    CHECK_ALLOC(
        wasm_extend_types(ctx->allocator, &sig->param_types, param_types));
  }
  *out_index = index;
}

static int get_signature_index(WasmContext* ctx,
                               const WasmModule* module,
                               WasmFuncSignatureVector* sigs,
                               const WasmFuncDeclaration* decl) {
  int index = -1;
  if (wasm_decl_has_signature(decl)) {
    get_or_create_func_signature(ctx, sigs, decl->sig.result_type,
                                 &decl->sig.param_types, &index);
  } else {
    assert(wasm_decl_has_func_type(decl));
    index = wasm_get_func_type_index_by_var(module, &decl->type_var);
    assert(index != -1);
  }
  return index;
}

static void get_func_signatures(WasmContext* ctx,
                                const WasmModule* module,
                                WasmFuncSignatureVector* sigs) {
  /* function types are not deduped; we don't want the signature index to match
   if they were specified separately in the source */
  size_t i;
  for (i = 0; i < module->func_types.size; ++i) {
    const WasmFuncType* func_type = module->func_types.data[i];
    WasmFuncSignature* sig = wasm_append_func_signature(ctx->allocator, sigs);
    CHECK_ALLOC_NULL(sig);
    sig->result_type = func_type->sig.result_type;
    WASM_ZERO_MEMORY(sig->param_types);
    CHECK_ALLOC(wasm_extend_types(ctx->allocator, &sig->param_types,
                                  &func_type->sig.param_types));
  }

  ctx->import_sig_indexes =
      wasm_realloc(ctx->allocator, ctx->import_sig_indexes,
                   module->imports.size * sizeof(int), WASM_DEFAULT_ALIGN);
  if (module->imports.size)
    CHECK_ALLOC_NULL(ctx->import_sig_indexes);
  for (i = 0; i < module->imports.size; ++i) {
    const WasmImport* import = module->imports.data[i];
    ctx->import_sig_indexes[i] =
        get_signature_index(ctx, module, sigs, &import->decl);
  }

  ctx->func_sig_indexes =
      wasm_realloc(ctx->allocator, ctx->func_sig_indexes,
                   module->funcs.size * sizeof(int), WASM_DEFAULT_ALIGN);
  if (module->funcs.size)
    CHECK_ALLOC_NULL(ctx->func_sig_indexes);
  for (i = 0; i < module->funcs.size; ++i) {
    const WasmFunc* func = module->funcs.data[i];
    ctx->func_sig_indexes[i] =
        get_signature_index(ctx, module, sigs, &func->decl);
  }
}

static void remap_locals(WasmContext* ctx, const WasmFunc* func) {
  uint32_t i;
  uint32_t num_params = wasm_get_num_params(func);
  uint32_t num_locals = func->local_types.size;
  uint32_t num_params_and_locals = num_params + num_locals;
  ctx->remapped_locals = wasm_realloc(ctx->allocator, ctx->remapped_locals,
                                      num_params_and_locals * sizeof(uint32_t),
                                      WASM_DEFAULT_ALIGN);
  if (num_params_and_locals)
    CHECK_ALLOC_NULL(ctx->remapped_locals);

  ctx->reverse_remapped_locals = wasm_realloc(
      ctx->allocator, ctx->reverse_remapped_locals,
      num_params_and_locals * sizeof(uint32_t), WASM_DEFAULT_ALIGN);
  if (num_locals)
    CHECK_ALLOC_NULL(ctx->reverse_remapped_locals);

  if (!ctx->options->remap_locals) {
    /* just pass the index straight through */
    for (i = 0; i < num_params_and_locals; ++i)
      ctx->remapped_locals[i] = i;
    for (i = 0; i < num_params_and_locals; ++i)
      ctx->reverse_remapped_locals[i] = i;
    return;
  }

  uint32_t max[WASM_NUM_TYPES];
  WASM_ZERO_MEMORY(max);
  for (i = 0; i < num_locals; ++i) {
    WasmType type = func->local_types.data[i];
    max[type]++;
  }

  /* params don't need remapping */
  for (i = 0; i < num_params; ++i) {
    ctx->remapped_locals[i] = i;
    ctx->reverse_remapped_locals[i] = i;
  }

  uint32_t start[WASM_NUM_TYPES];
  start[WASM_TYPE_I32] = num_params;
  start[WASM_TYPE_I64] = start[WASM_TYPE_I32] + max[WASM_TYPE_I32];
  start[WASM_TYPE_F32] = start[WASM_TYPE_I64] + max[WASM_TYPE_I64];
  start[WASM_TYPE_F64] = start[WASM_TYPE_F32] + max[WASM_TYPE_F32];

  uint32_t seen[WASM_NUM_TYPES];
  WASM_ZERO_MEMORY(seen);
  for (i = 0; i < num_locals; ++i) {
    WasmType type = func->local_types.data[i];
    uint32_t unpacked_index = num_params + i;
    uint32_t packed_index = start[type] + seen[type]++;
    ctx->remapped_locals[unpacked_index] = packed_index;
    ctx->reverse_remapped_locals[packed_index] = unpacked_index;
  }
}

static void write_expr_list(WasmContext* ctx,
                            const WasmModule* module,
                            const WasmFunc* func,
                            const WasmExprPtrVector* exprs);

static void write_expr(WasmContext* ctx,
                       const WasmModule* module,
                       const WasmFunc* func,
                       const WasmExpr* expr) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      write_expr(ctx, module, func, expr->binary.left);
      write_expr(ctx, module, func, expr->binary.right);
      write_opcode(&ctx->stream, expr->binary.opcode);
      break;
    case WASM_EXPR_TYPE_BLOCK: {
      WasmLabelNode node;
      push_label(ctx, &node, &expr->block.label);
      write_opcode(&ctx->stream, WASM_OPCODE_BLOCK);
      write_expr_list(ctx, module, func, &expr->block.exprs);
      write_opcode(&ctx->stream, WASM_OPCODE_END);
      pop_label(ctx, &expr->block.label);
      break;
    }
    case WASM_EXPR_TYPE_BR: {
      WasmLabelNode* node = find_label_by_var(ctx->top_label, &expr->br.var);
      assert(node);
      if (expr->br.expr)
        write_expr(ctx, module, func, expr->br.expr);
      else
        write_opcode(&ctx->stream, WASM_OPCODE_NOP);
      write_opcode(&ctx->stream, WASM_OPCODE_BR);
      write_u32_leb128(&ctx->stream, ctx->max_depth - node->depth - 1,
                       "break depth");
      break;
    }
    case WASM_EXPR_TYPE_BR_IF: {
      WasmLabelNode* node = find_label_by_var(ctx->top_label, &expr->br_if.var);
      assert(node);
      if (expr->br_if.expr)
        write_expr(ctx, module, func, expr->br_if.expr);
      else
        write_opcode(&ctx->stream, WASM_OPCODE_NOP);
      write_expr(ctx, module, func, expr->br_if.cond);
      write_opcode(&ctx->stream, WASM_OPCODE_BR_IF);
      write_u32_leb128(&ctx->stream, ctx->max_depth - node->depth - 1,
                       "break depth");
      break;
    }
    case WASM_EXPR_TYPE_CALL: {
      int index = wasm_get_func_index_by_var(module, &expr->call.var);
      assert(index >= 0 && (size_t)index < module->funcs.size);
      write_expr_list(ctx, module, func, &expr->call.args);
      write_opcode(&ctx->stream, WASM_OPCODE_CALL_FUNCTION);
      write_u32_leb128(&ctx->stream, index, "func index");
      break;
    }
    case WASM_EXPR_TYPE_CALL_IMPORT: {
      int index = wasm_get_import_index_by_var(module, &expr->call.var);
      assert(index >= 0 && (size_t)index < module->imports.size);
      write_expr_list(ctx, module, func, &expr->call.args);
      write_opcode(&ctx->stream, WASM_OPCODE_CALL_IMPORT);
      write_u32_leb128(&ctx->stream, index, "import index");
      break;
    }
    case WASM_EXPR_TYPE_CALL_INDIRECT: {
      int index =
          wasm_get_func_type_index_by_var(module, &expr->call_indirect.var);
      assert(index >= 0 && (size_t)index < module->func_types.size);
      write_expr(ctx, module, func, expr->call_indirect.expr);
      write_expr_list(ctx, module, func, &expr->call_indirect.args);
      write_opcode(&ctx->stream, WASM_OPCODE_CALL_INDIRECT);
      write_u32_leb128(&ctx->stream, index, "signature index");
      break;
    }
    case WASM_EXPR_TYPE_COMPARE:
      write_expr(ctx, module, func, expr->compare.left);
      write_expr(ctx, module, func, expr->compare.right);
      write_opcode(&ctx->stream, expr->compare.opcode);
      break;
    case WASM_EXPR_TYPE_CONST:
      switch (expr->const_.type) {
        case WASM_TYPE_I32: {
          write_opcode(&ctx->stream, WASM_OPCODE_I32_CONST);
          write_i32_leb128(&ctx->stream, (int32_t)expr->const_.u32,
                           "i32 literal");
          break;
        }
        case WASM_TYPE_I64:
          write_opcode(&ctx->stream, WASM_OPCODE_I64_CONST);
          write_i64_leb128(&ctx->stream, (int64_t)expr->const_.u64,
                           "i64 literal");
          break;
        case WASM_TYPE_F32:
          write_opcode(&ctx->stream, WASM_OPCODE_F32_CONST);
          wasm_write_u32(&ctx->stream, expr->const_.f32_bits, "f32 literal");
          break;
        case WASM_TYPE_F64:
          write_opcode(&ctx->stream, WASM_OPCODE_F64_CONST);
          wasm_write_u64(&ctx->stream, expr->const_.f64_bits, "f64 literal");
          break;
        default:
          assert(0);
      }
      break;
    case WASM_EXPR_TYPE_CONVERT:
      write_expr(ctx, module, func, expr->convert.expr);
      write_opcode(&ctx->stream, expr->convert.opcode);
      break;
    case WASM_EXPR_TYPE_GET_LOCAL: {
      int index = wasm_get_local_index_by_var(func, &expr->get_local.var);
      write_opcode(&ctx->stream, WASM_OPCODE_GET_LOCAL);
      write_u32_leb128(&ctx->stream, ctx->remapped_locals[index],
                       "remapped local index");
      break;
    }
    case WASM_EXPR_TYPE_GROW_MEMORY:
      write_expr(ctx, module, func, expr->grow_memory.expr);
      write_opcode(&ctx->stream, WASM_OPCODE_GROW_MEMORY);
      break;
    case WASM_EXPR_TYPE_IF: {
      WasmLabelNode node;
      write_expr(ctx, module, func, expr->if_.cond);
      write_opcode(&ctx->stream, WASM_OPCODE_IF);
      push_label(ctx, &node, &expr->if_.true_.label);
      write_expr_list(ctx, module, func, &expr->if_.true_.exprs);
      pop_label(ctx, &expr->if_.true_.label);
      write_opcode(&ctx->stream, WASM_OPCODE_END);
      break;
    }
    case WASM_EXPR_TYPE_IF_ELSE: {
      WasmLabelNode node;
      write_expr(ctx, module, func, expr->if_else.cond);
      write_opcode(&ctx->stream, WASM_OPCODE_IF);
      push_label(ctx, &node, &expr->if_else.true_.label);
      write_expr_list(ctx, module, func, &expr->if_else.true_.exprs);
      pop_label(ctx, &expr->if_else.true_.label);
      write_opcode(&ctx->stream, WASM_OPCODE_ELSE);
      push_label(ctx, &node, &expr->if_else.false_.label);
      write_expr_list(ctx, module, func, &expr->if_else.false_.exprs);
      pop_label(ctx, &expr->if_else.false_.label);
      write_opcode(&ctx->stream, WASM_OPCODE_END);
      break;
    }
    case WASM_EXPR_TYPE_LOAD: {
      write_expr(ctx, module, func, expr->load.addr);
      write_opcode(&ctx->stream, expr->load.opcode);
      uint32_t align =
          wasm_get_opcode_alignment(expr->load.opcode, expr->load.align);
      uint8_t align_log = 0;
      while (align > 1) {
        align >>= 1;
        align_log++;
      }
      wasm_write_u8(&ctx->stream, align_log, "alignment");
      write_u32_leb128(&ctx->stream, (uint32_t)expr->load.offset,
                       "load offset");
      break;
    }
    case WASM_EXPR_TYPE_LOOP: {
      WasmLabelNode outer;
      WasmLabelNode inner;
      push_label(ctx, &outer, &expr->loop.outer);
      push_label(ctx, &inner, &expr->loop.inner);
      write_opcode(&ctx->stream, WASM_OPCODE_LOOP);
      write_expr_list(ctx, module, func, &expr->loop.exprs);
      write_opcode(&ctx->stream, WASM_OPCODE_END);
      pop_label(ctx, &expr->loop.inner);
      pop_label(ctx, &expr->loop.outer);
      break;
    }
    case WASM_EXPR_TYPE_MEMORY_SIZE:
      write_opcode(&ctx->stream, WASM_OPCODE_MEMORY_SIZE);
      break;
    case WASM_EXPR_TYPE_NOP:
      write_opcode(&ctx->stream, WASM_OPCODE_NOP);
      break;
    case WASM_EXPR_TYPE_RETURN:
      if (expr->return_.expr)
        write_expr(ctx, module, func, expr->return_.expr);
      write_opcode(&ctx->stream, WASM_OPCODE_RETURN);
      break;
    case WASM_EXPR_TYPE_SELECT:
      write_expr(ctx, module, func, expr->select.true_);
      write_expr(ctx, module, func, expr->select.false_);
      write_expr(ctx, module, func, expr->select.cond);
      write_opcode(&ctx->stream, WASM_OPCODE_SELECT);
      break;
    case WASM_EXPR_TYPE_SET_LOCAL: {
      int index = wasm_get_local_index_by_var(func, &expr->get_local.var);
      write_expr(ctx, module, func, expr->set_local.expr);
      write_opcode(&ctx->stream, WASM_OPCODE_SET_LOCAL);
      write_u32_leb128(&ctx->stream, ctx->remapped_locals[index],
                       "remapped local index");
      break;
    }
    case WASM_EXPR_TYPE_STORE: {
      write_expr(ctx, module, func, expr->store.addr);
      write_expr(ctx, module, func, expr->store.value);
      write_opcode(&ctx->stream, expr->store.opcode);
      uint32_t align =
          wasm_get_opcode_alignment(expr->store.opcode, expr->store.align);
      uint8_t align_log = 0;
      while (align > 1) {
        align >>= 1;
        align_log++;
      }
      wasm_write_u8(&ctx->stream, align_log, "alignment");
      write_u32_leb128(&ctx->stream, (uint32_t)expr->store.offset,
                       "store offset");
      break;
    }
    case WASM_EXPR_TYPE_BR_TABLE: {
      write_expr(ctx, module, func, expr->br_table.expr);
      write_opcode(&ctx->stream, WASM_OPCODE_BR_TABLE);
      write_u32_leb128(&ctx->stream, expr->br_table.targets.size,
                       "num targets");
      size_t i;
      WasmLabelNode* node;
      for (i = 0; i < expr->br_table.targets.size; ++i) {
        const WasmVar* var = &expr->br_table.targets.data[i];
        node = find_label_by_var(ctx->top_label, var);
        wasm_write_u32(&ctx->stream, ctx->max_depth - node->depth - 1,
                       "break depth");
      }
      node = find_label_by_var(ctx->top_label, &expr->br_table.default_target);
      wasm_write_u32(&ctx->stream, ctx->max_depth - node->depth - 1,
                     "break depth for default");
      break;
    }
    case WASM_EXPR_TYPE_UNARY:
      write_expr(ctx, module, func, expr->unary.expr);
      write_opcode(&ctx->stream, expr->unary.opcode);
      break;
    case WASM_EXPR_TYPE_UNREACHABLE:
      write_opcode(&ctx->stream, WASM_OPCODE_UNREACHABLE);
      break;
  }
}

static void write_expr_list(WasmContext* ctx,
                            const WasmModule* module,
                            const WasmFunc* func,
                            const WasmExprPtrVector* exprs) {
  size_t i;
  for (i = 0; i < exprs->size; ++i)
    write_expr(ctx, module, func, exprs->data[i]);
}

static void write_func_locals(WasmContext* ctx,
                              const WasmModule* module,
                              const WasmFunc* func,
                              const WasmTypeVector* local_types) {
  remap_locals(ctx, func);

  if (local_types->size == 0) {
    write_u32_leb128(&ctx->stream, 0, "local decl count");
    return;
  }

  uint32_t num_params = wasm_get_num_params(func);

#define FIRST_LOCAL_INDEX (num_params)
#define LAST_LOCAL_INDEX (num_params + local_types->size)
#define GET_LOCAL_TYPE(x) \
  (local_types->data[ctx->reverse_remapped_locals[x] - num_params])

  /* loop through once to count the number of local declaration runs */
  WasmType current_type = GET_LOCAL_TYPE(FIRST_LOCAL_INDEX);
  uint32_t local_decl_count = 1;
  uint32_t i;
  for (i = FIRST_LOCAL_INDEX + 1; i < LAST_LOCAL_INDEX; ++i) {
    WasmType type = GET_LOCAL_TYPE(i);
    if (current_type != type) {
      local_decl_count++;
      current_type = type;
    }
  }

  /* loop through again to write everything out */
  write_u32_leb128(&ctx->stream, local_decl_count, "local decl count");
  current_type = GET_LOCAL_TYPE(FIRST_LOCAL_INDEX);
  uint32_t local_type_count = 1;
  for (i = FIRST_LOCAL_INDEX + 1; i <= LAST_LOCAL_INDEX; ++i) {
    /* loop through an extra time to catch the final type transition */
    WasmType type = i == LAST_LOCAL_INDEX ? WASM_TYPE_VOID : GET_LOCAL_TYPE(i);
    if (current_type == type) {
      local_type_count++;
    } else {
      write_u32_leb128(&ctx->stream, local_type_count, "local type count");
      wasm_write_u8(&ctx->stream, current_type, s_type_names[current_type]);
      local_type_count = 1;
      current_type = type;
    }
  }
}

static void write_func(WasmContext* ctx,
                       const WasmModule* module,
                       const WasmFunc* func) {
  write_func_locals(ctx, module, func, &func->local_types);
  write_expr_list(ctx, module, func, &func->exprs);
}

static void write_module(WasmContext* ctx, const WasmModule* module) {
  /* TODO(binji): better leb size guess. Some sections we know will only be 1
   byte, but others we can be fairly certain will be larger. */
  const size_t leb_size_guess = 1;

  size_t i;
  wasm_write_u32(&ctx->stream, WASM_BINARY_MAGIC, "WASM_BINARY_MAGIC");
  wasm_write_u32(&ctx->stream, WASM_BINARY_VERSION, "WASM_BINARY_VERSION");

  WasmFuncSignatureVector sigs;
  WASM_ZERO_MEMORY(sigs);
  get_func_signatures(ctx, module, &sigs);
  if (sigs.size) {
    begin_section(ctx, WASM_SECTION_NAME_SIGNATURES, leb_size_guess);
    write_u32_leb128(&ctx->stream, sigs.size, "num signatures");
    for (i = 0; i < sigs.size; ++i) {
      const WasmFuncSignature* sig = &sigs.data[i];
      write_header(ctx, "signature", i);
      wasm_write_u8(&ctx->stream, sig->param_types.size, "num params");
      wasm_write_u8(&ctx->stream, sig->result_type, "result_type");
      size_t j;
      for (j = 0; j < sig->param_types.size; ++j)
        wasm_write_u8(&ctx->stream, sig->param_types.data[j], "param type");
    }
    end_section(ctx);
  }

  if (module->imports.size) {
    begin_section(ctx, WASM_SECTION_NAME_IMPORT_TABLE, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->imports.size, "num imports");

    for (i = 0; i < module->imports.size; ++i) {
      const WasmImport* import = module->imports.data[i];
      write_header(ctx, "import header", i);
      write_u32_leb128(&ctx->stream, ctx->import_sig_indexes[i],
                       "import signature index");
      write_str(&ctx->stream, import->module_name.start,
                import->module_name.length, WASM_PRINT_CHARS,
                "import module name");
      write_str(&ctx->stream, import->func_name.start, import->func_name.length,
                WASM_PRINT_CHARS, "import function name");
    }
    end_section(ctx);
  }

  if (module->funcs.size) {
    begin_section(ctx, WASM_SECTION_NAME_FUNCTION_SIGNATURES, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->funcs.size, "num functions");

    for (i = 0; i < module->funcs.size; ++i) {
      char desc[100];
      wasm_snprintf(desc, sizeof(desc), "function %" PRIzd " signature index",
                    i);
      write_u32_leb128(&ctx->stream, ctx->func_sig_indexes[i], desc);
    }
    end_section(ctx);
  }

  if (module->table && module->table->size) {
    begin_section(ctx, WASM_SECTION_NAME_FUNCTION_TABLE, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->table->size,
                     "num function table entries");
    for (i = 0; i < module->table->size; ++i) {
      int index = wasm_get_func_index_by_var(module, &module->table->data[i]);
      assert(index >= 0 && (size_t)index < module->funcs.size);
      write_u32_leb128(&ctx->stream, index, "function table entry");
    }
    end_section(ctx);
  }

  if (module->memory) {
    WasmBool export_memory = module->export_memory != NULL;
    begin_section(ctx, WASM_SECTION_NAME_MEMORY, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->memory->initial_pages,
                     "min mem pages");
    write_u32_leb128(&ctx->stream, module->memory->max_pages, "max mem pages");
    wasm_write_u8(&ctx->stream, export_memory, "export mem");
    end_section(ctx);
  }

  if (module->exports.size) {
    begin_section(ctx, WASM_SECTION_NAME_EXPORT_TABLE, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->exports.size, "num exports");

    for (i = 0; i < module->exports.size; ++i) {
      const WasmExport* export = module->exports.data[i];
      int func_index = wasm_get_func_index_by_var(module, &export->var);
      assert(func_index >= 0 && (size_t)func_index < module->funcs.size);
      write_u32_leb128(&ctx->stream, func_index, "export func index");
      write_str(&ctx->stream, export->name.start, export->name.length,
                WASM_PRINT_CHARS, "export name");
    }
    end_section(ctx);
  }

  if (module->start) {
    int start_func_index = wasm_get_func_index_by_var(module, module->start);
    if (start_func_index != -1) {
      begin_section(ctx, WASM_SECTION_NAME_START_FUNCTION, leb_size_guess);
      write_u32_leb128(&ctx->stream, start_func_index, "start func index");
      end_section(ctx);
    }
  }

  if (module->funcs.size) {
    begin_section(ctx, WASM_SECTION_NAME_FUNCTION_BODIES, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->funcs.size, "num functions");

    for (i = 0; i < module->funcs.size; ++i) {
      write_header(ctx, "function body", i);
      const WasmFunc* func = module->funcs.data[i];

      /* TODO(binji): better guess of the size of the function body section */
      const uint32_t leb_size_guess = 1;
      uint32_t body_size_offset =
          write_u32_leb128_space(ctx, leb_size_guess, "func body size (guess)");
      write_func(ctx, module, func);
      write_fixup_u32_leb128_size(ctx, body_size_offset, leb_size_guess,
                                  "FIXUP func body size");
    }
    end_section(ctx);
  }

  if (module->memory && module->memory->segments.size) {
    begin_section(ctx, WASM_SECTION_NAME_DATA_SEGMENTS, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->memory->segments.size,
                     "num data segments");
    for (i = 0; i < module->memory->segments.size; ++i) {
      const WasmSegment* segment = &module->memory->segments.data[i];
      write_header(ctx, "segment header", i);
      write_u32_leb128(&ctx->stream, segment->addr, "segment address");
      write_u32_leb128(&ctx->stream, segment->size, "segment size");
      write_header(ctx, "segment data", i);
      wasm_write_data(&ctx->stream, segment->data, segment->size,
                      "segment data");
    }
    end_section(ctx);
  }

  if (ctx->options->write_debug_names) {
    WasmStringSliceVector index_to_name;
    WASM_ZERO_MEMORY(index_to_name);

    char desc[100];
    begin_section(ctx, WASM_SECTION_NAME_NAMES, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->funcs.size, "num functions");
    for (i = 0; i < module->funcs.size; ++i) {
      const WasmFunc* func = module->funcs.data[i];
      uint32_t num_params = wasm_get_num_params(func);
      uint32_t num_locals = func->local_types.size;
      uint32_t num_params_and_locals = wasm_get_num_params_and_locals(func);

      wasm_snprintf(desc, sizeof(desc), "func name %" PRIzd, i);
      write_str(&ctx->stream, func->name.start, func->name.length,
                WASM_PRINT_CHARS, desc);
      write_u32_leb128(&ctx->stream, num_params_and_locals, "num locals");

      if (num_params_and_locals) {
        remap_locals(ctx, func);

        CHECK_ALLOC(wasm_make_type_binding_reverse_mapping(
            ctx->allocator, &func->decl.sig.param_types, &func->param_bindings,
            &index_to_name));
        size_t j;
        for (j = 0; j < num_params; ++j) {
          WasmStringSlice name = index_to_name.data[j];
          wasm_snprintf(desc, sizeof(desc), "remapped local name %" PRIzd, j);
          write_str(&ctx->stream, name.start, name.length, WASM_PRINT_CHARS,
                    desc);
        }

        CHECK_ALLOC(wasm_make_type_binding_reverse_mapping(
            ctx->allocator, &func->local_types, &func->local_bindings,
            &index_to_name));
        for (j = 0; j < num_locals; ++j) {
          WasmStringSlice name =
              index_to_name.data[ctx->reverse_remapped_locals[num_params + j] -
                                 num_params];
          wasm_snprintf(desc, sizeof(desc), "remapped local name %" PRIzd,
                        num_params + j);
          write_str(&ctx->stream, name.start, name.length, WASM_PRINT_CHARS,
                    desc);
        }
      }
    }
    end_section(ctx);

    wasm_destroy_string_slice_vector(ctx->allocator, &index_to_name);
  }
  destroy_func_signature_vector_and_elements(ctx->allocator, &sigs);
}

static void write_commands(WasmContext* ctx, const WasmScript* script) {
  size_t i;
  WasmBool wrote_module = WASM_FALSE;
  for (i = 0; i < script->commands.size; ++i) {
    const WasmCommand* command = &script->commands.data[i];
    if (command->type != WASM_COMMAND_TYPE_MODULE)
      continue;

    write_module(ctx, &command->module);
    wrote_module = WASM_TRUE;
    break;
  }
  if (!wrote_module) {
    /* just write an empty module */
    WasmModule module;
    WASM_ZERO_MEMORY(module);
    write_module(ctx, &module);
  }
}

static void cleanup_context(WasmContext* ctx) {
  wasm_free(ctx->allocator, ctx->import_sig_indexes);
  wasm_free(ctx->allocator, ctx->func_sig_indexes);
  wasm_free(ctx->allocator, ctx->remapped_locals);
  wasm_free(ctx->allocator, ctx->reverse_remapped_locals);
}

WasmResult wasm_write_binary_module(WasmAllocator* allocator,
                                    WasmWriter* writer,
                                    const WasmModule* module,
                                    const WasmWriteBinaryOptions* options) {
  WasmContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.options = options;
  ctx.log_stream = options->log_stream;
  wasm_init_stream(&ctx.stream, writer, ctx.log_stream);

  write_module(&ctx, module);

  cleanup_context(&ctx);
  return ctx.stream.result;
}

WasmResult wasm_write_binary_script(WasmAllocator* allocator,
                                    WasmWriter* writer,
                                    const WasmScript* script,
                                    const WasmWriteBinaryOptions* options) {
  WasmContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.options = options;
  ctx.log_stream = options->log_stream;
  wasm_init_stream(&ctx.stream, writer, ctx.log_stream);

  write_commands(&ctx, script);

  cleanup_context(&ctx);
  return ctx.stream.result;
}

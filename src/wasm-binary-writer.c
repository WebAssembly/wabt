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
#include "wasm-writer.h"

#define DUMP_OCTETS_PER_LINE 16
#define PRINT_HEADER_NO_INDEX -1
#define MAX_U32_LEB128_BYTES 5
#define MAX_U64_LEB128_BYTES 10

/* whether to display the ASCII characters in the debug output */
#define PRINT_CHARS 1
#define DONT_PRINT_CHARS 0

#define ALLOC_FAILURE \
  fprintf(stderr, "%s:%d: allocation failed\n", __FILE__, __LINE__)

#define CHECK_ALLOC_(cond)      \
  do {                          \
    if ((cond)) {               \
      ALLOC_FAILURE;            \
      ctx->result = WASM_ERROR; \
      return;                   \
    }                           \
  } while (0)

#define CHECK_ALLOC(e) CHECK_ALLOC_((e) != WASM_OK)
#define CHECK_ALLOC_NULL(v) CHECK_ALLOC_(!(v))

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

#define V(type1, type2, mem_size, code, name) [code] = "OPCODE_" #name,
static const char* s_opcode_names[] = {WASM_FOREACH_OPCODE(V)};
#undef V

#define V(type1, type2, mem_size, code, name) [code] = mem_size,
static uint8_t s_opcode_mem_size[] = {WASM_FOREACH_OPCODE(V)};
#undef V

typedef struct WasmLabelNode {
  WasmLabel* label;
  int depth;
  struct WasmLabelNode* next;
} WasmLabelNode;

typedef struct WasmWriterState {
  WasmWriter* writer;
  size_t offset;
  WasmResult* result;
  int log_writes;
} WasmWriterState;

typedef struct WasmWriteContext {
  WasmAllocator* allocator;
  WasmWriterState writer_state;
  WasmWriterState spec_writer_state;
  WasmWriteBinaryOptions* options;
  WasmMemoryWriter* mem_writer;
  WasmLabelNode* top_label;
  int max_depth;
  WasmResult result;

  size_t last_section_offset;
  size_t last_section_leb_size_guess;

  int* import_sig_indexes;
  int* func_sig_indexes;
  int* remapped_locals;         /* from unpacked -> packed index, has params */
  int* reverse_remapped_locals; /* from packed -> unpacked index, no params */
  WasmStringSlice** local_type_names; /* from packed -> local name */
} WasmWriteContext;

WASM_DECLARE_VECTOR(func_signature, WasmFuncSignature);
WASM_DEFINE_VECTOR(func_signature, WasmFuncSignature);

static int is_nan_f32(uint32_t bits) {
  return (bits & 0x7f800000) == 0x7f800000 && (bits & 0x007fffff) != 0;
}

static int is_nan_f64(uint64_t bits) {
  return (bits & 0x7ff0000000000000LL) == 0x7ff0000000000000LL &&
         (bits & 0x000fffffffffffffLL) != 0;
}

static void destroy_func_signature_vector_and_elements(
    WasmAllocator* allocator,
    WasmFuncSignatureVector* sigs) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, *sigs, func_signature);
}

static void print_header(WasmWriterState* writer_state,
                         const char* name,
                         int index) {
  if (writer_state->log_writes) {
    if (index == PRINT_HEADER_NO_INDEX) {
      printf("; %s\n", name);
    } else {
      printf("; %s %d\n", name, index);
    }
  }
}

static void out_data_at(WasmWriterState* writer_state,
                        size_t offset,
                        const void* src,
                        size_t size,
                        int print_chars,
                        const char* desc) {
  if (*writer_state->result != WASM_OK)
    return;
  if (writer_state->log_writes)
    wasm_print_memory(src, size, offset, print_chars, desc);
  if (writer_state->writer->write_data)
    *writer_state->result = writer_state->writer->write_data(
        offset, src, size, writer_state->writer->user_data);
}

static void out_data(WasmWriterState* writer_state,
                     const void* src,
                     size_t length,
                     const char* desc) {
  out_data_at(writer_state, writer_state->offset, src, length, 0, desc);
  writer_state->offset += length;
}

static void move_data(WasmWriterState* writer_state,
                      size_t dst_offset,
                      size_t src_offset,
                      size_t size) {
  if (*writer_state->result != WASM_OK)
    return;
  if (writer_state->log_writes)
    printf("; move data: [%zx, %zx) -> [%zx, %zx)\n", src_offset,
           src_offset + size, dst_offset, dst_offset + size);
  if (writer_state->writer->write_data)
    *writer_state->result = writer_state->writer->move_data(
        dst_offset, src_offset, size, writer_state->writer->user_data);
}

static void out_u8(WasmWriterState* writer_state,
                   uint32_t value,
                   const char* desc) {
  assert(value <= UINT8_MAX);
  uint8_t value8 = value;
  out_data_at(writer_state, writer_state->offset, &value8, sizeof(value8),
              DONT_PRINT_CHARS, desc);
  writer_state->offset += sizeof(value8);
}

/* TODO(binji): endianness */
static void out_u32(WasmWriterState* writer_state,
                    uint32_t value,
                    const char* desc) {
  out_data_at(writer_state, writer_state->offset, &value, sizeof(value),
              DONT_PRINT_CHARS, desc);
  writer_state->offset += sizeof(value);
}

static void out_u64(WasmWriterState* writer_state,
                    uint64_t value,
                    const char* desc) {
  out_data_at(writer_state, writer_state->offset, &value, sizeof(value),
              DONT_PRINT_CHARS, desc);
  writer_state->offset += sizeof(value);
}

#undef OUT_AT_TYPE

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
static uint32_t out_u32_leb128_at(WasmWriterState* writer_state,
                                  uint32_t offset,
                                  uint32_t value,
                                  const char* desc) {
  uint8_t data[MAX_U32_LEB128_BYTES];
  int i = 0;
  LEB128_LOOP_UNTIL(value == 0);
  int length = i;
  out_data_at(writer_state, offset, data, length, DONT_PRINT_CHARS, desc);
  return length;
}

static uint32_t out_fixed_u32_leb128_at(WasmWriterState* writer_state,
                                        uint32_t offset,
                                        uint32_t value,
                                        const char* desc) {
  uint8_t data[MAX_U32_LEB128_BYTES];
  data[0] = (value & 0x7f) | 0x80;
  data[1] = ((value >> 7) & 0x7f) | 0x80;
  data[2] = ((value >> 14) & 0x7f) | 0x80;
  data[3] = ((value >> 21) & 0x7f) | 0x80;
  data[4] = ((value >> 28) & 0x0f);
  out_data_at(writer_state, offset, data, MAX_U32_LEB128_BYTES,
              DONT_PRINT_CHARS, desc);
  return MAX_U32_LEB128_BYTES;
}

static void out_u32_leb128(WasmWriterState* writer_state,
                           uint32_t value,
                           const char* desc) {
  uint32_t length =
      out_u32_leb128_at(writer_state, writer_state->offset, value, desc);
  writer_state->offset += length;
}

static void out_i32_leb128(WasmWriterState* writer_state,
                           int32_t value,
                           const char* desc) {
  uint8_t data[MAX_U32_LEB128_BYTES];
  int i = 0;
  if (value < 0)
    LEB128_LOOP_UNTIL(value == -1 && (byte & 0x40));
  else
    LEB128_LOOP_UNTIL(value == 0 && !(byte & 0x40));

  int length = i;
  out_data_at(writer_state, writer_state->offset, data, length,
              DONT_PRINT_CHARS, desc);
  writer_state->offset += length;
}

static void out_i64_leb128(WasmWriterState* writer_state,
                           int64_t value,
                           const char* desc) {
  uint8_t data[MAX_U64_LEB128_BYTES];
  int i = 0;
  if (value < 0)
    LEB128_LOOP_UNTIL(value == -1 && (byte & 0x40));
  else
    LEB128_LOOP_UNTIL(value == 0 && !(byte & 0x40));

  int length = i;
  out_data_at(writer_state, writer_state->offset, data, length,
              DONT_PRINT_CHARS, desc);
  writer_state->offset += length;
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
static uint32_t out_u32_leb128_space(WasmWriteContext* ctx,
                                     uint32_t leb_size_guess,
                                     const char* desc) {
  assert(leb_size_guess <= MAX_U32_LEB128_BYTES);
  uint8_t data[MAX_U32_LEB128_BYTES] = {0};
  uint32_t result = ctx->writer_state.offset;
  uint32_t bytes_to_write =
      ctx->options->canonicalize_lebs ? leb_size_guess : MAX_U32_LEB128_BYTES;
  out_data(&ctx->writer_state, data, bytes_to_write, desc);
  return result;
}

static void out_fixup_u32_leb128_size(WasmWriteContext* ctx,
                                      uint32_t offset,
                                      uint32_t leb_size_guess,
                                      const char* desc) {
  WasmWriterState* ws = &ctx->writer_state;
  if (ctx->options->canonicalize_lebs) {
    uint32_t size = ws->offset - offset - leb_size_guess;
    uint32_t leb_size = size_u32_leb128(size);
    if (leb_size != leb_size_guess) {
      uint32_t src_offset = offset + leb_size_guess;
      uint32_t dst_offset = offset + leb_size;
      move_data(ws, dst_offset, src_offset, size);
    }
    out_u32_leb128_at(ws, offset, size, desc);
    ws->offset += leb_size - leb_size_guess;
  } else {
    uint32_t size = ws->offset - offset - MAX_U32_LEB128_BYTES;
    out_fixed_u32_leb128_at(ws, offset, size, desc);
  }
}

static void out_str(WasmWriterState* writer_state,
                    const char* s,
                    size_t length,
                    int print_chars,
                    const char* desc) {
  out_u32_leb128(writer_state, length, "string length");
  out_data_at(writer_state, writer_state->offset, s, length, print_chars, desc);
  writer_state->offset += length;
}

static void out_opcode(WasmWriterState* writer_state, uint8_t opcode) {
  out_u8(writer_state, opcode, s_opcode_names[opcode]);
}

static void out_printf(WasmWriterState* writer_state, const char* format, ...) {
  va_list args;
  va_list args_copy;
  va_start(args, format);
  va_copy(args_copy, args);
  /* + 1 to account for the \0 that will be added automatically by
   wasm_vsnprintf */
  int len = wasm_vsnprintf(NULL, 0, format, args) + 1;
  va_end(args);
  char* buffer = alloca(len);
  wasm_vsnprintf(buffer, len, format, args_copy);
  va_end(args_copy);
  /* - 1 to remove the trailing \0 that was added by wasm_vsnprintf */
  out_data_at(writer_state, writer_state->offset, buffer, len - 1,
              DONT_PRINT_CHARS, "");
  writer_state->offset += len - 1;
}

static void begin_section(WasmWriteContext* ctx,
                          const char* name,
                          size_t leb_size_guess) {
  WasmWriterState* ws = &ctx->writer_state;
  assert(ctx->last_section_leb_size_guess == 0);
  char desc[100];
  wasm_snprintf(desc, sizeof(desc), "section \"%s\"", name);
  print_header(ws, desc, PRINT_HEADER_NO_INDEX);
  ctx->last_section_offset =
      out_u32_leb128_space(ctx, leb_size_guess, "section size (guess)");
  ctx->last_section_leb_size_guess = leb_size_guess;
  wasm_snprintf(desc, sizeof(desc), "section id: \"%s\"", name);
  out_str(ws, name, strlen(name), DONT_PRINT_CHARS, desc);
}

static void end_section(WasmWriteContext* ctx) {
  assert(ctx->last_section_leb_size_guess != 0);
  out_fixup_u32_leb128_size(ctx, ctx->last_section_offset,
                            ctx->last_section_leb_size_guess,
                            "FIXUP section size");
  ctx->last_section_leb_size_guess = 0;
}

static WasmLabelNode* find_label_by_name(WasmLabelNode* top_label,
                                         WasmStringSlice* name) {
  WasmLabelNode* node = top_label;
  while (node) {
    if (node->label && wasm_string_slices_are_equal(node->label, name))
      return node;
    node = node->next;
  }
  return NULL;
}

static WasmLabelNode* find_label_by_var(WasmLabelNode* top_label,
                                        WasmVar* var) {
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

static void push_unused_label(WasmWriteContext* ctx,
                              WasmLabelNode* node,
                              WasmLabel* label) {
  assert(label);
  node->label = label;
  node->next = ctx->top_label;
  node->depth = ctx->max_depth;
  ctx->top_label = node;
}

static void pop_unused_label(WasmWriteContext* ctx, WasmLabel* label) {
  if (ctx->top_label && ctx->top_label->label == label)
    ctx->top_label = ctx->top_label->next;
}

static void push_label(WasmWriteContext* ctx,
                       WasmLabelNode* node,
                       WasmLabel* label) {
  push_unused_label(ctx, node, label);
  ctx->max_depth++;
}

static void pop_label(WasmWriteContext* ctx, WasmLabel* label) {
  ctx->max_depth--;
  pop_unused_label(ctx, label);
}

static int find_func_signature(WasmFuncSignatureVector* sigs,
                               WasmType result_type,
                               WasmTypeVector* param_types) {
  int i;
  for (i = 0; i < sigs->size; ++i) {
    WasmFuncSignature* sig2 = &sigs->data[i];
    if (sig2->result_type != result_type)
      continue;
    if (sig2->param_types.size != param_types->size)
      continue;
    int j;
    for (j = 0; j < param_types->size; ++j) {
      if (sig2->param_types.data[j] != param_types->data[j])
        break;
    }
    if (j == param_types->size)
      return i;
  }
  return -1;
}

static void get_func_signatures(WasmWriteContext* ctx,
                                WasmModule* module,
                                WasmFuncSignatureVector* sigs) {
  /* function types are not deduped; we don't want the signature index to match
   if they were specified separately in the source */
  int i;
  for (i = 0; i < module->func_types.size; ++i) {
    WasmFuncType* func_type = module->func_types.data[i];
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
    WasmImport* import = module->imports.data[i];
    int index;
    if (import->import_type == WASM_IMPORT_HAS_FUNC_SIGNATURE) {
      index = find_func_signature(sigs, import->func_sig.result_type,
                                  &import->func_sig.param_types);
      if (index == -1) {
        index = sigs->size;
        WasmFuncSignature* sig =
            wasm_append_func_signature(ctx->allocator, sigs);
        CHECK_ALLOC_NULL(sig);
        sig->result_type = import->func_sig.result_type;
        WASM_ZERO_MEMORY(sig->param_types);
        CHECK_ALLOC(wasm_extend_types(ctx->allocator, &sig->param_types,
                                      &import->func_sig.param_types));
      }
    } else {
      assert(import->import_type == WASM_IMPORT_HAS_TYPE);
      WasmFuncType* func_type =
          wasm_get_func_type_by_var(module, &import->type_var);
      assert(func_type);
      index = find_func_signature(sigs, func_type->sig.result_type,
                                  &func_type->sig.param_types);
      assert(index != -1);
    }

    ctx->import_sig_indexes[i] = index;
  }

  ctx->func_sig_indexes =
      wasm_realloc(ctx->allocator, ctx->func_sig_indexes,
                   module->funcs.size * sizeof(int), WASM_DEFAULT_ALIGN);
  if (module->funcs.size)
    CHECK_ALLOC_NULL(ctx->func_sig_indexes);
  for (i = 0; i < module->funcs.size; ++i) {
    WasmFunc* func = module->funcs.data[i];
    int index;
    if (func->flags & WASM_FUNC_FLAG_HAS_FUNC_TYPE) {
      index = wasm_get_func_type_index_by_var(module, &func->type_var);
      assert(index != -1);
    } else {
      assert(func->flags & WASM_FUNC_FLAG_HAS_SIGNATURE);
      index = find_func_signature(sigs, func->result_type, &func->params.types);
      if (index == -1) {
        index = sigs->size;
        WasmFuncSignature* sig =
            wasm_append_func_signature(ctx->allocator, sigs);
        CHECK_ALLOC_NULL(sig);
        sig->result_type = func->result_type;
        WASM_ZERO_MEMORY(sig->param_types);
        CHECK_ALLOC(wasm_extend_types(ctx->allocator, &sig->param_types,
                                      &func->params.types));
      }
    }

    ctx->func_sig_indexes[i] = index;
  }
}

static void remap_locals(WasmWriteContext* ctx, WasmFunc* func) {
  int i;
  int num_params = func->params.types.size;
  int num_locals = func->locals.types.size;
  int num_params_and_locals = num_params + num_locals;
  ctx->remapped_locals =
      wasm_realloc(ctx->allocator, ctx->remapped_locals,
                   num_params_and_locals * sizeof(int), WASM_DEFAULT_ALIGN);
  if (num_params_and_locals)
    CHECK_ALLOC_NULL(ctx->remapped_locals);

  ctx->reverse_remapped_locals =
      wasm_realloc(ctx->allocator, ctx->reverse_remapped_locals,
                   num_locals * sizeof(int), WASM_DEFAULT_ALIGN);
  if (num_locals)
    CHECK_ALLOC_NULL(ctx->reverse_remapped_locals);

  if (!ctx->options->remap_locals) {
    /* just pass the index straight through */
    for (i = 0; i < num_params_and_locals; ++i)
      ctx->remapped_locals[i] = i;
    for (i = 0; i < num_locals; ++i)
      ctx->reverse_remapped_locals[i] = i;
    return;
  }

  int max[WASM_NUM_TYPES];
  WASM_ZERO_MEMORY(max);
  for (i = 0; i < num_locals; ++i) {
    WasmType type = func->locals.types.data[i];
    max[type]++;
  }

  /* params don't need remapping */
  for (i = 0; i < num_params; ++i)
    ctx->remapped_locals[i] = i;

  int start[WASM_NUM_TYPES];
  start[WASM_TYPE_I32] = num_params;
  start[WASM_TYPE_I64] = start[WASM_TYPE_I32] + max[WASM_TYPE_I32];
  start[WASM_TYPE_F32] = start[WASM_TYPE_I64] + max[WASM_TYPE_I64];
  start[WASM_TYPE_F64] = start[WASM_TYPE_F32] + max[WASM_TYPE_F32];

  int seen[WASM_NUM_TYPES];
  WASM_ZERO_MEMORY(seen);
  for (i = 0; i < num_locals; ++i) {
    WasmType type = func->locals.types.data[i];
    int unpacked_index = num_params + i;
    int packed_index = start[type] + seen[type]++;
    ctx->remapped_locals[unpacked_index] = packed_index;
    ctx->reverse_remapped_locals[packed_index - num_params] = i;
  }
}

static void write_expr_list(WasmWriteContext* ctx,
                            WasmModule* module,
                            WasmFunc* func,
                            WasmExprPtrVector* exprs);

static void write_expr_list_with_count(WasmWriteContext* ctx,
                                       WasmModule* module,
                                       WasmFunc* func,
                                       WasmExprPtrVector* exprs);

static void write_block(WasmWriteContext* ctx,
                        WasmModule* module,
                        WasmFunc* func,
                        WasmBlock* block);

static void write_expr(WasmWriteContext* ctx,
                       WasmModule* module,
                       WasmFunc* func,
                       WasmExpr* expr) {
  WasmWriterState* ws = &ctx->writer_state;
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      out_opcode(ws, expr->binary.opcode);
      write_expr(ctx, module, func, expr->binary.left);
      write_expr(ctx, module, func, expr->binary.right);
      break;
    case WASM_EXPR_TYPE_BLOCK:
      write_block(ctx, module, func, &expr->block);
      break;
    case WASM_EXPR_TYPE_BR: {
      WasmLabelNode* node = find_label_by_var(ctx->top_label, &expr->br.var);
      assert(node);
      out_opcode(ws, WASM_OPCODE_BR);
      out_u32_leb128(ws, ctx->max_depth - node->depth - 1, "break depth");
      if (expr->br.expr)
        write_expr(ctx, module, func, expr->br.expr);
      else
        out_opcode(ws, WASM_OPCODE_NOP);
      break;
    }
    case WASM_EXPR_TYPE_BR_IF: {
      WasmLabelNode* node = find_label_by_var(ctx->top_label, &expr->br_if.var);
      assert(node);
      out_opcode(ws, WASM_OPCODE_BR_IF);
      out_u32_leb128(ws, ctx->max_depth - node->depth - 1, "break depth");
      if (expr->br_if.expr)
        write_expr(ctx, module, func, expr->br_if.expr);
      else
        out_opcode(ws, WASM_OPCODE_NOP);
      write_expr(ctx, module, func, expr->br_if.cond);
      break;
    }
    case WASM_EXPR_TYPE_CALL: {
      int index = wasm_get_func_index_by_var(module, &expr->call.var);
      assert(index >= 0 && index < module->funcs.size);
      out_opcode(ws, WASM_OPCODE_CALL_FUNCTION);
      out_u32_leb128(ws, index, "func index");
      write_expr_list(ctx, module, func, &expr->call.args);
      break;
    }
    case WASM_EXPR_TYPE_CALL_IMPORT: {
      int index = wasm_get_import_index_by_var(module, &expr->call.var);
      assert(index >= 0 && index < module->imports.size);
      out_opcode(ws, WASM_OPCODE_CALL_IMPORT);
      out_u32_leb128(ws, index, "import index");
      write_expr_list(ctx, module, func, &expr->call.args);
      break;
    }
    case WASM_EXPR_TYPE_CALL_INDIRECT: {
      int index =
          wasm_get_func_type_index_by_var(module, &expr->call_indirect.var);
      assert(index >= 0 && index < module->func_types.size);
      out_opcode(ws, WASM_OPCODE_CALL_INDIRECT);
      out_u32_leb128(ws, index, "signature index");
      write_expr(ctx, module, func, expr->call_indirect.expr);
      write_expr_list(ctx, module, func, &expr->call_indirect.args);
      break;
    }
    case WASM_EXPR_TYPE_COMPARE:
      out_opcode(ws, expr->compare.opcode);
      write_expr(ctx, module, func, expr->compare.left);
      write_expr(ctx, module, func, expr->compare.right);
      break;
    case WASM_EXPR_TYPE_CONST:
      switch (expr->const_.type) {
        case WASM_TYPE_I32: {
          out_opcode(ws, WASM_OPCODE_I32_CONST);
          out_i32_leb128(ws, (int32_t)expr->const_.u32, "i32 literal");
          break;
        }
        case WASM_TYPE_I64:
          out_opcode(ws, WASM_OPCODE_I64_CONST);
          out_i64_leb128(ws, (int64_t)expr->const_.u64, "i64 literal");
          break;
        case WASM_TYPE_F32:
          out_opcode(ws, WASM_OPCODE_F32_CONST);
          out_u32(ws, expr->const_.f32_bits, "f32 literal");
          break;
        case WASM_TYPE_F64:
          out_opcode(ws, WASM_OPCODE_F64_CONST);
          out_u64(ws, expr->const_.f64_bits, "f64 literal");
          break;
        default:
          assert(0);
      }
      break;
    case WASM_EXPR_TYPE_CONVERT:
      out_opcode(ws, expr->convert.opcode);
      write_expr(ctx, module, func, expr->convert.expr);
      break;
    case WASM_EXPR_TYPE_GET_LOCAL: {
      int index = wasm_get_local_index_by_var(func, &expr->get_local.var);
      assert(index >= 0 && index < func->params_and_locals.types.size);
      out_opcode(ws, WASM_OPCODE_GET_LOCAL);
      out_u32_leb128(ws, ctx->remapped_locals[index], "remapped local index");
      break;
    }
    case WASM_EXPR_TYPE_GROW_MEMORY:
      out_opcode(ws, WASM_OPCODE_GROW_MEMORY);
      write_expr(ctx, module, func, expr->grow_memory.expr);
      break;
    case WASM_EXPR_TYPE_IF:
      out_opcode(ws, WASM_OPCODE_IF);
      write_expr(ctx, module, func, expr->if_.cond);
      write_expr(ctx, module, func, expr->if_.true_);
      break;
    case WASM_EXPR_TYPE_IF_ELSE:
      out_opcode(ws, WASM_OPCODE_IF_ELSE);
      write_expr(ctx, module, func, expr->if_else.cond);
      write_expr(ctx, module, func, expr->if_else.true_);
      write_expr(ctx, module, func, expr->if_else.false_);
      break;
    case WASM_EXPR_TYPE_LOAD: {
      out_opcode(ws, expr->load.opcode);
      uint32_t align = expr->load.align;
      if (align == WASM_USE_NATURAL_ALIGNMENT)
        align = s_opcode_mem_size[expr->load.opcode];
      uint8_t align_log = 0;
      while (align > 1) {
        align >>= 1;
        align_log++;
      }
      out_u8(ws, align_log, "alignment");
      out_u32_leb128(ws, (uint32_t)expr->load.offset, "load offset");
      write_expr(ctx, module, func, expr->load.addr);
      break;
    }
    case WASM_EXPR_TYPE_LOOP: {
      WasmLabelNode outer;
      WasmLabelNode inner;
      push_label(ctx, &outer, &expr->loop.outer);
      push_label(ctx, &inner, &expr->loop.inner);
      out_opcode(ws, WASM_OPCODE_LOOP);
      write_expr_list_with_count(ctx, module, func, &expr->loop.exprs);
      pop_label(ctx, &expr->loop.inner);
      pop_label(ctx, &expr->loop.outer);
      break;
    }
    case WASM_EXPR_TYPE_MEMORY_SIZE:
      out_opcode(ws, WASM_OPCODE_MEMORY_SIZE);
      break;
    case WASM_EXPR_TYPE_NOP:
      out_opcode(ws, WASM_OPCODE_NOP);
      break;
    case WASM_EXPR_TYPE_RETURN:
      out_opcode(ws, WASM_OPCODE_RETURN);
      if (expr->return_.expr)
        write_expr(ctx, module, func, expr->return_.expr);
      break;
    case WASM_EXPR_TYPE_SELECT:
      out_opcode(ws, WASM_OPCODE_SELECT);
      write_expr(ctx, module, func, expr->select.true_);
      write_expr(ctx, module, func, expr->select.false_);
      write_expr(ctx, module, func, expr->select.cond);
      break;
    case WASM_EXPR_TYPE_SET_LOCAL: {
      int index = wasm_get_local_index_by_var(func, &expr->get_local.var);
      out_opcode(ws, WASM_OPCODE_SET_LOCAL);
      out_u32_leb128(ws, ctx->remapped_locals[index], "remapped local index");
      write_expr(ctx, module, func, expr->set_local.expr);
      break;
    }
    case WASM_EXPR_TYPE_STORE: {
      out_opcode(ws, expr->store.opcode);
      uint32_t align = expr->store.align;
      if (align == WASM_USE_NATURAL_ALIGNMENT)
        align = s_opcode_mem_size[expr->store.opcode];
      uint8_t align_log = 0;
      while (align > 1) {
        align >>= 1;
        align_log++;
      }
      out_u8(ws, align_log, "alignment");
      out_u32_leb128(ws, (uint32_t)expr->store.offset, "store offset");
      write_expr(ctx, module, func, expr->store.addr);
      write_expr(ctx, module, func, expr->store.value);
      break;
    }
    case WASM_EXPR_TYPE_BR_TABLE: {
      out_opcode(ws, WASM_OPCODE_BR_TABLE);
      out_u32_leb128(ws, expr->br_table.targets.size, "num targets");
      int i;
      WasmLabelNode* node;
      for (i = 0; i < expr->br_table.targets.size; ++i) {
        WasmVar* var = &expr->br_table.targets.data[i];
        node = find_label_by_var(ctx->top_label, var);
        out_u32(ws, ctx->max_depth - node->depth - 1, "break depth");
      }
      node = find_label_by_var(ctx->top_label, &expr->br_table.default_target);
      out_u32(ws, ctx->max_depth - node->depth - 1, "break depth for default");
      write_expr(ctx, module, func, expr->br_table.expr);
      break;
    }
    case WASM_EXPR_TYPE_UNARY:
      out_opcode(ws, expr->unary.opcode);
      write_expr(ctx, module, func, expr->unary.expr);
      break;
    case WASM_EXPR_TYPE_UNREACHABLE:
      out_opcode(ws, WASM_OPCODE_UNREACHABLE);
      break;
  }
}

static void write_expr_list(WasmWriteContext* ctx,
                            WasmModule* module,
                            WasmFunc* func,
                            WasmExprPtrVector* exprs) {
  int i;
  for (i = 0; i < exprs->size; ++i)
    write_expr(ctx, module, func, exprs->data[i]);
}

static void write_expr_list_with_count(WasmWriteContext* ctx,
                                       WasmModule* module,
                                       WasmFunc* func,
                                       WasmExprPtrVector* exprs) {
  WasmWriterState* ws = &ctx->writer_state;
  out_u32_leb128(ws, exprs->size, "num expressions");
  write_expr_list(ctx, module, func, exprs);
}

static void write_block(WasmWriteContext* ctx,
                        WasmModule* module,
                        WasmFunc* func,
                        WasmBlock* block) {
  WasmLabelNode node;
  if (block->exprs.size == 1 && !block->used) {
    push_unused_label(ctx, &node, &block->label);
    write_expr(ctx, module, func, block->exprs.data[0]);
    pop_unused_label(ctx, &block->label);
  } else {
    push_label(ctx, &node, &block->label);
    out_opcode(&ctx->writer_state, WASM_OPCODE_BLOCK);
    write_expr_list_with_count(ctx, module, func, &block->exprs);
    pop_label(ctx, &block->label);
  }
}

static void write_func_locals(WasmWriteContext* ctx,
                              WasmModule* module,
                              WasmFunc* func,
                              WasmTypeVector* local_types) {
  WasmWriterState* ws = &ctx->writer_state;
  remap_locals(ctx, func);

  if (local_types->size == 0) {
    out_u32_leb128(ws, 0, "local decl count");
    return;
  }

/* remapped_locals includes the parameter list, so skip those */
#define FIRST_LOCAL_INDEX (0)
#define LAST_LOCAL_INDEX (local_types->size)
#define GET_LOCAL_TYPE(x) (local_types->data[ctx->reverse_remapped_locals[x]])

  /* loop through once to count the number of local declaration runs */
  WasmType current_type = GET_LOCAL_TYPE(FIRST_LOCAL_INDEX);
  uint32_t local_decl_count = 1;
  int i;
  for (i = FIRST_LOCAL_INDEX + 1; i < LAST_LOCAL_INDEX; ++i) {
    WasmType type = GET_LOCAL_TYPE(i);
    if (current_type != type) {
      local_decl_count++;
      current_type = type;
    }
  }

  /* loop through again to write everything out */
  out_u32_leb128(ws, local_decl_count, "local decl count");
  current_type = GET_LOCAL_TYPE(FIRST_LOCAL_INDEX);
  uint32_t local_type_count = 1;
  for (i = FIRST_LOCAL_INDEX + 1; i <= LAST_LOCAL_INDEX; ++i) {
    /* loop through an extra time to catch the final type transition */
    WasmType type = i == LAST_LOCAL_INDEX ? WASM_TYPE_VOID : GET_LOCAL_TYPE(i);
    if (current_type == type) {
      local_type_count++;
    } else {
      out_u32_leb128(ws, local_type_count, "local type count");
      out_u8(ws, current_type, s_type_names[current_type]);
      local_type_count = 1;
      current_type = type;
    }
  }
}

static void write_func(WasmWriteContext* ctx,
                       WasmModule* module,
                       WasmFunc* func) {
  write_func_locals(ctx, module, func, &func->locals.types);
  write_expr_list(ctx, module, func, &func->exprs);
}

static void write_module(WasmWriteContext* ctx, WasmModule* module) {
  /* TODO(binji): better leb size guess. Some sections we know will only be 1
   byte, but others we can be fairly certain will be larger. */
  const size_t leb_size_guess = 1;

  int i;
  WasmWriterState* ws = &ctx->writer_state;
  ctx->writer_state.offset = 0;
  out_u32(ws, WASM_BINARY_MAGIC, "WASM_BINARY_MAGIC");
  out_u32(ws, WASM_BINARY_VERSION, "WASM_BINARY_VERSION");

  WasmFuncSignatureVector sigs;
  WASM_ZERO_MEMORY(sigs);
  get_func_signatures(ctx, module, &sigs);
  if (sigs.size) {
    begin_section(ctx, WASM_BINARY_SECTION_SIGNATURES, leb_size_guess);
    out_u32_leb128(ws, sigs.size, "num signatures");
    for (i = 0; i < sigs.size; ++i) {
      WasmFuncSignature* sig = &sigs.data[i];
      print_header(ws, "signature", i);
      out_u8(ws, sig->param_types.size, "num params");
      out_u8(ws, sig->result_type, "result_type");
      int j;
      for (j = 0; j < sig->param_types.size; ++j)
        out_u8(ws, sig->param_types.data[j], "param type");
    }
    end_section(ctx);
  }

  if (module->imports.size) {
    begin_section(ctx, WASM_BINARY_SECTION_IMPORTS, leb_size_guess);
    out_u32_leb128(ws, module->imports.size, "num imports");

    for (i = 0; i < module->imports.size; ++i) {
      WasmImport* import = module->imports.data[i];
      print_header(ws, "import header", i);
      out_u32_leb128(ws, ctx->import_sig_indexes[i], "import signature index");
      out_str(ws, import->module_name.start, import->module_name.length,
              PRINT_CHARS, "import module name");
      out_str(ws, import->func_name.start, import->func_name.length,
              PRINT_CHARS, "import function name");
    }
    end_section(ctx);
  }

  if (module->funcs.size) {
    begin_section(ctx, WASM_BINARY_SECTION_FUNCTION_SIGNATURES, leb_size_guess);
    out_u32_leb128(ws, module->funcs.size, "num functions");

    for (i = 0; i < module->funcs.size; ++i) {
      char desc[100];
      wasm_snprintf(desc, sizeof(desc), "function %d signature index", i);
      out_u32_leb128(ws, ctx->func_sig_indexes[i], desc);
    }
    end_section(ctx);
  }

  if (module->table && module->table->size) {
    begin_section(ctx, WASM_BINARY_SECTION_FUNCTION_TABLE, leb_size_guess);
    out_u32_leb128(ws, module->table->size, "num function table entries");
    for (i = 0; i < module->table->size; ++i) {
      int index = wasm_get_func_index_by_var(module, &module->table->data[i]);
      assert(index >= 0 && index < module->funcs.size);
      out_u32_leb128(ws, index, "function table entry");
    }
    end_section(ctx);
  }

  if (module->memory) {
    int export_memory = module->export_memory != NULL;
    begin_section(ctx, WASM_BINARY_SECTION_MEMORY, leb_size_guess);
    out_u32_leb128(ws, module->memory->initial_pages, "min mem pages");
    out_u32_leb128(ws, module->memory->max_pages, "max mem pages");
    out_u8(ws, export_memory, "export mem");
    end_section(ctx);
  }

  if (module->exports.size) {
    begin_section(ctx, WASM_BINARY_SECTION_EXPORTS, leb_size_guess);
    out_u32_leb128(ws, module->exports.size, "num exports");

    for (i = 0; i < module->exports.size; ++i) {
      WasmExport* export = module->exports.data[i];
      int func_index = wasm_get_func_index_by_var(module, &export->var);
      assert(func_index >= 0 && func_index < module->funcs.size);
      out_u32_leb128(ws, func_index, "export func index");
      out_str(ws, export->name.start, export->name.length, PRINT_CHARS,
              "export name");
    }
    end_section(ctx);
  }

  int start_func_index = wasm_get_func_index_by_var(module, &module->start);
  if (start_func_index != -1) {
    begin_section(ctx, WASM_BINARY_SECTION_START, leb_size_guess);
    out_u32_leb128(ws, start_func_index, "start func index");
    end_section(ctx);
  }

  if (module->funcs.size) {
    begin_section(ctx, WASM_BINARY_SECTION_FUNCTION_BODIES, leb_size_guess);
    out_u32_leb128(ws, module->funcs.size, "num functions");

    for (i = 0; i < module->funcs.size; ++i) {
      print_header(ws, "function body", i);
      WasmFunc* func = module->funcs.data[i];

      /* TODO(binji): better guess of the size of the function body section */
      const uint32_t leb_size_guess = 1;
      uint32_t body_size_offset =
          out_u32_leb128_space(ctx, leb_size_guess, "func body size (guess)");
      write_func(ctx, module, func);
      out_fixup_u32_leb128_size(ctx, body_size_offset, leb_size_guess,
                                "FIXUP func body size");
    }
    end_section(ctx);
  }

  if (module->memory && module->memory->segments.size) {
    begin_section(ctx, WASM_BINARY_SECTION_DATA_SEGMENTS, leb_size_guess);
    out_u32_leb128(ws, module->memory->segments.size, "num data segments");
    for (i = 0; i < module->memory->segments.size; ++i) {
      WasmSegment* segment = &module->memory->segments.data[i];
      print_header(ws, "segment header", i);
      out_u32_leb128(ws, segment->addr, "segment address");
      out_u32_leb128(ws, segment->size, "segment size");
      print_header(ws, "segment data", i);
      out_data(ws, segment->data, segment->size, "segment data");
    }
    end_section(ctx);
  }

  if (ctx->options->write_debug_names) {
    char desc[100];
    begin_section(ctx, WASM_BINARY_SECTION_NAMES, leb_size_guess);
    out_u32_leb128(ws, module->funcs.size, "num functions");
    for (i = 0; i < module->funcs.size; ++i) {
      WasmFunc* func = module->funcs.data[i];
      wasm_snprintf(desc, sizeof(desc), "func name %d", i);
      out_str(ws, func->name.start, func->name.length, PRINT_CHARS, desc);
      out_u32_leb128(ws, func->params_and_locals.types.size, "num locals");

      if (func->params_and_locals.types.size) {
        remap_locals(ctx, func);

        /* create mapping from packed index to its name (if any) */
        size_t local_type_names_size =
            func->params_and_locals.types.size * sizeof(WasmStringSlice*);
        ctx->local_type_names =
            wasm_realloc(ctx->allocator, ctx->local_type_names,
                         local_type_names_size, WASM_DEFAULT_ALIGN);
        CHECK_ALLOC_NULL(ctx->local_type_names);
        memset(ctx->local_type_names, 0, local_type_names_size);

        int j;
        for (j = 0; j < func->params_and_locals.bindings.entries.capacity;
             ++j) {
          WasmBindingHashEntry* entry =
              &func->params_and_locals.bindings.entries.data[j];
          if (wasm_hash_entry_is_free(entry))
            continue;

          ctx->local_type_names[ctx->remapped_locals[entry->binding.index]] =
              &entry->binding.name;
        }

        for (j = 0; j < func->params_and_locals.types.size; ++j) {
          WasmStringSlice name;
          if (ctx->local_type_names[j]) {
            name = *ctx->local_type_names[j];
          } else {
            name.start = "";
            name.length = 0;
          }

          wasm_snprintf(desc, sizeof(desc), "remapped local name %d", j);
          out_str(ws, name.start, name.length, PRINT_CHARS, desc);
        }
      }
    }
    end_section(ctx);
  }
  destroy_func_signature_vector_and_elements(ctx->allocator, &sigs);
}

static WasmExpr* create_const_expr(WasmAllocator* allocator,
                                   WasmConst* const_) {
  WasmExpr* expr = wasm_new_const_expr(allocator);
  if (!expr)
    return NULL;
  expr->const_ = *const_;
  return expr;
}

static WasmExpr* create_invoke_expr(WasmAllocator* allocator,
                                    WasmCommandInvoke* invoke,
                                    int func_index) {
  WasmExpr* expr = wasm_new_call_expr(allocator);
  if (!expr)
    return NULL;
  expr->call.var.type = WASM_VAR_TYPE_INDEX;
  expr->call.var.index = func_index;
  int i;
  for (i = 0; i < invoke->args.size; ++i) {
    WasmConst* arg = &invoke->args.data[i];
    WasmExprPtr* expr_ptr = wasm_append_expr_ptr(allocator, &expr->call.args);
    if (!expr_ptr)
      goto fail;
    *expr_ptr = create_const_expr(allocator, arg);
    if (!*expr_ptr)
      goto fail;
  }
  return expr;
fail:
  for (i = 0; i < invoke->args.size; ++i)
    wasm_destroy_expr_ptr(allocator, &expr->call.args.data[i]);
  wasm_free(allocator, expr);
  return NULL;
}

static WasmExpr* create_eq_expr(WasmAllocator* allocator,
                                WasmType type,
                                WasmExpr* left,
                                WasmExpr* right) {
  if (!left || !right) {
    wasm_destroy_expr_ptr(allocator, &left);
    wasm_destroy_expr_ptr(allocator, &right);
    return NULL;
  }

  WasmExpr* expr = wasm_new_compare_expr(allocator);
  if (!expr)
    return NULL;
  switch (type) {
    case WASM_TYPE_I32:
      expr->compare.opcode = WASM_OPCODE_I32_EQ;
      break;
    case WASM_TYPE_I64:
      expr->compare.opcode = WASM_OPCODE_I64_EQ;
      break;
    case WASM_TYPE_F32:
      expr->compare.opcode = WASM_OPCODE_F32_EQ;
      break;
    case WASM_TYPE_F64:
      expr->compare.opcode = WASM_OPCODE_F64_EQ;
      break;
    default:
      assert(0);
  }
  expr->compare.left = left;
  expr->compare.right = right;
  return expr;
}

static WasmExpr* create_ne_expr(WasmAllocator* allocator,
                                WasmType type,
                                WasmExpr* left,
                                WasmExpr* right) {
  if (!left || !right) {
    wasm_destroy_expr_ptr(allocator, &left);
    wasm_destroy_expr_ptr(allocator, &right);
    return NULL;
  }

  WasmExpr* expr = wasm_new_compare_expr(allocator);
  if (!expr)
    return NULL;
  switch (type) {
    case WASM_TYPE_I32:
      expr->compare.opcode = WASM_OPCODE_I32_NE;
      break;
    case WASM_TYPE_I64:
      expr->compare.opcode = WASM_OPCODE_I64_NE;
      break;
    case WASM_TYPE_F32:
      expr->compare.opcode = WASM_OPCODE_F32_NE;
      break;
    case WASM_TYPE_F64:
      expr->compare.opcode = WASM_OPCODE_F64_NE;
      break;
    default:
      assert(0);
  }
  expr->compare.left = left;
  expr->compare.right = right;
  return expr;
}

static WasmExpr* create_set_local_expr(WasmAllocator* allocator,
                                       int index,
                                       WasmExpr* value) {
  if (!value) {
    wasm_destroy_expr_ptr(allocator, &value);
    return NULL;
  }

  WasmExpr* expr = wasm_new_set_local_expr(allocator);
  if (!expr)
    return NULL;
  expr->set_local.var.type = WASM_VAR_TYPE_INDEX;
  expr->set_local.var.index = index;
  expr->set_local.expr = value;
  return expr;
}

static WasmExpr* create_get_local_expr(WasmAllocator* allocator, int index) {
  WasmExpr* expr = wasm_new_get_local_expr(allocator);
  if (!expr)
    return NULL;
  expr->get_local.var.type = WASM_VAR_TYPE_INDEX;
  expr->get_local.var.index = index;
  return expr;
}

static WasmExpr* create_reinterpret_expr(WasmAllocator* allocator,
                                         WasmType type,
                                         WasmExpr* expr) {
  if (!expr) {
    wasm_destroy_expr_ptr(allocator, &expr);
    return NULL;
  }

  WasmExpr* result = wasm_new_convert_expr(allocator);
  if (!result)
    return NULL;
  switch (type) {
    case WASM_TYPE_F32:
      result->convert.opcode = WASM_OPCODE_I32_REINTERPRET_F32;
      break;
    case WASM_TYPE_F64:
      result->convert.opcode = WASM_OPCODE_I64_REINTERPRET_F64;
      break;
    default:
      assert(0);
      break;
  }
  result->convert.expr = expr;
  return result;
}

static WasmModuleField* append_module_field(
    WasmAllocator* allocator,
    WasmModule* module,
    WasmModuleFieldType module_field_type) {
  WasmModuleField* result = wasm_append_module_field(allocator, module);
  result->type = module_field_type;

  switch (module_field_type) {
    case WASM_MODULE_FIELD_TYPE_FUNC: {
      WasmFuncPtr* func_ptr = wasm_append_func_ptr(allocator, &module->funcs);
      if (!func_ptr)
        goto fail;
      *func_ptr = &result->func;
      break;
    }
    case WASM_MODULE_FIELD_TYPE_IMPORT: {
      WasmImportPtr* import_ptr =
          wasm_append_import_ptr(allocator, &module->imports);
      if (!import_ptr)
        goto fail;
      *import_ptr = &result->import;
      break;
    }
    case WASM_MODULE_FIELD_TYPE_EXPORT: {
      WasmExportPtr* export_ptr =
          wasm_append_export_ptr(allocator, &module->exports);
      if (!export_ptr)
        goto fail;
      *export_ptr = &result->export_;
      break;
    }
    case WASM_MODULE_FIELD_TYPE_FUNC_TYPE: {
      WasmFuncTypePtr* func_type_ptr =
          wasm_append_func_type_ptr(allocator, &module->func_types);
      if (!func_type_ptr)
        goto fail;
      *func_type_ptr = &result->func_type;
      break;
    }

    case WASM_MODULE_FIELD_TYPE_TABLE:
    case WASM_MODULE_FIELD_TYPE_MEMORY:
    case WASM_MODULE_FIELD_TYPE_START:
    case WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY:
      /* not supported */
      assert(0);
      break;
  }

  return result;
fail:
  wasm_free(allocator, result);
  return NULL;
}

static WasmStringSlice create_assert_func_name(WasmAllocator* allocator,
                                               const char* format,
                                               int format_index) {
  WasmStringSlice name;
  char buffer[256];
  int buffer_len = wasm_snprintf(buffer, 256, format, format_index);
  name.start = wasm_strndup(allocator, buffer, buffer_len);
  name.length = buffer_len;
  return name;
}

static WasmFunc* append_nullary_func(WasmAllocator* allocator,
                                     WasmModule* module,
                                     WasmType result_type,
                                     WasmStringSlice export_name) {
  WasmModuleField* func_field =
      append_module_field(allocator, module, WASM_MODULE_FIELD_TYPE_FUNC);
  if (!func_field)
    return NULL;
  WasmFunc* func = &func_field->func;
  func->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
  func->result_type = result_type;
  int func_index = module->funcs.size - 1;
  /* invalidated by append_module_field below */
  func_field = NULL;
  func = NULL;

  WasmModuleField* export_field =
      append_module_field(allocator, module, WASM_MODULE_FIELD_TYPE_EXPORT);
  if (!export_field) {
    /* leave the func field, it will be cleaned up later */
    return NULL;
  }
  WasmExport* export = &export_field->export_;
  export->var.type = WASM_VAR_TYPE_INDEX;
  export->var.index = func_index;
  export->name = export_name;
  return module->funcs.data[func_index];
}

static void write_header_spec(WasmWriteContext* ctx) {
  out_printf(&ctx->spec_writer_state, "var quiet = %s;\n",
             ctx->options->spec_verbose ? "false" : "true");
}

static void write_footer_spec(WasmWriteContext* ctx) {
  out_printf(&ctx->spec_writer_state, "end();\n");
}

static void write_module_header_spec(WasmWriteContext* ctx) {
  out_printf(&ctx->spec_writer_state, "var tests = function(m) {\n");
}

static void write_module_spec(WasmWriteContext* ctx, WasmModule* module) {
  /* reset the memory writer in case it's already been used */
  ctx->mem_writer->buf.size = 0;
  ctx->writer_state.offset = 0;

  write_module(ctx, module);
  /* copy the written module from the output buffer */
  out_printf(&ctx->spec_writer_state, "};\nvar m = createModule([\n");
  WasmOutputBuffer* module_buf = &ctx->mem_writer->buf;
  const uint8_t* p = module_buf->start;
  const uint8_t* end = p + module_buf->size;
  while (p < end) {
    out_printf(&ctx->spec_writer_state, " ");
    const uint8_t* line_end = p + DUMP_OCTETS_PER_LINE;
    if (line_end > end)
      line_end = end;
    for (; p < line_end; ++p)
      out_printf(&ctx->spec_writer_state, "%4d,", *p);
    out_printf(&ctx->spec_writer_state, "\n");
  }
  out_printf(&ctx->spec_writer_state, "]);\ntests(m);\n");
}

static void write_commands_spec(WasmWriteContext* ctx, WasmScript* script) {
  int i;
  WasmModule* last_module = NULL;
  int num_assert_funcs = 0;
  for (i = 0; i < script->commands.size; ++i) {
    WasmCommand* command = &script->commands.data[i];
    if (command->type == WASM_COMMAND_TYPE_MODULE) {
      if (last_module)
        write_module_spec(ctx, last_module);
      write_module_header_spec(ctx);
      last_module = &command->module;
      num_assert_funcs = 0;
    } else {
      const char* js_call = NULL;
      const char* format = NULL;
      WasmCommandInvoke* invoke = NULL;
      switch (command->type) {
        case WASM_COMMAND_TYPE_INVOKE:
          js_call = "invoke";
          format = "$invoke_%d";
          invoke = &command->invoke;
          break;
        case WASM_COMMAND_TYPE_ASSERT_RETURN:
          js_call = "assertReturn";
          format = "$assert_return_%d";
          invoke = &command->assert_return.invoke;
          break;
        case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN:
          js_call = "assertReturn";
          format = "$assert_return_nan_%d";
          invoke = &command->assert_return_nan.invoke;
          break;
        case WASM_COMMAND_TYPE_ASSERT_TRAP:
          js_call = "assertTrap";
          format = "$assert_trap_%d";
          invoke = &command->assert_trap.invoke;
          break;
        default:
          continue;
      }

      WasmExport* export = wasm_get_export_by_name(last_module, &invoke->name);
      assert(export);
      int func_index = wasm_get_func_index_by_var(last_module, &export->var);
      assert(func_index >= 0 && func_index < last_module->funcs.size);
      WasmFunc* callee = last_module->funcs.data[func_index];
      WasmType result_type = callee->result_type;
      /* these pointers will be invalidated later, so we can't use them */
      export = NULL;
      callee = NULL;

      WasmStringSlice name = create_assert_func_name(script->allocator, format,
                                                     num_assert_funcs++);
      out_printf(&ctx->spec_writer_state, "  %s(m, \"%.*s\", \"%s\", %d);\n",
                 js_call, name.length, name.start, invoke->loc.filename,
                 invoke->loc.line);

      WasmExprPtr* expr_ptr;
      switch (command->type) {
        case WASM_COMMAND_TYPE_INVOKE: {
          WasmFunc* caller = append_nullary_func(script->allocator, last_module,
                                                 result_type, name);
          CHECK_ALLOC_NULL(caller);
          expr_ptr = wasm_append_expr_ptr(script->allocator, &caller->exprs);
          CHECK_ALLOC_NULL(expr_ptr);
          *expr_ptr = create_invoke_expr(script->allocator, &command->invoke,
                                         func_index);
          CHECK_ALLOC_NULL(*expr_ptr);
          break;
        }

        case WASM_COMMAND_TYPE_ASSERT_RETURN: {
          WasmFunc* caller = append_nullary_func(script->allocator, last_module,
                                                 WASM_TYPE_I32, name);
          CHECK_ALLOC_NULL(caller);

          expr_ptr = wasm_append_expr_ptr(script->allocator, &caller->exprs);
          CHECK_ALLOC_NULL(expr_ptr);

          WasmExpr* invoke_expr = create_invoke_expr(
              script->allocator, &command->assert_return.invoke, func_index);
          CHECK_ALLOC_NULL(invoke_expr);

          if (result_type == WASM_TYPE_VOID) {
            /* The return type of the assert_return function is i32, but this
             invoked function has a return type of void, so we have nothing to
             compare to. Just return 1 to the caller, signifying everything is
             OK. */
            *expr_ptr = invoke_expr;
            WasmConst const_;
            const_.type = WASM_TYPE_I32;
            const_.u32 = 1;
            expr_ptr = wasm_append_expr_ptr(script->allocator, &caller->exprs);
            CHECK_ALLOC_NULL(expr_ptr);
            *expr_ptr = create_const_expr(script->allocator, &const_);
            CHECK_ALLOC_NULL(*expr_ptr);
          } else {
            WasmConst* expected = &command->assert_return.expected;
            WasmExpr* const_expr =
                create_const_expr(script->allocator, expected);
            CHECK_ALLOC_NULL(const_expr);

            if (expected->type == WASM_TYPE_F32 &&
                is_nan_f32(expected->f32_bits)) {
              *expr_ptr = create_eq_expr(
                  script->allocator, WASM_TYPE_I32,
                  create_reinterpret_expr(script->allocator, WASM_TYPE_F32,
                                          invoke_expr),
                  create_reinterpret_expr(script->allocator, WASM_TYPE_F32,
                                          const_expr));
              CHECK_ALLOC_NULL(*expr_ptr);
            } else if (expected->type == WASM_TYPE_F64 &&
                       is_nan_f64(expected->f64_bits)) {
              *expr_ptr = create_eq_expr(
                  script->allocator, WASM_TYPE_I64,
                  create_reinterpret_expr(script->allocator, WASM_TYPE_F64,
                                          invoke_expr),
                  create_reinterpret_expr(script->allocator, WASM_TYPE_F64,
                                          const_expr));
              CHECK_ALLOC_NULL(*expr_ptr);
            } else {
              *expr_ptr = create_eq_expr(script->allocator, result_type,
                                         invoke_expr, const_expr);
              CHECK_ALLOC_NULL(*expr_ptr);
            }
          }
          break;
        }

        case WASM_COMMAND_TYPE_ASSERT_RETURN_NAN: {
          WasmFunc* caller = append_nullary_func(script->allocator, last_module,
                                                 WASM_TYPE_I32, name);
          CHECK_ALLOC_NULL(caller);
          CHECK_ALLOC(wasm_append_type_value(
              script->allocator, &caller->locals.types, &result_type));
          CHECK_ALLOC(wasm_append_type_value(script->allocator,
                                             &caller->params_and_locals.types,
                                             &result_type));
          expr_ptr = wasm_append_expr_ptr(script->allocator, &caller->exprs);
          CHECK_ALLOC_NULL(expr_ptr);
          *expr_ptr = create_set_local_expr(
              script->allocator, 0,
              create_invoke_expr(script->allocator,
                                 &command->assert_return_nan.invoke,
                                 func_index));
          CHECK_ALLOC_NULL(*expr_ptr);
          /* x != x is true iff x is NaN */
          expr_ptr = wasm_append_expr_ptr(script->allocator, &caller->exprs);
          CHECK_ALLOC_NULL(expr_ptr);
          *expr_ptr =
              create_ne_expr(script->allocator, result_type,
                             create_get_local_expr(script->allocator, 0),
                             create_get_local_expr(script->allocator, 0));
          CHECK_ALLOC_NULL(*expr_ptr);
          break;
        }

        case WASM_COMMAND_TYPE_ASSERT_TRAP: {
          WasmFunc* caller = append_nullary_func(script->allocator, last_module,
                                                 result_type, name);
          CHECK_ALLOC_NULL(caller);
          expr_ptr = wasm_append_expr_ptr(script->allocator, &caller->exprs);
          CHECK_ALLOC_NULL(expr_ptr);
          *expr_ptr = create_invoke_expr(script->allocator, &command->invoke,
                                         func_index);
          CHECK_ALLOC_NULL(*expr_ptr);
          break;
        }

        default:
          assert(0);
      }
    }
  }
  if (last_module)
    write_module_spec(ctx, last_module);
}

static void write_commands_no_spec(WasmWriteContext* ctx, WasmScript* script) {
  int i;
  for (i = 0; i < script->commands.size; ++i) {
    WasmCommand* command = &script->commands.data[i];
    if (command->type != WASM_COMMAND_TYPE_MODULE)
      continue;

    write_module(ctx, &command->module);
  }
}

static void cleanup_context(WasmWriteContext* ctx) {
  wasm_free(ctx->allocator, ctx->import_sig_indexes);
  wasm_free(ctx->allocator, ctx->func_sig_indexes);
  wasm_free(ctx->allocator, ctx->remapped_locals);
  wasm_free(ctx->allocator, ctx->reverse_remapped_locals);
  wasm_free(ctx->allocator, ctx->local_type_names);
}

WasmResult wasm_write_binary(WasmAllocator* allocator,
                             WasmWriter* writer,
                             WasmScript* script,
                             WasmWriteBinaryOptions* options) {
  WasmWriteContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.options = options;
  ctx.result = WASM_OK;

  if (options->spec) {
    WasmMemoryWriter mem_writer;
    WASM_ZERO_MEMORY(mem_writer);
    WasmResult result = wasm_init_mem_writer(allocator, &mem_writer);
    if (result != WASM_OK)
      return result;

    ctx.spec_writer_state.writer = writer;
    ctx.spec_writer_state.result = &ctx.result;
    ctx.writer_state.writer = &mem_writer.base;
    ctx.writer_state.result = &ctx.result;
    ctx.writer_state.log_writes = options->log_writes;
    ctx.mem_writer = &mem_writer;
    write_header_spec(&ctx);
    write_commands_spec(&ctx, script);
    write_footer_spec(&ctx);
    wasm_close_mem_writer(&mem_writer);
  } else {
    ctx.writer_state.writer = writer;
    ctx.writer_state.result = &ctx.result;
    ctx.writer_state.log_writes = options->log_writes;
    write_commands_no_spec(&ctx, script);
  }

  cleanup_context(&ctx);
  return ctx.result;
}

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
  WasmWriter* writer;
  size_t writer_offset;
  const WasmWriteBinaryOptions* options;
  WasmLabelNode* top_label;
  int max_depth;
  WasmResult result;

  size_t last_section_offset;
  size_t last_section_leb_size_guess;

  int* import_sig_indexes;
  int* func_sig_indexes;
  int* remapped_locals;         /* from unpacked -> packed index, has params */
  int* reverse_remapped_locals; /* from packed -> unpacked index, no params */
  const WasmStringSlice** local_type_names; /* from packed -> local name */
} WasmContext;

WASM_DEFINE_VECTOR(func_signature, WasmFuncSignature);

static void destroy_func_signature_vector_and_elements(
    WasmAllocator* allocator,
    WasmFuncSignatureVector* sigs) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, *sigs, func_signature);
}

static void print_header(WasmContext* ctx, const char* name, int index) {
  if (ctx->options->log_writes) {
    if (index == PRINT_HEADER_NO_INDEX) {
      printf("; %s\n", name);
    } else {
      printf("; %s %d\n", name, index);
    }
  }
}

static void out_data_at(WasmContext* ctx,
                        size_t offset,
                        const void* src,
                        size_t size,
                        int print_chars,
                        const char* desc) {
  if (ctx->result != WASM_OK)
    return;
  if (ctx->options->log_writes)
    wasm_print_memory(src, size, offset, print_chars, desc);
  if (ctx->writer->write_data) {
    ctx->result =
        ctx->writer->write_data(offset, src, size, ctx->writer->user_data);
  }
}

static void out_data(WasmContext* ctx,
                     const void* src,
                     size_t length,
                     const char* desc) {
  out_data_at(ctx, ctx->writer_offset, src, length, 0, desc);
  ctx->writer_offset += length;
}

static void move_data(WasmContext* ctx,
                      size_t dst_offset,
                      size_t src_offset,
                      size_t size) {
  if (ctx->result != WASM_OK)
    return;
  if (ctx->options->log_writes)
    printf("; move data: [%" PRIzx ", %" PRIzx ") -> [%" PRIzx ", %" PRIzx
           ")\n",
           src_offset, src_offset + size, dst_offset, dst_offset + size);
  if (ctx->writer->write_data) {
    ctx->result = ctx->writer->move_data(dst_offset, src_offset, size,
                                         ctx->writer->user_data);
  }
}

static void out_u8(WasmContext* ctx, uint32_t value, const char* desc) {
  assert(value <= UINT8_MAX);
  uint8_t value8 = value;
  out_data_at(ctx, ctx->writer_offset, &value8, sizeof(value8),
              DONT_PRINT_CHARS, desc);
  ctx->writer_offset += sizeof(value8);
}

/* TODO(binji): endianness */
static void out_u32(WasmContext* ctx, uint32_t value, const char* desc) {
  out_data_at(ctx, ctx->writer_offset, &value, sizeof(value), DONT_PRINT_CHARS,
              desc);
  ctx->writer_offset += sizeof(value);
}

static void out_u64(WasmContext* ctx, uint64_t value, const char* desc) {
  out_data_at(ctx, ctx->writer_offset, &value, sizeof(value), DONT_PRINT_CHARS,
              desc);
  ctx->writer_offset += sizeof(value);
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
static uint32_t out_u32_leb128_at(WasmContext* ctx,
                                  uint32_t offset,
                                  uint32_t value,
                                  const char* desc) {
  uint8_t data[MAX_U32_LEB128_BYTES];
  int i = 0;
  LEB128_LOOP_UNTIL(value == 0);
  int length = i;
  out_data_at(ctx, offset, data, length, DONT_PRINT_CHARS, desc);
  return length;
}

static uint32_t out_fixed_u32_leb128_at(WasmContext* ctx,
                                        uint32_t offset,
                                        uint32_t value,
                                        const char* desc) {
  uint8_t data[MAX_U32_LEB128_BYTES];
  data[0] = (value & 0x7f) | 0x80;
  data[1] = ((value >> 7) & 0x7f) | 0x80;
  data[2] = ((value >> 14) & 0x7f) | 0x80;
  data[3] = ((value >> 21) & 0x7f) | 0x80;
  data[4] = ((value >> 28) & 0x0f);
  out_data_at(ctx, offset, data, MAX_U32_LEB128_BYTES, DONT_PRINT_CHARS, desc);
  return MAX_U32_LEB128_BYTES;
}

static void out_u32_leb128(WasmContext* ctx, uint32_t value, const char* desc) {
  uint32_t length = out_u32_leb128_at(ctx, ctx->writer_offset, value, desc);
  ctx->writer_offset += length;
}

static void out_i32_leb128(WasmContext* ctx, int32_t value, const char* desc) {
  uint8_t data[MAX_U32_LEB128_BYTES];
  int i = 0;
  if (value < 0)
    LEB128_LOOP_UNTIL(value == -1 && (byte & 0x40));
  else
    LEB128_LOOP_UNTIL(value == 0 && !(byte & 0x40));

  int length = i;
  out_data_at(ctx, ctx->writer_offset, data, length, DONT_PRINT_CHARS, desc);
  ctx->writer_offset += length;
}

static void out_i64_leb128(WasmContext* ctx, int64_t value, const char* desc) {
  uint8_t data[MAX_U64_LEB128_BYTES];
  int i = 0;
  if (value < 0)
    LEB128_LOOP_UNTIL(value == -1 && (byte & 0x40));
  else
    LEB128_LOOP_UNTIL(value == 0 && !(byte & 0x40));

  int length = i;
  out_data_at(ctx, ctx->writer_offset, data, length, DONT_PRINT_CHARS, desc);
  ctx->writer_offset += length;
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
static uint32_t out_u32_leb128_space(WasmContext* ctx,
                                     uint32_t leb_size_guess,
                                     const char* desc) {
  assert(leb_size_guess <= MAX_U32_LEB128_BYTES);
  uint8_t data[MAX_U32_LEB128_BYTES] = {0};
  uint32_t result = ctx->writer_offset;
  uint32_t bytes_to_write =
      ctx->options->canonicalize_lebs ? leb_size_guess : MAX_U32_LEB128_BYTES;
  out_data(ctx, data, bytes_to_write, desc);
  return result;
}

static void out_fixup_u32_leb128_size(WasmContext* ctx,
                                      uint32_t offset,
                                      uint32_t leb_size_guess,
                                      const char* desc) {
  if (ctx->options->canonicalize_lebs) {
    uint32_t size = ctx->writer_offset - offset - leb_size_guess;
    uint32_t leb_size = size_u32_leb128(size);
    if (leb_size != leb_size_guess) {
      uint32_t src_offset = offset + leb_size_guess;
      uint32_t dst_offset = offset + leb_size;
      move_data(ctx, dst_offset, src_offset, size);
    }
    out_u32_leb128_at(ctx, offset, size, desc);
    ctx->writer_offset += leb_size - leb_size_guess;
  } else {
    uint32_t size = ctx->writer_offset - offset - MAX_U32_LEB128_BYTES;
    out_fixed_u32_leb128_at(ctx, offset, size, desc);
  }
}

static void out_str(WasmContext* ctx,
                    const char* s,
                    size_t length,
                    int print_chars,
                    const char* desc) {
  out_u32_leb128(ctx, length, "string length");
  out_data_at(ctx, ctx->writer_offset, s, length, print_chars, desc);
  ctx->writer_offset += length;
}

static void out_opcode(WasmContext* ctx, uint8_t opcode) {
  out_u8(ctx, opcode, s_opcode_name[opcode]);
}

static void begin_section(WasmContext* ctx,
                          const char* name,
                          size_t leb_size_guess) {
  assert(ctx->last_section_leb_size_guess == 0);
  char desc[100];
  wasm_snprintf(desc, sizeof(desc), "section \"%s\"", name);
  print_header(ctx, desc, PRINT_HEADER_NO_INDEX);
  ctx->last_section_offset =
      out_u32_leb128_space(ctx, leb_size_guess, "section size (guess)");
  ctx->last_section_leb_size_guess = leb_size_guess;
  wasm_snprintf(desc, sizeof(desc), "section id: \"%s\"", name);
  out_str(ctx, name, strlen(name), DONT_PRINT_CHARS, desc);
}

static void end_section(WasmContext* ctx) {
  assert(ctx->last_section_leb_size_guess != 0);
  out_fixup_u32_leb128_size(ctx, ctx->last_section_offset,
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
  int i;
  for (i = 0; i < sigs->size; ++i) {
    const WasmFuncSignature* sig2 = &sigs->data[i];
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

static void get_func_signatures(WasmContext* ctx,
                                const WasmModule* module,
                                WasmFuncSignatureVector* sigs) {
  /* function types are not deduped; we don't want the signature index to match
   if they were specified separately in the source */
  int i;
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
      const WasmFuncType* func_type =
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
    const WasmFunc* func = module->funcs.data[i];
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

static void remap_locals(WasmContext* ctx, const WasmFunc* func) {
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

static void write_expr_list(WasmContext* ctx,
                            const WasmModule* module,
                            const WasmFunc* func,
                            const WasmExprPtrVector* exprs);

static void write_expr_list_with_count(WasmContext* ctx,
                                       const WasmModule* module,
                                       const WasmFunc* func,
                                       const WasmExprPtrVector* exprs);

static void write_block(WasmContext* ctx,
                        const WasmModule* module,
                        const WasmFunc* func,
                        const WasmBlock* block);

static void write_expr(WasmContext* ctx,
                       const WasmModule* module,
                       const WasmFunc* func,
                       const WasmExpr* expr) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      out_opcode(ctx, expr->binary.opcode);
      write_expr(ctx, module, func, expr->binary.left);
      write_expr(ctx, module, func, expr->binary.right);
      break;
    case WASM_EXPR_TYPE_BLOCK:
      write_block(ctx, module, func, &expr->block);
      break;
    case WASM_EXPR_TYPE_BR: {
      WasmLabelNode* node = find_label_by_var(ctx->top_label, &expr->br.var);
      assert(node);
      out_opcode(ctx, WASM_OPCODE_BR);
      out_u32_leb128(ctx, ctx->max_depth - node->depth - 1, "break depth");
      if (expr->br.expr)
        write_expr(ctx, module, func, expr->br.expr);
      else
        out_opcode(ctx, WASM_OPCODE_NOP);
      break;
    }
    case WASM_EXPR_TYPE_BR_IF: {
      WasmLabelNode* node = find_label_by_var(ctx->top_label, &expr->br_if.var);
      assert(node);
      out_opcode(ctx, WASM_OPCODE_BR_IF);
      out_u32_leb128(ctx, ctx->max_depth - node->depth - 1, "break depth");
      if (expr->br_if.expr)
        write_expr(ctx, module, func, expr->br_if.expr);
      else
        out_opcode(ctx, WASM_OPCODE_NOP);
      write_expr(ctx, module, func, expr->br_if.cond);
      break;
    }
    case WASM_EXPR_TYPE_CALL: {
      int index = wasm_get_func_index_by_var(module, &expr->call.var);
      assert(index >= 0 && index < module->funcs.size);
      out_opcode(ctx, WASM_OPCODE_CALL_FUNCTION);
      out_u32_leb128(ctx, index, "func index");
      write_expr_list(ctx, module, func, &expr->call.args);
      break;
    }
    case WASM_EXPR_TYPE_CALL_IMPORT: {
      int index = wasm_get_import_index_by_var(module, &expr->call.var);
      assert(index >= 0 && index < module->imports.size);
      out_opcode(ctx, WASM_OPCODE_CALL_IMPORT);
      out_u32_leb128(ctx, index, "import index");
      write_expr_list(ctx, module, func, &expr->call.args);
      break;
    }
    case WASM_EXPR_TYPE_CALL_INDIRECT: {
      int index =
          wasm_get_func_type_index_by_var(module, &expr->call_indirect.var);
      assert(index >= 0 && index < module->func_types.size);
      out_opcode(ctx, WASM_OPCODE_CALL_INDIRECT);
      out_u32_leb128(ctx, index, "signature index");
      write_expr(ctx, module, func, expr->call_indirect.expr);
      write_expr_list(ctx, module, func, &expr->call_indirect.args);
      break;
    }
    case WASM_EXPR_TYPE_COMPARE:
      out_opcode(ctx, expr->compare.opcode);
      write_expr(ctx, module, func, expr->compare.left);
      write_expr(ctx, module, func, expr->compare.right);
      break;
    case WASM_EXPR_TYPE_CONST:
      switch (expr->const_.type) {
        case WASM_TYPE_I32: {
          out_opcode(ctx, WASM_OPCODE_I32_CONST);
          out_i32_leb128(ctx, (int32_t)expr->const_.u32, "i32 literal");
          break;
        }
        case WASM_TYPE_I64:
          out_opcode(ctx, WASM_OPCODE_I64_CONST);
          out_i64_leb128(ctx, (int64_t)expr->const_.u64, "i64 literal");
          break;
        case WASM_TYPE_F32:
          out_opcode(ctx, WASM_OPCODE_F32_CONST);
          out_u32(ctx, expr->const_.f32_bits, "f32 literal");
          break;
        case WASM_TYPE_F64:
          out_opcode(ctx, WASM_OPCODE_F64_CONST);
          out_u64(ctx, expr->const_.f64_bits, "f64 literal");
          break;
        default:
          assert(0);
      }
      break;
    case WASM_EXPR_TYPE_CONVERT:
      out_opcode(ctx, expr->convert.opcode);
      write_expr(ctx, module, func, expr->convert.expr);
      break;
    case WASM_EXPR_TYPE_GET_LOCAL: {
      int index = wasm_get_local_index_by_var(func, &expr->get_local.var);
      assert(index >= 0 && index < func->params_and_locals.types.size);
      out_opcode(ctx, WASM_OPCODE_GET_LOCAL);
      out_u32_leb128(ctx, ctx->remapped_locals[index], "remapped local index");
      break;
    }
    case WASM_EXPR_TYPE_GROW_MEMORY:
      out_opcode(ctx, WASM_OPCODE_GROW_MEMORY);
      write_expr(ctx, module, func, expr->grow_memory.expr);
      break;
    case WASM_EXPR_TYPE_IF:
      out_opcode(ctx, WASM_OPCODE_IF);
      write_expr(ctx, module, func, expr->if_.cond);
      write_expr(ctx, module, func, expr->if_.true_);
      break;
    case WASM_EXPR_TYPE_IF_ELSE:
      out_opcode(ctx, WASM_OPCODE_IF_ELSE);
      write_expr(ctx, module, func, expr->if_else.cond);
      write_expr(ctx, module, func, expr->if_else.true_);
      write_expr(ctx, module, func, expr->if_else.false_);
      break;
    case WASM_EXPR_TYPE_LOAD: {
      out_opcode(ctx, expr->load.opcode);
      uint32_t align =
          wasm_get_opcode_alignment(expr->load.opcode, expr->load.align);
      uint8_t align_log = 0;
      while (align > 1) {
        align >>= 1;
        align_log++;
      }
      out_u8(ctx, align_log, "alignment");
      out_u32_leb128(ctx, (uint32_t)expr->load.offset, "load offset");
      write_expr(ctx, module, func, expr->load.addr);
      break;
    }
    case WASM_EXPR_TYPE_LOOP: {
      WasmLabelNode outer;
      WasmLabelNode inner;
      push_label(ctx, &outer, &expr->loop.outer);
      push_label(ctx, &inner, &expr->loop.inner);
      out_opcode(ctx, WASM_OPCODE_LOOP);
      write_expr_list_with_count(ctx, module, func, &expr->loop.exprs);
      pop_label(ctx, &expr->loop.inner);
      pop_label(ctx, &expr->loop.outer);
      break;
    }
    case WASM_EXPR_TYPE_MEMORY_SIZE:
      out_opcode(ctx, WASM_OPCODE_MEMORY_SIZE);
      break;
    case WASM_EXPR_TYPE_NOP:
      out_opcode(ctx, WASM_OPCODE_NOP);
      break;
    case WASM_EXPR_TYPE_RETURN:
      out_opcode(ctx, WASM_OPCODE_RETURN);
      if (expr->return_.expr)
        write_expr(ctx, module, func, expr->return_.expr);
      break;
    case WASM_EXPR_TYPE_SELECT:
      out_opcode(ctx, WASM_OPCODE_SELECT);
      write_expr(ctx, module, func, expr->select.true_);
      write_expr(ctx, module, func, expr->select.false_);
      write_expr(ctx, module, func, expr->select.cond);
      break;
    case WASM_EXPR_TYPE_SET_LOCAL: {
      int index = wasm_get_local_index_by_var(func, &expr->get_local.var);
      out_opcode(ctx, WASM_OPCODE_SET_LOCAL);
      out_u32_leb128(ctx, ctx->remapped_locals[index], "remapped local index");
      write_expr(ctx, module, func, expr->set_local.expr);
      break;
    }
    case WASM_EXPR_TYPE_STORE: {
      out_opcode(ctx, expr->store.opcode);
      uint32_t align =
          wasm_get_opcode_alignment(expr->store.opcode, expr->store.align);
      uint8_t align_log = 0;
      while (align > 1) {
        align >>= 1;
        align_log++;
      }
      out_u8(ctx, align_log, "alignment");
      out_u32_leb128(ctx, (uint32_t)expr->store.offset, "store offset");
      write_expr(ctx, module, func, expr->store.addr);
      write_expr(ctx, module, func, expr->store.value);
      break;
    }
    case WASM_EXPR_TYPE_BR_TABLE: {
      out_opcode(ctx, WASM_OPCODE_BR_TABLE);
      out_u32_leb128(ctx, expr->br_table.targets.size, "num targets");
      int i;
      WasmLabelNode* node;
      for (i = 0; i < expr->br_table.targets.size; ++i) {
        const WasmVar* var = &expr->br_table.targets.data[i];
        node = find_label_by_var(ctx->top_label, var);
        out_u32(ctx, ctx->max_depth - node->depth - 1, "break depth");
      }
      node = find_label_by_var(ctx->top_label, &expr->br_table.default_target);
      out_u32(ctx, ctx->max_depth - node->depth - 1, "break depth for default");
      write_expr(ctx, module, func, expr->br_table.expr);
      break;
    }
    case WASM_EXPR_TYPE_UNARY:
      out_opcode(ctx, expr->unary.opcode);
      write_expr(ctx, module, func, expr->unary.expr);
      break;
    case WASM_EXPR_TYPE_UNREACHABLE:
      out_opcode(ctx, WASM_OPCODE_UNREACHABLE);
      break;
  }
}

static void write_expr_list(WasmContext* ctx,
                            const WasmModule* module,
                            const WasmFunc* func,
                            const WasmExprPtrVector* exprs) {
  int i;
  for (i = 0; i < exprs->size; ++i)
    write_expr(ctx, module, func, exprs->data[i]);
}

static void write_expr_list_with_count(WasmContext* ctx,
                                       const WasmModule* module,
                                       const WasmFunc* func,
                                       const WasmExprPtrVector* exprs) {
  out_u32_leb128(ctx, exprs->size, "num expressions");
  write_expr_list(ctx, module, func, exprs);
}

static void write_block(WasmContext* ctx,
                        const WasmModule* module,
                        const WasmFunc* func,
                        const WasmBlock* block) {
  WasmLabelNode node;
  if (block->exprs.size == 1 && !block->used) {
    push_unused_label(ctx, &node, &block->label);
    write_expr(ctx, module, func, block->exprs.data[0]);
    pop_unused_label(ctx, &block->label);
  } else {
    push_label(ctx, &node, &block->label);
    out_opcode(ctx, WASM_OPCODE_BLOCK);
    write_expr_list_with_count(ctx, module, func, &block->exprs);
    pop_label(ctx, &block->label);
  }
}

static void write_func_locals(WasmContext* ctx,
                              const WasmModule* module,
                              const WasmFunc* func,
                              const WasmTypeVector* local_types) {
  remap_locals(ctx, func);

  if (local_types->size == 0) {
    out_u32_leb128(ctx, 0, "local decl count");
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
  out_u32_leb128(ctx, local_decl_count, "local decl count");
  current_type = GET_LOCAL_TYPE(FIRST_LOCAL_INDEX);
  uint32_t local_type_count = 1;
  for (i = FIRST_LOCAL_INDEX + 1; i <= LAST_LOCAL_INDEX; ++i) {
    /* loop through an extra time to catch the final type transition */
    WasmType type = i == LAST_LOCAL_INDEX ? WASM_TYPE_VOID : GET_LOCAL_TYPE(i);
    if (current_type == type) {
      local_type_count++;
    } else {
      out_u32_leb128(ctx, local_type_count, "local type count");
      out_u8(ctx, current_type, s_type_names[current_type]);
      local_type_count = 1;
      current_type = type;
    }
  }
}

static void write_func(WasmContext* ctx,
                       const WasmModule* module,
                       const WasmFunc* func) {
  write_func_locals(ctx, module, func, &func->locals.types);
  write_expr_list(ctx, module, func, &func->exprs);
}

static void write_module(WasmContext* ctx, const WasmModule* module) {
  /* TODO(binji): better leb size guess. Some sections we know will only be 1
   byte, but others we can be fairly certain will be larger. */
  const size_t leb_size_guess = 1;

  int i;
  out_u32(ctx, WASM_BINARY_MAGIC, "WASM_BINARY_MAGIC");
  out_u32(ctx, WASM_BINARY_VERSION, "WASM_BINARY_VERSION");

  WasmFuncSignatureVector sigs;
  WASM_ZERO_MEMORY(sigs);
  get_func_signatures(ctx, module, &sigs);
  if (sigs.size) {
    begin_section(ctx, WASM_SECTION_NAME_SIGNATURES, leb_size_guess);
    out_u32_leb128(ctx, sigs.size, "num signatures");
    for (i = 0; i < sigs.size; ++i) {
      const WasmFuncSignature* sig = &sigs.data[i];
      print_header(ctx, "signature", i);
      out_u8(ctx, sig->param_types.size, "num params");
      out_u8(ctx, sig->result_type, "result_type");
      int j;
      for (j = 0; j < sig->param_types.size; ++j)
        out_u8(ctx, sig->param_types.data[j], "param type");
    }
    end_section(ctx);
  }

  if (module->imports.size) {
    begin_section(ctx, WASM_SECTION_NAME_IMPORT_TABLE, leb_size_guess);
    out_u32_leb128(ctx, module->imports.size, "num imports");

    for (i = 0; i < module->imports.size; ++i) {
      const WasmImport* import = module->imports.data[i];
      print_header(ctx, "import header", i);
      out_u32_leb128(ctx, ctx->import_sig_indexes[i], "import signature index");
      out_str(ctx, import->module_name.start, import->module_name.length,
              PRINT_CHARS, "import module name");
      out_str(ctx, import->func_name.start, import->func_name.length,
              PRINT_CHARS, "import function name");
    }
    end_section(ctx);
  }

  if (module->funcs.size) {
    begin_section(ctx, WASM_SECTION_NAME_FUNCTION_SIGNATURES, leb_size_guess);
    out_u32_leb128(ctx, module->funcs.size, "num functions");

    for (i = 0; i < module->funcs.size; ++i) {
      char desc[100];
      wasm_snprintf(desc, sizeof(desc), "function %d signature index", i);
      out_u32_leb128(ctx, ctx->func_sig_indexes[i], desc);
    }
    end_section(ctx);
  }

  if (module->table && module->table->size) {
    begin_section(ctx, WASM_SECTION_NAME_FUNCTION_TABLE, leb_size_guess);
    out_u32_leb128(ctx, module->table->size, "num function table entries");
    for (i = 0; i < module->table->size; ++i) {
      int index = wasm_get_func_index_by_var(module, &module->table->data[i]);
      assert(index >= 0 && index < module->funcs.size);
      out_u32_leb128(ctx, index, "function table entry");
    }
    end_section(ctx);
  }

  if (module->memory) {
    int export_memory = module->export_memory != NULL;
    begin_section(ctx, WASM_SECTION_NAME_MEMORY, leb_size_guess);
    out_u32_leb128(ctx, module->memory->initial_pages, "min mem pages");
    out_u32_leb128(ctx, module->memory->max_pages, "max mem pages");
    out_u8(ctx, export_memory, "export mem");
    end_section(ctx);
  }

  if (module->exports.size) {
    begin_section(ctx, WASM_SECTION_NAME_EXPORT_TABLE, leb_size_guess);
    out_u32_leb128(ctx, module->exports.size, "num exports");

    for (i = 0; i < module->exports.size; ++i) {
      const WasmExport* export = module->exports.data[i];
      int func_index = wasm_get_func_index_by_var(module, &export->var);
      assert(func_index >= 0 && func_index < module->funcs.size);
      out_u32_leb128(ctx, func_index, "export func index");
      out_str(ctx, export->name.start, export->name.length, PRINT_CHARS,
              "export name");
    }
    end_section(ctx);
  }

  if (module->start) {
    int start_func_index = wasm_get_func_index_by_var(module, module->start);
    if (start_func_index != -1) {
      begin_section(ctx, WASM_SECTION_NAME_START_FUNCTION, leb_size_guess);
      out_u32_leb128(ctx, start_func_index, "start func index");
      end_section(ctx);
    }
  }

  if (module->funcs.size) {
    begin_section(ctx, WASM_SECTION_NAME_FUNCTION_BODIES, leb_size_guess);
    out_u32_leb128(ctx, module->funcs.size, "num functions");

    for (i = 0; i < module->funcs.size; ++i) {
      print_header(ctx, "function body", i);
      const WasmFunc* func = module->funcs.data[i];

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
    begin_section(ctx, WASM_SECTION_NAME_DATA_SEGMENTS, leb_size_guess);
    out_u32_leb128(ctx, module->memory->segments.size, "num data segments");
    for (i = 0; i < module->memory->segments.size; ++i) {
      const WasmSegment* segment = &module->memory->segments.data[i];
      print_header(ctx, "segment header", i);
      out_u32_leb128(ctx, segment->addr, "segment address");
      out_u32_leb128(ctx, segment->size, "segment size");
      print_header(ctx, "segment data", i);
      out_data(ctx, segment->data, segment->size, "segment data");
    }
    end_section(ctx);
  }

  if (ctx->options->write_debug_names) {
    char desc[100];
    begin_section(ctx, WASM_SECTION_NAME_NAMES, leb_size_guess);
    out_u32_leb128(ctx, module->funcs.size, "num functions");
    for (i = 0; i < module->funcs.size; ++i) {
      const WasmFunc* func = module->funcs.data[i];
      wasm_snprintf(desc, sizeof(desc), "func name %d", i);
      out_str(ctx, func->name.start, func->name.length, PRINT_CHARS, desc);
      out_u32_leb128(ctx, func->params_and_locals.types.size, "num locals");

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
          const WasmBindingHashEntry* entry =
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
          out_str(ctx, name.start, name.length, PRINT_CHARS, desc);
        }
      }
    }
    end_section(ctx);
  }
  destroy_func_signature_vector_and_elements(ctx->allocator, &sigs);
}

static void write_commands(WasmContext* ctx, const WasmScript* script) {
  int i;
  int wrote_module = 0;
  for (i = 0; i < script->commands.size; ++i) {
    const WasmCommand* command = &script->commands.data[i];
    if (command->type != WASM_COMMAND_TYPE_MODULE)
      continue;

    write_module(ctx, &command->module);
    wrote_module = 1;
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
  wasm_free(ctx->allocator, ctx->local_type_names);
}

WasmResult wasm_write_binary_module(WasmAllocator* allocator,
                                    WasmWriter* writer,
                                    const WasmModule* module,
                                    const WasmWriteBinaryOptions* options) {
  WasmContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.options = options;
  ctx.result = WASM_OK;
  ctx.writer = writer;

  write_module(&ctx, module);

  cleanup_context(&ctx);
  return ctx.result;
}

WasmResult wasm_write_binary_script(WasmAllocator* allocator,
                                    WasmWriter* writer,
                                    const WasmScript* script,
                                    const WasmWriteBinaryOptions* options) {
  WasmContext ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.options = options;
  ctx.result = WASM_OK;
  ctx.writer = writer;

  write_commands(&ctx, script);

  cleanup_context(&ctx);
  return ctx.result;
}

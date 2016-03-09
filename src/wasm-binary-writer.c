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

#include <alloca.h>
#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "wasm-binary.h"
#include "wasm-internal.h"
#include "wasm-writer.h"

#define DEFAULT_MEMORY_EXPORT 1
#define DUMP_OCTETS_PER_LINE 16

#define SEGMENT_SIZE 13
#define SEGMENT_OFFSET_OFFSET 4

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

STATIC_ASSERT(WASM_TYPE_VOID == 0);
STATIC_ASSERT(WASM_TYPE_I32 == 1);
STATIC_ASSERT(WASM_TYPE_I64 == 2);
STATIC_ASSERT(WASM_TYPE_F32 == 3);
STATIC_ASSERT(WASM_TYPE_F64 == 4);

#define V(name, code) [code] = "OPCODE_" #name,
static const char* s_opcode_names[] = {WASM_FOREACH_OPCODE(V)};
#undef V

static uint8_t s_binary_opcodes[] = {
    WASM_OPCODE_F32_ADD,   WASM_OPCODE_F32_COPYSIGN, WASM_OPCODE_F32_DIV,
    WASM_OPCODE_F32_MAX,   WASM_OPCODE_F32_MIN,      WASM_OPCODE_F32_MUL,
    WASM_OPCODE_F32_SUB,   WASM_OPCODE_F64_ADD,      WASM_OPCODE_F64_COPYSIGN,
    WASM_OPCODE_F64_DIV,   WASM_OPCODE_F64_MAX,      WASM_OPCODE_F64_MIN,
    WASM_OPCODE_F64_MUL,   WASM_OPCODE_F64_SUB,      WASM_OPCODE_I32_ADD,
    WASM_OPCODE_I32_AND,   WASM_OPCODE_I32_DIV_S,    WASM_OPCODE_I32_DIV_U,
    WASM_OPCODE_I32_MUL,   WASM_OPCODE_I32_OR,       WASM_OPCODE_I32_REM_S,
    WASM_OPCODE_I32_REM_U, WASM_OPCODE_I32_SHL,      WASM_OPCODE_I32_SHR_S,
    WASM_OPCODE_I32_SHR_U, WASM_OPCODE_I32_ROL,      WASM_OPCODE_I32_ROR,
    WASM_OPCODE_I32_SUB,   WASM_OPCODE_I32_XOR,      WASM_OPCODE_I64_ADD,
    WASM_OPCODE_I64_AND,   WASM_OPCODE_I64_DIV_S,    WASM_OPCODE_I64_DIV_U,
    WASM_OPCODE_I64_MUL,   WASM_OPCODE_I64_OR,       WASM_OPCODE_I64_REM_S,
    WASM_OPCODE_I64_REM_U, WASM_OPCODE_I64_SHL,      WASM_OPCODE_I64_SHR_S,
    WASM_OPCODE_I64_SHR_U, WASM_OPCODE_I64_ROL,      WASM_OPCODE_I64_ROR,
    WASM_OPCODE_I64_SUB,   WASM_OPCODE_I64_XOR,
};
STATIC_ASSERT(ARRAY_SIZE(s_binary_opcodes) == WASM_NUM_BINARY_OP_TYPES);

static uint8_t s_compare_opcodes[] = {
    WASM_OPCODE_F32_EQ,   WASM_OPCODE_F32_GE,   WASM_OPCODE_F32_GT,
    WASM_OPCODE_F32_LE,   WASM_OPCODE_F32_LT,   WASM_OPCODE_F32_NE,
    WASM_OPCODE_F64_EQ,   WASM_OPCODE_F64_GE,   WASM_OPCODE_F64_GT,
    WASM_OPCODE_F64_LE,   WASM_OPCODE_F64_LT,   WASM_OPCODE_F64_NE,
    WASM_OPCODE_I32_EQ,   WASM_OPCODE_I32_GE_S, WASM_OPCODE_I32_GE_U,
    WASM_OPCODE_I32_GT_S, WASM_OPCODE_I32_GT_U, WASM_OPCODE_I32_LE_S,
    WASM_OPCODE_I32_LE_U, WASM_OPCODE_I32_LT_S, WASM_OPCODE_I32_LT_U,
    WASM_OPCODE_I32_NE,   WASM_OPCODE_I64_EQ,   WASM_OPCODE_I64_GE_S,
    WASM_OPCODE_I64_GE_U, WASM_OPCODE_I64_GT_S, WASM_OPCODE_I64_GT_U,
    WASM_OPCODE_I64_LE_S, WASM_OPCODE_I64_LE_U, WASM_OPCODE_I64_LT_S,
    WASM_OPCODE_I64_LT_U, WASM_OPCODE_I64_NE,
};
STATIC_ASSERT(ARRAY_SIZE(s_compare_opcodes) == WASM_NUM_COMPARE_OP_TYPES);

static uint8_t s_convert_opcodes[] = {
    WASM_OPCODE_F32_SCONVERT_I32,    WASM_OPCODE_F32_SCONVERT_I64,
    WASM_OPCODE_F32_UCONVERT_I32,    WASM_OPCODE_F32_UCONVERT_I64,
    WASM_OPCODE_F32_CONVERT_F64,     WASM_OPCODE_F64_SCONVERT_I32,
    WASM_OPCODE_F64_SCONVERT_I64,    WASM_OPCODE_F64_UCONVERT_I32,
    WASM_OPCODE_F64_UCONVERT_I64,    WASM_OPCODE_F64_CONVERT_F32,
    WASM_OPCODE_I32_SCONVERT_F32,    WASM_OPCODE_I32_SCONVERT_F64,
    WASM_OPCODE_I32_UCONVERT_F32,    WASM_OPCODE_I32_UCONVERT_F64,
    WASM_OPCODE_I32_CONVERT_I64,     WASM_OPCODE_I64_SCONVERT_I32,
    WASM_OPCODE_I64_UCONVERT_I32,    WASM_OPCODE_I64_SCONVERT_F32,
    WASM_OPCODE_I64_SCONVERT_F64,    WASM_OPCODE_I64_UCONVERT_F32,
    WASM_OPCODE_I64_UCONVERT_F64,    WASM_OPCODE_F32_REINTERPRET_I32,
    WASM_OPCODE_F64_REINTERPRET_I64, WASM_OPCODE_I32_REINTERPRET_F32,
    WASM_OPCODE_I64_REINTERPRET_F64,
};
STATIC_ASSERT(ARRAY_SIZE(s_convert_opcodes) == WASM_NUM_CONVERT_OP_TYPES);

static uint8_t s_mem_opcodes[] = {
    WASM_OPCODE_F32_LOAD_MEM,     WASM_OPCODE_F32_STORE_MEM,
    WASM_OPCODE_F64_LOAD_MEM,     WASM_OPCODE_F64_STORE_MEM,
    WASM_OPCODE_I32_LOAD_MEM,     WASM_OPCODE_I32_LOAD_MEM8_S,
    WASM_OPCODE_I32_LOAD_MEM8_U,  WASM_OPCODE_I32_LOAD_MEM16_S,
    WASM_OPCODE_I32_LOAD_MEM16_U, WASM_OPCODE_I32_STORE_MEM,
    WASM_OPCODE_I32_STORE_MEM8,   WASM_OPCODE_I32_STORE_MEM16,
    WASM_OPCODE_I64_LOAD_MEM,     WASM_OPCODE_I64_LOAD_MEM8_S,
    WASM_OPCODE_I64_LOAD_MEM8_U,  WASM_OPCODE_I64_LOAD_MEM16_S,
    WASM_OPCODE_I64_LOAD_MEM16_U, WASM_OPCODE_I64_LOAD_MEM32_S,
    WASM_OPCODE_I64_LOAD_MEM32_U, WASM_OPCODE_I64_STORE_MEM,
    WASM_OPCODE_I64_STORE_MEM8,   WASM_OPCODE_I64_STORE_MEM16,
    WASM_OPCODE_I64_STORE_MEM32,
};
STATIC_ASSERT(ARRAY_SIZE(s_mem_opcodes) == WASM_NUM_MEM_OP_TYPES);

static uint8_t s_unary_opcodes[] = {
    WASM_OPCODE_F32_ABS,         WASM_OPCODE_F32_CEIL,
    WASM_OPCODE_F32_FLOOR,       WASM_OPCODE_F32_NEAREST_INT,
    WASM_OPCODE_F32_NEG,         WASM_OPCODE_F32_SQRT,
    WASM_OPCODE_F32_TRUNC,       WASM_OPCODE_F64_ABS,
    WASM_OPCODE_F64_CEIL,        WASM_OPCODE_F64_FLOOR,
    WASM_OPCODE_F64_NEAREST_INT, WASM_OPCODE_F64_NEG,
    WASM_OPCODE_F64_SQRT,        WASM_OPCODE_F64_TRUNC,
    WASM_OPCODE_I32_CLZ,         WASM_OPCODE_I32_CTZ,
    WASM_OPCODE_BOOL_NOT,        WASM_OPCODE_I32_POPCNT,
    WASM_OPCODE_I64_CLZ,         WASM_OPCODE_I64_CTZ,
    WASM_OPCODE_I64_POPCNT,
};
STATIC_ASSERT(ARRAY_SIZE(s_unary_opcodes) == WASM_NUM_UNARY_OP_TYPES);

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

  int* import_sig_indexes;
  int* func_sig_indexes;
  int* remapped_locals;
} WasmWriteContext;

DECLARE_VECTOR(func_signature, WasmFuncSignature);
DEFINE_VECTOR(func_signature, WasmFuncSignature);

static void destroy_func_signature_vector_and_elements(
    WasmAllocator* allocator,
    WasmFuncSignatureVector* sigs) {
  DESTROY_VECTOR_AND_ELEMENTS(allocator, *sigs, func_signature);
}

static void print_header(WasmWriteContext* ctx, const char* name, int index) {
  if (ctx->options->log_writes)
    printf("; %s %d\n", name, index);
}

static void out_data_at(WasmWriterState* writer_state,
                        size_t offset,
                        const void* src,
                        size_t size,
                        const char* desc) {
  if (*writer_state->result != WASM_OK)
    return;
  if (writer_state->log_writes)
    wasm_print_memory(src, size, offset, 0, desc);
  if (writer_state->writer->write_data)
    *writer_state->result = writer_state->writer->write_data(
        offset, src, size, writer_state->writer->user_data);
}

static void out_u8(WasmWriterState* writer_state,
                   uint32_t value,
                   const char* desc) {
  assert(value <= UINT8_MAX);
  uint8_t value8 = value;
  out_data_at(writer_state, writer_state->offset, &value8, sizeof(value8),
              desc);
  writer_state->offset += sizeof(value8);
}

/* TODO(binji): endianness */
static void out_u16(WasmWriterState* writer_state,
                    uint32_t value,
                    const char* desc) {
  assert(value <= UINT16_MAX);
  uint16_t value16 = value;
  out_data_at(writer_state, writer_state->offset, &value16, sizeof(value16),
              desc);
  writer_state->offset += sizeof(value16);
}

static void out_u32(WasmWriterState* writer_state,
                    uint32_t value,
                    const char* desc) {
  out_data_at(writer_state, writer_state->offset, &value, sizeof(value), desc);
  writer_state->offset += sizeof(value);
}

static void out_u64(WasmWriterState* writer_state,
                    uint64_t value,
                    const char* desc) {
  out_data_at(writer_state, writer_state->offset, &value, sizeof(value), desc);
  writer_state->offset += sizeof(value);
}

static void out_f32(WasmWriterState* writer_state,
                    float value,
                    const char* desc) {
  out_data_at(writer_state, writer_state->offset, &value, sizeof(value), desc);
  writer_state->offset += sizeof(value);
}

static void out_f64(WasmWriterState* writer_state,
                    double value,
                    const char* desc) {
  out_data_at(writer_state, writer_state->offset, &value, sizeof(value), desc);
  writer_state->offset += sizeof(value);
}

static void out_u8_at(WasmWriterState* writer_state,
                      uint32_t offset,
                      uint32_t value,
                      const char* desc) {
  assert(value <= UINT8_MAX);
  uint8_t value8 = value;
  out_data_at(writer_state, offset, &value8, sizeof(value8), desc);
}

static void out_u16_at(WasmWriterState* writer_state,
                       uint32_t offset,
                       uint32_t value,
                       const char* desc) {
  assert(value <= UINT16_MAX);
  uint16_t value16 = value;
  out_data_at(writer_state, offset, &value16, sizeof(value16), desc);
}

static void out_u32_at(WasmWriterState* writer_state,
                       uint32_t offset,
                       uint32_t value,
                       const char* desc) {
  out_data_at(writer_state, offset, &value, sizeof(value), desc);
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

static void out_u32_leb128(WasmWriterState* writer_state,
                           uint32_t value,
                           const char* desc) {
  uint8_t data[5]; /* max 5 bytes */
  int i = 0;
  LEB128_LOOP_UNTIL(value == 0);
  int length = i;
  out_data_at(writer_state, writer_state->offset, data, length, desc);
  writer_state->offset += length;
}

static void out_i32_leb128(WasmWriterState* writer_state,
                           int32_t value,
                           const char* desc) {
  uint8_t data[5]; /* max 5 bytes */
  int i = 0;
  if (value < 0)
    LEB128_LOOP_UNTIL(value == -1 && (byte & 0x40));
  else
    LEB128_LOOP_UNTIL(value == 0 && !(byte & 0x40));

  int length = i;
  out_data_at(writer_state, writer_state->offset, data, length, desc);
  writer_state->offset += length;
}

static void out_i64_leb128(WasmWriterState* writer_state,
                           int64_t value,
                           const char* desc) {
  uint8_t data[10]; /* max 10 bytes */
  int i = 0;
  if (value < 0)
    LEB128_LOOP_UNTIL(value == -1 && (byte & 0x40));
  else
    LEB128_LOOP_UNTIL(value == 0 && !(byte & 0x40));

  int length = i;
  out_data_at(writer_state, writer_state->offset, data, length, desc);
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

static void out_str(WasmWriterState* writer_state,
                    const char* s,
                    size_t length,
                    const char* desc) {
  out_data_at(writer_state, writer_state->offset, s, length, desc);
  writer_state->offset += length;
  out_u8(writer_state, 0, "\\0");
}

static void out_opcode(WasmWriterState* writer_state, uint8_t opcode) {
  out_u8(writer_state, opcode, s_opcode_names[opcode]);
}

static void out_printf(WasmWriterState* writer_state, const char* format, ...) {
  va_list args;
  va_list args_copy;
  va_start(args, format);
  va_copy(args_copy, args);
  /* + 1 to account for the \0 that will be added automatically by vsnprintf */
  int len = vsnprintf(NULL, 0, format, args) + 1;
  va_end(args);
  char* buffer = alloca(len);
  vsnprintf(buffer, len, format, args_copy);
  va_end(args_copy);
  /* - 1 to remove the trailing \0 that was added by vsnprintf */
  out_data_at(writer_state, writer_state->offset, buffer, len - 1, "");
  writer_state->offset += len - 1;
}

static WasmLabelNode* find_label_by_name(WasmLabelNode* top_label,
                                         WasmStringSlice* name) {
  WasmLabelNode* node = top_label;
  while (node) {
    if (node->label && wasm_string_slices_are_equal(node->label, name))
      return node;
    node = node->next;
  }
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

static void push_label(WasmWriteContext* ctx,
                       WasmLabelNode* node,
                       WasmLabel* label) {
  assert(label);
  node->label = label;
  node->next = ctx->top_label;
  node->depth = ctx->max_depth;
  ctx->top_label = node;
  ctx->max_depth++;
}

static void pop_label(WasmWriteContext* ctx, WasmLabel* label) {
  ctx->max_depth--;
  if (ctx->top_label && ctx->top_label->label == label)
    ctx->top_label = ctx->top_label->next;
}

static void push_implicit_block(WasmWriteContext* ctx) {
  ctx->max_depth++;
}

static void pop_implicit_block(WasmWriteContext* ctx) {
  ctx->max_depth--;
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
    ZERO_MEMORY(sig->param_types);
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
        ZERO_MEMORY(sig->param_types);
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
        ZERO_MEMORY(sig->param_types);
        CHECK_ALLOC(wasm_extend_types(ctx->allocator, &sig->param_types,
                                      &func->params.types));
      }
    }

    ctx->func_sig_indexes[i] = index;
  }
}

static void remap_locals(WasmWriteContext* ctx, WasmFunc* func) {
  int num_params = func->params.types.size;
  int num_locals = func->locals.types.size;
  int num_params_and_locals = num_params + num_locals;
  ctx->remapped_locals =
      wasm_realloc(ctx->allocator, ctx->remapped_locals,
                   num_params_and_locals * sizeof(int), WASM_DEFAULT_ALIGN);
  if (num_params_and_locals)
    CHECK_ALLOC_NULL(ctx->remapped_locals);

  int max[WASM_NUM_TYPES];
  ZERO_MEMORY(max);
  int i;
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
  ZERO_MEMORY(seen);
  for (i = 0; i < num_locals; ++i) {
    WasmType type = func->locals.types.data[i];
    ctx->remapped_locals[num_params + i] = start[type] + seen[type]++;
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

static void write_expr(WasmWriteContext* ctx,
                       WasmModule* module,
                       WasmFunc* func,
                       WasmExpr* expr) {
  WasmWriterState* ws = &ctx->writer_state;
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      out_opcode(ws, s_binary_opcodes[expr->binary.op.op_type]);
      write_expr(ctx, module, func, expr->binary.left);
      write_expr(ctx, module, func, expr->binary.right);
      break;
    case WASM_EXPR_TYPE_BLOCK: {
      WasmLabelNode node;
      push_label(ctx, &node, &expr->block.label);
      out_opcode(ws, WASM_OPCODE_BLOCK);
      write_expr_list_with_count(ctx, module, func, &expr->block.exprs);
      pop_label(ctx, &expr->block.label);
      break;
    }
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
      out_opcode(ws, s_compare_opcodes[expr->compare.op.op_type]);
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
          out_f32(ws, expr->const_.f32, "f32 literal");
          break;
        case WASM_TYPE_F64:
          out_opcode(ws, WASM_OPCODE_F64_CONST);
          out_f64(ws, expr->const_.f64, "f64 literal");
          break;
        default:
          assert(0);
      }
      break;
    case WASM_EXPR_TYPE_CONVERT:
      out_opcode(ws, s_convert_opcodes[expr->convert.op.op_type]);
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
    case WASM_EXPR_TYPE_IF: {
      WasmLabelNode node;
      out_opcode(ws, WASM_OPCODE_IF);
      write_expr(ctx, module, func, expr->if_.cond);
      push_label(ctx, &node, &expr->if_.true_.label);
      /* TODO(binji): optimize so block is only included if the expression
       * count > 1, or if the expression list contains a br/br_if */
      out_opcode(ws, WASM_OPCODE_BLOCK);
      write_expr_list_with_count(ctx, module, func, &expr->if_.true_.exprs);
      pop_label(ctx, &expr->if_.true_.label);
      break;
    }
    case WASM_EXPR_TYPE_IF_ELSE: {
      WasmLabelNode true_node, false_node;
      out_opcode(ws, WASM_OPCODE_IF_THEN);
      write_expr(ctx, module, func, expr->if_else.cond);
      push_label(ctx, &true_node, &expr->if_else.true_.label);
      out_opcode(ws, WASM_OPCODE_BLOCK);
      write_expr_list_with_count(ctx, module, func, &expr->if_else.true_.exprs);
      pop_label(ctx, &expr->if_else.true_.label);
      push_label(ctx, &false_node, &expr->if_else.false_.label);
      out_opcode(ws, WASM_OPCODE_BLOCK);
      write_expr_list_with_count(ctx, module, func,
                                 &expr->if_else.false_.exprs);
      pop_label(ctx, &expr->if_else.false_.label);
      break;
    }
    case WASM_EXPR_TYPE_LOAD: {
      out_opcode(ws, s_mem_opcodes[expr->load.op.op_type]);
      uint32_t align = expr->load.align;
      if (align == WASM_USE_NATURAL_ALIGNMENT) {
        align = expr->load.op.size >> 3;
      }
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
      out_opcode(ws, s_mem_opcodes[expr->store.op.op_type]);
      uint32_t align = expr->store.align;
      if (align == WASM_USE_NATURAL_ALIGNMENT) {
        align = expr->store.op.size >> 3;
      }
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
      out_opcode(ws, s_unary_opcodes[expr->unary.op.op_type]);
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

typedef enum WasmWriteBlockOpcode {
  WASM_DONT_WRITE_BLOCK_OPCODE,
  WASM_WRITE_BLOCK_OPCODE,
} WasmWriteBlockOpcode;

static void write_expr_list_with_count(WasmWriteContext* ctx,
                                       WasmModule* module,
                                       WasmFunc* func,
                                       WasmExprPtrVector* exprs) {
  WasmWriterState* ws = &ctx->writer_state;
  out_u32_leb128(ws, exprs->size, "num expressions");
  write_expr_list(ctx, module, func, exprs);
}

static void write_func(WasmWriteContext* ctx,
                       WasmModule* module,
                       WasmFunc* func) {
  write_expr_list(ctx, module, func, &func->exprs);
}

static void write_module(WasmWriteContext* ctx, WasmModule* module) {
  int i;
  WasmWriterState* ws = &ctx->writer_state;
  ctx->writer_state.offset = 0;
  out_u32(ws, WASM_BINARY_MAGIC, "WASM_BINARY_MAGIC");
  out_u32(ws, WASM_BINARY_VERSION, "WASM_BINARY_VERSION");

  size_t segments_offset = 0;
  if (module->memory) {
    out_u8(ws, WASM_BINARY_SECTION_MEMORY, "WASM_BINARY_SECTION_MEMORY");
    out_u32_leb128(ws, module->memory->initial_pages, "min mem pages");
    out_u32_leb128(ws, module->memory->max_pages, "max mem pages");
    out_u8(ws, DEFAULT_MEMORY_EXPORT, "export mem");

    if (module->memory->segments.size) {
      out_u8(ws, WASM_BINARY_SECTION_DATA_SEGMENTS,
             "WASM_BINARY_SECTION_DATA_SEGMENTS");
      out_u32_leb128(ws, module->memory->segments.size, "num data segments");
      segments_offset = ctx->writer_state.offset;
      for (i = 0; i < module->memory->segments.size; ++i) {
        WasmSegment* segment = &module->memory->segments.data[i];
        print_header(ctx, "segment header", i);
        out_u32(ws, segment->addr, "segment address");
        out_u32(ws, 0, "segment data offset");
        out_u32(ws, segment->size, "segment size");
        out_u8(ws, 1, "segment init");
      }
    }
  }

  WasmFuncSignatureVector sigs;
  ZERO_MEMORY(sigs);
  get_func_signatures(ctx, module, &sigs);
  if (sigs.size) {
    out_u8(ws, WASM_BINARY_SECTION_SIGNATURES,
           "WASM_BINARY_SECTION_SIGNATURES");
    out_u32_leb128(ws, sigs.size, "num signatures");
    for (i = 0; i < sigs.size; ++i) {
      WasmFuncSignature* sig = &sigs.data[i];
      print_header(ctx, "signature", i);
      out_u8(ws, sig->param_types.size, "num params");
      out_u8(ws, sig->result_type, "result_type");
      int j;
      for (j = 0; j < sig->param_types.size; ++j)
        out_u8(ws, sig->param_types.data[j], "param type");
    }
  }

  size_t imports_offset = 0;
  if (module->imports.size) {
    out_u8(ws, WASM_BINARY_SECTION_IMPORTS, "WASM_BINARY_SECTION_IMPORTS");
    out_u32_leb128(ws, module->imports.size, "num imports");

    imports_offset = ctx->writer_state.offset;
    for (i = 0; i < module->imports.size; ++i) {
      print_header(ctx, "import header", i);
      out_u32_leb128(ws, ctx->import_sig_indexes[i], "import signature index");
      out_u32(ws, 0, "import module name offset");
      out_u32(ws, 0, "import function name offset");
    }
  }

  if (module->funcs.size) {
    out_u8(ws, WASM_BINARY_SECTION_FUNCTIONS, "WASM_BINARY_SECTION_FUNCTIONS");
    out_u32_leb128(ws, module->funcs.size, "num functions");

    for (i = 0; i < module->funcs.size; ++i) {
      WasmFunc* func = module->funcs.data[i];
      print_header(ctx, "function", i);
      /* TODO(binji): we're still remapping the locals here, but we may want to
       have this be an option, now that it is not necessary. */
      remap_locals(ctx, func);

      uint8_t flags = 0;
      out_u8(ws, flags, "func flags");
      out_u16(ws, ctx->func_sig_indexes[i], "func signature index");
      size_t func_body_offset = ctx->writer_state.offset;
      out_u16(ws, 0, "func body size");

      int num_locals[WASM_NUM_TYPES];
      ZERO_MEMORY(num_locals);
      int j;
      for (j = 0; j < func->locals.types.size; ++j)
        num_locals[func->locals.types.data[j]]++;

      int num_local_types = 0;
      for (j = 0; j < WASM_NUM_TYPES; ++j)
        if (num_locals[j])
          num_local_types++;

      out_u32_leb128(ws, num_local_types, "local decl count");
      if (num_locals[WASM_TYPE_I32]) {
        out_u32_leb128(ws, num_locals[WASM_TYPE_I32], "local i32 count");
        out_u8(ws, WASM_TYPE_I32, "WASM_TYPE_I32");
      }
      if (num_locals[WASM_TYPE_I64]) {
        out_u32_leb128(ws, num_locals[WASM_TYPE_I64], "local i64 count");
        out_u8(ws, WASM_TYPE_I64, "WASM_TYPE_I64");
      }
      if (num_locals[WASM_TYPE_F32]) {
        out_u32_leb128(ws, num_locals[WASM_TYPE_F32], "local f32 count");
        out_u8(ws, WASM_TYPE_F32, "WASM_TYPE_F32");
      }
      if (num_locals[WASM_TYPE_F64]) {
        out_u32_leb128(ws, num_locals[WASM_TYPE_F64], "local f64 count");
        out_u8(ws, WASM_TYPE_F64, "WASM_TYPE_F64");
      }

      write_func(ctx, module, func);
      int func_size =
          ctx->writer_state.offset - func_body_offset - sizeof(uint16_t);
      out_u16_at(ws, func_body_offset, func_size, "FIXUP func body size");
    }
  }

  size_t exports_offset = 0;
  if (module->exports.size) {
    out_u8(ws, WASM_BINARY_SECTION_EXPORTS, "WASM_BINARY_SECTION_EXPORTS");
    out_u32_leb128(ws, module->exports.size, "num exports");

    exports_offset = ctx->writer_state.offset;
    for (i = 0; i < module->exports.size; ++i) {
      WasmExport* export = module->exports.data[i];
      int func_index = wasm_get_func_index_by_var(module, &export->var);
      assert(func_index >= 0 && func_index < module->funcs.size);
      out_u32_leb128(ws, func_index, "export func index");
      out_u32(ws, 0, "export name offset");
    }
  }

  int start_func_index = wasm_get_func_index_by_var(module, &module->start);
  if (start_func_index != -1) {
    out_u8(ws, WASM_BINARY_SECTION_START, "WASM_BINARY_SECTION_START");
    out_u32_leb128(ws, start_func_index, "start func index");
  }

  if (module->table && module->table->size) {
    out_u8(ws, WASM_BINARY_SECTION_FUNCTION_TABLE,
           "WASM_BINARY_SECTION_FUNCTION_TABLE");
    out_u32_leb128(ws, module->table->size, "num function table entries");
    for (i = 0; i < module->table->size; ++i) {
      int index = wasm_get_func_index_by_var(module, &module->table->data[i]);
      assert(index >= 0 && index < module->funcs.size);
      out_u32_leb128(ws, index, "function table entry");
    }
  }

  out_u8(ws, WASM_BINARY_SECTION_END, "WASM_BINARY_SECTION_END");

  /* output segment data */
  size_t offset;
  if (module->memory) {
    offset = segments_offset;
    for (i = 0; i < module->memory->segments.size; ++i) {
      print_header(ctx, "segment data", i);
      WasmSegment* segment = &module->memory->segments.data[i];
      out_u32_at(ws, offset + SEGMENT_OFFSET_OFFSET, ctx->writer_state.offset,
                 "FIXUP segment data offset");
      out_data_at(ws, ctx->writer_state.offset, segment->data, segment->size,
                  "segment data");
      ctx->writer_state.offset += segment->size;
      offset += SEGMENT_SIZE;
    }
  }

  /* output import names */
  offset = imports_offset;
  for (i = 0; i < module->imports.size; ++i) {
    print_header(ctx, "import", i);
    WasmImport* import = module->imports.data[i];
    offset += size_u32_leb128(ctx->import_sig_indexes[i]);
    out_u32_at(ws, offset, ctx->writer_state.offset,
               "FIXUP import module name offset");
    out_str(ws, import->module_name.start, import->module_name.length,
            "import module name");
    offset += sizeof(uint32_t);
    out_u32_at(ws, offset, ctx->writer_state.offset,
               "FIXUP import function name offset");
    out_str(ws, import->func_name.start, import->func_name.length,
            "import function name");
    offset += sizeof(uint32_t);
  }

  /* output exported func names */
  offset = exports_offset;
  for (i = 0; i < module->exports.size; ++i) {
    print_header(ctx, "export", i);
    WasmExport* export = module->exports.data[i];
    int func_index = wasm_get_func_index_by_var(module, &export->var);
    assert(func_index >= 0 && func_index < module->funcs.size);
    offset += size_u32_leb128(func_index);
    out_u32_at(ws, offset, ctx->writer_state.offset, "FIXUP func name offset");
    out_str(ws, export->name.start, export->name.length, "export name");
    offset += sizeof(uint32_t);
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
  expr->compare.op.type = type;
  switch (type) {
    case WASM_TYPE_I32:
      expr->compare.op.op_type = WASM_COMPARE_OP_TYPE_I32_EQ;
      break;
    case WASM_TYPE_I64:
      expr->compare.op.op_type = WASM_COMPARE_OP_TYPE_I64_EQ;
      break;
    case WASM_TYPE_F32:
      expr->compare.op.op_type = WASM_COMPARE_OP_TYPE_F32_EQ;
      break;
    case WASM_TYPE_F64:
      expr->compare.op.op_type = WASM_COMPARE_OP_TYPE_F64_EQ;
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
  expr->compare.op.type = type;
  switch (type) {
    case WASM_TYPE_I32:
      expr->compare.op.op_type = WASM_COMPARE_OP_TYPE_I32_NE;
      break;
    case WASM_TYPE_I64:
      expr->compare.op.op_type = WASM_COMPARE_OP_TYPE_I64_NE;
      break;
    case WASM_TYPE_F32:
      expr->compare.op.op_type = WASM_COMPARE_OP_TYPE_F32_NE;
      break;
    case WASM_TYPE_F64:
      expr->compare.op.op_type = WASM_COMPARE_OP_TYPE_F64_NE;
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
      result->convert.op.op_type = WASM_CONVERT_OP_TYPE_I32_REINTERPRET_F32;
      break;
    case WASM_TYPE_F64:
      result->convert.op.op_type = WASM_CONVERT_OP_TYPE_I64_REINTERPRET_F64;
      break;
    default:
      assert(0);
      break;
  }
  result->convert.expr = expr;
  return result;
}

static WasmModuleField* append_module_field_and_fixup(
    WasmAllocator* allocator,
    WasmModule* module,
    WasmModuleFieldType module_field_type) {
  WasmModuleField* first_field = &module->fields.data[0];
  WasmModuleField* result =
      wasm_append_module_field(allocator, &module->fields);
  if (!result)
    return NULL;
  ZERO_MEMORY(*result);
  result->type = module_field_type;

  /* make space for the new entry, it will be assigned twice, once here, once
   below. It is easier to do this than check below whether the new value was
   already assigned. */
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

  if (first_field == &module->fields.data[0])
    return result;

  /* the first element moved, so we need to fixup all the pointers */
  int i;
  int num_funcs = 0;
  int num_imports = 0;
  int num_exports = 0;
  int num_func_types = 0;
  for (i = 0; i < module->fields.size; ++i) {
    WasmModuleField* field = &module->fields.data[i];
    switch (field->type) {
      case WASM_MODULE_FIELD_TYPE_FUNC:
        assert(num_funcs < module->funcs.size);
        module->funcs.data[num_funcs++] = &field->func;
        break;
      case WASM_MODULE_FIELD_TYPE_IMPORT:
        assert(num_imports < module->imports.size);
        module->imports.data[num_imports++] = &field->import;
        break;
      case WASM_MODULE_FIELD_TYPE_EXPORT:
        assert(num_exports < module->exports.size);
        module->exports.data[num_exports++] = &field->export_;
        break;
      case WASM_MODULE_FIELD_TYPE_EXPORT_MEMORY:
        module->export_memory = &field->export_memory;
        break;
      case WASM_MODULE_FIELD_TYPE_TABLE:
        module->table = &field->table;
        break;
      case WASM_MODULE_FIELD_TYPE_FUNC_TYPE:
        assert(num_func_types < module->func_types.size);
        module->func_types.data[num_func_types++] = &field->func_type;
        break;
      case WASM_MODULE_FIELD_TYPE_MEMORY:
        module->memory = &field->memory;
        break;
      case WASM_MODULE_FIELD_TYPE_START:
        /* not pointers, so they don't need to be fixed */
        break;
    }
  }

  return result;
fail:
  module->fields.size--;
  return NULL;
}

static WasmStringSlice create_assert_func_name(WasmAllocator* allocator,
                                               const char* format,
                                               int format_index) {
  WasmStringSlice name;
  char buffer[256];
  int buffer_len = snprintf(buffer, 256, format, format_index);
  name.start = wasm_strndup(allocator, buffer, buffer_len);
  name.length = buffer_len;
  return name;
}

static WasmFunc* append_nullary_func(WasmAllocator* allocator,
                                     WasmModule* module,
                                     WasmType result_type,
                                     WasmStringSlice export_name) {
  WasmModuleField* func_field = append_module_field_and_fixup(
      allocator, module, WASM_MODULE_FIELD_TYPE_FUNC);
  if (!func_field)
    return NULL;
  WasmFunc* func = &func_field->func;
  func->flags = WASM_FUNC_FLAG_HAS_SIGNATURE;
  func->result_type = result_type;
  int func_index = module->funcs.size - 1;
  /* invalidated by append_module_field_and_fixup below */
  func_field = NULL;
  func = NULL;

  WasmModuleField* export_field = append_module_field_and_fixup(
      allocator, module, WASM_MODULE_FIELD_TYPE_EXPORT);
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

            if (expected->type == WASM_TYPE_F32 && isnan(expected->f32)) {
              *expr_ptr = create_eq_expr(
                  script->allocator, WASM_TYPE_I32,
                  create_reinterpret_expr(script->allocator, WASM_TYPE_F32,
                                          invoke_expr),
                  create_reinterpret_expr(script->allocator, WASM_TYPE_F32,
                                          const_expr));
              CHECK_ALLOC_NULL(*expr_ptr);
            } else if (expected->type == WASM_TYPE_F64 &&
                       isnan(expected->f64)) {
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
}

WasmResult wasm_write_binary(WasmAllocator* allocator,
                             WasmWriter* writer,
                             WasmScript* script,
                             WasmWriteBinaryOptions* options) {
  WasmWriteContext ctx;
  ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.options = options;
  ctx.result = WASM_OK;

  if (options->spec) {
    WasmMemoryWriter mem_writer;
    ZERO_MEMORY(mem_writer);
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

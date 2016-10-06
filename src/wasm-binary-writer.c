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

#define V(NAME, code) [code] = #NAME,
static const char* s_section_name[] = {WASM_FOREACH_BINARY_SECTION(V)};
#undef V

typedef struct WasmLabelNode {
  const WasmLabel* label;
  int depth;
  struct WasmLabelNode* next;
} WasmLabelNode;

typedef struct Context {
  WasmAllocator* allocator;
  WasmStream stream;
  WasmStream* log_stream;
  const WasmWriteBinaryOptions* options;
  WasmLabelNode* top_label;
  int max_depth;

  size_t last_section_offset;
  size_t last_section_leb_size_guess;
} Context;

static uint8_t log2_u32(uint32_t x) {
  uint8_t result = 0;
  while (x > 1) {
    x >>= 1;
    result++;
  }
  return result;
}

static void write_header(Context* ctx, const char* name, int index) {
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
static uint32_t write_u32_leb128_space(Context* ctx,
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

static void write_fixup_u32_leb128_size(Context* ctx,
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
  wasm_write_u8(stream, opcode, wasm_get_opcode_name(opcode));
}

static void write_inline_signature_type(WasmStream* stream,
                                        const WasmBlockSignature* sig) {
  if (sig->size == 0) {
    wasm_write_u8(stream, 0, s_type_names[WASM_TYPE_VOID]);
  } else if (sig->size == 1) {
    wasm_write_u8(stream, sig->data[0], s_type_names[sig->data[0]]);
  } else {
    /* this is currently unrepresentable */
    wasm_write_u8(stream, 0xff, "INVALID INLINE SIGNATURE");
  }
}

static void begin_known_section(Context* ctx,
                                WasmBinarySection section_code,
                                size_t leb_size_guess) {
  assert(ctx->last_section_leb_size_guess == 0);
  char desc[100];
  wasm_snprintf(desc, sizeof(desc), "section \"%s\" (%u)",
                s_section_name[section_code], section_code);
  write_header(ctx, desc, PRINT_HEADER_NO_INDEX);
  wasm_write_u8(&ctx->stream, section_code, "section code");
  ctx->last_section_leb_size_guess = leb_size_guess;
  ctx->last_section_offset =
      write_u32_leb128_space(ctx, leb_size_guess, "section size (guess)");
}

static void begin_user_section(Context* ctx,
                               const char* name,
                               size_t leb_size_guess) {
  assert(ctx->last_section_leb_size_guess == 0);
  char desc[100];
  wasm_snprintf(desc, sizeof(desc), "section \"%s\"", name);
  write_header(ctx, desc, PRINT_HEADER_NO_INDEX);
  wasm_write_u8(&ctx->stream, WASM_BINARY_SECTION_UNKNOWN, "user section code");
  ctx->last_section_leb_size_guess = leb_size_guess;
  ctx->last_section_offset =
      write_u32_leb128_space(ctx, leb_size_guess, "section size (guess)");
  write_str(&ctx->stream, name, strlen(name), WASM_PRINT_CHARS,
            "user section name");
}

static void end_section(Context* ctx) {
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

static void push_label(Context* ctx,
                       WasmLabelNode* node,
                       const WasmLabel* label) {
  assert(label);
  node->label = label;
  node->next = ctx->top_label;
  node->depth = ctx->max_depth;
  ctx->top_label = node;
  ctx->max_depth++;
}

static void pop_label(Context* ctx, const WasmLabel* label) {
  ctx->max_depth--;
  if (ctx->top_label && ctx->top_label->label == label)
    ctx->top_label = ctx->top_label->next;
}

static void write_expr_list(Context* ctx,
                            const WasmModule* module,
                            const WasmFunc* func,
                            const WasmExpr* first_expr);

static void write_expr(Context* ctx,
                       const WasmModule* module,
                       const WasmFunc* func,
                       const WasmExpr* expr) {
  switch (expr->type) {
    case WASM_EXPR_TYPE_BINARY:
      write_opcode(&ctx->stream, expr->binary.opcode);
      break;
    case WASM_EXPR_TYPE_BLOCK: {
      WasmLabelNode node;
      push_label(ctx, &node, &expr->block.label);
      write_opcode(&ctx->stream, WASM_OPCODE_BLOCK);
      write_inline_signature_type(&ctx->stream, &expr->block.sig);
      write_expr_list(ctx, module, func, expr->block.first);
      write_opcode(&ctx->stream, WASM_OPCODE_END);
      pop_label(ctx, &expr->block.label);
      break;
    }
    case WASM_EXPR_TYPE_BR: {
      WasmLabelNode* node = find_label_by_var(ctx->top_label, &expr->br.var);
      assert(node);
      write_opcode(&ctx->stream, WASM_OPCODE_BR);
      write_u32_leb128(&ctx->stream, ctx->max_depth - node->depth - 1,
                       "break depth");
      break;
    }
    case WASM_EXPR_TYPE_BR_IF: {
      WasmLabelNode* node = find_label_by_var(ctx->top_label, &expr->br_if.var);
      assert(node);
      write_opcode(&ctx->stream, WASM_OPCODE_BR_IF);
      write_u32_leb128(&ctx->stream, ctx->max_depth - node->depth - 1,
                       "break depth");
      break;
    }
    case WASM_EXPR_TYPE_BR_TABLE: {
      write_opcode(&ctx->stream, WASM_OPCODE_BR_TABLE);
      write_u32_leb128(&ctx->stream, expr->br_table.targets.size,
                       "num targets");
      size_t i;
      WasmLabelNode* node;
      for (i = 0; i < expr->br_table.targets.size; ++i) {
        const WasmVar* var = &expr->br_table.targets.data[i];
        node = find_label_by_var(ctx->top_label, var);
        write_u32_leb128(&ctx->stream, ctx->max_depth - node->depth - 1,
                         "break depth");
      }
      node = find_label_by_var(ctx->top_label, &expr->br_table.default_target);
      write_u32_leb128(&ctx->stream, ctx->max_depth - node->depth - 1,
                       "break depth for default");
      break;
    }
    case WASM_EXPR_TYPE_CALL: {
      int index = wasm_get_func_index_by_var(module, &expr->call.var);
      assert(index >= 0 && (size_t)index < module->funcs.size);
      write_opcode(&ctx->stream, WASM_OPCODE_CALL_FUNCTION);
      write_u32_leb128(&ctx->stream, index, "func index");
      break;
    }
    case WASM_EXPR_TYPE_CALL_INDIRECT: {
      int index =
          wasm_get_func_type_index_by_var(module, &expr->call_indirect.var);
      assert(index >= 0 && (size_t)index < module->func_types.size);
      write_opcode(&ctx->stream, WASM_OPCODE_CALL_INDIRECT);
      write_u32_leb128(&ctx->stream, index, "signature index");
      break;
    }
    case WASM_EXPR_TYPE_COMPARE:
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
      write_opcode(&ctx->stream, expr->convert.opcode);
      break;
    case WASM_EXPR_TYPE_CURRENT_MEMORY:
      write_opcode(&ctx->stream, WASM_OPCODE_CURRENT_MEMORY);
      break;
    case WASM_EXPR_TYPE_DROP:
      write_opcode(&ctx->stream, WASM_OPCODE_DROP);
      break;
    case WASM_EXPR_TYPE_GET_GLOBAL: {
      int index = wasm_get_global_index_by_var(module, &expr->get_global.var);
      write_opcode(&ctx->stream, WASM_OPCODE_GET_GLOBAL);
      write_u32_leb128(&ctx->stream, index, "global index");
      break;
    }
    case WASM_EXPR_TYPE_GET_LOCAL: {
      int index = wasm_get_local_index_by_var(func, &expr->get_local.var);
      write_opcode(&ctx->stream, WASM_OPCODE_GET_LOCAL);
      write_u32_leb128(&ctx->stream, index, "local index");
      break;
    }
    case WASM_EXPR_TYPE_GROW_MEMORY:
      write_opcode(&ctx->stream, WASM_OPCODE_GROW_MEMORY);
      break;
    case WASM_EXPR_TYPE_IF: {
      WasmLabelNode node;
      write_opcode(&ctx->stream, WASM_OPCODE_IF);
      write_inline_signature_type(&ctx->stream, &expr->if_.true_.sig);
      push_label(ctx, &node, &expr->if_.true_.label);
      write_expr_list(ctx, module, func, expr->if_.true_.first);
      if (expr->if_.false_) {
        write_opcode(&ctx->stream, WASM_OPCODE_ELSE);
        write_expr_list(ctx, module, func, expr->if_.false_);
      }
      pop_label(ctx, &expr->if_.true_.label);
      write_opcode(&ctx->stream, WASM_OPCODE_END);
      break;
    }
    case WASM_EXPR_TYPE_LOAD: {
      write_opcode(&ctx->stream, expr->load.opcode);
      uint32_t align =
          wasm_get_opcode_alignment(expr->load.opcode, expr->load.align);
      wasm_write_u8(&ctx->stream, log2_u32(align), "alignment");
      write_u32_leb128(&ctx->stream, (uint32_t)expr->load.offset,
                       "load offset");
      break;
    }
    case WASM_EXPR_TYPE_LOOP: {
      WasmLabelNode node;
      push_label(ctx, &node, &expr->loop.label);
      write_opcode(&ctx->stream, WASM_OPCODE_LOOP);
      write_inline_signature_type(&ctx->stream, &expr->loop.sig);
      write_expr_list(ctx, module, func, expr->loop.first);
      write_opcode(&ctx->stream, WASM_OPCODE_END);
      pop_label(ctx, &expr->loop.label);
      break;
    }
    case WASM_EXPR_TYPE_NOP:
      write_opcode(&ctx->stream, WASM_OPCODE_NOP);
      break;
    case WASM_EXPR_TYPE_RETURN:
      write_opcode(&ctx->stream, WASM_OPCODE_RETURN);
      break;
    case WASM_EXPR_TYPE_SELECT:
      write_opcode(&ctx->stream, WASM_OPCODE_SELECT);
      break;
    case WASM_EXPR_TYPE_SET_GLOBAL: {
      int index = wasm_get_global_index_by_var(module, &expr->get_global.var);
      write_opcode(&ctx->stream, WASM_OPCODE_SET_GLOBAL);
      write_u32_leb128(&ctx->stream, index, "global index");
      break;
    }
    case WASM_EXPR_TYPE_SET_LOCAL: {
      int index = wasm_get_local_index_by_var(func, &expr->get_local.var);
      write_opcode(&ctx->stream, WASM_OPCODE_SET_LOCAL);
      write_u32_leb128(&ctx->stream, index, "local index");
      break;
    }
    case WASM_EXPR_TYPE_STORE: {
      write_opcode(&ctx->stream, expr->store.opcode);
      uint32_t align =
          wasm_get_opcode_alignment(expr->store.opcode, expr->store.align);
      wasm_write_u8(&ctx->stream, log2_u32(align), "alignment");
      write_u32_leb128(&ctx->stream, (uint32_t)expr->store.offset,
                       "store offset");
      break;
    }
    case WASM_EXPR_TYPE_TEE_LOCAL: {
      int index = wasm_get_local_index_by_var(func, &expr->get_local.var);
      write_opcode(&ctx->stream, WASM_OPCODE_TEE_LOCAL);
      write_u32_leb128(&ctx->stream, index, "local index");
      break;
    }
    case WASM_EXPR_TYPE_UNARY:
      write_opcode(&ctx->stream, expr->unary.opcode);
      break;
    case WASM_EXPR_TYPE_UNREACHABLE:
      write_opcode(&ctx->stream, WASM_OPCODE_UNREACHABLE);
      break;
  }
}

static void write_expr_list(Context* ctx,
                            const WasmModule* module,
                            const WasmFunc* func,
                            const WasmExpr* first) {
  const WasmExpr* expr;
  for (expr = first; expr; expr = expr->next)
    write_expr(ctx, module, func, expr);
}

static void write_init_expr(Context* ctx,
                            const WasmModule* module,
                            const WasmExpr* expr) {
  if (expr)
    write_expr(ctx, module, NULL, expr);
  write_opcode(&ctx->stream, WASM_OPCODE_END);
}

static void write_func_locals(Context* ctx,
                              const WasmModule* module,
                              const WasmFunc* func,
                              const WasmTypeVector* local_types) {
  if (local_types->size == 0) {
    write_u32_leb128(&ctx->stream, 0, "local decl count");
    return;
  }

  uint32_t num_params = wasm_get_num_params(func);

#define FIRST_LOCAL_INDEX (num_params)
#define LAST_LOCAL_INDEX (num_params + local_types->size)
#define GET_LOCAL_TYPE(x) (local_types->data[x - num_params])

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

static void write_func(Context* ctx,
                       const WasmModule* module,
                       const WasmFunc* func) {
  WasmLabelNode node;
  WasmLabel label = wasm_empty_string_slice();
  write_func_locals(ctx, module, func, &func->local_types);
  push_label(ctx, &node, &label);
  write_expr_list(ctx, module, func, func->first_expr);
  write_opcode(&ctx->stream, WASM_OPCODE_END);
  pop_label(ctx, &label);
}

static void write_table(Context* ctx, const WasmTable* table) {
  const WasmLimits* limits = &table->elem_limits;
  wasm_write_u8(&ctx->stream, WASM_BINARY_ELEM_TYPE_ANYFUNC, "table elem type");
  uint32_t flags = limits->has_max ? WASM_BINARY_LIMITS_HAS_MAX_FLAG : 0;
  write_u32_leb128(&ctx->stream, flags, "table flags");
  write_u32_leb128(&ctx->stream, limits->initial, "table initial elems");
  if (limits->has_max)
    write_u32_leb128(&ctx->stream, limits->max, "table max elems");
}

static void write_memory(Context* ctx, const WasmMemory* memory) {
  const WasmLimits* limits = &memory->page_limits;
  uint32_t flags = limits->has_max ? WASM_BINARY_LIMITS_HAS_MAX_FLAG : 0;
  write_u32_leb128(&ctx->stream, flags, "memory flags");
  write_u32_leb128(&ctx->stream, limits->initial, "memory initial pages");
  if (limits->has_max)
    write_u32_leb128(&ctx->stream, limits->max, "memory max pages");
}

static void write_global_header(Context* ctx, const WasmGlobal* global) {
  wasm_write_u8(&ctx->stream, global->type, s_type_names[global->type]);
  wasm_write_u8(&ctx->stream, global->mutable_, "global mutability");
}

static void write_module(Context* ctx, const WasmModule* module) {
  /* TODO(binji): better leb size guess. Some sections we know will only be 1
   byte, but others we can be fairly certain will be larger. */
  const size_t leb_size_guess = 1;

  size_t i;
  wasm_write_u32(&ctx->stream, WASM_BINARY_MAGIC, "WASM_BINARY_MAGIC");
  wasm_write_u32(&ctx->stream, WASM_BINARY_VERSION, "WASM_BINARY_VERSION");

  if (module->func_types.size) {
    begin_known_section(ctx, WASM_BINARY_SECTION_TYPE, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->func_types.size, "num types");
    for (i = 0; i < module->func_types.size; ++i) {
      const WasmFuncType* func_type = module->func_types.data[i];
      const WasmFuncSignature* sig = &func_type->sig;
      write_header(ctx, "type", i);
      wasm_write_u8(&ctx->stream, WASM_BINARY_TYPE_FORM_FUNCTION,
                    "function form");

      size_t j;
      uint32_t num_params = sig->param_types.size;
      uint32_t num_results = sig->result_types.size;
      write_u32_leb128(&ctx->stream, num_params, "num params");
      for (j = 0; j < num_params; ++j)
        wasm_write_u8(&ctx->stream, sig->param_types.data[j], "param type");

      write_u32_leb128(&ctx->stream, num_results, "num results");
      for (j = 0; j < num_results; ++j)
        wasm_write_u8(&ctx->stream, sig->result_types.data[j], "result type");
    }
    end_section(ctx);
  }

  if (module->imports.size) {
    begin_known_section(ctx, WASM_BINARY_SECTION_IMPORT, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->imports.size, "num imports");

    for (i = 0; i < module->imports.size; ++i) {
      const WasmImport* import = module->imports.data[i];
      write_header(ctx, "import header", i);
      write_str(&ctx->stream, import->module_name.start,
                import->module_name.length, WASM_PRINT_CHARS,
                "import module name");
      write_str(&ctx->stream, import->field_name.start,
                import->field_name.length, WASM_PRINT_CHARS,
                "import field name");
      wasm_write_u8(&ctx->stream, import->kind, "import kind");
      switch (import->kind) {
        case WASM_EXTERNAL_KIND_FUNC:
          write_u32_leb128(&ctx->stream, wasm_get_func_type_index_by_decl(
                                             module, &import->func.decl),
                           "import signature index");
          break;
        case WASM_EXTERNAL_KIND_TABLE:
          write_table(ctx, &import->table);
          break;
        case WASM_EXTERNAL_KIND_MEMORY:
          write_memory(ctx, &import->memory);
          break;
        case WASM_EXTERNAL_KIND_GLOBAL:
          write_global_header(ctx, &import->global);
          break;
        case WASM_NUM_EXTERNAL_KINDS:
          assert(0);
          break;
      }
    }
    end_section(ctx);
  }

  assert(module->funcs.size >= module->num_func_imports);
  uint32_t num_funcs = module->funcs.size - module->num_func_imports;
  if (num_funcs) {
    begin_known_section(ctx, WASM_BINARY_SECTION_FUNCTION, leb_size_guess);
    write_u32_leb128(&ctx->stream, num_funcs, "num functions");

    for (i = 0; i < num_funcs; ++i) {
      const WasmFunc* func = module->funcs.data[i + module->num_func_imports];
      char desc[100];
      wasm_snprintf(desc, sizeof(desc), "function %" PRIzd " signature index",
                    i);
      write_u32_leb128(&ctx->stream,
                       wasm_get_func_type_index_by_decl(module, &func->decl),
                       desc);
    }
    end_section(ctx);
  }

  assert(module->tables.size >= module->num_table_imports);
  uint32_t num_tables = module->tables.size - module->num_table_imports;
  if (num_tables) {
    begin_known_section(ctx, WASM_BINARY_SECTION_TABLE, leb_size_guess);
    write_u32_leb128(&ctx->stream, num_tables, "num tables");
    for (i = 0; i < num_tables; ++i) {
      const WasmTable* table =
          module->tables.data[i + module->num_table_imports];
      write_header(ctx, "table", i);
      write_table(ctx, table);
    }
    end_section(ctx);
  }

  assert(module->memories.size >= module->num_memory_imports);
  uint32_t num_memories = module->memories.size - module->num_memory_imports;
  if (num_memories) {
    begin_known_section(ctx, WASM_BINARY_SECTION_MEMORY, leb_size_guess);
    write_u32_leb128(&ctx->stream, num_memories, "num memories");
    for (i = 0; i < num_memories; ++i) {
      const WasmMemory* memory =
          module->memories.data[i + module->num_memory_imports];
      write_header(ctx, "memory", i);
      write_memory(ctx, memory);
    }
    end_section(ctx);
  }

  assert(module->globals.size >= module->num_global_imports);
  uint32_t num_globals = module->globals.size - module->num_global_imports;
  if (num_globals) {
    begin_known_section(ctx, WASM_BINARY_SECTION_GLOBAL, leb_size_guess);
    write_u32_leb128(&ctx->stream, num_globals, "num globals");

    for (i = 0; i < num_globals; ++i) {
      const WasmGlobal* global =
          module->globals.data[i + module->num_global_imports];
      write_global_header(ctx, global);
      write_init_expr(ctx, module, global->init_expr);
    }
    end_section(ctx);
  }

  if (module->exports.size) {
    begin_known_section(ctx, WASM_BINARY_SECTION_EXPORT, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->exports.size, "num exports");

    for (i = 0; i < module->exports.size; ++i) {
      const WasmExport* export = module->exports.data[i];
      write_str(&ctx->stream, export->name.start, export->name.length,
                WASM_PRINT_CHARS, "export name");
      wasm_write_u8(&ctx->stream, export->kind, "export kind");
      switch (export->kind) {
        case WASM_EXTERNAL_KIND_FUNC: {
          int index = wasm_get_func_index_by_var(module, &export->var);
          assert(index >= 0 && (size_t)index < module->funcs.size);
          write_u32_leb128(&ctx->stream, index, "export func index");
          break;
        }
        case WASM_EXTERNAL_KIND_TABLE: {
          int index = wasm_get_table_index_by_var(module, &export->var);
          assert(index >= 0 && (size_t)index < module->tables.size);
          write_u32_leb128(&ctx->stream, index, "export table index");
          break;
        }
        case WASM_EXTERNAL_KIND_MEMORY: {
          int index = wasm_get_memory_index_by_var(module, &export->var);
          assert(index >= 0 && (size_t)index < module->memories.size);
          write_u32_leb128(&ctx->stream, index, "export memory index");
          break;
        }
        case WASM_EXTERNAL_KIND_GLOBAL: {
          int index = wasm_get_global_index_by_var(module, &export->var);
          assert(index >= 0 && (size_t)index < module->globals.size);
          write_u32_leb128(&ctx->stream, index, "export global index");
          break;
        }
        case WASM_NUM_EXTERNAL_KINDS:
          assert(0);
          break;
      }
    }
    end_section(ctx);
  }

  if (module->start) {
    int start_func_index = wasm_get_func_index_by_var(module, module->start);
    if (start_func_index != -1) {
      begin_known_section(ctx, WASM_BINARY_SECTION_START, leb_size_guess);
      write_u32_leb128(&ctx->stream, start_func_index, "start func index");
      end_section(ctx);
    }
  }

  if (module->tables.size && module->elem_segments.size) {
    begin_known_section(ctx, WASM_BINARY_SECTION_ELEM, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->elem_segments.size,
                     "num elem segments");
    for (i = 0; i < module->elem_segments.size; ++i) {
      WasmElemSegment* segment = module->elem_segments.data[i];
      int table_index =
          wasm_get_table_index_by_var(module, &segment->table_var);
      write_header(ctx, "elem segment header", i);
      write_u32_leb128(&ctx->stream, table_index, "table index");
      write_init_expr(ctx, module, segment->offset);
      write_u32_leb128(&ctx->stream, segment->vars.size,
                       "num function indices");
      size_t j;
      for (j = 0; j < segment->vars.size; ++j) {
        int index = wasm_get_func_index_by_var(module, &segment->vars.data[j]);
        assert(index >= 0 && (size_t)index < module->funcs.size);
        write_u32_leb128(&ctx->stream, index, "function index");
      }
    }
    end_section(ctx);
  }

  if (num_funcs) {
    begin_known_section(ctx, WASM_BINARY_SECTION_CODE, leb_size_guess);
    write_u32_leb128(&ctx->stream, num_funcs, "num functions");

    for (i = 0; i < num_funcs; ++i) {
      write_header(ctx, "function body", i);
      const WasmFunc* func = module->funcs.data[i + module->num_func_imports];

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

  if (module->memories.size && module->data_segments.size) {
    begin_known_section(ctx, WASM_BINARY_SECTION_DATA, leb_size_guess);
    write_u32_leb128(&ctx->stream, module->data_segments.size,
                     "num data segments");
    for (i = 0; i < module->data_segments.size; ++i) {
      const WasmDataSegment* segment = module->data_segments.data[i];
      write_header(ctx, "data segment header", i);
      int memory_index =
          wasm_get_memory_index_by_var(module, &segment->memory_var);
      write_u32_leb128(&ctx->stream, memory_index, "memory index");
      write_init_expr(ctx, module, segment->offset);
      write_u32_leb128(&ctx->stream, segment->size, "data segment size");
      write_header(ctx, "data segment data", i);
      wasm_write_data(&ctx->stream, segment->data, segment->size,
                      "data segment data");
    }
    end_section(ctx);
  }

  if (ctx->options->write_debug_names) {
    WasmStringSliceVector index_to_name;
    WASM_ZERO_MEMORY(index_to_name);

    char desc[100];
    begin_user_section(ctx, WASM_BINARY_SECTION_NAME, leb_size_guess);
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
        wasm_make_type_binding_reverse_mapping(
            ctx->allocator, &func->decl.sig.param_types, &func->param_bindings,
            &index_to_name);
        size_t j;
        for (j = 0; j < num_params; ++j) {
          WasmStringSlice name = index_to_name.data[j];
          wasm_snprintf(desc, sizeof(desc), "local name %" PRIzd, j);
          write_str(&ctx->stream, name.start, name.length, WASM_PRINT_CHARS,
                    desc);
        }

        wasm_make_type_binding_reverse_mapping(
            ctx->allocator, &func->local_types, &func->local_bindings,
            &index_to_name);
        for (j = 0; j < num_locals; ++j) {
          WasmStringSlice name = index_to_name.data[j];
          wasm_snprintf(desc, sizeof(desc), "local name %" PRIzd,
                        num_params + j);
          write_str(&ctx->stream, name.start, name.length, WASM_PRINT_CHARS,
                    desc);
        }
      }
    }
    end_section(ctx);

    wasm_destroy_string_slice_vector(ctx->allocator, &index_to_name);
  }
}

static void write_commands(Context* ctx, const WasmScript* script) {
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

WasmResult wasm_write_binary_module(WasmAllocator* allocator,
                                    WasmWriter* writer,
                                    const WasmModule* module,
                                    const WasmWriteBinaryOptions* options) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.options = options;
  ctx.log_stream = options->log_stream;
  wasm_init_stream(&ctx.stream, writer, ctx.log_stream);
  write_module(&ctx, module);
  return ctx.stream.result;
}

WasmResult wasm_write_binary_script(WasmAllocator* allocator,
                                    WasmWriter* writer,
                                    const WasmScript* script,
                                    const WasmWriteBinaryOptions* options) {
  Context ctx;
  WASM_ZERO_MEMORY(ctx);
  ctx.allocator = allocator;
  ctx.options = options;
  ctx.log_stream = options->log_stream;
  wasm_init_stream(&ctx.stream, writer, ctx.log_stream);
  write_commands(&ctx, script);
  return ctx.stream.result;
}

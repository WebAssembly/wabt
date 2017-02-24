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

#include "binary-writer.h"
#include "config.h"

#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "ast.h"
#include "binary.h"
#include "stream.h"
#include "writer.h"

#define PRINT_HEADER_NO_INDEX -1
#define MAX_U32_LEB128_BYTES 5
#define MAX_U64_LEB128_BYTES 10

/* TODO(binji): better leb size guess. Some sections we know will only be 1
 byte, but others we can be fairly certain will be larger. */
static const size_t LEB_SECTION_SIZE_GUESS = 1;

#define ALLOC_FAILURE \
  fprintf(stderr, "%s:%d: allocation failed\n", __FILE__, __LINE__)

typedef struct Reloc {
  WabtRelocType type;
  size_t offset;
} Reloc;
WABT_DEFINE_VECTOR(reloc, Reloc);

typedef struct RelocSection {
  const char* name;
  WabtBinarySection section_code;
  RelocVector relocations;
} RelocSection;
WABT_DEFINE_VECTOR(reloc_section, RelocSection);

typedef struct Context {
  WabtStream stream;
  WabtStream* log_stream;
  const WabtWriteBinaryOptions* options;

  RelocSectionVector reloc_sections;
  RelocSection* current_reloc_section;

  size_t last_section_offset;
  size_t last_section_leb_size_guess;
  WabtBinarySection last_section_type;
  size_t last_section_payload_offset;
} Context;

void wabt_destroy_reloc_section(RelocSection* reloc_section) {
  wabt_destroy_reloc_vector(&reloc_section->relocations);
}

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
      wabt_writef(ctx->log_stream, "; %s\n", name);
    } else {
      wabt_writef(ctx->log_stream, "; %s %d\n", name, index);
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

uint32_t wabt_u32_leb128_length(uint32_t value) {
  uint8_t data[MAX_U32_LEB128_BYTES] WABT_UNUSED;
  uint32_t i = 0;
  LEB128_LOOP_UNTIL(value == 0);
  return i;
}

/* returns the length of the leb128 */
uint32_t wabt_write_u32_leb128_at(WabtStream* stream,
                                  uint32_t offset,
                                  uint32_t value,
                                  const char* desc) {
  uint8_t data[MAX_U32_LEB128_BYTES];
  uint32_t i = 0;
  LEB128_LOOP_UNTIL(value == 0);
  uint32_t length = i;
  wabt_write_data_at(stream, offset, data, length, WABT_DONT_PRINT_CHARS, desc);
  return length;
}

uint32_t wabt_write_fixed_u32_leb128_raw(uint8_t* data,
                                         uint8_t* end,
                                         uint32_t value) {
  if (end - data < MAX_U32_LEB128_BYTES)
    return 0;
  data[0] = (value & 0x7f) | 0x80;
  data[1] = ((value >> 7) & 0x7f) | 0x80;
  data[2] = ((value >> 14) & 0x7f) | 0x80;
  data[3] = ((value >> 21) & 0x7f) | 0x80;
  data[4] = ((value >> 28) & 0x0f);
  return MAX_U32_LEB128_BYTES;
}

uint32_t wabt_write_fixed_u32_leb128_at(WabtStream* stream,
                                        uint32_t offset,
                                        uint32_t value,
                                        const char* desc) {
  uint8_t data[MAX_U32_LEB128_BYTES];
  uint32_t length =
      wabt_write_fixed_u32_leb128_raw(data, data + MAX_U32_LEB128_BYTES, value);
  wabt_write_data_at(stream, offset, data, length, WABT_DONT_PRINT_CHARS, desc);
  return length;
}

void wabt_write_u32_leb128(WabtStream* stream,
                           uint32_t value,
                           const char* desc) {
  uint32_t length =
      wabt_write_u32_leb128_at(stream, stream->offset, value, desc);
  stream->offset += length;
}

void wabt_write_fixed_u32_leb128(WabtStream* stream,
                                 uint32_t value,
                                 const char* desc) {
  uint32_t length =
      wabt_write_fixed_u32_leb128_at(stream, stream->offset, value, desc);
  stream->offset += length;
}

void wabt_write_i32_leb128(WabtStream* stream,
                           int32_t value,
                           const char* desc) {
  uint8_t data[MAX_U32_LEB128_BYTES];
  uint32_t i = 0;
  if (value < 0)
    LEB128_LOOP_UNTIL(value == -1 && (byte & 0x40));
  else
    LEB128_LOOP_UNTIL(value == 0 && !(byte & 0x40));

  uint32_t length = i;
  wabt_write_data_at(stream, stream->offset, data, length,
                     WABT_DONT_PRINT_CHARS, desc);
  stream->offset += length;
}

static void write_i64_leb128(WabtStream* stream,
                             int64_t value,
                             const char* desc) {
  uint8_t data[MAX_U64_LEB128_BYTES];
  uint32_t i = 0;
  if (value < 0)
    LEB128_LOOP_UNTIL(value == -1 && (byte & 0x40));
  else
    LEB128_LOOP_UNTIL(value == 0 && !(byte & 0x40));

  int length = i;
  wabt_write_data_at(stream, stream->offset, data, length,
                     WABT_DONT_PRINT_CHARS, desc);
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
  wabt_write_data(&ctx->stream, data, bytes_to_write, desc);
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
      wabt_move_data(&ctx->stream, dst_offset, src_offset, size);
    }
    wabt_write_u32_leb128_at(&ctx->stream, offset, size, desc);
    ctx->stream.offset += leb_size - leb_size_guess;
  } else {
    uint32_t size = ctx->stream.offset - offset - MAX_U32_LEB128_BYTES;
    wabt_write_fixed_u32_leb128_at(&ctx->stream, offset, size, desc);
  }
}

void wabt_write_str(WabtStream* stream,
                    const char* s,
                    size_t length,
                    WabtPrintChars print_chars,
                    const char* desc) {
  wabt_write_u32_leb128(stream, length, "string length");
  wabt_write_data_at(stream, stream->offset, s, length, print_chars, desc);
  stream->offset += length;
}

void wabt_write_opcode(WabtStream* stream, WabtOpcode opcode) {
  wabt_write_u8(stream, opcode, wabt_get_opcode_name(opcode));
}

void wabt_write_type(WabtStream* stream, WabtType type) {
  wabt_write_i32_leb128(stream, type, wabt_get_type_name(type));
}

static void write_inline_signature_type(WabtStream* stream,
                                        const WabtBlockSignature* sig) {
  if (sig->size == 0) {
    wabt_write_type(stream, WABT_TYPE_VOID);
  } else if (sig->size == 1) {
    wabt_write_type(stream, sig->data[0]);
  } else {
    /* this is currently unrepresentable */
    wabt_write_u8(stream, 0xff, "INVALID INLINE SIGNATURE");
  }
}

static void begin_known_section(Context* ctx,
                                WabtBinarySection section_code,
                                size_t leb_size_guess) {
  assert(ctx->last_section_leb_size_guess == 0);
  char desc[100];
  wabt_snprintf(desc, sizeof(desc), "section \"%s\" (%u)",
                wabt_get_section_name(section_code), section_code);
  write_header(ctx, desc, PRINT_HEADER_NO_INDEX);
  wabt_write_u8(&ctx->stream, section_code, "section code");
  ctx->last_section_type = section_code;
  ctx->last_section_leb_size_guess = leb_size_guess;
  ctx->last_section_offset =
      write_u32_leb128_space(ctx, leb_size_guess, "section size (guess)");
  ctx->last_section_payload_offset = ctx->stream.offset;
}

static void begin_custom_section(Context* ctx,
                                 const char* name,
                                 size_t leb_size_guess) {
  assert(ctx->last_section_leb_size_guess == 0);
  char desc[100];
  wabt_snprintf(desc, sizeof(desc), "section \"%s\"", name);
  write_header(ctx, desc, PRINT_HEADER_NO_INDEX);
  wabt_write_u8(&ctx->stream, WABT_BINARY_SECTION_CUSTOM,
                "custom section code");
  ctx->last_section_type = WABT_BINARY_SECTION_CUSTOM;
  ctx->last_section_leb_size_guess = leb_size_guess;
  ctx->last_section_offset =
      write_u32_leb128_space(ctx, leb_size_guess, "section size (guess)");
  ctx->last_section_payload_offset = ctx->stream.offset;
  wabt_write_str(&ctx->stream, name, strlen(name), WABT_PRINT_CHARS,
                 "custom section name");
}

static void end_section(Context* ctx) {
  assert(ctx->last_section_leb_size_guess != 0);
  write_fixup_u32_leb128_size(ctx, ctx->last_section_offset,
                              ctx->last_section_leb_size_guess,
                              "FIXUP section size");
  ctx->last_section_leb_size_guess = 0;
}

static uint32_t get_label_var_depth(Context* ctx, const WabtVar* var) {
  assert(var->type == WABT_VAR_TYPE_INDEX);
  return var->index;
}

static void write_expr_list(Context* ctx,
                            const WabtModule* module,
                            const WabtFunc* func,
                            const WabtExpr* first_expr);

static void add_reloc(Context* ctx, WabtRelocType reloc_type) {
  // Add a new reloc section if needed
  if (!ctx->current_reloc_section ||
      ctx->current_reloc_section->section_code != ctx->last_section_type) {
    ctx->current_reloc_section =
        wabt_append_reloc_section(&ctx->reloc_sections);
    ctx->current_reloc_section->name =
        wabt_get_section_name(ctx->last_section_type);
    ctx->current_reloc_section->section_code = ctx->last_section_type;
  }

  // Add a new relocation to the curent reloc section
  Reloc* r = wabt_append_reloc(&ctx->current_reloc_section->relocations);
  r->type = reloc_type;
  r->offset = ctx->stream.offset - ctx->last_section_payload_offset;
}

static void write_u32_leb128_with_reloc(Context* ctx,
                                        uint32_t value,
                                        const char* desc,
                                        WabtRelocType reloc_type) {
  if (ctx->options->relocatable) {
    add_reloc(ctx, reloc_type);
    wabt_write_fixed_u32_leb128(&ctx->stream, value, desc);
  } else {
    wabt_write_u32_leb128(&ctx->stream, value, desc);
  }
}

static void write_expr(Context* ctx,
                       const WabtModule* module,
                       const WabtFunc* func,
                       const WabtExpr* expr) {
  switch (expr->type) {
    case WABT_EXPR_TYPE_BINARY:
      wabt_write_opcode(&ctx->stream, expr->binary.opcode);
      break;
    case WABT_EXPR_TYPE_BLOCK:
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_BLOCK);
      write_inline_signature_type(&ctx->stream, &expr->block.sig);
      write_expr_list(ctx, module, func, expr->block.first);
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_END);
      break;
    case WABT_EXPR_TYPE_BR:
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_BR);
      wabt_write_u32_leb128(
          &ctx->stream, get_label_var_depth(ctx, &expr->br.var), "break depth");
      break;
    case WABT_EXPR_TYPE_BR_IF:
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_BR_IF);
      wabt_write_u32_leb128(&ctx->stream,
                            get_label_var_depth(ctx, &expr->br_if.var),
                            "break depth");
      break;
    case WABT_EXPR_TYPE_BR_TABLE: {
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_BR_TABLE);
      wabt_write_u32_leb128(&ctx->stream, expr->br_table.targets.size,
                            "num targets");
      size_t i;
      uint32_t depth;
      for (i = 0; i < expr->br_table.targets.size; ++i) {
        depth = get_label_var_depth(ctx, &expr->br_table.targets.data[i]);
        wabt_write_u32_leb128(&ctx->stream, depth, "break depth");
      }
      depth = get_label_var_depth(ctx, &expr->br_table.default_target);
      wabt_write_u32_leb128(&ctx->stream, depth, "break depth for default");
      break;
    }
    case WABT_EXPR_TYPE_CALL: {
      int index = wabt_get_func_index_by_var(module, &expr->call.var);
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_CALL);
      write_u32_leb128_with_reloc(ctx, index, "function index",
                                  WABT_RELOC_FUNC_INDEX_LEB);
      break;
    }
    case WABT_EXPR_TYPE_CALL_INDIRECT: {
      int index =
          wabt_get_func_type_index_by_var(module, &expr->call_indirect.var);
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_CALL_INDIRECT);
      wabt_write_u32_leb128(&ctx->stream, index, "signature index");
      wabt_write_u32_leb128(&ctx->stream, 0, "call_indirect reserved");
      break;
    }
    case WABT_EXPR_TYPE_COMPARE:
      wabt_write_opcode(&ctx->stream, expr->compare.opcode);
      break;
    case WABT_EXPR_TYPE_CONST:
      switch (expr->const_.type) {
        case WABT_TYPE_I32: {
          wabt_write_opcode(&ctx->stream, WABT_OPCODE_I32_CONST);
          wabt_write_i32_leb128(&ctx->stream, (int32_t)expr->const_.u32,
                                "i32 literal");
          break;
        }
        case WABT_TYPE_I64:
          wabt_write_opcode(&ctx->stream, WABT_OPCODE_I64_CONST);
          write_i64_leb128(&ctx->stream, (int64_t)expr->const_.u64,
                           "i64 literal");
          break;
        case WABT_TYPE_F32:
          wabt_write_opcode(&ctx->stream, WABT_OPCODE_F32_CONST);
          wabt_write_u32(&ctx->stream, expr->const_.f32_bits, "f32 literal");
          break;
        case WABT_TYPE_F64:
          wabt_write_opcode(&ctx->stream, WABT_OPCODE_F64_CONST);
          wabt_write_u64(&ctx->stream, expr->const_.f64_bits, "f64 literal");
          break;
        default:
          assert(0);
      }
      break;
    case WABT_EXPR_TYPE_CONVERT:
      wabt_write_opcode(&ctx->stream, expr->convert.opcode);
      break;
    case WABT_EXPR_TYPE_CURRENT_MEMORY:
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_CURRENT_MEMORY);
      wabt_write_u32_leb128(&ctx->stream, 0, "current_memory reserved");
      break;
    case WABT_EXPR_TYPE_DROP:
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_DROP);
      break;
    case WABT_EXPR_TYPE_GET_GLOBAL: {
      int index = wabt_get_global_index_by_var(module, &expr->get_global.var);
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_GET_GLOBAL);
      write_u32_leb128_with_reloc(ctx, index, "global index",
                                  WABT_RELOC_GLOBAL_INDEX_LEB);
      break;
    }
    case WABT_EXPR_TYPE_GET_LOCAL: {
      int index = wabt_get_local_index_by_var(func, &expr->get_local.var);
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_GET_LOCAL);
      wabt_write_u32_leb128(&ctx->stream, index, "local index");
      break;
    }
    case WABT_EXPR_TYPE_GROW_MEMORY:
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_GROW_MEMORY);
      wabt_write_u32_leb128(&ctx->stream, 0, "grow_memory reserved");
      break;
    case WABT_EXPR_TYPE_IF:
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_IF);
      write_inline_signature_type(&ctx->stream, &expr->if_.true_.sig);
      write_expr_list(ctx, module, func, expr->if_.true_.first);
      if (expr->if_.false_) {
        wabt_write_opcode(&ctx->stream, WABT_OPCODE_ELSE);
        write_expr_list(ctx, module, func, expr->if_.false_);
      }
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_END);
      break;
    case WABT_EXPR_TYPE_LOAD: {
      wabt_write_opcode(&ctx->stream, expr->load.opcode);
      uint32_t align =
          wabt_get_opcode_alignment(expr->load.opcode, expr->load.align);
      wabt_write_u8(&ctx->stream, log2_u32(align), "alignment");
      wabt_write_u32_leb128(&ctx->stream, (uint32_t)expr->load.offset,
                            "load offset");
      break;
    }
    case WABT_EXPR_TYPE_LOOP:
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_LOOP);
      write_inline_signature_type(&ctx->stream, &expr->loop.sig);
      write_expr_list(ctx, module, func, expr->loop.first);
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_END);
      break;
    case WABT_EXPR_TYPE_NOP:
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_NOP);
      break;
    case WABT_EXPR_TYPE_RETURN:
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_RETURN);
      break;
    case WABT_EXPR_TYPE_SELECT:
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_SELECT);
      break;
    case WABT_EXPR_TYPE_SET_GLOBAL: {
      int index = wabt_get_global_index_by_var(module, &expr->get_global.var);
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_SET_GLOBAL);
      write_u32_leb128_with_reloc(ctx, index, "global index",
                                  WABT_RELOC_GLOBAL_INDEX_LEB);
      break;
    }
    case WABT_EXPR_TYPE_SET_LOCAL: {
      int index = wabt_get_local_index_by_var(func, &expr->get_local.var);
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_SET_LOCAL);
      wabt_write_u32_leb128(&ctx->stream, index, "local index");
      break;
    }
    case WABT_EXPR_TYPE_STORE: {
      wabt_write_opcode(&ctx->stream, expr->store.opcode);
      uint32_t align =
          wabt_get_opcode_alignment(expr->store.opcode, expr->store.align);
      wabt_write_u8(&ctx->stream, log2_u32(align), "alignment");
      wabt_write_u32_leb128(&ctx->stream, (uint32_t)expr->store.offset,
                            "store offset");
      break;
    }
    case WABT_EXPR_TYPE_TEE_LOCAL: {
      int index = wabt_get_local_index_by_var(func, &expr->get_local.var);
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_TEE_LOCAL);
      wabt_write_u32_leb128(&ctx->stream, index, "local index");
      break;
    }
    case WABT_EXPR_TYPE_UNARY:
      wabt_write_opcode(&ctx->stream, expr->unary.opcode);
      break;
    case WABT_EXPR_TYPE_UNREACHABLE:
      wabt_write_opcode(&ctx->stream, WABT_OPCODE_UNREACHABLE);
      break;
  }
}

static void write_expr_list(Context* ctx,
                            const WabtModule* module,
                            const WabtFunc* func,
                            const WabtExpr* first) {
  const WabtExpr* expr;
  for (expr = first; expr; expr = expr->next)
    write_expr(ctx, module, func, expr);
}

static void write_init_expr(Context* ctx,
                            const WabtModule* module,
                            const WabtExpr* expr) {
  if (expr)
    write_expr_list(ctx, module, nullptr, expr);
  wabt_write_opcode(&ctx->stream, WABT_OPCODE_END);
}

static void write_func_locals(Context* ctx,
                              const WabtModule* module,
                              const WabtFunc* func,
                              const WabtTypeVector* local_types) {
  if (local_types->size == 0) {
    wabt_write_u32_leb128(&ctx->stream, 0, "local decl count");
    return;
  }

  uint32_t num_params = wabt_get_num_params(func);

#define FIRST_LOCAL_INDEX (num_params)
#define LAST_LOCAL_INDEX (num_params + local_types->size)
#define GET_LOCAL_TYPE(x) (local_types->data[x - num_params])

  /* loop through once to count the number of local declaration runs */
  WabtType current_type = GET_LOCAL_TYPE(FIRST_LOCAL_INDEX);
  uint32_t local_decl_count = 1;
  uint32_t i;
  for (i = FIRST_LOCAL_INDEX + 1; i < LAST_LOCAL_INDEX; ++i) {
    WabtType type = GET_LOCAL_TYPE(i);
    if (current_type != type) {
      local_decl_count++;
      current_type = type;
    }
  }

  /* loop through again to write everything out */
  wabt_write_u32_leb128(&ctx->stream, local_decl_count, "local decl count");
  current_type = GET_LOCAL_TYPE(FIRST_LOCAL_INDEX);
  uint32_t local_type_count = 1;
  for (i = FIRST_LOCAL_INDEX + 1; i <= LAST_LOCAL_INDEX; ++i) {
    /* loop through an extra time to catch the final type transition */
    WabtType type = i == LAST_LOCAL_INDEX ? WABT_TYPE_VOID : GET_LOCAL_TYPE(i);
    if (current_type == type) {
      local_type_count++;
    } else {
      wabt_write_u32_leb128(&ctx->stream, local_type_count, "local type count");
      wabt_write_type(&ctx->stream, current_type);
      local_type_count = 1;
      current_type = type;
    }
  }
}

static void write_func(Context* ctx,
                       const WabtModule* module,
                       const WabtFunc* func) {
  write_func_locals(ctx, module, func, &func->local_types);
  write_expr_list(ctx, module, func, func->first_expr);
  wabt_write_opcode(&ctx->stream, WABT_OPCODE_END);
}

void wabt_write_limits(WabtStream* stream, const WabtLimits* limits) {
  uint32_t flags = limits->has_max ? WABT_BINARY_LIMITS_HAS_MAX_FLAG : 0;
  wabt_write_u32_leb128(stream, flags, "limits: flags");
  wabt_write_u32_leb128(stream, limits->initial, "limits: initial");
  if (limits->has_max)
    wabt_write_u32_leb128(stream, limits->max, "limits: max");
}

static void write_table(Context* ctx, const WabtTable* table) {
  wabt_write_type(&ctx->stream, WABT_TYPE_ANYFUNC);
  wabt_write_limits(&ctx->stream, &table->elem_limits);
}

static void write_memory(Context* ctx, const WabtMemory* memory) {
  wabt_write_limits(&ctx->stream, &memory->page_limits);
}

static void write_global_header(Context* ctx, const WabtGlobal* global) {
  wabt_write_type(&ctx->stream, global->type);
  wabt_write_u8(&ctx->stream, global->mutable_, "global mutability");
}

static void write_reloc_section(Context* ctx, RelocSection* reloc_section) {
  char section_name[128];
  sprintf(section_name, "%s.%s", WABT_BINARY_SECTION_RELOC,
          reloc_section->name);
  begin_custom_section(ctx, section_name, LEB_SECTION_SIZE_GUESS);
  wabt_write_u32_leb128(&ctx->stream, reloc_section->section_code,
                        "reloc section type");
  RelocVector* relocs = &reloc_section->relocations;
  wabt_write_u32_leb128(&ctx->stream, relocs->size, "num relocs");

  size_t i;
  for (i = 0; i < relocs->size; i++) {
    wabt_write_u32_leb128(&ctx->stream, relocs->data[i].type, "reloc type");
    wabt_write_u32_leb128(&ctx->stream, relocs->data[i].offset, "reloc offset");
  }

  end_section(ctx);
}

static WabtResult write_module(Context* ctx, const WabtModule* module) {
  size_t i;
  wabt_write_u32(&ctx->stream, WABT_BINARY_MAGIC, "WASM_BINARY_MAGIC");
  wabt_write_u32(&ctx->stream, WABT_BINARY_VERSION, "WASM_BINARY_VERSION");

  if (module->func_types.size) {
    begin_known_section(ctx, WABT_BINARY_SECTION_TYPE, LEB_SECTION_SIZE_GUESS);
    wabt_write_u32_leb128(&ctx->stream, module->func_types.size, "num types");
    for (i = 0; i < module->func_types.size; ++i) {
      const WabtFuncType* func_type = module->func_types.data[i];
      const WabtFuncSignature* sig = &func_type->sig;
      write_header(ctx, "type", i);
      wabt_write_type(&ctx->stream, WABT_TYPE_FUNC);

      size_t j;
      uint32_t num_params = sig->param_types.size;
      uint32_t num_results = sig->result_types.size;
      wabt_write_u32_leb128(&ctx->stream, num_params, "num params");
      for (j = 0; j < num_params; ++j)
        wabt_write_type(&ctx->stream, sig->param_types.data[j]);

      wabt_write_u32_leb128(&ctx->stream, num_results, "num results");
      for (j = 0; j < num_results; ++j)
        wabt_write_type(&ctx->stream, sig->result_types.data[j]);
    }
    end_section(ctx);
  }

  if (module->imports.size) {
    begin_known_section(ctx, WABT_BINARY_SECTION_IMPORT,
                        LEB_SECTION_SIZE_GUESS);
    wabt_write_u32_leb128(&ctx->stream, module->imports.size, "num imports");

    for (i = 0; i < module->imports.size; ++i) {
      const WabtImport* import = module->imports.data[i];
      write_header(ctx, "import header", i);
      wabt_write_str(&ctx->stream, import->module_name.start,
                     import->module_name.length, WABT_PRINT_CHARS,
                     "import module name");
      wabt_write_str(&ctx->stream, import->field_name.start,
                     import->field_name.length, WABT_PRINT_CHARS,
                     "import field name");
      wabt_write_u8(&ctx->stream, import->kind, "import kind");
      switch (import->kind) {
        case WABT_EXTERNAL_KIND_FUNC:
          wabt_write_u32_leb128(&ctx->stream, wabt_get_func_type_index_by_decl(
                                                  module, &import->func.decl),
                                "import signature index");
          break;
        case WABT_EXTERNAL_KIND_TABLE:
          write_table(ctx, &import->table);
          break;
        case WABT_EXTERNAL_KIND_MEMORY:
          write_memory(ctx, &import->memory);
          break;
        case WABT_EXTERNAL_KIND_GLOBAL:
          write_global_header(ctx, &import->global);
          break;
        case WABT_NUM_EXTERNAL_KINDS:
          assert(0);
          break;
      }
    }
    end_section(ctx);
  }

  assert(module->funcs.size >= module->num_func_imports);
  uint32_t num_funcs = module->funcs.size - module->num_func_imports;
  if (num_funcs) {
    begin_known_section(ctx, WABT_BINARY_SECTION_FUNCTION,
                        LEB_SECTION_SIZE_GUESS);
    wabt_write_u32_leb128(&ctx->stream, num_funcs, "num functions");

    for (i = 0; i < num_funcs; ++i) {
      const WabtFunc* func = module->funcs.data[i + module->num_func_imports];
      char desc[100];
      wabt_snprintf(desc, sizeof(desc), "function %" PRIzd " signature index",
                    i);
      wabt_write_u32_leb128(
          &ctx->stream, wabt_get_func_type_index_by_decl(module, &func->decl),
          desc);
    }
    end_section(ctx);
  }

  assert(module->tables.size >= module->num_table_imports);
  uint32_t num_tables = module->tables.size - module->num_table_imports;
  if (num_tables) {
    begin_known_section(ctx, WABT_BINARY_SECTION_TABLE, LEB_SECTION_SIZE_GUESS);
    wabt_write_u32_leb128(&ctx->stream, num_tables, "num tables");
    for (i = 0; i < num_tables; ++i) {
      const WabtTable* table =
          module->tables.data[i + module->num_table_imports];
      write_header(ctx, "table", i);
      write_table(ctx, table);
    }
    end_section(ctx);
  }

  assert(module->memories.size >= module->num_memory_imports);
  uint32_t num_memories = module->memories.size - module->num_memory_imports;
  if (num_memories) {
    begin_known_section(ctx, WABT_BINARY_SECTION_MEMORY,
                        LEB_SECTION_SIZE_GUESS);
    wabt_write_u32_leb128(&ctx->stream, num_memories, "num memories");
    for (i = 0; i < num_memories; ++i) {
      const WabtMemory* memory =
          module->memories.data[i + module->num_memory_imports];
      write_header(ctx, "memory", i);
      write_memory(ctx, memory);
    }
    end_section(ctx);
  }

  assert(module->globals.size >= module->num_global_imports);
  uint32_t num_globals = module->globals.size - module->num_global_imports;
  if (num_globals) {
    begin_known_section(ctx, WABT_BINARY_SECTION_GLOBAL,
                        LEB_SECTION_SIZE_GUESS);
    wabt_write_u32_leb128(&ctx->stream, num_globals, "num globals");

    for (i = 0; i < num_globals; ++i) {
      const WabtGlobal* global =
          module->globals.data[i + module->num_global_imports];
      write_global_header(ctx, global);
      write_init_expr(ctx, module, global->init_expr);
    }
    end_section(ctx);
  }

  if (module->exports.size) {
    begin_known_section(ctx, WABT_BINARY_SECTION_EXPORT,
                        LEB_SECTION_SIZE_GUESS);
    wabt_write_u32_leb128(&ctx->stream, module->exports.size, "num exports");

    for (i = 0; i < module->exports.size; ++i) {
      const WabtExport* export_ = module->exports.data[i];
      wabt_write_str(&ctx->stream, export_->name.start, export_->name.length,
                     WABT_PRINT_CHARS, "export name");
      wabt_write_u8(&ctx->stream, export_->kind, "export kind");
      switch (export_->kind) {
        case WABT_EXTERNAL_KIND_FUNC: {
          int index = wabt_get_func_index_by_var(module, &export_->var);
          wabt_write_u32_leb128(&ctx->stream, index, "export func index");
          break;
        }
        case WABT_EXTERNAL_KIND_TABLE: {
          int index = wabt_get_table_index_by_var(module, &export_->var);
          wabt_write_u32_leb128(&ctx->stream, index, "export table index");
          break;
        }
        case WABT_EXTERNAL_KIND_MEMORY: {
          int index = wabt_get_memory_index_by_var(module, &export_->var);
          wabt_write_u32_leb128(&ctx->stream, index, "export memory index");
          break;
        }
        case WABT_EXTERNAL_KIND_GLOBAL: {
          int index = wabt_get_global_index_by_var(module, &export_->var);
          wabt_write_u32_leb128(&ctx->stream, index, "export global index");
          break;
        }
        case WABT_NUM_EXTERNAL_KINDS:
          assert(0);
          break;
      }
    }
    end_section(ctx);
  }

  if (module->start) {
    int start_func_index = wabt_get_func_index_by_var(module, module->start);
    if (start_func_index != -1) {
      begin_known_section(ctx, WABT_BINARY_SECTION_START,
                          LEB_SECTION_SIZE_GUESS);
      wabt_write_u32_leb128(&ctx->stream, start_func_index, "start func index");
      end_section(ctx);
    }
  }

  if (module->elem_segments.size) {
    begin_known_section(ctx, WABT_BINARY_SECTION_ELEM, LEB_SECTION_SIZE_GUESS);
    wabt_write_u32_leb128(&ctx->stream, module->elem_segments.size,
                          "num elem segments");
    for (i = 0; i < module->elem_segments.size; ++i) {
      WabtElemSegment* segment = module->elem_segments.data[i];
      int table_index =
          wabt_get_table_index_by_var(module, &segment->table_var);
      write_header(ctx, "elem segment header", i);
      wabt_write_u32_leb128(&ctx->stream, table_index, "table index");
      write_init_expr(ctx, module, segment->offset);
      wabt_write_u32_leb128(&ctx->stream, segment->vars.size,
                            "num function indices");
      size_t j;
      for (j = 0; j < segment->vars.size; ++j) {
        int index = wabt_get_func_index_by_var(module, &segment->vars.data[j]);
        write_u32_leb128_with_reloc(ctx, index, "function index",
                                    WABT_RELOC_FUNC_INDEX_LEB);
      }
    }
    end_section(ctx);
  }

  if (num_funcs) {
    begin_known_section(ctx, WABT_BINARY_SECTION_CODE, LEB_SECTION_SIZE_GUESS);
    wabt_write_u32_leb128(&ctx->stream, num_funcs, "num functions");

    for (i = 0; i < num_funcs; ++i) {
      write_header(ctx, "function body", i);
      const WabtFunc* func = module->funcs.data[i + module->num_func_imports];

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

  if (module->data_segments.size) {
    begin_known_section(ctx, WABT_BINARY_SECTION_DATA, LEB_SECTION_SIZE_GUESS);
    wabt_write_u32_leb128(&ctx->stream, module->data_segments.size,
                          "num data segments");
    for (i = 0; i < module->data_segments.size; ++i) {
      const WabtDataSegment* segment = module->data_segments.data[i];
      write_header(ctx, "data segment header", i);
      int memory_index =
          wabt_get_memory_index_by_var(module, &segment->memory_var);
      wabt_write_u32_leb128(&ctx->stream, memory_index, "memory index");
      write_init_expr(ctx, module, segment->offset);
      wabt_write_u32_leb128(&ctx->stream, segment->size, "data segment size");
      write_header(ctx, "data segment data", i);
      wabt_write_data(&ctx->stream, segment->data, segment->size,
                      "data segment data");
    }
    end_section(ctx);
  }

  if (ctx->options->write_debug_names) {
    WabtStringSliceVector index_to_name;
    WABT_ZERO_MEMORY(index_to_name);

    char desc[100];
    begin_custom_section(ctx, WABT_BINARY_SECTION_NAME, LEB_SECTION_SIZE_GUESS);
    wabt_write_u32_leb128(&ctx->stream, module->funcs.size, "num functions");
    for (i = 0; i < module->funcs.size; ++i) {
      const WabtFunc* func = module->funcs.data[i];
      uint32_t num_params = wabt_get_num_params(func);
      uint32_t num_locals = func->local_types.size;
      uint32_t num_params_and_locals = wabt_get_num_params_and_locals(func);

      wabt_snprintf(desc, sizeof(desc), "func name %" PRIzd, i);
      wabt_write_str(&ctx->stream, func->name.start, func->name.length,
                     WABT_PRINT_CHARS, desc);
      wabt_write_u32_leb128(&ctx->stream, num_params_and_locals, "num locals");

      if (num_params_and_locals) {
        wabt_make_type_binding_reverse_mapping(
            &func->decl.sig.param_types, &func->param_bindings, &index_to_name);
        size_t j;
        for (j = 0; j < num_params; ++j) {
          WabtStringSlice name = index_to_name.data[j];
          wabt_snprintf(desc, sizeof(desc), "local name %" PRIzd, j);
          wabt_write_str(&ctx->stream, name.start, name.length,
                         WABT_PRINT_CHARS, desc);
        }

        wabt_make_type_binding_reverse_mapping(
            &func->local_types, &func->local_bindings, &index_to_name);
        for (j = 0; j < num_locals; ++j) {
          WabtStringSlice name = index_to_name.data[j];
          wabt_snprintf(desc, sizeof(desc), "local name %" PRIzd,
                        num_params + j);
          wabt_write_str(&ctx->stream, name.start, name.length,
                         WABT_PRINT_CHARS, desc);
        }
      }
    }
    end_section(ctx);

    wabt_destroy_string_slice_vector(&index_to_name);
  }

  if (ctx->options->relocatable) {
    for (i = 0; i < ctx->reloc_sections.size; i++) {
      write_reloc_section(ctx, &ctx->reloc_sections.data[i]);
    }
    WABT_DESTROY_VECTOR_AND_ELEMENTS(ctx->reloc_sections, reloc_section);
  }

  return ctx->stream.result;
}

WabtResult wabt_write_binary_module(WabtWriter* writer,
                                    const WabtModule* module,
                                    const WabtWriteBinaryOptions* options) {
  Context ctx;
  WABT_ZERO_MEMORY(ctx);
  ctx.options = options;
  ctx.log_stream = options->log_stream;
  wabt_init_stream(&ctx.stream, writer, ctx.log_stream);
  return write_module(&ctx, module);
}

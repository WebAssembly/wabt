/*
 * Copyright 2017 WebAssembly Community Group participants
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

#include "binary-reader-linker.h"

#include "binary-reader.h"
#include "wasm-link.h"

#define RELOC_SIZE 5

typedef struct Context {
  WasmAllocator* allocator;
  InputBinary* binary;

  Section* reloc_section;

  WasmStringSlice import_name;
  Section* current_section;
} Context;

static WasmResult on_reloc_count(uint32_t count, WasmBinarySection section_code,
                                 WasmStringSlice section_name,
                                 void* user_data) {
  Context* ctx = user_data;
  InputBinary* binary = ctx->binary;
  if (section_code == WASM_BINARY_SECTION_CUSTOM) {
    WASM_FATAL("relocation for custom sections not yet supported\n");
  }

  uint32_t i;
  for (i = 0; i < binary->sections.size; i++) {
    Section* sec = &binary->sections.data[i];
    if (sec->section_code != section_code)
      continue;
    ctx->reloc_section = sec;
    return WASM_OK;
  }

  WASM_FATAL("section not found: %d\n", section_code);
  return WASM_ERROR;
}

static WasmResult on_reloc(WasmRelocType type,
                           uint32_t offset,
                           void* user_data) {
  Context* ctx = user_data;

  if (offset + RELOC_SIZE > ctx->reloc_section->size) {
    WASM_FATAL("invalid relocation offset: %#x\n", offset);
  }

  Reloc* reloc = wasm_append_reloc(ctx->allocator, &ctx->reloc_section->relocations);
  reloc->type = type;
  reloc->offset = offset;

  return WASM_OK;
}

static WasmResult on_import(uint32_t index,
                            WasmStringSlice module_name,
                            WasmStringSlice field_name,
                            void* user_data) {
  Context* ctx = user_data;
  if (!wasm_string_slice_eq_cstr(&module_name, WASM_LINK_MODULE_NAME)) {
    WASM_FATAL("unsupported import module: " PRIstringslice,
               WASM_PRINTF_STRING_SLICE_ARG(module_name));
  }
  ctx->import_name = field_name;
  return WASM_OK;
}

static WasmResult on_import_func(uint32_t index,
                                 uint32_t sig_index,
                                 void* user_data) {
  Context* ctx = user_data;
  FunctionImport* import =
      wasm_append_function_import(ctx->allocator, &ctx->binary->function_imports);
  import->name = ctx->import_name;
  import->sig_index = sig_index;
  import->active = WASM_TRUE;
  ctx->binary->active_function_imports++;
  return WASM_OK;
}

static WasmResult on_import_global(uint32_t index,
                                   WasmType type,
                                   WasmBool mutable,
                                   void* user_data) {
  Context* ctx = user_data;
  GlobalImport* import =
      wasm_append_global_import(ctx->allocator, &ctx->binary->global_imports);
  import->name = ctx->import_name;
  import->type = type;
  import->mutable = mutable;
  ctx->binary->active_global_imports++;
  return WASM_OK;
}

static WasmResult begin_section(WasmBinaryReaderContext* ctx,
                                WasmBinarySection section_code,
                                uint32_t size) {
  Context* context = ctx->user_data;
  InputBinary* binary = context->binary;
  Section* sec = wasm_append_section(context->allocator, &binary->sections);
  context->current_section = sec;
  sec->section_code = section_code;
  sec->size = size;
  sec->offset = ctx->offset;
  sec->binary = binary;

  if (sec->section_code != WASM_BINARY_SECTION_CUSTOM &&
      sec->section_code != WASM_BINARY_SECTION_START) {
    size_t bytes_read = wasm_read_u32_leb128(
        &binary->data[sec->offset], &binary->data[binary->size], &sec->count);
    if (bytes_read == 0)
      WASM_FATAL("error reading section element count\n");
    sec->payload_offset = sec->offset + bytes_read;
    sec->payload_size = sec->size - bytes_read;
  }
  return WASM_OK;
}

static WasmResult begin_custom_section(WasmBinaryReaderContext* ctx,
                                       uint32_t size,
                                       WasmStringSlice section_name) {
  Context* context = ctx->user_data;
  InputBinary* binary = context->binary;
  Section* sec = context->current_section;
  sec->data_custom.name = section_name;

  /* Modify section size and offset to not include the name itself. */
  size_t delta = ctx->offset - sec->offset;
  sec->offset = sec->offset + delta;
  sec->size = sec->size - delta;
  sec->payload_offset = sec->offset;
  sec->payload_size = sec->size;

  /* Special handling for certain CUSTOM sections */
  if (wasm_string_slice_eq_cstr(&section_name, "name")) {
    size_t bytes_read = wasm_read_u32_leb128(
        &binary->data[sec->offset], &binary->data[binary->size], &sec->count);
    sec->payload_offset += bytes_read;
    sec->payload_size -= bytes_read;

    /* We don't currently support merging name sections unless they contain
     * a name for every function. */
    size_t i;
    uint32_t total_funcs = binary->function_imports.size;
    for (i = 0; i < binary->sections.size; i++) {
      if (binary->sections.data[i].section_code ==
          WASM_BINARY_SECTION_FUNCTION) {
        total_funcs += binary->sections.data[i].count;
        break;
      }
    }
    if (total_funcs != sec->count) {
      WASM_FATAL(
          "name section count (%d) does not match function count (%d)\n",
          sec->count, total_funcs);
    }
  }

  return WASM_OK;
}

static WasmResult on_table(uint32_t index,
                           WasmType elem_type,
                           const WasmLimits* elem_limits,
                           void* user_data) {
  if (elem_limits->has_max && (elem_limits->max != elem_limits->initial))
    WASM_FATAL("Tables with max != initial not supported by wasm-link\n");

  Context* ctx = user_data;
  ctx->binary->table_elem_count = elem_limits->initial;
  return WASM_OK;
}

static WasmResult on_elem_segment_function_index_count(
    WasmBinaryReaderContext* ctx,
    uint32_t index,
    uint32_t count) {
  Context* context = ctx->user_data;
  Section* sec = context->current_section;

  /* Modify the payload to include only the actual function indexes */
  size_t delta = ctx->offset - sec->payload_offset;
  sec->payload_offset += delta;
  sec->payload_size -= delta;
  return WASM_OK;
}

static WasmResult on_memory(uint32_t index,
                            const WasmLimits* page_limits,
                            void* user_data) {
  Context* ctx = user_data;
  Section* sec = ctx->current_section;
  sec->memory_limits = *page_limits;
  ctx->binary->memory_page_count = page_limits->initial;
  return WASM_OK;
}

static WasmResult begin_data_segment(uint32_t index,
                                     uint32_t memory_index,
                                     void* user_data) {
  Context* ctx = user_data;
  Section* sec = ctx->current_section;
  DataSegment* segment =
      wasm_append_data_segment(ctx->allocator, &sec->data_segments);
  segment->memory_index = memory_index;
  return WASM_OK;
}

static WasmResult on_init_expr_i32_const_expr(uint32_t index,
                                              uint32_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  Section* sec = ctx->current_section;
  if (sec->section_code != WASM_BINARY_SECTION_DATA)
    return WASM_OK;
  DataSegment* segment = &sec->data_segments.data[sec->data_segments.size - 1];
  segment->offset = value;
  return WASM_OK;
}

static WasmResult on_data_segment_data(uint32_t index,
                                       const void* src_data,
                                       uint32_t size,
                                       void* user_data) {
  Context* ctx = user_data;
  Section* sec = ctx->current_section;
  DataSegment* segment = &sec->data_segments.data[sec->data_segments.size - 1];
  segment->data = src_data;
  segment->size = size;
  return WASM_OK;
}

static WasmResult on_export(uint32_t index,
                            WasmExternalKind kind,
                            uint32_t item_index,
                            WasmStringSlice name,
                            void* user_data) {
  Context* ctx = user_data;
  Export* export = wasm_append_export(ctx->allocator, &ctx->binary->exports);
  export->name = name;
  export->kind = kind;
  export->index = item_index;
  return WASM_OK;
}

static WasmBinaryReader s_binary_reader = {
    .begin_section = begin_section,
    .begin_custom_section = begin_custom_section,

    .on_reloc_count = on_reloc_count,
    .on_reloc = on_reloc,

    .on_import = on_import,
    .on_import_func = on_import_func,
    .on_import_global = on_import_global,

    .on_export = on_export,

    .on_table = on_table,

    .on_memory = on_memory,

    .begin_data_segment = begin_data_segment,
    .on_init_expr_i32_const_expr = on_init_expr_i32_const_expr,
    .on_data_segment_data = on_data_segment_data,

    .on_elem_segment_function_index_count =
        on_elem_segment_function_index_count,
};

WasmResult wasm_read_binary_linker(struct WasmAllocator* allocator,
                                   InputBinary* input_info) {
  Context context;
  WASM_ZERO_MEMORY(context);
  context.allocator = allocator;
  context.binary = input_info;

  WasmBinaryReader reader;
  WASM_ZERO_MEMORY(reader);
  reader = s_binary_reader;
  reader.user_data = &context;

  WasmReadBinaryOptions read_options = WASM_READ_BINARY_OPTIONS_DEFAULT;
  return wasm_read_binary(allocator, input_info->data, input_info->size,
                          &reader, 1, &read_options);
}

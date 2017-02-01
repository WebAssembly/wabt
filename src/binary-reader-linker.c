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
  WabtAllocator* allocator;
  WabtLinkerInputBinary* binary;

  WabtSection* reloc_section;

  WabtStringSlice import_name;
  WabtSection* current_section;
} Context;

static WabtResult on_reloc_count(uint32_t count,
                                 WabtBinarySection section_code,
                                 WabtStringSlice section_name,
                                 void* user_data) {
  Context* ctx = user_data;
  WabtLinkerInputBinary* binary = ctx->binary;
  if (section_code == WABT_BINARY_SECTION_CUSTOM) {
    WABT_FATAL("relocation for custom sections not yet supported\n");
  }

  uint32_t i;
  for (i = 0; i < binary->sections.size; i++) {
    WabtSection* sec = &binary->sections.data[i];
    if (sec->section_code != section_code)
      continue;
    ctx->reloc_section = sec;
    return WABT_OK;
  }

  WABT_FATAL("section not found: %d\n", section_code);
  return WABT_ERROR;
}

static WabtResult on_reloc(WabtRelocType type,
                           uint32_t offset,
                           void* user_data) {
  Context* ctx = user_data;

  if (offset + RELOC_SIZE > ctx->reloc_section->size) {
    WABT_FATAL("invalid relocation offset: %#x\n", offset);
  }

  WabtReloc* reloc =
      wabt_append_reloc(ctx->allocator, &ctx->reloc_section->relocations);
  reloc->type = type;
  reloc->offset = offset;

  return WABT_OK;
}

static WabtResult on_import(uint32_t index,
                            WabtStringSlice module_name,
                            WabtStringSlice field_name,
                            void* user_data) {
  Context* ctx = user_data;
  if (!wabt_string_slice_eq_cstr(&module_name, WABT_LINK_MODULE_NAME)) {
    WABT_FATAL("unsupported import module: " PRIstringslice,
               WABT_PRINTF_STRING_SLICE_ARG(module_name));
  }
  ctx->import_name = field_name;
  return WABT_OK;
}

static WabtResult on_import_func(uint32_t import_index,
                                 uint32_t global_index,
                                 uint32_t sig_index,
                                 void* user_data) {
  Context* ctx = user_data;
  WabtFunctionImport* import = wabt_append_function_import(
      ctx->allocator, &ctx->binary->function_imports);
  import->name = ctx->import_name;
  import->sig_index = sig_index;
  import->active = WABT_TRUE;
  ctx->binary->active_function_imports++;
  return WABT_OK;
}

static WabtResult on_import_global(uint32_t import_index,
                                   uint32_t global_index,
                                   WabtType type,
                                   WabtBool mutable,
                                   void* user_data) {
  Context* ctx = user_data;
  WabtGlobalImport* import =
      wabt_append_global_import(ctx->allocator, &ctx->binary->global_imports);
  import->name = ctx->import_name;
  import->type = type;
  import->mutable = mutable;
  ctx->binary->active_global_imports++;
  return WABT_OK;
}

static WabtResult begin_section(WabtBinaryReaderContext* ctx,
                                WabtBinarySection section_code,
                                uint32_t size) {
  Context* context = ctx->user_data;
  WabtLinkerInputBinary* binary = context->binary;
  WabtSection* sec = wabt_append_section(context->allocator, &binary->sections);
  context->current_section = sec;
  sec->section_code = section_code;
  sec->size = size;
  sec->offset = ctx->offset;
  sec->binary = binary;

  if (sec->section_code != WABT_BINARY_SECTION_CUSTOM &&
      sec->section_code != WABT_BINARY_SECTION_START) {
    size_t bytes_read = wabt_read_u32_leb128(
        &binary->data[sec->offset], &binary->data[binary->size], &sec->count);
    if (bytes_read == 0)
      WABT_FATAL("error reading section element count\n");
    sec->payload_offset = sec->offset + bytes_read;
    sec->payload_size = sec->size - bytes_read;
  }
  return WABT_OK;
}

static WabtResult begin_custom_section(WabtBinaryReaderContext* ctx,
                                       uint32_t size,
                                       WabtStringSlice section_name) {
  Context* context = ctx->user_data;
  WabtLinkerInputBinary* binary = context->binary;
  WabtSection* sec = context->current_section;
  sec->data_custom.name = section_name;

  /* Modify section size and offset to not include the name itself. */
  size_t delta = ctx->offset - sec->offset;
  sec->offset = sec->offset + delta;
  sec->size = sec->size - delta;
  sec->payload_offset = sec->offset;
  sec->payload_size = sec->size;

  /* Special handling for certain CUSTOM sections */
  if (wabt_string_slice_eq_cstr(&section_name, "name")) {
    size_t bytes_read = wabt_read_u32_leb128(
        &binary->data[sec->offset], &binary->data[binary->size], &sec->count);
    sec->payload_offset += bytes_read;
    sec->payload_size -= bytes_read;

    /* We don't currently support merging name sections unless they contain
     * a name for every function. */
    size_t i;
    uint32_t total_funcs = binary->function_imports.size;
    for (i = 0; i < binary->sections.size; i++) {
      if (binary->sections.data[i].section_code ==
          WABT_BINARY_SECTION_FUNCTION) {
        total_funcs += binary->sections.data[i].count;
        break;
      }
    }
    if (total_funcs != sec->count) {
      WABT_FATAL("name section count (%d) does not match function count (%d)\n",
                 sec->count, total_funcs);
    }
  }

  return WABT_OK;
}

static WabtResult on_table(uint32_t index,
                           WabtType elem_type,
                           const WabtLimits* elem_limits,
                           void* user_data) {
  if (elem_limits->has_max && (elem_limits->max != elem_limits->initial))
    WABT_FATAL("Tables with max != initial not supported by wabt-link\n");

  Context* ctx = user_data;
  ctx->binary->table_elem_count = elem_limits->initial;
  return WABT_OK;
}

static WabtResult on_elem_segment_function_index_count(
    WabtBinaryReaderContext* ctx,
    uint32_t index,
    uint32_t count) {
  Context* context = ctx->user_data;
  WabtSection* sec = context->current_section;

  /* Modify the payload to include only the actual function indexes */
  size_t delta = ctx->offset - sec->payload_offset;
  sec->payload_offset += delta;
  sec->payload_size -= delta;
  return WABT_OK;
}

static WabtResult on_memory(uint32_t index,
                            const WabtLimits* page_limits,
                            void* user_data) {
  Context* ctx = user_data;
  WabtSection* sec = ctx->current_section;
  sec->memory_limits = *page_limits;
  ctx->binary->memory_page_count = page_limits->initial;
  return WABT_OK;
}

static WabtResult begin_data_segment(uint32_t index,
                                     uint32_t memory_index,
                                     void* user_data) {
  Context* ctx = user_data;
  WabtSection* sec = ctx->current_section;
  WabtDataSegment* segment =
      wabt_append_data_segment(ctx->allocator, &sec->data_segments);
  segment->memory_index = memory_index;
  return WABT_OK;
}

static WabtResult on_init_expr_i32_const_expr(uint32_t index,
                                              uint32_t value,
                                              void* user_data) {
  Context* ctx = user_data;
  WabtSection* sec = ctx->current_section;
  if (sec->section_code != WABT_BINARY_SECTION_DATA)
    return WABT_OK;
  WabtDataSegment* segment =
      &sec->data_segments.data[sec->data_segments.size - 1];
  segment->offset = value;
  return WABT_OK;
}

static WabtResult on_data_segment_data(uint32_t index,
                                       const void* src_data,
                                       uint32_t size,
                                       void* user_data) {
  Context* ctx = user_data;
  WabtSection* sec = ctx->current_section;
  WabtDataSegment* segment =
      &sec->data_segments.data[sec->data_segments.size - 1];
  segment->data = src_data;
  segment->size = size;
  return WABT_OK;
}

static WabtResult on_export(uint32_t index,
                            WabtExternalKind kind,
                            uint32_t item_index,
                            WabtStringSlice name,
                            void* user_data) {
  Context* ctx = user_data;
  WabtExport* export =
      wabt_append_export(ctx->allocator, &ctx->binary->exports);
  export->name = name;
  export->kind = kind;
  export->index = item_index;
  return WABT_OK;
}

static WabtResult on_function_name(uint32_t index,
                                   WabtStringSlice name,
                                   void* user_data) {
  Context* ctx = user_data;
  wabt_append_string_slice_value(ctx->allocator, &ctx->binary->debug_names,
                                 &name);
  return WABT_OK;
}

static WabtBinaryReader s_binary_reader = {
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

    .on_function_name = on_function_name,
};

WabtResult wabt_read_binary_linker(struct WabtAllocator* allocator,
                                   WabtLinkerInputBinary* input_info) {
  Context context;
  WABT_ZERO_MEMORY(context);
  context.allocator = allocator;
  context.binary = input_info;

  WabtBinaryReader reader;
  WABT_ZERO_MEMORY(reader);
  reader = s_binary_reader;
  reader.user_data = &context;

  WabtReadBinaryOptions read_options = WABT_READ_BINARY_OPTIONS_DEFAULT;
  read_options.read_debug_names = WABT_TRUE;
  return wabt_read_binary(allocator, input_info->data, input_info->size,
                          &reader, 1, &read_options);
}

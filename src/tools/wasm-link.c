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

#include "allocator.h"
#include "binary-reader.h"
#include "binary-writer.h"
#include "option-parser.h"
#include "stream.h"
#include "vector.h"
#include "writer.h"

#define PROGRAM_NAME "wasm-link"
#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT
#define RELOC_SIZE 5
#define FIRST_KNOWN_SECTION WASM_BINARY_SECTION_TYPE

enum { FLAG_VERBOSE, FLAG_OUTPUT, FLAG_HELP, NUM_FLAGS };

static const char s_description[] =
    "  link one or more wasm binary modules into a single binary module."
    "\n"
    "  $ wasm-link m1.wasm m2.wasm -o out.wasm\n";

static WasmOption s_options[] = {
    {FLAG_VERBOSE, 'v', "verbose", NULL, NOPE,
     "use multiple times for more info"},
    {FLAG_OUTPUT, 'o', "output", "FILE", YEP, "output wasm binary file"},
    {FLAG_HELP, 'h', "help", NULL, NOPE, "print this help message"},
};
WASM_STATIC_ASSERT(NUM_FLAGS == WASM_ARRAY_SIZE(s_options));

typedef const char* String;
WASM_DEFINE_VECTOR(string, String);

static WasmBool s_verbose;
static const char* s_outfile = "a.wasm";
static StringVector s_infiles;
static WasmAllocator* s_allocator;
static WasmFileWriter s_log_stream_writer;
static WasmStream s_log_stream;

static void on_option(struct WasmOptionParser* parser,
                      struct WasmOption* option,
                      const char* argument) {
  switch (option->id) {
    case FLAG_VERBOSE:
      s_verbose++;
      wasm_init_file_writer_existing(&s_log_stream_writer, stdout);
      wasm_init_stream(&s_log_stream, &s_log_stream_writer.base, NULL);
      break;

    case FLAG_OUTPUT:
      s_outfile = argument;
      break;

    case FLAG_HELP:
      wasm_print_help(parser, PROGRAM_NAME);
      exit(0);
      break;
  }
}

static void on_argument(struct WasmOptionParser* parser, const char* argument) {
  wasm_append_string_value(s_allocator, &s_infiles, &argument);
}

static void on_option_error(struct WasmOptionParser* parser,
                            const char* message) {
  WASM_FATAL("%s\n", message);
}

static void parse_options(int argc, char** argv) {
  WasmOptionParser parser;
  WASM_ZERO_MEMORY(parser);
  parser.description = s_description;
  parser.options = s_options;
  parser.num_options = WASM_ARRAY_SIZE(s_options);
  parser.on_option = on_option;
  parser.on_argument = on_argument;
  parser.on_error = on_option_error;
  wasm_parse_options(&parser, argc, argv);

  if (!s_infiles.size) {
    wasm_print_help(&parser, PROGRAM_NAME);
    WASM_FATAL("No inputs files specified.\n");
  }
}

typedef struct DataSegment {
  uint32_t memory_index;
  uint32_t offset;
  const uint8_t* data;
  size_t size;
} DataSegment;
WASM_DEFINE_VECTOR(data_segment, DataSegment);

typedef struct Reloc {
  WasmRelocType type;
  size_t offset;
} Reloc;
WASM_DEFINE_VECTOR(reloc, Reloc);

struct InputBinary;

typedef struct SectionDataCustom {
  WasmBool linked;
  /* Reference to string data stored in the containing InputBinary */
  WasmStringSlice name;
} SectionDataCustom;

typedef struct Section {
  struct InputBinary* binary; /* The binary to which this section belongs */
  RelocVector relocations;    /* The relocations for this section */

  WasmBinarySection section_code;
  size_t size;
  size_t offset;

  size_t payload_size;
  size_t payload_offset;

  /* For known sections, the count of the number of elements in the section */
  uint32_t count;

  union {
    /* CUSTOM section data */
    SectionDataCustom data_custom;
    /* DATA section data */
    DataSegmentVector data_segments;
    /* MEMORY section data */
    WasmLimits memory_limits;
  };

  /* The offset at which this section appears within the combined output
   * section. */
  size_t output_payload_offset;
} Section;
WASM_DEFINE_VECTOR(section, Section);

typedef Section* SectionPtr;
WASM_DEFINE_VECTOR(section_ptr, SectionPtr);

typedef struct InputBinary {
  uint8_t* data;
  size_t size;
  SectionVector sections;
  Section* current_section;

  uint32_t type_index_offset;
  uint32_t function_index_offset;
  uint32_t imported_function_index_offset;
  uint32_t global_index_offset;
  uint32_t imported_global_index_offset;
  uint32_t indirect_function_index_offset;
  uint32_t memory_page_count;
  uint32_t memory_page_offset;

  uint32_t num_func_imports;
  uint32_t num_global_imports;
  uint32_t table_elem_count;
} InputBinary;
WASM_DEFINE_VECTOR(binary, InputBinary);

typedef struct Context {
  WasmStream stream;
  InputBinaryVector inputs;
  uint32_t total_function_imports;
  uint32_t total_global_imports;
  ssize_t current_section_payload_offset;
} Context;

void wasm_destroy_section(WasmAllocator* allocator, Section* section) {
  wasm_destroy_reloc_vector(allocator, &section->relocations);
  switch (section->section_code) {
    case WASM_BINARY_SECTION_DATA:
      wasm_destroy_data_segment_vector(allocator, &section->data_segments);
      break;
    default:
      break;
  }
}

void wasm_destroy_binary(WasmAllocator* allocator, InputBinary* binary) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, binary->sections, section);
  wasm_free(allocator, binary->data);
}

static WasmResult on_reloc(WasmRelocType type,
                           uint32_t section_index,
                           uint32_t offset,
                           void* user_data) {
  InputBinary* binary = user_data;
  if (section_index >= binary->sections.size) {
    WASM_FATAL("invalid section index: %d\n", section_index);
  }
  Section* sec = &binary->sections.data[section_index - 1];
  if (offset + RELOC_SIZE > sec->size) {
    WASM_FATAL("invalid relocation offset: %#x\n", offset);
  }

  Reloc* reloc = wasm_append_reloc(s_allocator, &sec->relocations);
  reloc->type = type;
  reloc->offset = offset;

  return WASM_OK;
}

static WasmResult on_import_func(uint32_t index,
                                 uint32_t sig_index,
                                 void* user_data) {
  InputBinary* binary = user_data;
  binary->num_func_imports++;
  return WASM_OK;
}

static WasmResult on_import_global(uint32_t index,
                                   WasmType type,
                                   WasmBool mutable,
                                   void* user_data) {
  InputBinary* binary = user_data;
  binary->num_global_imports++;
  return WASM_OK;
}

static WasmResult begin_section(WasmBinaryReaderContext* ctx,
                                WasmBinarySection section_code,
                                uint32_t size) {
  InputBinary* binary = ctx->user_data;
  Section* sec = wasm_append_section(s_allocator, &binary->sections);
  binary->current_section = sec;
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
  InputBinary* binary = ctx->user_data;
  Section* sec = binary->current_section;
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
    for (i = 0; i < binary->sections.size; i++) {
      if (binary->sections.data[i].section_code ==
          WASM_BINARY_SECTION_FUNCTION) {
        if (binary->sections.data[i].count != sec->count) {
          WASM_FATAL(
              "name section count (%d) does not match function count (%d)\n",
              sec->count, binary->sections.data[i].count);
        }
      }
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

  InputBinary* binary = user_data;
  binary->table_elem_count = elem_limits->initial;
  return WASM_OK;
}

static WasmResult on_elem_segment_function_index_count(
    WasmBinaryReaderContext* ctx,
    uint32_t index,
    uint32_t count) {
  InputBinary* binary = ctx->user_data;
  Section* sec = binary->current_section;

  /* Modify the payload to include only the actual function indexes */
  size_t delta = ctx->offset - sec->payload_offset;
  sec->payload_offset += delta;
  sec->payload_size -= delta;
  return WASM_OK;
}

static WasmResult on_memory(uint32_t index,
                            const WasmLimits* page_limits,
                            void* user_data) {
  InputBinary* binary = user_data;
  Section* sec = binary->current_section;
  sec->memory_limits = *page_limits;
  binary->memory_page_count = page_limits->initial;
  return WASM_OK;
}

static WasmResult begin_data_segment(uint32_t index,
                                     uint32_t memory_index,
                                     void* user_data) {
  InputBinary* binary = user_data;
  Section* sec = binary->current_section;
  DataSegment* segment =
      wasm_append_data_segment(s_allocator, &sec->data_segments);
  segment->memory_index = memory_index;
  return WASM_OK;
}

static WasmResult on_init_expr_i32_const_expr(uint32_t index,
                                              uint32_t value,
                                              void* user_data) {
  InputBinary* binary = user_data;
  Section* sec = binary->current_section;
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
  InputBinary* binary = user_data;
  Section* sec = binary->current_section;
  DataSegment* segment = &sec->data_segments.data[sec->data_segments.size - 1];
  segment->data = src_data;
  segment->size = size;
  return WASM_OK;
}

static WasmResult read_binary(uint8_t* data,
                              size_t size,
                              InputBinary* input_info) {
  input_info->data = data;
  input_info->size = size;

  WasmBinaryReader reader;
  WASM_ZERO_MEMORY(reader);

  static WasmBinaryReader s_binary_reader = {
      .begin_section = begin_section,
      .begin_custom_section = begin_custom_section,

      .on_reloc = on_reloc,

      .on_import_func = on_import_func,
      .on_import_global = on_import_global,

      .on_table = on_table,

      .on_memory = on_memory,

      .begin_data_segment = begin_data_segment,
      .on_init_expr_i32_const_expr = on_init_expr_i32_const_expr,
      .on_data_segment_data = on_data_segment_data,

      .on_elem_segment_function_index_count =
          on_elem_segment_function_index_count,
  };

  reader = s_binary_reader;
  reader.user_data = input_info;

  WasmReadBinaryOptions read_options = WASM_READ_BINARY_OPTIONS_DEFAULT;
  return wasm_read_binary(s_allocator, data, size, &reader, 1, &read_options);
}

static void apply_relocation(Section* section, Reloc* r) {
  InputBinary* binary = section->binary;
  uint8_t* section_data = &binary->data[section->offset];
  size_t section_size = section->size;

  uint32_t cur_value = 0;
  wasm_read_u32_leb128(section_data + r->offset, section_data + section_size,
                       &cur_value);

  uint32_t offset = 0;
  switch (r->type) {
    case WASM_RELOC_FUNC_INDEX:
      if (cur_value >= binary->num_func_imports) {
        offset = binary->function_index_offset;
      } else {
        offset = binary->imported_function_index_offset;
      }
      break;
    case WASM_RELOC_GLOBAL_INDEX:
      if (cur_value >= binary->num_global_imports) {
        offset = binary->global_index_offset;
      } else {
        offset = binary->imported_global_index_offset;
      }
      break;
    case WASM_RELOC_TYPE_INDEX:
      offset = binary->type_index_offset;
      break;
    default:
      WASM_FATAL("unhandled relocation type: %d\n", r->type);
      break;
  }

  cur_value += offset;
  wasm_write_fixed_u32_leb128_raw(section_data + r->offset,
                                  section_data + section_size, cur_value);
}

static void apply_relocations(Section* section) {
  if (!section->relocations.size)
    return;

  /* Perform relocations in-place */
  size_t i;
  for (i = 0; i < section->relocations.size; i++) {
    Reloc* reloc = &section->relocations.data[i];
    apply_relocation(section, reloc);
  }
}

static void write_section_payload(Context* ctx, Section* sec) {
  assert(ctx->current_section_payload_offset != -1);

  sec->output_payload_offset =
      ctx->stream.offset - ctx->current_section_payload_offset;

  uint8_t* payload = &sec->binary->data[sec->payload_offset];
  wasm_write_data(&ctx->stream, payload, sec->payload_size, "section content");
}

#define WRITE_UNKNOWN_SIZE                                  \
  uint32_t fixup_offset = stream->offset;                   \
  wasm_write_fixed_u32_leb128(stream, 0, "unknown size");   \
  ctx->current_section_payload_offset = ctx->stream.offset; \
  uint32_t start = stream->offset;

#define FIXUP_SIZE                                                             \
  wasm_write_fixed_u32_leb128_at(stream, fixup_offset, stream->offset - start, \
                                 "fixup size");

static void write_combined_custom_section(Context* ctx,
                                          const SectionPtrVector* sections,
                                          const WasmStringSlice* name) {
  /* Write this section, along with all the following sections with the
   * same name. */
  size_t i;
  size_t total_size = 0;
  size_t total_count = 0;

  /* Reloc sections are handled seperatedly. */
  if (wasm_string_slice_startswith(name, "reloc"))
    return;

  if (!wasm_string_slice_eq_cstr(name, "name")) {
    WASM_FATAL("Don't know how to link custom section: " PRIstringslice "\n",
               WASM_PRINTF_STRING_SLICE_ARG(*name));
  }

  /* First pass to calculate total size and count */
  for (i = 0; i < sections->size; i++) {
    Section* sec = sections->data[i];
    if (!wasm_string_slices_are_equal(name, &sec->data_custom.name))
      continue;
    total_size += sec->payload_size;
    total_count += sec->count;
  }

  WasmStream* stream = &ctx->stream;
  wasm_write_u8(stream, WASM_BINARY_SECTION_CUSTOM, "section code");
  WRITE_UNKNOWN_SIZE;
  wasm_write_str(stream, name->start, name->length, WASM_PRINT_CHARS,
                 "custom section name");
  wasm_write_u32_leb128(stream, total_count, "element count");

  for (i = 0; i < sections->size; i++) {
    Section* sec = sections->data[i];
    if (!wasm_string_slices_are_equal(name, &sec->data_custom.name))
      continue;
    apply_relocations(sec);
    write_section_payload(ctx, sec);
    sec->data_custom.linked = WASM_TRUE;
  }

  FIXUP_SIZE;
}

static void write_combined_table_section(Context* ctx,
                                         const SectionPtrVector* sections) {
  /* Total section size includes the element count leb128 which is
   * always 1 in the current spec */
  uint32_t table_count = 1;
  uint32_t flags = WASM_BINARY_LIMITS_HAS_MAX_FLAG;
  uint32_t elem_count = 0;

  size_t i;
  for (i = 0; i < sections->size; i++) {
    Section* sec = sections->data[i];
    elem_count += sec->binary->table_elem_count;
  }

  WasmStream* stream = &ctx->stream;
  WRITE_UNKNOWN_SIZE;
  wasm_write_u32_leb128(stream, table_count, "table count");
  wasm_write_type(stream, WASM_TYPE_ANYFUNC);
  wasm_write_u32_leb128(stream, flags, "table elem flags");
  wasm_write_u32_leb128(stream, elem_count, "table initial length");
  wasm_write_u32_leb128(stream, elem_count, "table max length");
  FIXUP_SIZE;
}

static void write_combined_elem_section(Context* ctx,
                                        const SectionPtrVector* sections) {
  WasmStream* stream = &ctx->stream;
  WRITE_UNKNOWN_SIZE;

  size_t i;
  uint32_t total_elem_count = 0;
  for (i = 0; i < sections->size; i++) {
    Section* sec = sections->data[i];
    total_elem_count += sec->binary->table_elem_count;
  }

  wasm_write_u32_leb128(stream, 1, "segment count");
  wasm_write_u32_leb128(stream, 0, "table index");
  wasm_write_opcode(&ctx->stream, WASM_OPCODE_I32_CONST);
  wasm_write_i32_leb128(&ctx->stream, 0, "elem init literal");
  wasm_write_opcode(&ctx->stream, WASM_OPCODE_END);
  wasm_write_u32_leb128(stream, total_elem_count, "num elements");

  ctx->current_section_payload_offset = stream->offset;

  for (i = 0; i < sections->size; i++) {
    Section* sec = sections->data[i];
    apply_relocations(sec);
    write_section_payload(ctx, sec);
  }

  FIXUP_SIZE;
}

static void write_combined_memory_section(Context* ctx,
                                          const SectionPtrVector* sections) {
  WasmStream* stream = &ctx->stream;
  WRITE_UNKNOWN_SIZE;

  wasm_write_u32_leb128(stream, 1, "memory count");

  WasmLimits limits;
  WASM_ZERO_MEMORY(limits);
  limits.has_max = WASM_TRUE;
  size_t i;
  for (i = 0; i < sections->size; i++) {
    SectionPtr sec = sections->data[i];
    limits.initial += sec->memory_limits.initial;
  }
  limits.max = limits.initial;
  wasm_write_limits(stream, &limits);

  FIXUP_SIZE;
}

static void write_combined_function_section(Context* ctx,
                                            const SectionPtrVector* sections,
                                            uint32_t total_count) {
  WasmStream* stream = &ctx->stream;
  WRITE_UNKNOWN_SIZE;

  wasm_write_u32_leb128(stream, total_count, "function count");

  size_t i;
  for (i = 0; i < sections->size; i++) {
    SectionPtr sec = sections->data[i];
    uint32_t count = sec->count;
    uint32_t input_offset = 0;
    uint32_t sig_index = 0;
    const uint8_t* start = &sec->binary->data[sec->payload_offset];
    const uint8_t* end =
        &sec->binary->data[sec->payload_offset + sec->payload_size];
    while (count--) {
      input_offset +=
          wasm_read_u32_leb128(start + input_offset, end, &sig_index);
      sig_index += sec->binary->type_index_offset;
      wasm_write_u32_leb128(stream, sig_index, "sig");
    }
  }

  FIXUP_SIZE;
}

static void write_data_segment(WasmStream* stream,
                               DataSegment* segment,
                               uint32_t offset) {
  assert(segment->memory_index == 0);
  wasm_write_u32_leb128(stream, segment->memory_index, "memory index");
  wasm_write_opcode(stream, WASM_OPCODE_I32_CONST);
  wasm_write_u32_leb128(stream, segment->offset + offset, "offset");
  wasm_write_opcode(stream, WASM_OPCODE_END);
  wasm_write_u32_leb128(stream, segment->size, "segment size");
  wasm_write_data(stream, segment->data, segment->size, "segment data");
}

static void write_combined_data_section(Context* ctx,
                                        SectionPtrVector* sections,
                                        uint32_t total_count) {
  WasmStream* stream = &ctx->stream;
  WRITE_UNKNOWN_SIZE;

  wasm_write_u32_leb128(stream, total_count, "data segment count");
  size_t i, j;
  for (i = 0; i < sections->size; i++) {
    Section* sec = sections->data[i];
    for (j = 0; j < sec->data_segments.size; j++) {
      DataSegment* segment = &sec->data_segments.data[j];
      write_data_segment(stream, segment,
                         sec->binary->memory_page_offset * WASM_PAGE_SIZE);
    }
  }

  FIXUP_SIZE;
}

static void write_combined_reloc_section(Context* ctx,
                                         WasmBinarySection section_code,
                                         SectionPtrVector* sections,
                                         uint32_t section_index) {
  size_t i, j;
  uint32_t total_relocs = 0;

  /* First pass to know total reloc count */
  for (i = 0; i < sections->size; i++) {
    Section* sec = sections->data[i];
    total_relocs += sec->relocations.size;
  }

  if (!total_relocs)
    return;

  char section_name[128];
  wasm_snprintf(section_name, sizeof(section_name), "%s.%s",
                WASM_BINARY_SECTION_RELOC, wasm_get_section_name(section_code));

  WasmStream* stream = &ctx->stream;
  wasm_write_u8(stream, WASM_BINARY_SECTION_CUSTOM, "section code");
  WRITE_UNKNOWN_SIZE;
  wasm_write_str(stream, section_name, strlen(section_name), WASM_PRINT_CHARS,
                 "reloc section name");
  wasm_write_u32_leb128(&ctx->stream, section_index, "reloc section");
  wasm_write_u32_leb128(&ctx->stream, total_relocs, "num relocs");

  for (i = 0; i < sections->size; i++) {
    Section* sec = sections->data[i];
    RelocVector* relocs = &sec->relocations;
    for (j = 0; j < relocs->size; j++) {
      wasm_write_u32_leb128(&ctx->stream, relocs->data[j].type, "reloc type");
      uint32_t new_offset = relocs->data[j].offset + sec->output_payload_offset;
      wasm_write_u32_leb128(&ctx->stream, new_offset, "reloc offset");
    }
  }

  FIXUP_SIZE;
}

static WasmBool write_combined_section(Context* ctx,
                                       WasmBinarySection section_code,
                                       SectionPtrVector* sections) {
  if (!sections->size)
    return WASM_FALSE;

  if (section_code == WASM_BINARY_SECTION_START && sections->size > 1) {
    WASM_FATAL("Don't know how to combine sections of type: %s\n",
               wasm_get_section_name(section_code));
  }

  size_t i;
  uint32_t total_count = 0;
  uint32_t total_size = 0;

  /* Sum section size and element count */
  for (i = 0; i < sections->size; i++) {
    Section* sec = sections->data[i];
    total_size += sec->payload_size;
    total_count += sec->count;
  }

  wasm_write_u8(&ctx->stream, section_code, "section code");
  ctx->current_section_payload_offset = -1;

  switch (section_code) {
    case WASM_BINARY_SECTION_FUNCTION:
      write_combined_function_section(ctx, sections, total_count);
      break;
    case WASM_BINARY_SECTION_TABLE:
      write_combined_table_section(ctx, sections);
      break;
    case WASM_BINARY_SECTION_ELEM:
      write_combined_elem_section(ctx, sections);
      break;
    case WASM_BINARY_SECTION_MEMORY:
      write_combined_memory_section(ctx, sections);
      break;
    case WASM_BINARY_SECTION_DATA:
      write_combined_data_section(ctx, sections, total_count);
      break;
    default: {
      /* Total section size includes the element count leb128. */
      total_size += wasm_u32_leb128_length(total_count);

      /* Write section to stream */
      WasmStream* stream = &ctx->stream;
      wasm_write_u32_leb128(stream, total_size, "section size");
      wasm_write_u32_leb128(stream, total_count, "element count");
      ctx->current_section_payload_offset = ctx->stream.offset;
      for (i = 0; i < sections->size; i++) {
        Section* sec = sections->data[i];
        apply_relocations(sec);
        write_section_payload(ctx, sec);
      }
    }
  }

  return WASM_TRUE;
}

static void write_binary(Context* ctx) {
  /* Find all the sections of each type */
  SectionPtrVector sections[WASM_NUM_BINARY_SECTIONS];
  WASM_ZERO_MEMORY(sections);
  uint32_t section_indices[WASM_NUM_BINARY_SECTIONS];
  WASM_ZERO_MEMORY(section_indices);
  uint32_t section_index = 1;

  size_t i;
  size_t j;
  uint32_t memory_page_offset = 0;
  for (j = 0; j < ctx->inputs.size; j++) {
    InputBinary* binary = &ctx->inputs.data[j];
    /* The imported_function_index_offset is the sum of all the function
     * imports from objects that precede this one.  i.e. the current running
     * total */
    binary->imported_function_index_offset = ctx->total_function_imports;
    binary->imported_global_index_offset = ctx->total_global_imports;
    binary->memory_page_offset = memory_page_offset;
    memory_page_offset += binary->memory_page_count;
    ctx->total_function_imports += binary->num_func_imports;
    ctx->total_global_imports += binary->num_global_imports;
    for (i = 0; i < binary->sections.size; i++) {
      Section* s = &binary->sections.data[i];
      SectionPtrVector* sec_list = &sections[s->section_code];
      wasm_append_section_ptr_value(s_allocator, sec_list, &s);
    }
  }

  /* Calculate offsets needed for relocation */
  uint32_t total_count = 0;
  for (i = 0; i < sections[WASM_BINARY_SECTION_TYPE].size; i++) {
    Section* sec = sections[WASM_BINARY_SECTION_TYPE].data[i];
    sec->binary->type_index_offset = total_count;
    total_count += sec->count;
  }

  total_count = 0;
  for (i = 0; i < sections[WASM_BINARY_SECTION_GLOBAL].size; i++) {
    Section* sec = sections[WASM_BINARY_SECTION_GLOBAL].data[i];
    sec->binary->global_index_offset = ctx->total_global_imports -
                                       sec->binary->num_global_imports +
                                       total_count;
    total_count += sec->count;
  }

  total_count = 0;
  for (i = 0; i < sections[WASM_BINARY_SECTION_FUNCTION].size; i++) {
    Section* sec = sections[WASM_BINARY_SECTION_FUNCTION].data[i];
    sec->binary->function_index_offset = ctx->total_function_imports -
                                         sec->binary->num_func_imports +
                                         total_count;
    total_count += sec->count;
  }

  /* Write the final binary */
  wasm_write_u32(&ctx->stream, WASM_BINARY_MAGIC, "WASM_BINARY_MAGIC");
  wasm_write_u32(&ctx->stream, WASM_BINARY_VERSION, "WASM_BINARY_VERSION");

  /* Write known sections first */
  for (i = FIRST_KNOWN_SECTION; i < WASM_NUM_BINARY_SECTIONS; i++) {
    if (write_combined_section(ctx, i, &sections[i]))
      section_indices[i] = section_index++;
  }

  /* Write custom sections */
  SectionPtrVector* custom_sections = &sections[WASM_BINARY_SECTION_CUSTOM];
  for (i = 0; i < custom_sections->size; i++) {
    Section* section = custom_sections->data[i];
    if (section->data_custom.linked)
      continue;
    write_combined_custom_section(ctx, custom_sections,
                                  &section->data_custom.name);
  }

  /* Generate a new set of reloction sections */
  for (i = FIRST_KNOWN_SECTION; i < WASM_NUM_BINARY_SECTIONS; i++) {
    write_combined_reloc_section(ctx, i, &sections[i], section_indices[i]);
  }

  for (i = 0; i < WASM_NUM_BINARY_SECTIONS; i++) {
    wasm_destroy_section_ptr_vector(s_allocator, &sections[i]);
  }
}

static WasmResult perform_link(Context* ctx) {
  WasmMemoryWriter writer;
  WASM_ZERO_MEMORY(writer);
  if (WASM_FAILED(wasm_init_mem_writer(s_allocator, &writer)))
    WASM_FATAL("unable to open memory writer for writing\n");

  WasmStream* log_stream = NULL;
  if (s_verbose)
    log_stream = &s_log_stream;

  if (s_verbose)
    wasm_writef(&s_log_stream, "writing file: %s\n", s_outfile);

  wasm_init_stream(&ctx->stream, &writer.base, log_stream);
  write_binary(ctx);

  if (WASM_FAILED(wasm_write_output_buffer_to_file(&writer.buf, s_outfile)))
    WASM_FATAL("error writing linked output to file\n");

  wasm_close_mem_writer(&writer);
  return WASM_OK;
}

int main(int argc, char** argv) {
  s_allocator = &g_wasm_libc_allocator;
  wasm_init_stdio();
  parse_options(argc, argv);

  Context context;
  WASM_ZERO_MEMORY(context);

  WasmResult result = WASM_OK;
  size_t i;
  for (i = 0; i < s_infiles.size; i++) {
    const char* input_filename = s_infiles.data[i];
    if (s_verbose)
      wasm_writef(&s_log_stream, "reading file: %s\n", input_filename);
    void* data;
    size_t size;
    result = wasm_read_file(s_allocator, input_filename, &data, &size);
    if (WASM_FAILED(result))
      return result;
    InputBinary* b = wasm_append_binary(s_allocator, &context.inputs);
    result = read_binary(data, size, b);
    if (WASM_FAILED(result))
      WASM_FATAL("error parsing file: %s\n", input_filename);
  }

  result = perform_link(&context);
  if (WASM_FAILED(result))
    return result;

  /* Cleanup */
  WASM_DESTROY_VECTOR_AND_ELEMENTS(s_allocator, context.inputs, binary);
  wasm_destroy_string_vector(s_allocator, &s_infiles);

  wasm_print_allocator_stats(s_allocator);
  wasm_destroy_allocator(s_allocator);
  return result;
}

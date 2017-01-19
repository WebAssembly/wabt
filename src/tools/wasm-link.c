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

#include "wasm-link.h"

#include "allocator.h"
#include "binary-reader.h"
#include "binding-hash.h"
#include "binary-writer.h"
#include "option-parser.h"
#include "stream.h"
#include "vector.h"
#include "writer.h"
#include "binary-reader-linker.h"

#define PROGRAM_NAME "wasm-link"
#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT
#define FIRST_KNOWN_SECTION WASM_BINARY_SECTION_TYPE

enum { FLAG_VERBOSE, FLAG_OUTPUT, FLAG_RELOCATABLE, FLAG_HELP, NUM_FLAGS };

static const char s_description[] =
    "  link one or more wasm binary modules into a single binary module."
    "\n"
    "  $ wasm-link m1.wasm m2.wasm -o out.wasm\n";

static WasmOption s_options[] = {
    {FLAG_VERBOSE, 'v', "verbose", NULL, NOPE,
     "use multiple times for more info"},
    {FLAG_OUTPUT, 'o', "output", "FILE", YEP, "output wasm binary file"},
    {FLAG_RELOCATABLE, 'r', "relocatable", NULL, NOPE,
     "output a relocatable object file"},
    {FLAG_HELP, 'h', "help", NULL, NOPE, "print this help message"},
};
WASM_STATIC_ASSERT(NUM_FLAGS == WASM_ARRAY_SIZE(s_options));

typedef const char* String;
WASM_DEFINE_VECTOR(string, String);

static WasmBool s_verbose;
static WasmBool s_relocatable;
static const char* s_outfile = "a.wasm";
static StringVector s_infiles;
static WasmFileWriter s_log_stream_writer;
static WasmStream s_log_stream;

typedef struct Context {
  WasmStream stream;
  WasmLinkerInputBinaryVector inputs;
  ssize_t current_section_payload_offset;
  WasmAllocator* allocator;
} Context;

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

    case FLAG_RELOCATABLE:
      s_relocatable = WASM_TRUE;
      break;

    case FLAG_HELP:
      wasm_print_help(parser, PROGRAM_NAME);
      exit(0);
      break;
  }
}

static void on_argument(struct WasmOptionParser* parser, const char* argument) {
  WasmAllocator* allocator = parser->user_data;
  wasm_append_string_value(allocator, &s_infiles, &argument);
}

static void on_option_error(struct WasmOptionParser* parser,
                            const char* message) {
  WASM_FATAL("%s\n", message);
}

static void parse_options(WasmAllocator* allocator, int argc, char** argv) {
  WasmOptionParser parser;
  WASM_ZERO_MEMORY(parser);
  parser.description = s_description;
  parser.options = s_options;
  parser.num_options = WASM_ARRAY_SIZE(s_options);
  parser.on_option = on_option;
  parser.on_argument = on_argument;
  parser.on_error = on_option_error;
  parser.user_data = allocator;
  wasm_parse_options(&parser, argc, argv);

  if (!s_infiles.size) {
    wasm_print_help(&parser, PROGRAM_NAME);
    WASM_FATAL("No inputs files specified.\n");
  }
}

void wasm_destroy_section(WasmAllocator* allocator, WasmSection* section) {
  wasm_destroy_reloc_vector(allocator, &section->relocations);
  switch (section->section_code) {
    case WASM_BINARY_SECTION_DATA:
      wasm_destroy_data_segment_vector(allocator, &section->data_segments);
      break;
    default:
      break;
  }
}

void wasm_destroy_binary(WasmAllocator* allocator,
                         WasmLinkerInputBinary* binary) {
  WASM_DESTROY_VECTOR_AND_ELEMENTS(allocator, binary->sections, section);
  wasm_destroy_function_import_vector(allocator, &binary->function_imports);
  wasm_destroy_global_import_vector(allocator, &binary->global_imports);
  wasm_destroy_export_vector(allocator, &binary->exports);
  wasm_free(allocator, binary->data);
}

static void apply_relocation(WasmSection* section, WasmReloc* r) {
  WasmLinkerInputBinary* binary = section->binary;
  uint8_t* section_data = &binary->data[section->offset];
  size_t section_size = section->size;

  uint32_t cur_value = 0;
  wasm_read_u32_leb128(section_data + r->offset, section_data + section_size,
                       &cur_value);

  uint32_t offset = 0;
  switch (r->type) {
    case WASM_RELOC_FUNC_INDEX_LEB:
      if (cur_value >= binary->function_imports.size) {
        /* locally declared function call */
        offset = binary->function_index_offset;
        if (s_verbose)
          wasm_writef(&s_log_stream, "func reloc %d + %d\n", cur_value, offset);
      } else {
        /* imported function call */
        WasmFunctionImport* import = &binary->function_imports.data[cur_value];
        offset = binary->imported_function_index_offset;
        if (!import->active) {
          cur_value = import->foreign_index;
          offset = import->foreign_binary->function_index_offset;
          if (s_verbose)
            wasm_writef(&s_log_stream,
                        "reloc for disabled import. new index = %d + %d\n",
                        cur_value, offset);
        }
      }
      break;
    case WASM_RELOC_GLOBAL_INDEX_LEB:
      if (cur_value >= binary->global_imports.size) {
        offset = binary->global_index_offset;
      } else {
        offset = binary->imported_global_index_offset;
      }
      break;
    default:
      WASM_FATAL("unhandled relocation type: %d\n", r->type);
      break;
  }

  cur_value += offset;
  wasm_write_fixed_u32_leb128_raw(section_data + r->offset,
                                  section_data + section_size, cur_value);
}

static void apply_relocations(WasmSection* section) {
  if (!section->relocations.size)
    return;

  if (s_verbose)
    wasm_writef(&s_log_stream, "apply_relocations: %s\n",
                wasm_get_section_name(section->section_code));

  /* Perform relocations in-place */
  size_t i;
  for (i = 0; i < section->relocations.size; i++) {
    WasmReloc* reloc = &section->relocations.data[i];
    apply_relocation(section, reloc);
  }
}

static void write_section_payload(Context* ctx, WasmSection* sec) {
  assert(ctx->current_section_payload_offset != -1);

  sec->output_payload_offset =
      ctx->stream.offset - ctx->current_section_payload_offset;

  uint8_t* payload = &sec->binary->data[sec->payload_offset];
  wasm_write_data(&ctx->stream, payload, sec->payload_size, "section content");
}

#define WRITE_UNKNOWN_SIZE(STREAM)                        \
  uint32_t fixup_offset = (STREAM)->offset;               \
  wasm_write_fixed_u32_leb128(STREAM, 0, "unknown size"); \
  ctx->current_section_payload_offset = (STREAM)->offset; \
  uint32_t start = (STREAM)->offset;

#define FIXUP_SIZE(STREAM)                             \
  wasm_write_fixed_u32_leb128_at(STREAM, fixup_offset, \
                                 (STREAM)->offset - start, "fixup size");

static void write_combined_table_section(Context* ctx,
                                         const WasmSectionPtrVector* sections) {
  /* Total section size includes the element count leb128 which is
   * always 1 in the current spec */
  uint32_t table_count = 1;
  uint32_t flags = WASM_BINARY_LIMITS_HAS_MAX_FLAG;
  uint32_t elem_count = 0;

  size_t i;
  for (i = 0; i < sections->size; i++) {
    WasmSection* sec = sections->data[i];
    elem_count += sec->binary->table_elem_count;
  }

  WasmStream* stream = &ctx->stream;
  WRITE_UNKNOWN_SIZE(stream);
  wasm_write_u32_leb128(stream, table_count, "table count");
  wasm_write_type(stream, WASM_TYPE_ANYFUNC);
  wasm_write_u32_leb128(stream, flags, "table elem flags");
  wasm_write_u32_leb128(stream, elem_count, "table initial length");
  wasm_write_u32_leb128(stream, elem_count, "table max length");
  FIXUP_SIZE(stream);
}

static void write_combined_elem_section(Context* ctx,
                                        const WasmSectionPtrVector* sections) {
  WasmStream* stream = &ctx->stream;
  WRITE_UNKNOWN_SIZE(stream);

  size_t i;
  uint32_t total_elem_count = 0;
  for (i = 0; i < sections->size; i++) {
    WasmSection* sec = sections->data[i];
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
    WasmSection* sec = sections->data[i];
    apply_relocations(sec);
    write_section_payload(ctx, sec);
  }

  FIXUP_SIZE(stream);
}

static void write_combined_memory_section(
    Context* ctx,
    const WasmSectionPtrVector* sections) {
  WasmStream* stream = &ctx->stream;
  WRITE_UNKNOWN_SIZE(stream);

  wasm_write_u32_leb128(stream, 1, "memory count");

  WasmLimits limits;
  WASM_ZERO_MEMORY(limits);
  limits.has_max = WASM_TRUE;
  size_t i;
  for (i = 0; i < sections->size; i++) {
    WasmSection* sec = sections->data[i];
    limits.initial += sec->memory_limits.initial;
  }
  limits.max = limits.initial;
  wasm_write_limits(stream, &limits);

  FIXUP_SIZE(stream);
}

static void write_c_str(WasmStream* stream, const char* str, const char* desc) {
  wasm_write_str(stream, str, strlen(str), WASM_PRINT_CHARS, desc);
}

static void write_slice(WasmStream* stream,
                        WasmStringSlice str,
                        const char* desc) {
  wasm_write_str(stream, str.start, str.length, WASM_PRINT_CHARS, desc);
}

static void write_function_import(Context* ctx,
                                  WasmFunctionImport* import,
                                  uint32_t offset) {
  write_c_str(&ctx->stream, WASM_LINK_MODULE_NAME, "import module name");
  write_slice(&ctx->stream, import->name, "import field name");
  wasm_write_u8(&ctx->stream, WASM_EXTERNAL_KIND_FUNC, "import kind");
  wasm_write_u32_leb128(&ctx->stream, import->sig_index + offset,
                        "import signature index");
}

static void write_global_import(Context* ctx, WasmGlobalImport* import) {
  write_c_str(&ctx->stream, WASM_LINK_MODULE_NAME, "import module name");
  write_slice(&ctx->stream, import->name, "import field name");
  wasm_write_u8(&ctx->stream, WASM_EXTERNAL_KIND_GLOBAL, "import kind");
  wasm_write_type(&ctx->stream, import->type);
  wasm_write_u8(&ctx->stream, import->mutable, "global mutability");
}

static void write_import_section(Context* ctx) {
  WRITE_UNKNOWN_SIZE(&ctx->stream);

  uint32_t num_imports = 0;
  size_t i, j;
  for (i = 0; i < ctx->inputs.size; i++) {
    WasmLinkerInputBinary* binary = &ctx->inputs.data[i];
    WasmFunctionImportVector* imports = &binary->function_imports;
    for (j = 0; j < imports->size; j++) {
      WasmFunctionImport* import = &imports->data[j];
      if (import->active)
        num_imports++;
    }
    num_imports += binary->global_imports.size;
  }
  wasm_write_u32_leb128(&ctx->stream, num_imports, "num imports");

  for (i = 0; i < ctx->inputs.size; i++) {
    WasmLinkerInputBinary* binary = &ctx->inputs.data[i];
    WasmFunctionImportVector* imports = &binary->function_imports;
    for (j = 0; j < imports->size; j++) {
      WasmFunctionImport* import = &imports->data[j];
      if (import->active)
        write_function_import(ctx, import, binary->type_index_offset);
    }

    WasmGlobalImportVector* globals = &binary->global_imports;
    for (j = 0; j < globals->size; j++) {
      write_global_import(ctx, &globals->data[j]);
    }
  }

  FIXUP_SIZE(&ctx->stream);
}

static void write_combined_function_section(
    Context* ctx,
    const WasmSectionPtrVector* sections,
    uint32_t total_count) {
  WasmStream* stream = &ctx->stream;
  WRITE_UNKNOWN_SIZE(stream);

  wasm_write_u32_leb128(stream, total_count, "function count");

  size_t i;
  for (i = 0; i < sections->size; i++) {
    WasmSection* sec = sections->data[i];
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

  FIXUP_SIZE(stream);
}

static void write_data_segment(WasmStream* stream,
                               WasmDataSegment* segment,
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
                                        WasmSectionPtrVector* sections,
                                        uint32_t total_count) {
  WasmStream* stream = &ctx->stream;
  WRITE_UNKNOWN_SIZE(stream);

  wasm_write_u32_leb128(stream, total_count, "data segment count");
  size_t i, j;
  for (i = 0; i < sections->size; i++) {
    WasmSection* sec = sections->data[i];
    for (j = 0; j < sec->data_segments.size; j++) {
      WasmDataSegment* segment = &sec->data_segments.data[j];
      write_data_segment(stream, segment,
                         sec->binary->memory_page_offset * WASM_PAGE_SIZE);
    }
  }

  FIXUP_SIZE(stream);
}

static void write_names_section(Context* ctx) {
  uint32_t total_count = 0;
  size_t i, j;
  for (i = 0; i < ctx->inputs.size; i++) {
    WasmLinkerInputBinary* binary = &ctx->inputs.data[i];
    for (j = 0; j < binary->debug_names.size; j++) {
      if (j < binary->function_imports.size) {
        if (!binary->function_imports.data[j].active)
          continue;
      }
      total_count++;
    }
  }

  if (!total_count)
    return;

  WasmStream* stream = &ctx->stream;
  wasm_write_u8(stream, WASM_BINARY_SECTION_CUSTOM, "section code");
  WRITE_UNKNOWN_SIZE(stream);
  write_c_str(stream, "name", "custom section name");
  wasm_write_u32_leb128(stream, total_count, "element count");

  for (i = 0; i < ctx->inputs.size; i++) {
    WasmLinkerInputBinary* binary = &ctx->inputs.data[i];
    for (j = 0; j < binary->debug_names.size; j++) {
      if (j < binary->function_imports.size) {
        if (!binary->function_imports.data[j].active)
          continue;
      }
      write_slice(stream, binary->debug_names.data[j], "function name");
      wasm_write_u32_leb128(stream, 0, "local name count");
    }
  }

  FIXUP_SIZE(stream);
}

static void write_combined_reloc_section(Context* ctx,
                                         WasmBinarySection section_code,
                                         WasmSectionPtrVector* sections) {
  size_t i, j;
  uint32_t total_relocs = 0;

  /* First pass to know total reloc count */
  for (i = 0; i < sections->size; i++) {
    WasmSection* sec = sections->data[i];
    total_relocs += sec->relocations.size;
  }

  if (!total_relocs)
    return;

  char section_name[128];
  wasm_snprintf(section_name, sizeof(section_name), "%s.%s",
                WASM_BINARY_SECTION_RELOC, wasm_get_section_name(section_code));

  WasmStream* stream = &ctx->stream;
  wasm_write_u8(stream, WASM_BINARY_SECTION_CUSTOM, "section code");
  WRITE_UNKNOWN_SIZE(stream);
  write_c_str(stream, section_name, "reloc section name");
  wasm_write_u32_leb128(&ctx->stream, section_code, "reloc section");
  wasm_write_u32_leb128(&ctx->stream, total_relocs, "num relocs");

  for (i = 0; i < sections->size; i++) {
    WasmSection* sec = sections->data[i];
    WasmRelocVector* relocs = &sec->relocations;
    for (j = 0; j < relocs->size; j++) {
      wasm_write_u32_leb128(&ctx->stream, relocs->data[j].type, "reloc type");
      uint32_t new_offset = relocs->data[j].offset + sec->output_payload_offset;
      wasm_write_u32_leb128(&ctx->stream, new_offset, "reloc offset");
    }
  }

  FIXUP_SIZE(stream);
}

static WasmBool write_combined_section(Context* ctx,
                                       WasmBinarySection section_code,
                                       WasmSectionPtrVector* sections) {
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
    WasmSection* sec = sections->data[i];
    total_size += sec->payload_size;
    total_count += sec->count;
  }

  wasm_write_u8(&ctx->stream, section_code, "section code");
  ctx->current_section_payload_offset = -1;

  switch (section_code) {
    case WASM_BINARY_SECTION_IMPORT:
      write_import_section(ctx);
      break;
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
        WasmSection* sec = sections->data[i];
        apply_relocations(sec);
        write_section_payload(ctx, sec);
      }
    }
  }

  return WASM_TRUE;
}

typedef struct ExportInfo {
  WasmExport* export;
  WasmLinkerInputBinary* binary;
} ExportInfo;
WASM_DEFINE_VECTOR(export_info, ExportInfo);

static void resolve_symbols(Context* ctx) {
  /* Create hashmap of all exported symbols from all inputs */
  WasmBindingHash export_map;
  WASM_ZERO_MEMORY(export_map);
  ExportInfoVector export_list;
  WASM_ZERO_MEMORY(export_list);

  size_t i, j;
  for (i = 0; i < ctx->inputs.size; i++) {
    WasmLinkerInputBinary* binary = &ctx->inputs.data[i];
    for (j = 0; j < binary->exports.size; j++) {
      WasmExport* export = &binary->exports.data[j];
      ExportInfo* info = wasm_append_export_info(ctx->allocator, &export_list);
      info->export = export;
      info->binary = binary;

      /* TODO(sbc): Handle duplicate names */
      WasmStringSlice name =
          wasm_dup_string_slice(ctx->allocator, export->name);
      WasmBinding* binding =
          wasm_insert_binding(ctx->allocator, &export_map, &name);
      binding->index = export_list.size - 1;
    }
  }

  /*
   * Iterate through all imported functions resolving them against exported
   * ones.
   */
  for (i = 0; i < ctx->inputs.size; i++) {
    WasmLinkerInputBinary* binary = &ctx->inputs.data[i];
    for (j = 0; j < binary->function_imports.size; j++) {
      WasmFunctionImport* import = &binary->function_imports.data[j];
      int export_index =
          wasm_find_binding_index_by_name(&export_map, &import->name);
      if (export_index == -1) {
        if (!s_relocatable)
          WASM_FATAL("undefined symbol: " PRIstringslice "\n",
                     WASM_PRINTF_STRING_SLICE_ARG(import->name));
        continue;
      }

      /* We found the symbol exported by another module */
      ExportInfo* export_info = &export_list.data[export_index];

      /* TODO(sbc): verify the foriegn function has the correct signature */
      import->active = WASM_FALSE;
      import->foreign_binary = export_info->binary;
      import->foreign_index = export_info->export->index;
      binary->active_function_imports--;
    }
  }

  wasm_destroy_export_info_vector(ctx->allocator, &export_list);
  wasm_destroy_binding_hash(ctx->allocator, &export_map);
}

static void calculate_reloc_offsets(Context* ctx) {
  size_t i, j;
  uint32_t memory_page_offset = 0;
  uint32_t type_count = 0;
  uint32_t global_count = 0;
  uint32_t function_count = 0;
  uint32_t total_function_imports = 0;
  uint32_t total_global_imports = 0;
  for (i = 0; i < ctx->inputs.size; i++) {
    WasmLinkerInputBinary* binary = &ctx->inputs.data[i];
    /* The imported_function_index_offset is the sum of all the function
     * imports from objects that precede this one.  i.e. the current running
     * total */
    binary->imported_function_index_offset = total_function_imports;
    binary->imported_global_index_offset = total_global_imports;
    binary->memory_page_offset = memory_page_offset;
    memory_page_offset += binary->memory_page_count;
    total_function_imports += binary->active_function_imports;
    total_global_imports += binary->global_imports.size;
  }

  for (i = 0; i < ctx->inputs.size; i++) {
    WasmLinkerInputBinary* binary = &ctx->inputs.data[i];
    for (j = 0; j < binary->sections.size; j++) {
      WasmSection* sec = &binary->sections.data[j];
      switch (sec->section_code) {
        case WASM_BINARY_SECTION_TYPE:
          binary->type_index_offset = type_count;
          type_count += sec->count;
          break;
        case WASM_BINARY_SECTION_GLOBAL:
          binary->global_index_offset = total_global_imports -
                                        sec->binary->global_imports.size +
                                        global_count;
          global_count += sec->count;
          break;
        case WASM_BINARY_SECTION_FUNCTION:
          binary->function_index_offset = total_function_imports -
                                          sec->binary->function_imports.size +
                                          function_count;
          function_count += sec->count;
          break;
        default:
          break;
      }
    }
  }
}

static void write_binary(Context* ctx) {
  /* Find all the sections of each type */
  WasmSectionPtrVector sections[WASM_NUM_BINARY_SECTIONS];
  WASM_ZERO_MEMORY(sections);

  size_t i, j;
  for (j = 0; j < ctx->inputs.size; j++) {
    WasmLinkerInputBinary* binary = &ctx->inputs.data[j];
    for (i = 0; i < binary->sections.size; i++) {
      WasmSection* s = &binary->sections.data[i];
      WasmSectionPtrVector* sec_list = &sections[s->section_code];
      wasm_append_section_ptr_value(ctx->allocator, sec_list, &s);
    }
  }

  /* Write the final binary */
  wasm_write_u32(&ctx->stream, WASM_BINARY_MAGIC, "WASM_BINARY_MAGIC");
  wasm_write_u32(&ctx->stream, WASM_BINARY_VERSION, "WASM_BINARY_VERSION");

  /* Write known sections first */
  for (i = FIRST_KNOWN_SECTION; i < WASM_NUM_BINARY_SECTIONS; i++) {
    write_combined_section(ctx, i, &sections[i]);
  }

  write_names_section(ctx);

  /* Generate a new set of reloction sections */
  for (i = FIRST_KNOWN_SECTION; i < WASM_NUM_BINARY_SECTIONS; i++) {
    write_combined_reloc_section(ctx, i, &sections[i]);
  }

  for (i = 0; i < WASM_NUM_BINARY_SECTIONS; i++) {
    wasm_destroy_section_ptr_vector(ctx->allocator, &sections[i]);
  }
}

static void dump_reloc_offsets(Context* ctx) {
  if (s_verbose) {
    uint32_t i;
    for (i = 0; i < ctx->inputs.size; i++) {
      WasmLinkerInputBinary* binary = &ctx->inputs.data[i];
      wasm_writef(&s_log_stream, "Relocation info for: %s\n", binary->filename);
      wasm_writef(&s_log_stream, " - type index offset       : %d\n",
                  binary->type_index_offset);
      wasm_writef(&s_log_stream, " - mem page offset         : %d\n",
                  binary->memory_page_offset);
      wasm_writef(&s_log_stream, " - function index offset   : %d\n",
                  binary->function_index_offset);
      wasm_writef(&s_log_stream, " - global index offset     : %d\n",
                  binary->global_index_offset);
      wasm_writef(&s_log_stream, " - imported function offset: %d\n",
                  binary->imported_function_index_offset);
      wasm_writef(&s_log_stream, " - imported global offset  : %d\n",
                  binary->imported_global_index_offset);
    }
  }
}

static WasmResult perform_link(Context* ctx) {
  WasmMemoryWriter writer;
  WASM_ZERO_MEMORY(writer);
  if (WASM_FAILED(wasm_init_mem_writer(ctx->allocator, &writer)))
    WASM_FATAL("unable to open memory writer for writing\n");

  WasmStream* log_stream = NULL;
  if (s_verbose)
    log_stream = &s_log_stream;

  if (s_verbose)
    wasm_writef(&s_log_stream, "writing file: %s\n", s_outfile);

  calculate_reloc_offsets(ctx);
  resolve_symbols(ctx);
  calculate_reloc_offsets(ctx);
  dump_reloc_offsets(ctx);
  wasm_init_stream(&ctx->stream, &writer.base, log_stream);
  write_binary(ctx);

  if (WASM_FAILED(wasm_write_output_buffer_to_file(&writer.buf, s_outfile)))
    WASM_FATAL("error writing linked output to file\n");

  wasm_close_mem_writer(&writer);
  return WASM_OK;
}

int main(int argc, char** argv) {
  wasm_init_stdio();

  Context context;
  WASM_ZERO_MEMORY(context);
  context.allocator = &g_wasm_libc_allocator;

  parse_options(context.allocator, argc, argv);

  WasmResult result = WASM_OK;
  size_t i;
  for (i = 0; i < s_infiles.size; i++) {
    const char* input_filename = s_infiles.data[i];
    if (s_verbose)
      wasm_writef(&s_log_stream, "reading file: %s\n", input_filename);
    void* data;
    size_t size;
    result = wasm_read_file(context.allocator, input_filename, &data, &size);
    if (WASM_FAILED(result))
      return result;
    WasmLinkerInputBinary* b =
        wasm_append_binary(context.allocator, &context.inputs);
    b->data = data;
    b->size = size;
    b->filename = input_filename;
    result = wasm_read_binary_linker(context.allocator, b);
    if (WASM_FAILED(result))
      WASM_FATAL("error parsing file: %s\n", input_filename);
  }

  result = perform_link(&context);
  if (WASM_FAILED(result))
    return result;

  /* Cleanup */
  WASM_DESTROY_VECTOR_AND_ELEMENTS(context.allocator, context.inputs, binary);
  wasm_destroy_string_vector(context.allocator, &s_infiles);

  wasm_print_allocator_stats(context.allocator);
  wasm_destroy_allocator(context.allocator);
  return result;
}

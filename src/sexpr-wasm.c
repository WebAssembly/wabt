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

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "wasm-config.h"

#include "wasm-ast.h"
#include "wasm-ast-checker.h"
#include "wasm-binary-writer.h"
#include "wasm-binary-writer-spec.h"
#include "wasm-common.h"
#include "wasm-mark-used-blocks.h"
#include "wasm-option-parser.h"
#include "wasm-parser.h"
#include "wasm-stack-allocator.h"
#include "wasm-writer.h"

#define ALLOC_FAILURE \
  WASM_FATAL("%s:%d: allocation failed\n", __FILE__, __LINE__)

#define CHECK_ALLOC(cond) \
  do {                    \
    if ((cond) == NULL)   \
      ALLOC_FAILURE;      \
  } while (0)

static const char* s_infile;
static const char* s_outfile;
static WasmBool s_dump_module;
static WasmBool s_verbose;
static WasmWriteBinaryOptions s_write_binary_options =
    WASM_WRITE_BINARY_OPTIONS_DEFAULT;
static WasmWriteBinarySpecOptions s_write_binary_spec_options =
    WASM_WRITE_BINARY_SPEC_OPTIONS_DEFAULT;
static WasmBool s_spec;
static WasmBool s_use_libc_allocator;

#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  FLAG_DUMP_MODULE,
  FLAG_OUTPUT,
  FLAG_SPEC,
  FLAG_USE_LIBC_ALLOCATOR,
  FLAG_NO_CANONICALIZE_LEB128S,
  FLAG_NO_REMAP_LOCALS,
  FLAG_DEBUG_NAMES,
  NUM_FLAGS
};

static WasmOption s_options[] = {
    {FLAG_VERBOSE, 'v', "verbose", NULL, NOPE,
     "use multiple times for more info"},
    {FLAG_HELP, 'h', "help", NULL, NOPE, "print this help message"},
    {FLAG_DUMP_MODULE, 'd', "dump-module", NULL, NOPE,
     "print a hexdump of the module to stdout"},
    {FLAG_OUTPUT, 'o', "output", "FILE", YEP,
     "output file for the generated binary format"},
    {FLAG_SPEC, 0, "spec", NULL, NOPE,
     "parse a file with multiple modules and assertions, like the spec tests"},
    {FLAG_USE_LIBC_ALLOCATOR, 0, "use-libc-allocator", NULL, NOPE,
     "use malloc, free, etc. instead of stack allocator"},
    {FLAG_NO_CANONICALIZE_LEB128S, 0, "no-canonicalize-leb128s", NULL, NOPE,
     "Write all LEB128 sizes as 5-bytes instead of their minimal size"},
    {FLAG_NO_REMAP_LOCALS, 0, "no-remap-locals", NULL, NOPE,
     "If set, function locals are written in source order, instead of packing "
     "them to reduce size"},
    {FLAG_DEBUG_NAMES, 0, "debug-names", NULL, NOPE,
     "Write debug names to the generated binary file"},
};
WASM_STATIC_ASSERT(NUM_FLAGS == WASM_ARRAY_SIZE(s_options));

static void on_option(struct WasmOptionParser* parser,
                      struct WasmOption* option,
                      const char* argument) {
  switch (option->id) {
    case FLAG_VERBOSE:
      s_verbose++;
      s_write_binary_options.log_writes = WASM_TRUE;
      break;

    case FLAG_HELP:
      wasm_print_help(parser);
      exit(0);
      break;

    case FLAG_DUMP_MODULE:
      s_dump_module = WASM_TRUE;
      break;

    case FLAG_OUTPUT:
      s_outfile = argument;
      break;

    case FLAG_SPEC:
      s_spec = WASM_TRUE;
      break;

    case FLAG_USE_LIBC_ALLOCATOR:
      s_use_libc_allocator = WASM_TRUE;
      break;

    case FLAG_NO_CANONICALIZE_LEB128S:
      s_write_binary_options.canonicalize_lebs = WASM_FALSE;
      break;

    case FLAG_NO_REMAP_LOCALS:
      s_write_binary_options.remap_locals = WASM_FALSE;
      break;

    case FLAG_DEBUG_NAMES:
      s_write_binary_options.write_debug_names = WASM_TRUE;
      break;
  }
}

static void on_argument(struct WasmOptionParser* parser, const char* argument) {
  s_infile = argument;
}

static void on_option_error(struct WasmOptionParser* parser,
                            const char* message) {
  WASM_FATAL("%s\n", message);
}

static void parse_options(int argc, char** argv) {
  WasmOptionParser parser;
  WASM_ZERO_MEMORY(parser);
  parser.options = s_options;
  parser.num_options = WASM_ARRAY_SIZE(s_options);
  parser.on_option = on_option;
  parser.on_argument = on_argument;
  parser.on_error = on_option_error;
  wasm_parse_options(&parser, argc, argv);

  if (!s_infile) {
    wasm_print_help(&parser);
    WASM_FATAL("No filename given.\n");
  }
}

static void write_buffer_to_file(const char* out_filename,
                                 WasmOutputBuffer* buffer) {
  if (s_dump_module) {
    if (s_verbose)
      printf(";; dump\n");
    wasm_print_memory(buffer->start, buffer->size, 0, WASM_DONT_PRINT_CHARS,
                      NULL);
  }

  if (out_filename) {
    FILE* file = fopen(out_filename, "wb");
    if (!file)
      WASM_FATAL("unable to open %s for writing\n", s_outfile);

    ssize_t bytes = fwrite(buffer->start, 1, buffer->size, file);
    if (bytes < 0 || (size_t)bytes != buffer->size) {
      WASM_FATAL("failed to write %" PRIzd " bytes to %s\n", buffer->size,
                 out_filename);
    }
    fclose(file);
  }
}

static WasmStringSlice strip_extension(const char* s) {
  /* strip .json or .wasm, but leave other extensions, e.g.:
   *
   * s = "foo", => "foo"
   * s = "foo.json" => "foo"
   * s = "foo.wasm" => "foo"
   * s = "foo.bar" => "foo.bar"
   */
  if (s == NULL) {
    WasmStringSlice result;
    result.start = NULL;
    result.length = 0;
    return result;
  }

  size_t slen = strlen(s);
  const char* ext_start = strrchr(s, '.');
  if (ext_start == NULL)
    ext_start = s + slen;

  WasmStringSlice result;
  result.start = s;

  if (strcmp(ext_start, ".json") == 0 || strcmp(ext_start, ".wasm") == 0) {
    result.length = ext_start - s;
  } else {
    result.length = slen;
  }
  return result;
}

static WasmStringSlice get_basename(const char* s) {
  /* strip everything up to and including the last slash, e.g.:
   *
   * s = "/foo/bar/baz", => "baz"
   * s = "/usr/local/include/stdio.h", => "stdio.h"
   * s = "foo.bar", => "foo.bar"
   */
  size_t slen = strlen(s);
  const char* start = s;
  const char* last_slash = strrchr(s, '/');
  if (last_slash != NULL)
    start = last_slash + 1;

  WasmStringSlice result;
  result.start = start;
  result.length = s + slen - start;
  return result;
}

static char* get_module_filename(WasmAllocator* allocator,
                                 WasmStringSlice* filename_noext,
                                 uint32_t index) {
  size_t buflen = filename_noext->length + 20;
  char* str;
  CHECK_ALLOC(str = wasm_alloc(allocator, buflen, WASM_DEFAULT_ALIGN));
  wasm_snprintf(str, buflen, PRIstringslice ".%d.wasm",
                WASM_PRINTF_STRING_SLICE_ARG(*filename_noext), index);
  return str;
}

typedef struct WasmContext {
  WasmAllocator* allocator;
  WasmMemoryWriter json_writer;
  uint32_t json_writer_offset;
  WasmMemoryWriter module_writer;
  WasmStringSlice output_filename_noext;
  char* module_filename;
  WasmResult result;
} WasmContext;

static void out_data(WasmContext* ctx, const void* src, size_t size) {
  if (WASM_FAILED(ctx->result))
    return;
  WasmWriter* writer = &ctx->json_writer.base;
  if (writer->write_data) {
    ctx->result = writer->write_data(ctx->json_writer_offset, src, size,
                                     writer->user_data);
    ctx->json_writer_offset += size;
  }
}

static void out_printf(WasmContext* ctx, const char* format, ...) {
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
  out_data(ctx, buffer, len - 1);
}

static void on_script_begin(void* user_data) {
  WasmContext* ctx = user_data;

  if (WASM_FAILED(wasm_init_mem_writer(ctx->allocator, &ctx->module_writer)))
    WASM_FATAL("unable to open memory writer for writing\n");

  if (WASM_FAILED(wasm_init_mem_writer(ctx->allocator, &ctx->json_writer)))
    WASM_FATAL("unable to open memory writer for writing\n");

  out_printf(ctx, "{\"modules\": [\n");
}

static void on_module_begin(uint32_t index, void* user_data) {
  WasmContext* ctx = user_data;
  wasm_free(ctx->allocator, ctx->module_filename);
  ctx->module_filename =
      get_module_filename(ctx->allocator, &ctx->output_filename_noext, index);
  if (index != 0)
    out_printf(ctx, ",\n");
  WasmStringSlice module_basename = get_basename(ctx->module_filename);
  out_printf(ctx, "  {\"filename\": \"" PRIstringslice "\", \"commands\": [\n",
             WASM_PRINTF_STRING_SLICE_ARG(module_basename));
}

static void on_command(uint32_t index,
                       WasmCommandType type,
                       const WasmStringSlice* name,
                       const WasmLocation* loc,
                       void* user_data) {
  static const char* s_command_names[] = {
    "module",
    "invoke",
    "assert_invalid",
    "assert_return",
    "assert_return_nan",
    "assert_trap",
  };
  WASM_STATIC_ASSERT(WASM_ARRAY_SIZE(s_command_names) ==
                     WASM_NUM_COMMAND_TYPES);

  static const char* s_command_format =
      "    {"
      "\"type\": \"%s\", "
      "\"name\": \"" PRIstringslice
      "\", "
      "\"file\": \"%s\", "
      "\"line\": %d}";

  WasmContext* ctx = user_data;
  if (index != 0)
    out_printf(ctx, ",\n");
  out_printf(ctx, s_command_format, s_command_names[type],
             WASM_PRINTF_STRING_SLICE_ARG(*name), loc->filename, loc->line);
}

static void on_module_before_write(uint32_t index,
                                   WasmWriter** out_writer,
                                   void* user_data) {
  WasmContext* ctx = user_data;
  ctx->module_writer.buf.size = 0;
  *out_writer = &ctx->module_writer.base;
}

static void on_module_end(uint32_t index, WasmResult result, void* user_data) {
  WasmContext* ctx = user_data;
  out_printf(ctx, "\n  ]}");
  if (WASM_SUCCEEDED(result))
    write_buffer_to_file(ctx->module_filename, &ctx->module_writer.buf);
}

static void on_script_end(void* user_data) {
  WasmContext* ctx = user_data;
  out_printf(ctx, "\n]}\n");

  if (WASM_SUCCEEDED(ctx->result))
    write_buffer_to_file(s_outfile, &ctx->json_writer.buf);

  wasm_free(ctx->allocator, ctx->module_filename);
  wasm_close_mem_writer(&ctx->module_writer);
  wasm_close_mem_writer(&ctx->json_writer);
}

int main(int argc, char** argv) {
  WasmStackAllocator stack_allocator;
  WasmAllocator* allocator;

  parse_options(argc, argv);

  if (s_use_libc_allocator) {
    allocator = &g_wasm_libc_allocator;
  } else {
    wasm_init_stack_allocator(&stack_allocator, &g_wasm_libc_allocator);
    allocator = &stack_allocator.allocator;
  }

  WasmLexer lexer = wasm_new_lexer(allocator, s_infile);
  if (!lexer)
    WASM_FATAL("unable to read %s\n", s_infile);

  WasmScript script;
  WasmResult result = wasm_parse(lexer, &script);

  if (WASM_SUCCEEDED(result)) {
    result = wasm_check_ast(lexer, &script);

    if (WASM_SUCCEEDED(result)) {
      result = wasm_mark_used_blocks(allocator, &script);

      if (WASM_SUCCEEDED(result)) {
        if (s_spec) {
          WasmContext ctx;
          WASM_ZERO_MEMORY(ctx);
          ctx.allocator = allocator;
          ctx.output_filename_noext = strip_extension(s_outfile);

          s_write_binary_spec_options.on_script_begin = &on_script_begin;
          s_write_binary_spec_options.on_module_begin = &on_module_begin;
          s_write_binary_spec_options.on_command = &on_command,
          s_write_binary_spec_options.on_module_before_write =
              &on_module_before_write;
          s_write_binary_spec_options.on_module_end = &on_module_end;
          s_write_binary_spec_options.on_script_end = &on_script_end;
          s_write_binary_spec_options.user_data = &ctx;

          result = wasm_write_binary_spec_script(allocator, &script,
                                                 &s_write_binary_options,
                                                 &s_write_binary_spec_options);
        } else {
          WasmMemoryWriter writer;
          WASM_ZERO_MEMORY(writer);
          if (WASM_FAILED(wasm_init_mem_writer(allocator, &writer)))
            WASM_FATAL("unable to open memory writer for writing\n");

          result = wasm_write_binary_script(allocator, &writer.base, &script,
                                            &s_write_binary_options);
          if (WASM_SUCCEEDED(result))
            write_buffer_to_file(s_outfile, &writer.buf);
          wasm_close_mem_writer(&writer);
        }
      }
    }
  }

  wasm_destroy_lexer(lexer);

  if (s_use_libc_allocator)
    wasm_destroy_script(&script);
  wasm_print_allocator_stats(allocator);
  wasm_destroy_allocator(allocator);
  return result;
}

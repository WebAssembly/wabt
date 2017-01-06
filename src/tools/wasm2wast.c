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
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "allocator.h"
#include "apply-names.h"
#include "ast.h"
#include "ast-writer.h"
#include "binary-reader.h"
#include "binary-reader-ast.h"
#include "generate-names.h"
#include "option-parser.h"
#include "stack-allocator.h"
#include "stream.h"
#include "writer.h"

#define PROGRAM_NAME "wasm2wast"

static int s_verbose;
static const char* s_infile;
static const char* s_outfile;
static WasmReadBinaryOptions s_read_binary_options = {NULL, WASM_TRUE};
static WasmBool s_use_libc_allocator;
static WasmBool s_generate_names;

static WasmBinaryErrorHandler s_error_handler =
    WASM_BINARY_ERROR_HANDLER_DEFAULT;

static WasmFileWriter s_log_stream_writer;
static WasmStream s_log_stream;

#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  FLAG_OUTPUT,
  FLAG_USE_LIBC_ALLOCATOR,
  FLAG_NO_DEBUG_NAMES,
  FLAG_GENERATE_NAMES,
  NUM_FLAGS
};

static const char s_description[] =
    "  read a file in the wasm binary format, and convert it to the wasm\n"
    "  s-expression file format.\n"
    "\n"
    "examples:\n"
    "  # parse binary file test.wasm and write s-expression file test.wast\n"
    "  $ wasm2wast test.wasm -o test.wast\n"
    "\n"
    "  # parse test.wasm, write test.wast, but ignore the debug names, if any\n"
    "  $ wasm2wast test.wasm --no-debug-names -o test.wast\n";

static WasmOption s_options[] = {
    {FLAG_VERBOSE, 'v', "verbose", NULL, NOPE,
     "use multiple times for more info"},
    {FLAG_HELP, 'h', "help", NULL, NOPE, "print this help message"},
    {FLAG_OUTPUT, 'o', "output", "FILENAME", YEP,
     "output file for the generated wast file, by default use stdout"},
    {FLAG_USE_LIBC_ALLOCATOR, 0, "use-libc-allocator", NULL, NOPE,
     "use malloc, free, etc. instead of stack allocator"},
    {FLAG_NO_DEBUG_NAMES, 0, "no-debug-names", NULL, NOPE,
     "Ignore debug names in the binary file"},
    {FLAG_GENERATE_NAMES, 0, "generate-names", NULL, NOPE,
     "Give auto-generated names to non-named functions, types, etc."},
};
WASM_STATIC_ASSERT(NUM_FLAGS == WASM_ARRAY_SIZE(s_options));

static void on_option(struct WasmOptionParser* parser,
                      struct WasmOption* option,
                      const char* argument) {
  switch (option->id) {
    case FLAG_VERBOSE:
      s_verbose++;
      wasm_init_file_writer_existing(&s_log_stream_writer, stdout);
      wasm_init_stream(&s_log_stream, &s_log_stream_writer.base, NULL);
      s_read_binary_options.log_stream = &s_log_stream;
      break;

    case FLAG_HELP:
      wasm_print_help(parser, PROGRAM_NAME);
      exit(0);
      break;

    case FLAG_OUTPUT:
      s_outfile = argument;
      break;

    case FLAG_USE_LIBC_ALLOCATOR:
      s_use_libc_allocator = WASM_TRUE;
      break;

    case FLAG_NO_DEBUG_NAMES:
      s_read_binary_options.read_debug_names = WASM_FALSE;
      break;

    case FLAG_GENERATE_NAMES:
      s_generate_names = WASM_TRUE;
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
  parser.description = s_description;
  parser.options = s_options;
  parser.num_options = WASM_ARRAY_SIZE(s_options);
  parser.on_option = on_option;
  parser.on_argument = on_argument;
  parser.on_error = on_option_error;
  wasm_parse_options(&parser, argc, argv);

  if (!s_infile) {
    wasm_print_help(&parser, PROGRAM_NAME);
    WASM_FATAL("No filename given.\n");
  }
}

int main(int argc, char** argv) {
  WasmResult result;
  WasmStackAllocator stack_allocator;
  WasmAllocator* allocator;

  wasm_init_stdio();
  parse_options(argc, argv);

  if (s_use_libc_allocator) {
    allocator = &g_wasm_libc_allocator;
  } else {
    wasm_init_stack_allocator(&stack_allocator, &g_wasm_libc_allocator);
    allocator = &stack_allocator.allocator;
  }

  void* data;
  size_t size;
  result = wasm_read_file(allocator, s_infile, &data, &size);
  if (WASM_SUCCEEDED(result)) {
    WasmModule module;
    WASM_ZERO_MEMORY(module);
    result = wasm_read_binary_ast(allocator, data, size, &s_read_binary_options,
                                  &s_error_handler, &module);
    if (WASM_SUCCEEDED(result)) {
      if (s_generate_names)
        result = wasm_generate_names(allocator, &module);

      if (WASM_SUCCEEDED(result)) {
        /* TODO(binji): This shouldn't fail; if a name can't be applied
         * (because the index is invalid, say) it should just be skipped. */
        WasmResult dummy_result = wasm_apply_names(allocator, &module);
        WASM_USE(dummy_result);
      }

      if (WASM_SUCCEEDED(result)) {
        WasmFileWriter file_writer;
        if (s_outfile) {
          result = wasm_init_file_writer(&file_writer, s_outfile);
        } else {
          wasm_init_file_writer_existing(&file_writer, stdout);
        }

        if (WASM_SUCCEEDED(result)) {
          result = wasm_write_ast(allocator, &file_writer.base, &module);
          wasm_close_file_writer(&file_writer);
        }
      }

      if (s_use_libc_allocator)
        wasm_destroy_module(allocator, &module);
    }

    wasm_free(allocator, data);
    wasm_print_allocator_stats(allocator);
    wasm_destroy_allocator(allocator);
  }
  return result;
}

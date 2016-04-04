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

#include "wasm-allocator.h"
#include "wasm-ast.h"
#include "wasm-ast-writer.h"
#include "wasm-binary-reader.h"
#include "wasm-binary-reader-ast.h"
#include "wasm-option-parser.h"
#include "wasm-stack-allocator.h"
#include "wasm-writer.h"

static int s_verbose;
static const char* s_infile;
static const char* s_outfile;
static WasmReadBinaryOptions s_read_binary_options =
    WASM_READ_BINARY_OPTIONS_DEFAULT;
static int s_use_libc_allocator;

#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  FLAG_OUTPUT,
  FLAG_USE_LIBC_ALLOCATOR,
  FLAG_DEBUG_NAMES,
  NUM_FLAGS
};

static WasmOption s_options[] = {
    {FLAG_VERBOSE, 'v', "verbose", NULL, NOPE,
     "use multiple times for more info"},
    {FLAG_HELP, 'h', "help", NULL, NOPE, "print this help message"},
    {FLAG_OUTPUT, 'o', "output", "FILENAME", YEP,
     "output file for the generated wast file"},
    {FLAG_USE_LIBC_ALLOCATOR, 0, "use-libc-allocator", NULL, NOPE,
     "use malloc, free, etc. instead of stack allocator"},
    {FLAG_DEBUG_NAMES, 0, "debug-names", NULL, NOPE,
     "Read debug names from the binary file"},
};
WASM_STATIC_ASSERT(NUM_FLAGS == WASM_ARRAY_SIZE(s_options));

static void on_option(struct WasmOptionParser* parser,
                      struct WasmOption* option,
                      const char* argument) {
  switch (option->id) {
    case FLAG_VERBOSE:
      s_verbose++;
      break;

    case FLAG_HELP:
      wasm_print_help(parser);
      exit(0);
      break;

    case FLAG_OUTPUT:
      s_outfile = argument;
      break;

    case FLAG_USE_LIBC_ALLOCATOR:
      s_use_libc_allocator = 1;
      break;

    case FLAG_DEBUG_NAMES:
      s_read_binary_options.read_debug_names = 1;
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

static void read_file(const char* filename,
                      const void** out_data,
                      size_t* out_size) {
  FILE* infile = fopen(s_infile, "rb");
  if (!infile)
    WASM_FATAL("unable to read %s\n", s_infile);

  if (fseek(infile, 0, SEEK_END) < 0)
    WASM_FATAL("fseek to end failed.\n");

  long size = ftell(infile);
  if (size < 0)
    WASM_FATAL("ftell failed.\n");

  if (fseek(infile, 0, SEEK_SET) < 0)
    WASM_FATAL("fseek to beginning failed.\n");

  void* data = malloc(size);
  if (fread(data, size, 1, infile) != 1)
    WASM_FATAL("fread failed.\n");

  *out_data = data;
  *out_size = size;
  fclose(infile);
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

  const void* data;
  size_t size;
  read_file(s_infile, &data, &size);

  WasmModule module;
  WASM_ZERO_MEMORY(module);
  WasmResult result = wasm_read_binary_ast(allocator, data, size,
                                           &s_read_binary_options, &module);
  if (result == WASM_OK) {
    if (s_outfile) {
      WasmFileWriter file_writer;
      result = wasm_init_file_writer(&file_writer, s_outfile);
      if (result == WASM_OK) {
        result = wasm_write_ast(allocator, &file_writer.base, &module);
        wasm_close_file_writer(&file_writer);
      }
    }
  }

  if (s_use_libc_allocator)
    wasm_destroy_module(allocator, &module);
  wasm_print_allocator_stats(allocator);
  wasm_destroy_allocator(allocator);
  free((void*)data);
  return result;
}

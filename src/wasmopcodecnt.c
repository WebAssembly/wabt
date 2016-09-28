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
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "wasm-allocator.h"
#include "wasm-binary-reader.h"
#include "wasm-binary-reader-opcnt.h"
#include "wasm-option-parser.h"
#include "wasm-stack-allocator.h"
#include "wasm-stream.h"

#define PROGRAM_NAME "wasmopcodecnt"
#define WASM_OPCODE_LIMIT 256

#define ERROR(fmt, ...) \
  fprintf(stderr, "%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)

static int s_verbose;
static const char* s_infile;
static const char* s_outfile;

static WasmReadBinaryOptions s_read_binary_options =
    WASM_READ_BINARY_OPTIONS_DEFAULT;

static WasmBool s_use_libc_allocator;

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
  NUM_FLAGS
};

static const char s_description[] =
    "  Read a file in the was binary format, and count opcode usage for\n"
    "  instructions.\n"
    "\n"
    "examples:\n"
    "  # parse binary file test.wasm and write opcode dist file test.dist\n"
    "  $ wasmopcodecnt test.wasm -o test.dist\n";

static WasmOption s_options[] = {
    {FLAG_VERBOSE, 'v', "verbose", NULL, NOPE,
     "use multiple times for more info"},
    {FLAG_HELP, 'h', "help", NULL, NOPE, "print this help message"},
    {FLAG_OUTPUT, 'o', "output", "FILENAME", YEP,
     "output file for the generated wast file, by default use stdout"},
    {FLAG_USE_LIBC_ALLOCATOR, 0, "use-libc-allocator", NULL, NOPE,
     "use malloc, free, etc. instead of stack allocator"},
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

#define V(rtype, type1, type2, mem_size, code, NAME, text) [code] = text,
static const char* s_opcode_name[] = {WASM_FOREACH_OPCODE(V)};
#undef V

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
    size_t wasm_opcode_count[WASM_ARRAY_SIZE(s_opcode_name)];
    WASM_ZERO_MEMORY(wasm_opcode_count);
    fprintf(stderr, "wasm_opcode_count size = %u",
            (unsigned)sizeof(wasm_opcode_count));
    result = wasm_read_binary_opcnt(allocator, data, size, &s_read_binary_options,
                                    &s_error_handler, wasm_opcode_count,
                                    (size_t)WASM_ARRAY_SIZE(wasm_opcode_count));
    FILE* out = stdout;
    if (s_outfile) {
      out = fopen(s_outfile, "w");
      if (!out)
        ERROR("fopen \"%s\" failed, errno=%d\n", s_outfile, errno);
      result = WASM_ERROR;
    }
    if (WASM_SUCCEEDED(result)) {
      for (size_t i = 0; i < WASM_OPCODE_LIMIT; ++i) {
        const char* Name = s_opcode_name[i];
        if (Name != 0) {
          fprintf(out, "%s: %" PRIuMAX "\n", Name,
                  (uintmax_t)wasm_opcode_count[i]);
        }
      }
    }
    wasm_free(allocator, data);
    wasm_print_allocator_stats(allocator);
    wasm_destroy_allocator(allocator);
  }
  return result;
}

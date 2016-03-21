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
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "wasm-allocator.h"
#include "wasm-ast.h"
#include "wasm-ast-writer.h"
#include "wasm-binary-reader-ast.h"
#include "wasm-option-parser.h"
#include "wasm-stack-allocator.h"
#include "wasm-writer.h"

static int s_verbose;
static const char* s_infile;
static const char* s_outfile;

#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  FLAG_OUTPUT,
  NUM_FLAGS
};

static WasmOption s_options[] = {
    {FLAG_VERBOSE, 'v', "verbose", NULL, NOPE,
     "use multiple times for more info"},
    {FLAG_HELP, 'h', "help", NULL, NOPE, "print this help message"},
    {FLAG_OUTPUT, 'o', "output", "FILENAME", YEP,
     "output file for the generated wast file"},
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

int main(int argc, char** argv) {
  parse_options(argc, argv);

  int fd = open(s_infile, O_RDONLY);
  if (fd == -1)
    WASM_FATAL("Unable to open file %s.\n", s_infile);

  struct stat statbuf;
  if (fstat(fd, &statbuf) == -1)
    WASM_FATAL("Unable to stat file %s.\n", s_infile);

  size_t length = statbuf.st_size;
  void* addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED)
    WASM_FATAL("Unable to mmap file %s.\n", s_infile);

  WasmStackAllocator stack_allocator;
  wasm_init_stack_allocator(&stack_allocator, &g_wasm_libc_allocator);
  WasmAllocator* allocator = &stack_allocator.allocator;
  WasmModule module;
  WASM_ZERO_MEMORY(module);
  WasmResult result = wasm_read_binary_ast(allocator, addr, length, &module);
  if (result == WASM_OK) {
    if (s_outfile) {
      WasmFileWriter file_writer;
      result = wasm_init_file_writer(&file_writer, s_outfile);
      if (result == WASM_OK) {
        result = wasm_write_ast(&file_writer.base, &module);
        fprintf(stdout, "\n");
        wasm_close_file_writer(&file_writer);
      }
    }
  }

  wasm_destroy_stack_allocator(&stack_allocator);
  return result;
}

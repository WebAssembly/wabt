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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "wasm-config.h"

#include "wasm-ast.h"
#include "wasm-binary-writer.h"
#include "wasm-binary-writer-spec.h"
#include "wasm-check.h"
#include "wasm-common.h"
#include "wasm-option-parser.h"
#include "wasm-parser.h"
#include "wasm-stack-allocator.h"
#include "wasm-writer.h"

static const char* s_infile;
static const char* s_outfile;
static int s_dump_module;
static int s_verbose;
static WasmWriteBinaryOptions s_write_binary_options =
    WASM_WRITE_BINARY_OPTIONS_DEFAULT;
static WasmWriteBinarySpecOptions s_write_binary_spec_options =
    WASM_WRITE_BINARY_SPEC_OPTIONS_DEFAULT;
static int s_spec;
static int s_use_libc_allocator;

#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  FLAG_DUMP_MODULE,
  FLAG_OUTPUT,
  FLAG_SPEC,
  FLAG_SPEC_VERBOSE,
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
    {FLAG_SPEC_VERBOSE, 0, "spec-verbose", NULL, NOPE,
     "print logging messages when running spec files"},
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
      s_write_binary_options.log_writes = 1;
      break;

    case FLAG_HELP:
      wasm_print_help(parser);
      exit(0);
      break;

    case FLAG_DUMP_MODULE:
      s_dump_module = 1;
      break;

    case FLAG_OUTPUT:
      s_outfile = argument;
      break;

    case FLAG_SPEC:
      s_spec = 1;
      break;

    case FLAG_SPEC_VERBOSE:
      s_write_binary_spec_options.verbose = 1;
      break;

    case FLAG_USE_LIBC_ALLOCATOR:
      s_use_libc_allocator = 1;
      break;

    case FLAG_NO_CANONICALIZE_LEB128S:
      s_write_binary_options.canonicalize_lebs = 0;
      break;

    case FLAG_NO_REMAP_LOCALS:
      s_write_binary_options.remap_locals = 0;
      break;

    case FLAG_DEBUG_NAMES:
      s_write_binary_options.write_debug_names = 1;
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

  if (s_dump_module && s_spec)
    WASM_FATAL("--dump-module flag incompatible with --spec flag\n");

  if (!s_infile) {
    wasm_print_help(&parser);
    WASM_FATAL("No filename given.\n");
  }
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

  if (result == WASM_OK) {
    result = wasm_check_script(lexer, &script);
    if (result == WASM_OK) {
      WasmMemoryWriter writer;
      WASM_ZERO_MEMORY(writer);
      if (wasm_init_mem_writer(allocator, &writer) != WASM_OK)
        WASM_FATAL("unable to open memory writer for writing\n");

      if (s_spec) {
        result = wasm_write_binary_spec_script(allocator, &writer.base, &script,
                                               &s_write_binary_options,
                                               &s_write_binary_spec_options);
      } else {
        result = wasm_write_binary_script(allocator, &writer.base, &script,
                                          &s_write_binary_options);
      }

      if (result == WASM_OK) {
        if (s_dump_module) {
          if (s_verbose)
            printf(";; dump\n");
          wasm_print_memory(writer.buf.start, writer.buf.size, 0, 0, NULL);
        }

        if (s_outfile) {
          FILE* f = fopen(s_outfile, "wb");
          if (!f)
            WASM_FATAL("unable to open %s for writing\n", s_outfile);

          ssize_t bytes = fwrite(writer.buf.start, 1, writer.buf.size, f);
          if (bytes != writer.buf.size)
            WASM_FATAL("failed to write %" PRIzd " bytes to %s\n",
                       writer.buf.size, s_outfile);
          fclose(f);
        }
      }
      wasm_close_mem_writer(&writer);
    }
  }

  wasm_destroy_lexer(lexer);

  if (s_use_libc_allocator)
    wasm_destroy_script(&script);
  else
    wasm_destroy_stack_allocator(&stack_allocator);

  return result;
}

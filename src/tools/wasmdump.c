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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"
#include "common.h"
#include "option-parser.h"
#include "stream.h"
#include "writer.h"
#include "binary-reader.h"
#include "binary-reader-objdump.h"

#define PROGRAM_NAME "wasmdump"

#define NOPE WASM_OPTION_NO_ARGUMENT
#define YEP WASM_OPTION_HAS_ARGUMENT

enum {
  FLAG_HEADERS,
  FLAG_SECTION,
  FLAG_RAW,
  FLAG_DISASSEMBLE,
  FLAG_DEBUG,
  FLAG_DETAILS,
  FLAG_RELOCS,
  FLAG_HELP,
  NUM_FLAGS
};

static const char s_description[] =
    "  Print information about the contents of a wasm binary file.\n"
    "\n"
    "examples:\n"
    "  $ wasmdump test.wasm\n";

static WasmOption s_options[] = {
    {FLAG_HEADERS, 'h', "headers", NULL, NOPE, "print headers"},
    {FLAG_SECTION, 'j', "section", NULL, YEP, "select just one section"},
    {FLAG_RAW, 's', "full-contents", NULL, NOPE, "print raw section contents"},
    {FLAG_DISASSEMBLE, 'd', "disassemble", NULL, NOPE,
     "disassemble function bodies"},
    {FLAG_DEBUG, '\0', "debug", NULL, NOPE, "disassemble function bodies"},
    {FLAG_DETAILS, 'x', "details", NULL, NOPE, "Show section details"},
    {FLAG_RELOCS, 'r', "reloc", NULL, NOPE,
     "show relocations inline with disassembly"},
    {FLAG_HELP, 'h', "help", NULL, NOPE, "print this help message"},
};

WASM_STATIC_ASSERT(NUM_FLAGS == WASM_ARRAY_SIZE(s_options));

static WasmObjdumpOptions s_objdump_options;

static void on_argument(struct WasmOptionParser* parser, const char* argument) {
  s_objdump_options.infile = argument;
}

static void on_option(struct WasmOptionParser* parser,
                      struct WasmOption* option,
                      const char* argument) {
  switch (option->id) {
    case FLAG_HEADERS:
      s_objdump_options.headers = WASM_TRUE;
      break;

    case FLAG_RAW:
      s_objdump_options.raw = WASM_TRUE;
      break;

    case FLAG_DEBUG:
      s_objdump_options.debug = WASM_TRUE;

    case FLAG_DISASSEMBLE:
      s_objdump_options.disassemble = WASM_TRUE;
      break;

    case FLAG_DETAILS:
      s_objdump_options.details = WASM_TRUE;
      break;

    case FLAG_RELOCS:
      s_objdump_options.relocs = WASM_TRUE;
      break;

    case FLAG_SECTION:
      s_objdump_options.section_name = argument;
      break;

    case FLAG_HELP:
      wasm_print_help(parser, PROGRAM_NAME);
      exit(0);
      break;
  }
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

  if (!s_objdump_options.infile) {
    wasm_print_help(&parser, PROGRAM_NAME);
    WASM_FATAL("No filename given.\n");
  }
}

int main(int argc, char** argv) {
  wasm_init_stdio();

  parse_options(argc, argv);
  if (!s_objdump_options.headers && !s_objdump_options.details && !s_objdump_options.disassemble && !s_objdump_options.raw) {
    fprintf(stderr, "At least one of the following switches must be given:\n");
    fprintf(stderr, " -d/--disassemble\n");
    fprintf(stderr, " -h/--headers\n");
    fprintf(stderr, " -x/--details\n");
    fprintf(stderr, " -s/--full-contents\n");
    return 1;
  }

  WasmAllocator* allocator = &g_wasm_libc_allocator;

  void* data;
  size_t size;
  WasmResult result = wasm_read_file(allocator, s_objdump_options.infile, &data, &size);
  if (WASM_FAILED(result))
    return result;

  // Perform serveral passed over the binary in order to print out different
  // types of information.
  s_objdump_options.print_header = 1;
  if (!s_objdump_options.headers && !s_objdump_options.details &&
      !s_objdump_options.disassemble && !s_objdump_options.raw) {
    printf("At least one of the following switches must be given:\n");
    printf(" -d/--disassemble\n");
    printf(" -h/--headers\n");
    printf(" -x/--details\n");
    printf(" -s/--full-contents\n");
    return 1;
  }

  s_objdump_options.mode = WASM_DUMP_PREPASS;
  result = wasm_read_binary_objdump(allocator, data, size, &s_objdump_options);
  if (WASM_FAILED(result))
    goto done;

  // Pass 1: Print the section headers
  if (s_objdump_options.headers) {
    s_objdump_options.mode = WASM_DUMP_HEADERS;
    result = wasm_read_binary_objdump(allocator, data, size, &s_objdump_options);
    if (WASM_FAILED(result))
      goto done;
    s_objdump_options.print_header = 0;
  }
  // Pass 2: Print extra information based on section type
  if (s_objdump_options.details) {
    s_objdump_options.mode = WASM_DUMP_DETAILS;
    result = wasm_read_binary_objdump(allocator, data, size, &s_objdump_options);
    if (WASM_FAILED(result))
      goto done;
    s_objdump_options.print_header = 0;
  }
  if (s_objdump_options.disassemble) {
    s_objdump_options.mode = WASM_DUMP_DISASSEMBLE;
    result = wasm_read_binary_objdump(allocator, data, size, &s_objdump_options);
    if (WASM_FAILED(result))
      goto done;
    s_objdump_options.print_header = 0;
  }
  // Pass 3: Dump to raw contents of the sections
  if (s_objdump_options.raw) {
    s_objdump_options.mode = WASM_DUMP_RAW_DATA;
    result = wasm_read_binary_objdump(allocator, data, size, &s_objdump_options);
  }

done:
  wasm_free(allocator, data);
  return result;
}

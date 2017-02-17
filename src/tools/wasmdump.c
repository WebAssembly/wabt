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

#include "common.h"
#include "option-parser.h"
#include "stream.h"
#include "writer.h"
#include "binary-reader.h"
#include "binary-reader-objdump.h"

#define PROGRAM_NAME "wasmdump"

#define NOPE WABT_OPTION_NO_ARGUMENT
#define YEP WABT_OPTION_HAS_ARGUMENT

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

static WabtOption s_options[] = {
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

WABT_STATIC_ASSERT(NUM_FLAGS == WABT_ARRAY_SIZE(s_options));

static WabtObjdumpOptions s_objdump_options;

static void on_argument(struct WabtOptionParser* parser, const char* argument) {
  s_objdump_options.infile = argument;
}

static void on_option(struct WabtOptionParser* parser,
                      struct WabtOption* option,
                      const char* argument) {
  switch (option->id) {
    case FLAG_HEADERS:
      s_objdump_options.headers = WABT_TRUE;
      break;

    case FLAG_RAW:
      s_objdump_options.raw = WABT_TRUE;
      break;

    case FLAG_DEBUG:
      s_objdump_options.debug = WABT_TRUE;

    case FLAG_DISASSEMBLE:
      s_objdump_options.disassemble = WABT_TRUE;
      break;

    case FLAG_DETAILS:
      s_objdump_options.details = WABT_TRUE;
      break;

    case FLAG_RELOCS:
      s_objdump_options.relocs = WABT_TRUE;
      break;

    case FLAG_SECTION:
      s_objdump_options.section_name = argument;
      break;

    case FLAG_HELP:
      wabt_print_help(parser, PROGRAM_NAME);
      exit(0);
      break;
  }
}

static void on_option_error(struct WabtOptionParser* parser,
                            const char* message) {
  WABT_FATAL("%s\n", message);
}

static void parse_options(int argc, char** argv) {
  WabtOptionParser parser;
  WABT_ZERO_MEMORY(parser);
  parser.description = s_description;
  parser.options = s_options;
  parser.num_options = WABT_ARRAY_SIZE(s_options);
  parser.on_option = on_option;
  parser.on_argument = on_argument;
  parser.on_error = on_option_error;
  wabt_parse_options(&parser, argc, argv);

  if (!s_objdump_options.infile) {
    wabt_print_help(&parser, PROGRAM_NAME);
    WABT_FATAL("No filename given.\n");
  }
}

int main(int argc, char** argv) {
  wabt_init_stdio();

  parse_options(argc, argv);
  if (!s_objdump_options.headers && !s_objdump_options.details && !s_objdump_options.disassemble && !s_objdump_options.raw) {
    fprintf(stderr, "At least one of the following switches must be given:\n");
    fprintf(stderr, " -d/--disassemble\n");
    fprintf(stderr, " -h/--headers\n");
    fprintf(stderr, " -x/--details\n");
    fprintf(stderr, " -s/--full-contents\n");
    return 1;
  }

  void* data;
  size_t size;
  WabtResult result = wabt_read_file(s_objdump_options.infile, &data, &size);
  if (WABT_FAILED(result))
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

  s_objdump_options.mode = WABT_DUMP_PREPASS;
  result = wabt_read_binary_objdump(data, size, &s_objdump_options);
  if (WABT_FAILED(result))
    goto done;

  // Pass 1: Print the section headers
  if (s_objdump_options.headers) {
    s_objdump_options.mode = WABT_DUMP_HEADERS;
    result = wabt_read_binary_objdump(data, size, &s_objdump_options);
    if (WABT_FAILED(result))
      goto done;
    s_objdump_options.print_header = 0;
  }
  // Pass 2: Print extra information based on section type
  if (s_objdump_options.details) {
    s_objdump_options.mode = WABT_DUMP_DETAILS;
    result = wabt_read_binary_objdump(data, size, &s_objdump_options);
    if (WABT_FAILED(result))
      goto done;
    s_objdump_options.print_header = 0;
  }
  if (s_objdump_options.disassemble) {
    s_objdump_options.mode = WABT_DUMP_DISASSEMBLE;
    result = wabt_read_binary_objdump(data, size, &s_objdump_options);
    if (WABT_FAILED(result))
      goto done;
    s_objdump_options.print_header = 0;
  }
  // Pass 3: Dump to raw contents of the sections
  if (s_objdump_options.raw) {
    s_objdump_options.mode = WABT_DUMP_RAW_DATA;
    result = wabt_read_binary_objdump(data, size, &s_objdump_options);
  }

done:
  wabt_free(data);
  return result;
}

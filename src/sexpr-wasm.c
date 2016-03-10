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

#if HAVE_GETOPT_H
#include <getopt.h>
#else
#define USE_MIN_PARSER 1
#endif

#include "wasm-binary-writer.h"
#include "wasm-check.h"
#include "wasm-internal.h"
#include "wasm-parser.h"
#include "wasm-stack-allocator.h"
#include "wasm-writer.h"

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

static const char* s_infile;
static const char* s_outfile;
static int s_dump_module;
static int s_verbose;
static WasmWriteBinaryOptions s_write_binary_options =
    WASM_WRITE_BINARY_OPTIONS_DEFAULT;
static int s_use_libc_allocator;

#ifdef USE_MIN_PARSER
typedef struct option {
  const char* name;
  int has_arg;
  int* flag;
  int val;
} option;
#define null_argument 0     /*Argument Null*/
#define no_argument 0       /*Argument Switch Only*/
#define required_argument 1 /*Argument Required*/
#define optional_argument 2 /*Argument Optional*/

typedef struct OptionText {
  char long_name[64];
  char short_name[64];
} OptionText;
#endif /* HAVE_GETOPT_H */

static struct option s_long_options[] = {
    {"verbose", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {"dump-module", no_argument, NULL, 'd'},
    {"output", required_argument, NULL, 'o'},
    {"spec", no_argument, NULL, 0},
    {"spec-verbose", no_argument, NULL, 0},
    {"use-libc-allocator", no_argument, NULL, 0},
    {"no-canonicalize-leb128s", no_argument, NULL, 0},
    {"no-remap-locals", no_argument, NULL, 0},
    {"debug-names", no_argument, NULL, 0},
    {NULL, 0, NULL, 0},
};
#define OPTIONS_LENGTH (ARRAY_SIZE(s_long_options) - 1)
STATIC_ASSERT(NUM_FLAGS == OPTIONS_LENGTH);

typedef struct OptionHelp {
  int flag;
  const char* metavar;
  const char* help;
} OptionHelp;

static OptionHelp s_option_help[] = {
    {FLAG_VERBOSE, NULL, "use multiple times for more info"},
    {FLAG_HELP, NULL, "print this help message"},
    {FLAG_DUMP_MODULE, NULL, "print a hexdump of the module to stdout"},
    {FLAG_OUTPUT, "FILENAME", "output file for the generated binary format"},
    {FLAG_SPEC, NULL,
     "parse a file with multiple modules and assertions, like the spec tests"},
    {FLAG_SPEC_VERBOSE, NULL, "print logging messages when running spec files"},
    {FLAG_USE_LIBC_ALLOCATOR, NULL,
     "use malloc, free, etc. instead of stack allocator"},
    {FLAG_NO_CANONICALIZE_LEB128S, NULL,
     "Write all LEB128 sizes as 5-bytes instead of their minimal size"},
    {FLAG_NO_REMAP_LOCALS, NULL,
     "If set, function locals are written in source order, instead of packing "
     "them to reduce size"},
    {FLAG_DEBUG_NAMES, NULL, "Write debug names to the generated binary file"},
    {NUM_FLAGS, NULL},
};
#define OPTIONS_HELP_LENGTH (ARRAY_SIZE(s_option_help) - 1)
STATIC_ASSERT(NUM_FLAGS == OPTIONS_HELP_LENGTH);

static void usage(const char* prog) {
  printf("usage: %s [option] filename\n", prog);
  printf("options:\n");
  struct option* opt = &s_long_options[0];
  int i = 0;
  for (; opt->name; ++i, ++opt) {
    OptionHelp* help = NULL;

    int n = 0;
    while (s_option_help[n].help) {
      if (i == s_option_help[n].flag) {
        help = &s_option_help[n];
        break;
      }
      n++;
    }

    if (opt->val) {
      printf("  -%c, ", opt->val);
    } else {
      printf("      ");
    }

    if (help && help->metavar) {
      char buf[100];
      SNPRINTF(buf, 100, "%s=%s", opt->name, help->metavar);
      printf("--%-30s", buf);
    } else {
      printf("--%-30s", opt->name);
    }

    if (help) {
      printf("%s", help->help);
    }

    printf("\n");
  }
  exit(0);
}

static void parse_options(int argc, char** argv) {
  int c;
  int option_index = 0;
#ifdef USE_MIN_PARSER
  const char* optarg = NULL;
  int optind = 1;
  OptionText options_text[OPTIONS_LENGTH];
  int i = 0;
  for (; i < OPTIONS_LENGTH; i++) {
    struct option* opt = &s_long_options[i];
    sprintf(options_text[i].long_name, "--%s", opt->name);
    if (opt->val) {
      sprintf(options_text[i].short_name, "-%c", (char)opt->val);
    }
  }
#endif /* USE_MIN_PARSER */
  while (1) {
#ifndef USE_MIN_PARSER
    c = getopt_long(argc, argv, "vhdo:", s_long_options, &option_index);
    if (c == -1) {
      break;
    }

#else
    if (optind >= argc) {
      break;
    }
    optarg = NULL;
    option_index = -1;
    const char* in_opt = argv[optind++];
    i = 0;
    for (; i < OPTIONS_LENGTH; i++) {
      const struct OptionText* optText = &options_text[i];
      const struct option* opt = &s_long_options[i];
      if (strcmp(in_opt, optText->long_name) == 0 ||
          (opt->val && strcmp(in_opt, optText->short_name) == 0)) {
        option_index = i;
        c = opt->val;
        if (opt->has_arg != no_argument) {
          if (optind < argc) {
            optarg = argv[optind++];
          } else if (opt->has_arg == required_argument) {
            FATAL("Missing argument for option %s.\n", opt->name);
            usage(argv[0]);
          }
        }
        break;
      }
    }

    // no argument found
    if (option_index == -1) {
      --optind;
      break;
    }
#endif /* USE_MIN_PARSER */

  redo_switch:
    switch (c) {
      case 0:
        c = s_long_options[option_index].val;
        if (c) {
          goto redo_switch;
        }

        switch (option_index) {
          case FLAG_VERBOSE:
          case FLAG_HELP:
          case FLAG_DUMP_MODULE:
          case FLAG_OUTPUT:
            /* Handled above by goto */
            assert(0);
            break;

          case FLAG_SPEC:
            s_write_binary_options.spec = 1;
            break;

          case FLAG_SPEC_VERBOSE:
            s_write_binary_options.spec_verbose = 1;
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
        break;

      case 'v':
        s_verbose++;
        s_write_binary_options.log_writes = 1;
        break;

      case 'h':
        usage(argv[0]);

      case 'd':
        s_dump_module = 1;
        break;

      case 'o':
        s_outfile = optarg;
        break;

      case '?':
        break;

      default:
        FATAL("getopt_long returned '%c' (%d)\n", c, c);
        break;
    }
  }

  if (s_dump_module && s_write_binary_options.spec)
    FATAL("--dump-module flag incompatible with --spec flag\n");

  if (optind < argc) {
    s_infile = argv[optind];
  } else {
    FATAL("No filename given.\n");
    usage(argv[0]);
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
    FATAL("unable to read %s\n", s_infile);

  WasmScript script;
  WasmResult result = wasm_parse(lexer, &script);

  if (result == WASM_OK) {
    result = wasm_check_script(lexer, &script);
    if (result == WASM_OK) {
      WasmMemoryWriter writer;
      ZERO_MEMORY(writer);
      if (wasm_init_mem_writer(&g_wasm_libc_allocator, &writer) != WASM_OK)
        FATAL("unable to open memory writer for writing\n");

      result = wasm_write_binary(&g_wasm_libc_allocator, &writer.base, &script,
                                 &s_write_binary_options);
      if (result == WASM_OK) {
        if (s_dump_module) {
          if (s_verbose)
            printf(";; dump\n");
          wasm_print_memory(writer.buf.start, writer.buf.size, 0, 0, NULL);
        }

        if (s_outfile) {
          FILE* f = fopen(s_outfile, "wb");
          if (!f)
            FATAL("unable to open %s for writing\n", s_outfile);

          ssize_t bytes = fwrite(writer.buf.start, 1, writer.buf.size, f);
          if (bytes != writer.buf.size)
            FATAL("failed to write %zd bytes to %s\n", writer.buf.size,
                  s_outfile);
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

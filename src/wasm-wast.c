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
#include <unistd.h>

#include "wasm-allocator.h"
#include "wasm-ast.h"
#include "wasm-ast-writer.h"
#include "wasm-binary-reader-ast.h"
#include "wasm-stack-allocator.h"
#include "wasm-writer.h"

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  NUM_FLAGS
};

static int s_verbose;
static const char* s_infile;

static struct option s_long_options[] = {
    {"verbose", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0},
};
WASM_STATIC_ASSERT(NUM_FLAGS + 1 == WASM_ARRAY_SIZE(s_long_options));

typedef struct OptionHelp {
  int flag;
  const char* metavar;
  const char* help;
} OptionHelp;

static OptionHelp s_option_help[] = {
    {FLAG_VERBOSE, NULL, "use multiple times for more info"},
    {NUM_FLAGS, NULL},
};

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
      snprintf(buf, 100, "%s=%s", opt->name, help->metavar);
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

  while (1) {
    c = getopt_long(argc, argv, "vh", s_long_options, &option_index);
    if (c == -1) {
      break;
    }

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
            /* Handled above by goto */
            assert(0);
            break;
        }
        break;

      case 'v':
        s_verbose++;
        break;

      case 'h':
        usage(argv[0]);

      case '?':
        break;

      default:
        WASM_FATAL("getopt_long returned '%c' (%d)\n", c, c);
        break;
    }
  }

  if (optind < argc) {
    s_infile = argv[optind];
  } else {
    WASM_FATAL("No filename given.\n");
    usage(argv[0]);
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
    WasmFileWriter file_writer;
    result = wasm_init_file_writer_existing(&file_writer, stdout);
    if (result == WASM_OK) {
      result = wasm_write_ast(&file_writer.base, &module);
      fprintf(stdout, "\n");
      wasm_close_file_writer(&file_writer);
    }
  }

  wasm_destroy_stack_allocator(&stack_allocator);
  return result;
}

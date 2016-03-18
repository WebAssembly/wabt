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
#include "wasm-binary-reader-ast.h"

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

#if 0
#if 0
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOG(...) (void)0
#endif

static void on_error(const char* message, void* user_data) {
#if 0
  LOG("error: %s\n", message);
#else
  fprintf(stderr, "error: %s\n", message);
#endif
}

static void begin_module(void* user_data) {
  LOG("begin_module\n");
}

static void end_module(void* user_data) {
  LOG("end_module\n");
}

static void begin_memory_section(void* user_data) {
  LOG("begin_memory_section\n");
}

static void on_memory_initial_size_pages(uint32_t pages, void* user_data) {
  LOG("on_memory_initial_size(%u)\n", pages);
}

static void on_memory_max_size_pages(uint32_t pages, void* user_data) {
  LOG("on_memory_max_size(%u)\n", pages);
}

static void on_memory_exported(int exported, void* user_data) {
  LOG("on_memory_exported(%d)\n", exported);
}

static void end_memory_section(void* user_data) {
  LOG("end_memory_section\n");
}

static void begin_data_segment_section(void* user_data) {
  LOG("begin_data_segment_section\n");
}

static void on_data_segment_count(uint32_t count, void* user_data) {
  LOG("on_data_segment_count(%u)\n", count);
}

static void on_data_segment(uint32_t index,
                            uint32_t address,
                            const void* data,
                            uint32_t size,
                            void* user_data) {
  LOG("on_data_segment(%u, addr:%u, size:%u)\n", index, address, size);
}

static void end_data_segment_section(void* user_data) {
  LOG("end_data_segment_section\n");
}

static void begin_signature_section(void* user_data) {
  LOG("begin_signature_section\n");
}

static void on_signature_count(uint32_t count, void* user_data) {
  LOG("on_signature_count(%u)\n", count);
}

static void on_signature(uint32_t index,
                         WasmType result_type,
                         uint32_t param_count,
                         WasmType* param_types,
                         void* user_data) {
  LOG("on_signature(%u, %d, %d)\n", index, result_type, param_count);
}

static void end_signature_section(void* user_data) {
  LOG("end_signature_section\n");
}

static void begin_import_section(void* user_data) {
  LOG("begin_import_section\n");
}

static void on_import_count(uint32_t count, void* user_data) {
  LOG("on_import_count(%u)\n", count);
}

static void on_import(uint32_t index,
                      uint32_t sig_index,
                      WasmStringSlice module_name,
                      WasmStringSlice function_name,
                      void* user_data) {
  LOG("on_import(%u, %u, %.*s, %.*s)\n", index, sig_index,
      (int)module_name.length, module_name.start, (int)function_name.length,
      function_name.start);
}

static void end_import_section(void* user_data) {
  LOG("end_import_section\n");
}

static void begin_function_signatures_section(void* user_data) {
  LOG("begin_function_signatures_section\n");
}

static void on_function_signatures_count(uint32_t count, void* user_data) {
  LOG("on_function_signatures_count(%u)\n", count);
}

static void on_function_signature(uint32_t index,
                                  uint32_t sig_index,
                                  void* user_data) {
  LOG("on_function_signature(%u, %u)\n", index, sig_index);
}

static void end_function_signatures_section(void* user_data) {
  LOG("end_function_signatures_section\n");
}

static void begin_function_bodies_section(void* user_data) {
  LOG("begin_function_bodies_section\n");
}

static void end_function_bodies_section(void* user_data) {
  LOG("end_function_bodies_section\n");
}

static void on_function_bodies_count(uint32_t count, void* user_data) {
  LOG("on_function_bodies_count(%u)\n", count);
}

static void begin_function_body(uint32_t index, void* user_data) {
  LOG("begin_function_body(%u)\n", index);
}

static void end_function_body(uint32_t index, void* user_data) {
  LOG("end_function_body(%u)\n", index);
}

static void on_local_decl_count(uint32_t count, void* user_data) {
  LOG("on_local_decl_count(%u)\n", count);
}

static void on_local_decl(uint32_t decl_index,
                          uint32_t count,
                          WasmType type,
                          void* user_data) {
  LOG("on_local_decl(%u, %u, %u)\n", decl_index, count, type);
}

static void on_binary_expr(WasmOpcode opcode, void* user_data) {
  LOG("on_binary_expr(%u)\n", opcode);
}

static void on_block_expr(uint32_t count, void* user_data) {
  LOG("on_block_expr(%u)\n", count);
}

static void on_br_expr(uint32_t depth, void* user_data) {
  LOG("on_br_expr(%u)\n", depth);
}

static void on_br_if_expr(uint32_t depth, void* user_data) {
  LOG("on_br_if_expr(%u)\n", depth);
}

static void on_call_expr(uint32_t func_index, void* user_data) {
  LOG("on_call_expr(%u)\n", func_index);
}

static void on_call_import_expr(uint32_t import_index, void* user_data) {
  LOG("on_call_import_expr(%u)\n", import_index);
}

static void on_call_indirect_expr(uint32_t sig_index, void* user_data) {
  LOG("on_call_indirect_expr(%u)\n", sig_index);
}

static void on_compare_expr(WasmOpcode opcode, void* user_data) {
  LOG("on_compare_expr(%u)\n", opcode);
}

static void on_i32_const_expr(uint32_t value, void* user_data) {
  LOG("on_i32_const_expr(%u)\n", value);
}

static void on_i64_const_expr(uint64_t value, void* user_data) {
  LOG("on_i64_const_expr(%" PRIu64 ")\n", value);
}

static void on_f32_const_expr(uint32_t value_bits, void* user_data) {
  LOG("on_f32_const_expr(%u)\n", value_bits);
}

static void on_f64_const_expr(uint64_t value_bits, void* user_data) {
  LOG("on_f64_const_expr(%" PRId64 ")\n", value_bits);
}

static void on_convert_expr(WasmOpcode opcode, void* user_data) {
  LOG("on_convert_expr(%u)\n", opcode);
}

static void on_get_local_expr(uint32_t local_index, void* user_data) {
  LOG("on_get_local_expr(%u)\n", local_index);
}

static void on_grow_memory_expr(void* user_data) {
  LOG("on_grow_memory_expr\n");
}

static void on_if_expr(void* user_data) {
  LOG("on_if_expr\n");
}

static void on_if_else_expr(void* user_data) {
  LOG("on_if_else_expr\n");
}

static void on_load_expr(WasmOpcode opcode,
                         uint32_t alignment_log2,
                         uint32_t offset,
                         void* user_data) {
  LOG("on_load_expr(%u, alignment_log2:%u, offset:%u)\n", opcode,
      alignment_log2, offset);
}

static void on_loop_expr(uint32_t count, void* user_data) {
  LOG("on_loop_expr(%u)\n", count);
}

static void on_memory_size_expr(void* user_data) {
  LOG("on_memory_size_expr\n");
}

static void on_nop_expr(void* user_data) {
  LOG("on_nop_expr\n");
}

static void on_return_expr(void* user_data) {
  LOG("on_return_expr\n");
}

static void on_select_expr(void* user_data) {
  LOG("on_select_expr\n");
}

static void on_set_local_expr(uint32_t local_index, void* user_data) {
  LOG("on_set_local_expr(%u)\n", local_index);
}

static void on_store_expr(WasmOpcode opcode,
                          uint32_t alignment_log2,
                          uint32_t offset,
                          void* user_data) {
  LOG("on_store_expr(%u, alignment_log2:%d, offset:%u)\n", opcode,
      alignment_log2, offset);
}

static void on_br_table_expr(uint32_t num_targets,
                             uint32_t* target_depths,
                             uint32_t default_target_depth,
                             void* user_data) {
  LOG("on_br_table_expr(%u, %u)\n", num_targets, default_target_depth);
}

static void on_unary_expr(WasmOpcode opcode, void* user_data) {
  LOG("on_unary_expr(%u)\n", opcode);
}

static void on_unreachable_expr(void* user_data) {
  LOG("on_unreachable_expr\n");
}

static void begin_function_table_section(void* user_data) {
  LOG("begin_function_table_section\n");
}

static void on_function_table_count(uint32_t count, void* user_data) {
  LOG("on_function_table_count(%u)\n", count);
}

static void on_function_table_entry(uint32_t index,
                                    uint32_t func_index,
                                    void* user_data) {
  LOG("on_function_table_entry(%u, %u)\n", index, func_index);
}

static void end_function_table_section(void* user_data) {
  LOG("end_function_table_section\n");
}

static void begin_start_section(void* user_data) {
  LOG("begin_start_section\n");
}

static void on_start_function(uint32_t func_index, void* user_data) {
  LOG("on_start_function(%u)\n", func_index);
}

static void end_start_section(void* user_data) {
  LOG("end_start_section\n");
}

static void begin_export_section(void* user_data) {
  LOG("begin_export_section\n");
}

static void on_export_count(uint32_t count, void* user_data) {
  LOG("on_export_count(%u)\n", count);
}

static void on_export(uint32_t index,
                      uint32_t func_index,
                      WasmStringSlice name,
                      void* user_data) {
  LOG("on_export(%u, %u, %.*s)\n", index, func_index, (int)name.length,
      name.start);
}

static void end_export_section(void* user_data) {
  LOG("end_export_section\n");
}

static void begin_names_section(void* user_data) {
  LOG("begin_export_section\n");
}

static void on_function_names_count(uint32_t count, void* user_data) {
  LOG("on_function_names_count(%u)\n", count);
}

static void on_function_name(uint32_t index,
                         WasmStringSlice name,
                         void* user_data) {
  LOG("on_function_names_count(%.*s)\n", (int)name.length, name.start);
}

static void on_local_names_count(uint32_t index,
                                 uint32_t count,
                                 void* user_data) {
  LOG("on_local_names_count(%u, %u)\n", index, count);
}

static void on_local_name(uint32_t func_index,
                          uint32_t local_index,
                          WasmStringSlice name,
                          void* user_data) {
  LOG("on_local_name(%u, %u, %.*s)\n", func_index, local_index,
      (int)name.length, name.start);
}

static void end_names_section(void* user_data) {
  LOG("end_export_section\n");
}

static WasmBinaryReader s_binary_reader = {
  .user_data = NULL,
  .on_error = &on_error,
  .begin_module = &begin_module,
  .end_module = &end_module,

  .begin_memory_section = &begin_memory_section,
  .on_memory_initial_size_pages = &on_memory_initial_size_pages,
  .on_memory_max_size_pages = &on_memory_max_size_pages,
  .on_memory_exported = &on_memory_exported,
  .end_memory_section = &end_memory_section,

  .begin_data_segment_section = &begin_data_segment_section,
  .on_data_segment_count = &on_data_segment_count,
  .on_data_segment = &on_data_segment,
  .end_data_segment_section = &end_data_segment_section,

  .begin_signature_section = &begin_signature_section,
  .on_signature_count = &on_signature_count,
  .on_signature = &on_signature,
  .end_signature_section = &end_signature_section,

  .begin_import_section = &begin_import_section,
  .on_import_count = &on_import_count,
  .on_import = &on_import,
  .end_import_section = &end_import_section,

  .begin_function_signatures_section = &begin_function_signatures_section,
  .on_function_signatures_count = &on_function_signatures_count,
  .on_function_signature = &on_function_signature,
  .end_function_signatures_section = &end_function_signatures_section,

  .begin_function_bodies_section = &begin_function_bodies_section,
  .on_function_bodies_count = &on_function_bodies_count,
  .begin_function_body = &begin_function_body,
  .on_local_decl_count = &on_local_decl_count,
  .on_local_decl = &on_local_decl,
  .on_binary_expr = &on_binary_expr,
  .on_block_expr = &on_block_expr,
  .on_br_expr = &on_br_expr,
  .on_br_if_expr = &on_br_if_expr,
  .on_br_table_expr = &on_br_table_expr,
  .on_call_expr = &on_call_expr,
  .on_call_import_expr = &on_call_import_expr,
  .on_call_indirect_expr = &on_call_indirect_expr,
  .on_compare_expr = &on_compare_expr,
  .on_i32_const_expr = &on_i32_const_expr,
  .on_i64_const_expr = &on_i64_const_expr,
  .on_f32_const_expr = &on_f32_const_expr,
  .on_f64_const_expr = &on_f64_const_expr,
  .on_convert_expr = &on_convert_expr,
  .on_get_local_expr = &on_get_local_expr,
  .on_grow_memory_expr = &on_grow_memory_expr,
  .on_if_expr = &on_if_expr,
  .on_if_else_expr = &on_if_else_expr,
  .on_load_expr = &on_load_expr,
  .on_loop_expr = &on_loop_expr,
  .on_memory_size_expr = &on_memory_size_expr,
  .on_nop_expr = &on_nop_expr,
  .on_return_expr = &on_return_expr,
  .on_select_expr = &on_select_expr,
  .on_set_local_expr = &on_set_local_expr,
  .on_store_expr = &on_store_expr,
  .on_unary_expr = &on_unary_expr,
  .on_unreachable_expr = &on_unreachable_expr,
  .end_function_body = &end_function_body,
  .end_function_bodies_section = &end_function_bodies_section,

  .begin_function_table_section = &begin_function_table_section,
  .on_function_table_count = &on_function_table_count,
  .on_function_table_entry = &on_function_table_entry,
  .end_function_table_section = &end_function_table_section,

  .begin_start_section = &begin_start_section,
  .on_start_function = &on_start_function,
  .end_start_section = &end_start_section,

  .begin_export_section = &begin_export_section,
  .on_export_count = &on_export_count,
  .on_export = &on_export,
  .end_export_section = &end_export_section,

  .begin_names_section = &begin_names_section,
  .on_function_names_count = &on_function_names_count,
  .on_function_name = &on_function_name,
  .on_local_names_count = &on_local_names_count,
  .on_local_name = &on_local_name,
  .end_names_section = &end_names_section,
};
#elif 0
static void on_error(const char* message, void* user_data) {
  fprintf(stderr, "error: %s\n", message);
}

static WasmBinaryReader s_binary_reader = {
  .user_data = NULL,
  .on_error = &on_error,
};
#endif

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

#if 0
  WasmResult result =
      wasm_read_binary(&g_wasm_libc_allocator, addr, length, &s_binary_reader);
#else
  WasmAllocator* allocator = &g_wasm_libc_allocator;
  WasmModule module;
  WASM_ZERO_MEMORY(module);
  WasmResult result = wasm_read_binary_ast(allocator, addr, length, &module);
#endif
  if (result == WASM_ERROR)
    return 1;
  return 0;
}

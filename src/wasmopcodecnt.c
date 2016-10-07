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
#include "wasm-vector-sort.h"

#define PROGRAM_NAME "wasmopcodecnt"

#define ERROR(fmt, ...) \
  fprintf(stderr, "%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)

static int s_verbose;
static const char* s_infile;
static const char* s_outfile;
static size_t s_cutoff = 0;
static const char* s_separator = ": ";

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
  FLAG_CUTOFF,
  FLAG_SEPARATOR,
  NUM_FLAGS
};

static const char s_description[] =
    "  Read a file in the wasm binary format, and count opcode usage for\n"
    "  instructions.\n"
    "\n"
    "examples:\n"
    "  # parse binary file test.wasm and write pcode dist file test.dist\n"
    "  $ wasmopcodecnt test.wasm -o test.dist\n";

static WasmOption s_options[] = {
    {FLAG_VERBOSE,
     'v',
     "verbose",
     NULL,
     NOPE,
     "use multiple times for more info"},
    {FLAG_HELP, 'h', "help", NULL, NOPE, "print this help message"},
    {FLAG_OUTPUT,
     'o',
     "output",
     "FILENAME",
     YEP,
     "output file for the opcode counts, by default use stdout"},
    {FLAG_USE_LIBC_ALLOCATOR,
     0,
     "use-libc-allocator",
     NULL,
     NOPE,
     "use malloc, free, etc. instead of stack allocator"},
    {FLAG_CUTOFF,
     'c',
     "cutoff",
     "N",
     YEP,
     "cutoff for reporting counts less than N"},
    {FLAG_SEPARATOR,
     's',
     "separator",
     "SEPARATOR",
     YEP,
     "Separator text between element and count when reporting counts"}
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

    case FLAG_CUTOFF:
      s_cutoff = atol(argument);
      break;

    case FLAG_SEPARATOR:
      s_separator = argument;
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

WASM_DEFINE_VECTOR_SORT(int_counter, WasmIntCounter);

typedef int (int_counter_lt_fcn)(WasmIntCounter*, WasmIntCounter*);

WASM_DEFINE_VECTOR_SORT(int_pair_counter, WasmIntPairCounter);

typedef int (int_pair_counter_lt_fcn)(WasmIntPairCounter*, WasmIntPairCounter*);

typedef void (*display_name_fcn)(FILE* out, intmax_t value);

static void display_opcode_name(FILE* out, intmax_t opcode) {
  if (opcode >= 0 && opcode < WASM_NUM_OPCODES)
    fprintf(out, "%s", wasm_get_opcode_name(opcode));
  else
    fprintf(out, "?(%" PRIdMAX ")", opcode);
}

static void display_intmax(FILE* out, intmax_t value) {
  fprintf(out, "%" PRIdMAX, value);
}

static void display_int_counter_vector(
    FILE* out, WasmIntCounterVector* vec, display_name_fcn display_fcn,
    const char* opcode_name) {
  size_t i;
  for (i = 0; i < vec->size; ++i) {
    if (vec->data[i].count == 0)
      continue;
    if (opcode_name)
      fprintf(out, "(%s ", opcode_name);
    display_fcn(out, vec->data[i].value);
    if (opcode_name)
      fprintf(out, ")");
    fprintf(out, "%s%" PRIzd "\n", s_separator, vec->data[i].count);
  }
}

static void display_int_pair_counter_vector(
    FILE* out, WasmIntPairCounterVector* vec,
    display_name_fcn display_first_fcn, display_name_fcn display_second_fcn,
    const char* opcode_name) {
  size_t i;
  for (i = 0; i < vec->size; ++i) {
    if (vec->data[i].count == 0)
      continue;
    if (opcode_name)
      fprintf(out, "(%s ", opcode_name);
    display_first_fcn(out, vec->data[i].first);
    fputc(' ', out);
    display_second_fcn(out, vec->data[i].second);
    if (opcode_name)
      fprintf(out, ")");
    fprintf(out, "%s%" PRIzd "\n", s_separator, vec->data[i].count);
  }
}

static void swap_int_counters(WasmIntCounter* v1, WasmIntCounter* v2) {
  WasmIntCounter tmp;
  tmp.value = v1->value;
  tmp.count = v1->count;

  v1->value = v2->value;
  v1->count = v2->count;

  v2->value = tmp.value;
  v2->count = tmp.count;
}

static int opcode_counter_gt(WasmIntCounter* counter_1,
                             WasmIntCounter* counter_2) {
  if (counter_1->count > counter_2->count)
    return 1;
  if (counter_1->count < counter_2->count)
    return 0;
  const char* name_1 = "?1";
  const char* name_2 = "?2";
  if (counter_1->value < WASM_NUM_OPCODES) {
    const char* opcode_name = wasm_get_opcode_name(counter_1->value);
    if (opcode_name)
      name_1 = opcode_name;
  }
  if (counter_2->value < WASM_NUM_OPCODES) {
    const char* opcode_name = wasm_get_opcode_name(counter_2->value);
    if (opcode_name)
      name_2 = opcode_name;
  }
  int diff = strcmp(name_1, name_2);
  if (diff > 0)
    return 1;
  return 0;
}

static int int_counter_gt(WasmIntCounter* counter_1,
                          WasmIntCounter* counter_2) {
  if (counter_1->count < counter_2->count)
    return 0;
  if (counter_1->count > counter_2->count)
    return 1;
  if (counter_1->value < counter_2->value)
    return 0;
  if (counter_1->value > counter_2->value)
    return 1;
  return 0;
}

static void swap_int_pair_counters(WasmIntPairCounter* v1,
                                   WasmIntPairCounter* v2) {
  WasmIntPairCounter tmp;
  tmp.first = v1->first;
  tmp.second = v1->second;
  tmp.count = v1->count;

  v1->first = v2->first;
  v1->second = v2->second;
  v1->count = v2->count;

  v2->first = tmp.first;
  v2->second = tmp.second;
  v2->count = tmp.count;
}

static int int_pair_counter_gt(WasmIntPairCounter* counter_1,
                               WasmIntPairCounter* counter_2) {
  if (counter_1->count < counter_2->count)
    return 0;
  if (counter_1->count > counter_2->count)
    return 1;
  if (counter_1->first < counter_2->first)
    return 0;
  if (counter_1->first > counter_2->first)
    return 1;
  if (counter_1->second < counter_2->second)
    return 0;
  if (counter_1->second > counter_2->second)
    return 1;
  return 0;
}

static void display_sorted_int_counter_vector(
    FILE* out, const char* title, struct WasmAllocator* allocator,
    WasmIntCounterVector* vec, int_counter_lt_fcn lt_fcn,
    display_name_fcn display_fcn, const char* opcode_name) {
  if (vec->size == 0)
    return;

  /* First filter out values less than cutoff. This speeds up sorting. */
  WasmIntCounterVector filtered_vec;
  WASM_ZERO_MEMORY(filtered_vec);
  size_t i;
  for (i = 0; i < vec->size; ++i) {
    if (vec->data[i].count < s_cutoff)
      continue;
    wasm_append_int_counter_value(allocator, &filtered_vec, &vec->data[i]);
  }
  WasmIntCounterVector sorted_vec;
  WASM_ZERO_MEMORY(sorted_vec);
  wasm_sort_int_counter_vector(allocator, &filtered_vec, &sorted_vec,
                               swap_int_counters, lt_fcn);
  fprintf(out, "%s\n", title);
  display_int_counter_vector(out, &sorted_vec, display_fcn, opcode_name);
  wasm_destroy_int_counter_vector(allocator, &filtered_vec);
  wasm_destroy_int_counter_vector(allocator, &sorted_vec);
}

static void display_sorted_int_pair_counter_vector(
    FILE* out, const char* title, struct WasmAllocator* allocator,
    WasmIntPairCounterVector* vec, int_pair_counter_lt_fcn lt_fcn,
    display_name_fcn display_first_fcn, display_name_fcn display_second_fcn,
    const char* opcode_name) {
  if (vec->size == 0)
    return;

  WasmIntPairCounterVector filtered_vec;
  WASM_ZERO_MEMORY(filtered_vec);
  WasmIntPairCounterVector sorted_vec;
  size_t i;
  for (i = 0; i < vec->size; ++i) {
    if (vec->data[i].count < s_cutoff)
      continue;
    wasm_append_int_pair_counter_value(allocator, &filtered_vec, &vec->data[i]);
  }
  WASM_ZERO_MEMORY(sorted_vec);
  wasm_sort_int_pair_counter_vector(allocator, &filtered_vec, &sorted_vec,
                                    swap_int_pair_counters, lt_fcn);
  fprintf(out, "%s\n", title);
  display_int_pair_counter_vector(out, &sorted_vec, display_first_fcn,
                                  display_second_fcn, opcode_name);
  wasm_destroy_int_pair_counter_vector(allocator, &filtered_vec);
  wasm_destroy_int_pair_counter_vector(allocator, &sorted_vec);
}

int main(int argc, char** argv) {

  wasm_init_stdio();
  parse_options(argc, argv);

  WasmStackAllocator stack_allocator;
  WasmAllocator *allocator;
  if (s_use_libc_allocator) {
    allocator = &g_wasm_libc_allocator;
  } else {
    wasm_init_stack_allocator(&stack_allocator, &g_wasm_libc_allocator);
    allocator = &stack_allocator.allocator;
  }


  void* data;
  size_t size;
  WasmResult result = wasm_read_file(allocator, s_infile, &data, &size);
  if (WASM_FAILED(result)) {
    const char* input_name = s_infile ? s_infile : "stdin";
    ERROR("Unable to parse: %s", input_name);
    wasm_free(allocator, data);
    wasm_print_allocator_stats(allocator);
    wasm_destroy_allocator(allocator);
  }
  FILE* out = stdout;
  if (s_outfile) {
    out = fopen(s_outfile, "w");
    if (!out)
      ERROR("fopen \"%s\" failed, errno=%d\n", s_outfile, errno);
    result = WASM_ERROR;
  }
  if (WASM_SUCCEEDED(result)) {
    WasmOpcntData opcnt_data;
    wasm_init_opcnt_data(allocator, &opcnt_data);
    result = wasm_read_binary_opcnt(
        allocator, data, size, &s_read_binary_options, &s_error_handler,
        &opcnt_data);
    if (WASM_SUCCEEDED(result)) {
      display_sorted_int_counter_vector(
          out, "Opcode counts:", allocator, &opcnt_data.opcode_vec,
          opcode_counter_gt, display_opcode_name, NULL);
      display_sorted_int_counter_vector(
          out, "\ni32.const:", allocator, &opcnt_data.i32_const_vec,
          int_counter_gt, display_intmax,
          wasm_get_opcode_name(WASM_OPCODE_I32_CONST));
      display_sorted_int_counter_vector(
          out, "\nget_local:", allocator, &opcnt_data.get_local_vec,
          int_counter_gt, display_intmax,
          wasm_get_opcode_name(WASM_OPCODE_GET_LOCAL));
      display_sorted_int_counter_vector(
          out, "\nset_local:", allocator, &opcnt_data.set_local_vec,
          int_counter_gt, display_intmax,
          wasm_get_opcode_name(WASM_OPCODE_SET_LOCAL));
      display_sorted_int_counter_vector(
          out, "\ntee_local:", allocator, &opcnt_data.tee_local_vec,
          int_counter_gt, display_intmax,
          wasm_get_opcode_name(WASM_OPCODE_TEE_LOCAL));
      display_sorted_int_pair_counter_vector(
          out, "\ni32.load:", allocator, &opcnt_data.i32_load_vec,
          int_pair_counter_gt, display_intmax, display_intmax,
          wasm_get_opcode_name(WASM_OPCODE_I32_LOAD));
      display_sorted_int_pair_counter_vector(
          out, "\ni32.store:", allocator, &opcnt_data.i32_store_vec,
          int_pair_counter_gt, display_intmax, display_intmax,
          wasm_get_opcode_name(WASM_OPCODE_I32_STORE));
    }
    wasm_destroy_opcnt_data(allocator, &opcnt_data);
  }
  wasm_free(allocator, data);
  wasm_print_allocator_stats(allocator);
  wasm_destroy_allocator(allocator);
  return result;
}

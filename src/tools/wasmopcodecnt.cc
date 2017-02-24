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

#include "binary-reader.h"
#include "binary-reader-opcnt.h"
#include "option-parser.h"
#include "stream.h"
#include "vector-sort.h"

#define PROGRAM_NAME "wasmopcodecnt"

#define ERROR(fmt, ...) \
  fprintf(stderr, "%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)

static int s_verbose;
static const char* s_infile;
static const char* s_outfile;
static size_t s_cutoff = 0;
static const char* s_separator = ": ";

static WabtReadBinaryOptions s_read_binary_options =
    WABT_READ_BINARY_OPTIONS_DEFAULT;

static WabtFileWriter s_log_stream_writer;
static WabtStream s_log_stream;

#define NOPE WABT_OPTION_NO_ARGUMENT
#define YEP WABT_OPTION_HAS_ARGUMENT

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  FLAG_OUTPUT,
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

static WabtOption s_options[] = {
    {FLAG_VERBOSE, 'v', "verbose", nullptr, NOPE,
     "use multiple times for more info"},
    {FLAG_HELP, 'h', "help", nullptr, NOPE, "print this help message"},
    {FLAG_OUTPUT, 'o', "output", "FILENAME", YEP,
     "output file for the opcode counts, by default use stdout"},
    {FLAG_CUTOFF, 'c', "cutoff", "N", YEP,
     "cutoff for reporting counts less than N"},
    {FLAG_SEPARATOR, 's', "separator", "SEPARATOR", YEP,
     "Separator text between element and count when reporting counts"}};

WABT_STATIC_ASSERT(NUM_FLAGS == WABT_ARRAY_SIZE(s_options));

static void on_option(struct WabtOptionParser* parser,
                      struct WabtOption* option,
                      const char* argument) {
  switch (option->id) {
    case FLAG_VERBOSE:
      s_verbose++;
      wabt_init_file_writer_existing(&s_log_stream_writer, stdout);
      wabt_init_stream(&s_log_stream, &s_log_stream_writer.base, nullptr);
      s_read_binary_options.log_stream = &s_log_stream;
      break;

    case FLAG_HELP:
      wabt_print_help(parser, PROGRAM_NAME);
      exit(0);
      break;

    case FLAG_OUTPUT:
      s_outfile = argument;
      break;

    case FLAG_CUTOFF:
      s_cutoff = atol(argument);
      break;

    case FLAG_SEPARATOR:
      s_separator = argument;
      break;
  }
}

static void on_argument(struct WabtOptionParser* parser, const char* argument) {
  s_infile = argument;
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

  if (!s_infile) {
    wabt_print_help(&parser, PROGRAM_NAME);
    WABT_FATAL("No filename given.\n");
  }
}

WABT_DEFINE_VECTOR_SORT(int_counter, WabtIntCounter);

typedef int (int_counter_lt_fcn)(WabtIntCounter*, WabtIntCounter*);

WABT_DEFINE_VECTOR_SORT(int_pair_counter, WabtIntPairCounter);

typedef int (int_pair_counter_lt_fcn)(WabtIntPairCounter*, WabtIntPairCounter*);

typedef void (*display_name_fcn)(FILE* out, intmax_t value);

static void display_opcode_name(FILE* out, intmax_t opcode) {
  if (opcode >= 0 && opcode < WABT_NUM_OPCODES)
    fprintf(out, "%s", wabt_get_opcode_name((WabtOpcode)opcode));
  else
    fprintf(out, "?(%" PRIdMAX ")", opcode);
}

static void display_intmax(FILE* out, intmax_t value) {
  fprintf(out, "%" PRIdMAX, value);
}

static void display_int_counter_vector(
    FILE* out, WabtIntCounterVector* vec, display_name_fcn display_fcn,
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
    FILE* out, WabtIntPairCounterVector* vec,
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

static void swap_int_counters(WabtIntCounter* v1, WabtIntCounter* v2) {
  WabtIntCounter tmp;
  tmp.value = v1->value;
  tmp.count = v1->count;

  v1->value = v2->value;
  v1->count = v2->count;

  v2->value = tmp.value;
  v2->count = tmp.count;
}

static int opcode_counter_gt(WabtIntCounter* counter_1,
                             WabtIntCounter* counter_2) {
  if (counter_1->count > counter_2->count)
    return 1;
  if (counter_1->count < counter_2->count)
    return 0;
  const char* name_1 = "?1";
  const char* name_2 = "?2";
  if (counter_1->value < WABT_NUM_OPCODES) {
    const char* opcode_name =
        wabt_get_opcode_name((WabtOpcode)counter_1->value);
    if (opcode_name)
      name_1 = opcode_name;
  }
  if (counter_2->value < WABT_NUM_OPCODES) {
    const char* opcode_name =
        wabt_get_opcode_name((WabtOpcode)counter_2->value);
    if (opcode_name)
      name_2 = opcode_name;
  }
  int diff = strcmp(name_1, name_2);
  if (diff > 0)
    return 1;
  return 0;
}

static int int_counter_gt(WabtIntCounter* counter_1,
                          WabtIntCounter* counter_2) {
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

static void swap_int_pair_counters(WabtIntPairCounter* v1,
                                   WabtIntPairCounter* v2) {
  WabtIntPairCounter tmp;
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

static int int_pair_counter_gt(WabtIntPairCounter* counter_1,
                               WabtIntPairCounter* counter_2) {
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

static void display_sorted_int_counter_vector(FILE* out,
                                              const char* title,
                                              WabtIntCounterVector* vec,
                                              int_counter_lt_fcn lt_fcn,
                                              display_name_fcn display_fcn,
                                              const char* opcode_name) {
  if (vec->size == 0)
    return;

  /* First filter out values less than cutoff. This speeds up sorting. */
  WabtIntCounterVector filtered_vec;
  WABT_ZERO_MEMORY(filtered_vec);
  size_t i;
  for (i = 0; i < vec->size; ++i) {
    if (vec->data[i].count < s_cutoff)
      continue;
    wabt_append_int_counter_value(&filtered_vec, &vec->data[i]);
  }
  WabtIntCounterVector sorted_vec;
  WABT_ZERO_MEMORY(sorted_vec);
  wabt_sort_int_counter_vector(&filtered_vec, &sorted_vec,
                               swap_int_counters, lt_fcn);
  fprintf(out, "%s\n", title);
  display_int_counter_vector(out, &sorted_vec, display_fcn, opcode_name);
  wabt_destroy_int_counter_vector(&filtered_vec);
  wabt_destroy_int_counter_vector(&sorted_vec);
}

static void display_sorted_int_pair_counter_vector(
    FILE* out,
    const char* title,
    WabtIntPairCounterVector* vec,
    int_pair_counter_lt_fcn lt_fcn,
    display_name_fcn display_first_fcn,
    display_name_fcn display_second_fcn,
    const char* opcode_name) {
  if (vec->size == 0)
    return;

  WabtIntPairCounterVector filtered_vec;
  WABT_ZERO_MEMORY(filtered_vec);
  WabtIntPairCounterVector sorted_vec;
  size_t i;
  for (i = 0; i < vec->size; ++i) {
    if (vec->data[i].count < s_cutoff)
      continue;
    wabt_append_int_pair_counter_value(&filtered_vec, &vec->data[i]);
  }
  WABT_ZERO_MEMORY(sorted_vec);
  wabt_sort_int_pair_counter_vector(&filtered_vec, &sorted_vec,
                                    swap_int_pair_counters, lt_fcn);
  fprintf(out, "%s\n", title);
  display_int_pair_counter_vector(out, &sorted_vec, display_first_fcn,
                                  display_second_fcn, opcode_name);
  wabt_destroy_int_pair_counter_vector(&filtered_vec);
  wabt_destroy_int_pair_counter_vector(&sorted_vec);
}

int main(int argc, char** argv) {

  wabt_init_stdio();
  parse_options(argc, argv);

  void* data;
  size_t size;
  WabtResult result = wabt_read_file(s_infile, &data, &size);
  if (WABT_FAILED(result)) {
    const char* input_name = s_infile ? s_infile : "stdin";
    ERROR("Unable to parse: %s", input_name);
    wabt_free(data);
  }
  FILE* out = stdout;
  if (s_outfile) {
    out = fopen(s_outfile, "w");
    if (!out)
      ERROR("fopen \"%s\" failed, errno=%d\n", s_outfile, errno);
    result = WABT_ERROR;
  }
  if (WABT_SUCCEEDED(result)) {
    WabtOpcntData opcnt_data;
    wabt_init_opcnt_data(&opcnt_data);
    result =
        wabt_read_binary_opcnt(data, size, &s_read_binary_options, &opcnt_data);
    if (WABT_SUCCEEDED(result)) {
      display_sorted_int_counter_vector(
          out, "Opcode counts:", &opcnt_data.opcode_vec, opcode_counter_gt,
          display_opcode_name, nullptr);
      display_sorted_int_counter_vector(
          out, "\ni32.const:", &opcnt_data.i32_const_vec, int_counter_gt,
          display_intmax, wabt_get_opcode_name(WABT_OPCODE_I32_CONST));
      display_sorted_int_counter_vector(
          out, "\nget_local:", &opcnt_data.get_local_vec, int_counter_gt,
          display_intmax, wabt_get_opcode_name(WABT_OPCODE_GET_LOCAL));
      display_sorted_int_counter_vector(
          out, "\nset_local:", &opcnt_data.set_local_vec, int_counter_gt,
          display_intmax, wabt_get_opcode_name(WABT_OPCODE_SET_LOCAL));
      display_sorted_int_counter_vector(
          out, "\ntee_local:", &opcnt_data.tee_local_vec, int_counter_gt,
          display_intmax, wabt_get_opcode_name(WABT_OPCODE_TEE_LOCAL));
      display_sorted_int_pair_counter_vector(
          out, "\ni32.load:", &opcnt_data.i32_load_vec, int_pair_counter_gt,
          display_intmax, display_intmax,
          wabt_get_opcode_name(WABT_OPCODE_I32_LOAD));
      display_sorted_int_pair_counter_vector(
          out, "\ni32.store:", &opcnt_data.i32_store_vec, int_pair_counter_gt,
          display_intmax, display_intmax,
          wabt_get_opcode_name(WABT_OPCODE_I32_STORE));
    }
    wabt_destroy_opcnt_data(&opcnt_data);
  }
  wabt_free(data);
  return result;
}

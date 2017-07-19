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

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>

#include "binary-reader.h"
#include "binary-reader-opcnt.h"
#include "option-parser.h"
#include "stream.h"

#define ERROR(fmt, ...) \
  fprintf(stderr, "%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)

using namespace wabt;

static int s_verbose;
static const char* s_infile;
static const char* s_outfile;
static size_t s_cutoff = 0;
static const char* s_separator = ": ";

static ReadBinaryOptions s_read_binary_options;
static std::unique_ptr<FileStream> s_log_stream;

static const char s_description[] =
R"(  Read a file in the wasm binary format, and count opcode usage for
  instructions.

examples:
  # parse binary file test.wasm and write pcode dist file test.dist
  $ wasm-opcodecnt test.wasm -o test.dist
)";

static void ParseOptions(int argc, char** argv) {
  OptionParser parser("wasm-opcodecnt", s_description);

  parser.AddOption('v', "verbose", "Use multiple times for more info", []() {
    s_verbose++;
    s_log_stream = FileStream::CreateStdout();
    s_read_binary_options.log_stream = s_log_stream.get();
  });
  parser.AddOption('h', "help", "Print this help message", [&parser]() {
    parser.PrintHelp();
    exit(0);
  });
  parser.AddOption('o', "output", "FILENAME",
                   "Output file for the opcode counts, by default use stdout",
                   [](const char* argument) { s_outfile = argument; });
  parser.AddOption(
      'c', "cutoff", "N", "Cutoff for reporting counts less than N",
      [](const std::string& argument) { s_cutoff = atol(argument.c_str()); });
  parser.AddOption(
      's', "separator", "SEPARATOR",
      "Separator text between element and count when reporting counts",
      [](const char* argument) { s_separator = argument; });
  parser.AddArgument("filename", OptionParser::ArgumentCount::OneOrMore,
                     [](const char* argument) { s_infile = argument; });
  parser.Parse(argc, argv);
}

typedef int(int_counter_lt_fcn)(const IntCounter&, const IntCounter&);

typedef int(int_pair_counter_lt_fcn)(const IntPairCounter&,
                                     const IntPairCounter&);

typedef void (*display_name_fcn)(FILE* out, intmax_t value);

static void DisplayOpcodeName(FILE* out, intmax_t opcode) {
  fprintf(out, "%s", Opcode(Opcode::Enum(opcode)).GetName());
}

static void DisplayIntmax(FILE* out, intmax_t value) {
  fprintf(out, "%" PRIdMAX, value);
}

static void DisplayIntCounterVector(FILE* out,
                                    const IntCounterVector& vec,
                                    display_name_fcn display_fcn,
                                    const char* opcode_name) {
  for (const IntCounter& counter : vec) {
    if (counter.count == 0)
      continue;
    if (opcode_name)
      fprintf(out, "(%s ", opcode_name);
    display_fcn(out, counter.value);
    if (opcode_name)
      fprintf(out, ")");
    fprintf(out, "%s%" PRIzd "\n", s_separator, counter.count);
  }
}

static void DisplayIntPairCounterVector(FILE* out,
                                        const IntPairCounterVector& vec,
                                        display_name_fcn display_first_fcn,
                                        display_name_fcn display_second_fcn,
                                        const char* opcode_name) {
  for (const IntPairCounter& pair : vec) {
    if (pair.count == 0)
      continue;
    if (opcode_name)
      fprintf(out, "(%s ", opcode_name);
    display_first_fcn(out, pair.first);
    fputc(' ', out);
    display_second_fcn(out, pair.second);
    if (opcode_name)
      fprintf(out, ")");
    fprintf(out, "%s%" PRIzd "\n", s_separator, pair.count);
  }
}

static int OpcodeCounterGt(const IntCounter& counter_1,
                           const IntCounter& counter_2) {
  if (counter_1.count > counter_2.count)
    return 1;
  if (counter_1.count < counter_2.count)
    return 0;
  const char* name_1 = Opcode(Opcode::Enum(counter_1.value)).GetName();
  const char* name_2 = Opcode(Opcode::Enum(counter_2.value)).GetName();
  int diff = strcmp(name_1, name_2);
  if (diff > 0)
    return 1;
  return 0;
}

static int IntCounterGt(const IntCounter& counter_1,
                        const IntCounter& counter_2) {
  if (counter_1.count < counter_2.count)
    return 0;
  if (counter_1.count > counter_2.count)
    return 1;
  if (counter_1.value < counter_2.value)
    return 0;
  if (counter_1.value > counter_2.value)
    return 1;
  return 0;
}

static int IntPairCounterGt(const IntPairCounter& counter_1,
                            const IntPairCounter& counter_2) {
  if (counter_1.count < counter_2.count)
    return 0;
  if (counter_1.count > counter_2.count)
    return 1;
  if (counter_1.first < counter_2.first)
    return 0;
  if (counter_1.first > counter_2.first)
    return 1;
  if (counter_1.second < counter_2.second)
    return 0;
  if (counter_1.second > counter_2.second)
    return 1;
  return 0;
}

static void DisplaySortedIntCounterVector(FILE* out,
                                          const char* title,
                                          const IntCounterVector& vec,
                                          int_counter_lt_fcn lt_fcn,
                                          display_name_fcn display_fcn,
                                          const char* opcode_name) {
  if (vec.size() == 0)
    return;

  /* First filter out values less than cutoff. This speeds up sorting. */
  IntCounterVector filtered_vec;
  for (const IntCounter& counter: vec) {
    if (counter.count < s_cutoff)
      continue;
    filtered_vec.push_back(counter);
  }
  std::sort(filtered_vec.begin(), filtered_vec.end(), lt_fcn);
  fprintf(out, "%s\n", title);
  DisplayIntCounterVector(out, filtered_vec, display_fcn, opcode_name);
}

static void DisplaySortedIntPairCounterVector(
    FILE* out,
    const char* title,
    const IntPairCounterVector& vec,
    int_pair_counter_lt_fcn lt_fcn,
    display_name_fcn display_first_fcn,
    display_name_fcn display_second_fcn,
    const char* opcode_name) {
  if (vec.size() == 0)
    return;

  IntPairCounterVector filtered_vec;
  for (const IntPairCounter& pair : vec) {
    if (pair.count < s_cutoff)
      continue;
    filtered_vec.push_back(pair);
  }
  std::sort(filtered_vec.begin(), filtered_vec.end(), lt_fcn);
  fprintf(out, "%s\n", title);
  DisplayIntPairCounterVector(out, filtered_vec, display_first_fcn,
                              display_second_fcn, opcode_name);
}

int ProgramMain(int argc, char** argv) {
  InitStdio();
  ParseOptions(argc, argv);

  std::vector<uint8_t> file_data;
  Result result = ReadFile(s_infile, &file_data);
  if (Failed(result)) {
    const char* input_name = s_infile ? s_infile : "stdin";
    ERROR("Unable to parse: %s", input_name);
  }
  FILE* out = stdout;
  if (s_outfile) {
    out = fopen(s_outfile, "w");
    if (!out)
      ERROR("fopen \"%s\" failed, errno=%d\n", s_outfile, errno);
    result = Result::Error;
  }
  if (Succeeded(result)) {
    OpcntData opcnt_data;
    result = ReadBinaryOpcnt(DataOrNull(file_data), file_data.size(),
                             &s_read_binary_options, &opcnt_data);
    if (Succeeded(result)) {
      DisplaySortedIntCounterVector(
          out, "Opcode counts:", opcnt_data.opcode_vec, OpcodeCounterGt,
          DisplayOpcodeName, nullptr);
      DisplaySortedIntCounterVector(
          out, "\ni32.const:", opcnt_data.i32_const_vec, IntCounterGt,
          DisplayIntmax, Opcode::I32Const_Opcode.GetName());
      DisplaySortedIntCounterVector(
          out, "\nget_local:", opcnt_data.get_local_vec, IntCounterGt,
          DisplayIntmax, Opcode::GetLocal_Opcode.GetName());
      DisplaySortedIntCounterVector(
          out, "\nset_local:", opcnt_data.set_local_vec, IntCounterGt,
          DisplayIntmax, Opcode::SetLocal_Opcode.GetName());
      DisplaySortedIntCounterVector(
          out, "\ntee_local:", opcnt_data.tee_local_vec, IntCounterGt,
          DisplayIntmax, Opcode::TeeLocal_Opcode.GetName());
      DisplaySortedIntPairCounterVector(
          out, "\ni32.load:", opcnt_data.i32_load_vec, IntPairCounterGt,
          DisplayIntmax, DisplayIntmax, Opcode::I32Load_Opcode.GetName());
      DisplaySortedIntPairCounterVector(
          out, "\ni32.store:", opcnt_data.i32_store_vec, IntPairCounterGt,
          DisplayIntmax, DisplayIntmax, Opcode::I32Store_Opcode.GetName());
    }
  }
  return result != Result::Ok;
}

int main(int argc, char** argv) {
  WABT_TRY
  return ProgramMain(argc, argv);
  WABT_CATCH_BAD_ALLOC_AND_EXIT
}

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

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "wabt/config.h"

#include "wabt/binary-reader-ir.h"
#include "wabt/binary-reader.h"
#include "wabt/binary-writer.h"
#include "wabt/binary.h"
#include "wabt/common.h"
#include "wabt/error-formatter.h"
#include "wabt/filenames.h"
#include "wabt/option-parser.h"
#include "wabt/stream.h"
#include "wabt/wast-parser.h"
#include "wabt/wat-writer.h"

using namespace wabt;

static const char* s_infile;
static std::string s_outfile;
static bool s_read_debug_names = true;
static bool s_fail_on_custom_section_error = true;
static int s_verbose;
static WriteBinaryOptions s_write_binary_options;
static Features s_features;

static std::unique_ptr<FileStream> s_log_stream;

static const char s_description[] =
    R"(  read a component file in the wasm text or binary format,
  check it for errors, and convert it to the other format.

examples:
  ...
)";

static void ParseOptions(int argc, char* argv[]) {
  OptionParser parser("component", s_description);

  parser.AddOption('v', "verbose", "Use multiple times for more info", []() {
    s_verbose++;
    s_log_stream = FileStream::CreateStderr();
  });

  s_features.AddOptions(&parser);
  parser.AddOption('o', "output", "FILE",
                   "Output wasm binary file. Use \"-\" to write to stdout.",
                   [](const char* argument) { s_outfile = argument; });
  parser.AddArgument("filename", OptionParser::ArgumentCount::One,
                     [](const char* argument) { s_infile = argument; });

  parser.Parse(argc, argv);
}

static void WriteBufferToFile(std::string_view filename,
                              const OutputBuffer& buffer) {
  if (filename == "-") {
    buffer.WriteToStdout();
  } else {
    buffer.WriteToFile(filename);
  }
}

static std::string DefaultOuputName(std::string_view input_name) {
  // Strip existing extension and add .wasm
  std::string result(StripExtension(GetBasename(input_name)));
  result += kWasmExtension;

  return result;
}

#define MAGIC(i) ((WABT_BINARY_MAGIC >> ((i) * 8)) & 0xff)

int ProgramMain(int argc, char** argv) {
  InitStdio();

  ParseOptions(argc, argv);

  std::vector<uint8_t> file_data;
  Result result = ReadFile(s_infile, &file_data);
  Errors errors;
  if (Failed(result)) {
    WABT_FATAL("unable to read file: %s\n", s_infile);
  }

  if (file_data.size() > 8 && file_data[0] == MAGIC(0) &&
      file_data[1] == MAGIC(1) && file_data[2] == MAGIC(2) &&
      file_data[3] == MAGIC(3)) {
    // Binary format.
    Component component(nullptr);
    const bool kStopOnFirstError = true;
    ReadBinaryOptions options(s_features, s_log_stream.get(),
                              s_read_debug_names, kStopOnFirstError,
                              s_fail_on_custom_section_error);

    result = ReadBinaryComponentIr(s_infile, file_data.data(), file_data.size(),
                                   options, &errors, &component);
    if (Succeeded(result)) {
      WriteWatOptions wat_options(s_features);
      wat_options.fold_exprs = true;
      wat_options.inline_import = true;
      wat_options.inline_export = true;
      FileStream stream(!s_outfile.empty() ? FileStream(s_outfile)
                                           : FileStream(stdout));
      result = WriteComponentWat(&stream, &component, wat_options);
    }
  } else {
    // Text format.
    std::unique_ptr<Component> component;
    std::unique_ptr<WastLexer> lexer = WastLexer::CreateBufferLexer(
        s_infile, file_data.data(), file_data.size(), &errors);

    WastParseOptions parse_wast_options(s_features);
    result = ParseWatComponent(lexer.get(), &component, &errors,
                               &parse_wast_options);

    if (Succeeded(result)) {
      MemoryStream stream(s_log_stream.get());
      s_write_binary_options.features = s_features;
      result = WriteBinaryComponent(&stream, component.get(),
                                    s_write_binary_options);

      if (Succeeded(result)) {
        if (s_outfile.empty()) {
          s_outfile = DefaultOuputName(s_infile);
        }
        WriteBufferToFile(s_outfile.c_str(), stream.output_buffer());
      }
    }

    auto line_finder = lexer->MakeLineFinder();
    FormatErrorsToFile(errors, Location::Type::Text, line_finder.get());
  }

  return result != Result::Ok;
}

int main(int argc, char** argv) {
  WABT_TRY
  return ProgramMain(argc, argv);
  WABT_CATCH_BAD_ALLOC_AND_EXIT
}

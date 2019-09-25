/*
 * Copyright 2019 WebAssembly Community Group participants
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

#include <cstdio>
#include <cstdlib>

#include <string>
#include <vector>

#include "src/binary-reader.h"
#include "src/binary-reader-ir.h"
#include "src/binary-writer.h"
#include "src/common.h"
#include "src/error.h"
#include "src/error-formatter.h"
#include "src/feature.h"
#include "src/ir.h"
#include "src/option-parser.h"
#include "src/validator.h"

using namespace wabt;

static WriteBinaryOptions s_write_binary_options;

static bool TryModule(string_view scriptfile_name,
                      const Module* module) {
  // TODO: enforced timeouts.
  // TODO: unique temp filename selection so multiple bugpoints can run.
  {
    // TODO: FileWriter::MoveData not implemented!
    // FileStream stream("wasm-bugpoint.tmp.wasm");
    MemoryStream stream;
    Result result = WriteBinaryModule(&stream, module,
                                      s_write_binary_options);
    if (Failed(result)) {
      abort();
    }
    stream.WriteToFile("wasm-bugpoint.tmp.wasm");
  }
  int interesting =
    system((scriptfile_name.to_string() + " wasm-bugpoint.tmp.wasm").c_str());
  remove("wasm-bugpoint.tmp.wasm");
  return interesting != 0;
}

static void AbsolutePath(std::string* path) {
  if (char *abs_path = realpath(path->c_str(), nullptr)) {
    *path = std::string(abs_path);
    free(abs_path);
  }
}

static int ProgramMain(int argc, char** argv) {
  InitStdio();

  std::string outfile = "wasm-bugpoint.wasm";
  std::string scriptfile;
  std::string wasmfile;
  Features features;
  {
    const char s_description[] =
      "  WebAssembly testcase reducer.\n"
      "\n"
      "  Given a script that determines whether a file in WebAssembly binary\n"
      "  format is interesting, indicated by returning exit code 0, and an\n"
      "  interesting file, we attempt to produce a smaller variation of the\n"
      "  file which is still interesting.\n"
      "\n"
      "example:\n"
      "  $ wasm-bugpoint delta.sh test.wasm\n";
    OptionParser parser("wasm-bugpoint", s_description);
    parser.AddHelpOption();
    parser.AddOption(
        'o', "output", "FILENAME",
        "Output file for the reduced testcase, defaults to \"wasm-bugpoint.wasm\"",
        [&](const char* argument) {
          outfile = argument;
          ConvertBackslashToSlash(&outfile);
        });
    features.AddOptions(&parser);
    parser.AddArgument("scriptfilename", OptionParser::ArgumentCount::One,
                       [&](const char* argument) {
                         scriptfile = argument;
                         ConvertBackslashToSlash(&scriptfile);
                         AbsolutePath(&scriptfile);
                       });
    parser.AddArgument("wasmfilename", OptionParser::ArgumentCount::One,
                       [&](const char* argument) {
                         wasmfile = argument;
                         ConvertBackslashToSlash(&wasmfile);
                       });
    parser.Parse(argc, argv);
  }

  s_write_binary_options.write_debug_names = true;
  s_write_binary_options.features = features;

  std::vector<uint8_t> wasmfile_data;
  Result result = ReadFile(wasmfile.c_str(), &wasmfile_data);
  if (Succeeded(result)) {
    Errors errors;
    Module module;
    ReadBinaryOptions options(features, nullptr, true, true, false);
    result = ReadBinaryIr(wasmfile.c_str(), wasmfile_data.data(),
                          wasmfile_data.size(), options, &errors, &module);
    if (Succeeded(result)) {
      ValidateOptions options(features);
      result = ValidateModule(&module, &errors, options);
    }
    if (Succeeded(result)) {
      if (TryModule(scriptfile, &module)) {
        printf("A\n");
      } else {
        printf("B\n");
      }
    }
    FormatErrorsToFile(errors, Location::Type::Binary);
  }
  return result != Result::Ok;
}

int main(int argc, char** argv) {
  WABT_TRY
  return ProgramMain(argc, argv);
  WABT_CATCH_BAD_ALLOC_AND_EXIT
}

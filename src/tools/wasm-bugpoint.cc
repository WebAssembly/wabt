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

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
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
#include "src/make-unique.h"
#include "src/option-parser.h"
#include "src/validator.h"

using namespace wabt;

static WriteBinaryOptions s_write_binary_options;

class Bisect {
public:
  explicit Bisect(unsigned length)
    : length_(length), pos_(0), window_(length / 2), remainder_(0) {
    if (window_ != 0) {
      remainder_ = length_ % window_;
    }
  }

  static constexpr std::pair<unsigned, unsigned> kEndBisect = std::make_pair(0, 0);

  // Returns a pair of values <start, end+1> where start < end+1 and
  // end+1 <= length. It will return approx-evenly spaced ranges over
  // [0..length), and each time it reaches the end, it will return back to zero
  // and use smaller ranges. It continues until the ranges are size one each,
  // after which point it returns kEndBisect.
  std::pair<unsigned, unsigned> Next() {
    if (pos_ >= length_) {
      pos_ = 0;
      window_ /= 2;
      if (window_ != 0) {
        remainder_ = length_ % window_;
      }
    }

    if (window_ == 0) {
      return kEndBisect;
    }

    unsigned end = pos_ + window_ + remainder_;
    if (remainder_ != 0) {
      --remainder_;
    }
    auto range = std::make_pair(pos_, end);
    pos_ = end;
    return range;
  }

  bool ShouldConsiderInverse() const {
    return window_ < length_ / 2;
  }
  
  // Shrink this Bisect down to the range last returned by Next() and restart
  // at index zero.
  void Keep() {
    assert(window_ != 0);
    length_ = window_;
    window_ = std::min(window_, length_ / 2);
    pos_ = 0;
    remainder_ = 0;
    if (window_ != 0) {
      remainder_ = length_ % window_;
    }
  }

  // Shrink this Bisect by removing the range last returned by Next() and
  // restart at index zero.
  void KeepInverse() {
    assert(window_ != 0);
    length_ -= window_;
    window_ = std::min(window_, length_ / 2);
    pos_ = 0;
    remainder_ = 0;
    if (window_ != 0) {
      remainder_ = length_ % window_;
    }
  }

private:
  unsigned length_;
  unsigned pos_;
  unsigned window_;
  unsigned remainder_;
};
constexpr std::pair<unsigned, unsigned> Bisect::kEndBisect;

static Module CopyModule(const Module* orig) {
  // TODO: build a copy without involving streams
  MemoryStream stream;
  Result result = WriteBinaryModule(&stream, orig, s_write_binary_options);
  if (Failed(result)) {
    abort();
  }

  Errors errors;
  Module copy;
  Features features;
  features.EnableAll();
  ReadBinaryOptions options(features, nullptr, true, true, false);
  result = ReadBinaryIr("memory stream", stream.output_buffer().data.data(),
                        stream.output_buffer().size(), options, &errors, &copy);
  if (Failed(result)) {
    abort();
  }
  return copy;
}

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
      if (!TryModule(scriptfile, &module)) {
        // TODO: inform the user. This can happen because the parse and
        // reserialization removed the interestingness.
        abort();
      }
      Bisect bisect(module.funcs.size());

      // Instead of deleting the functions, we remove their expressions,
      // replacing them with constants as needed to meet the function signature.
      // Because they aren't deleted, we need to map the contiguous space of
      // not-deleted functions to the function indexes in the Module.
      std::vector<int> func_map;
      func_map.reserve(module.funcs.size());
      for (int i = 0, e = module.funcs.size(); i != e; ++i) {
        func_map.push_back(i);
      }

      do {
        Module copy = CopyModule(&module);
        auto range = bisect.Next();
        if (range == Bisect::kEndBisect) {
          break;
        }
        printf("[%u, %u]\n", func_map[range.first], func_map[range.second-1]);
        for (unsigned i = 0, e = func_map.size(); i != e; ++i) {
          if (i == range.first) {
            i = range.second - 1;
            continue;
          }
          assert(i < func_map.size());
          unsigned fi = func_map[i];
          copy.funcs[fi]->exprs = ExprList();
          for (int j = 0, je = copy.funcs[fi]->GetNumResults(); j != je; ++j) {
            switch (copy.funcs[fi]->GetResultType(j)) {
            case Type::I32:
              copy.funcs[fi]->exprs.push_back(MakeUnique<ConstExpr>(Const::I32()));
              break;
            case Type::I64:
              copy.funcs[fi]->exprs.push_back(MakeUnique<ConstExpr>(Const::I64()));
              break;
            case Type::F32:
              copy.funcs[fi]->exprs.push_back(MakeUnique<ConstExpr>(Const::F32()));
              break;
            case Type::F64:
              copy.funcs[fi]->exprs.push_back(MakeUnique<ConstExpr>(Const::F64()));
              break;
            case Type::V128:
              copy.funcs[fi]->exprs.push_back(MakeUnique<ConstExpr>(Const::V128({0, 0, 0, 0})));
              break;
            default:
              abort();
            }
          }
        }
        if (TryModule(scriptfile, &copy)) {
          unsigned window = range.second - range.first;
          std::vector<int> new_func_map;
          new_func_map.reserve(window);
          for (unsigned i = 0; i < window; ++i) {
            new_func_map.push_back(func_map[range.first + i]);
          }
          std::swap(func_map, new_func_map);

          bisect.Keep();
          module = CopyModule(&copy);
        }
      } while (1);

      // Reduction complete.
      {
        MemoryStream stream;
        Result result = WriteBinaryModule(&stream, &module,
                                          s_write_binary_options);
        if (Failed(result)) {
          abort();
        }
        stream.WriteToFile(outfile.c_str());
        printf("Reduced testcase written out to %s\n", outfile.c_str());
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

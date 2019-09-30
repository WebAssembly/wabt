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

#include "src/binary-reader-ir.h"
#include "src/binary-reader.h"
#include "src/binary-writer.h"
#include "src/cast.h"
#include "src/common.h"
#include "src/error-formatter.h"
#include "src/error.h"
#include "src/feature.h"
#include "src/ir.h"
#include "src/make-unique.h"
#include "src/option-parser.h"
#include "src/validator.h"

using namespace wabt;

static WriteBinaryOptions s_write_binary_options;
static std::string s_scriptfile;

class Bisect {
 public:
  explicit Bisect(unsigned length)
      : length_(length), pos_(0), window_(length / 2), remainder_(0) {
    if (window_ != 0) {
      remainder_ = length_ % window_;
    }
  }

  static constexpr std::pair<unsigned, unsigned> kEndBisect =
      std::make_pair(0, 0);

  // Returns a pair of values <start, end+1> where start < end+1 and
  // end+1 <= length. It will return approx-evenly spaced ranges over
  // [0..length), and each time it reaches the end, it will return back to zero
  // and use smaller ranges. It continues until the ranges are size one each,
  // after which point it returns kEndBisect.
  std::pair<unsigned, unsigned> Next() {
    if (pos_ == length_) {
      pos_ = 0;
      window_ /= 2;
      remainder_ = 0;
      if (window_ != 0) {
        remainder_ = length_ % window_;
      }
    }

    if (window_ == 0 && remainder_ <= 0) {
      return kEndBisect;
    }

    unsigned end = pos_ + window_;
    if (remainder_ > 0) {
      ++end;
    }
    --remainder_;
    auto range = std::make_pair(pos_, end);
    pos_ = end;
    return range;
  }

  bool ShouldConsiderInverse() const { return window_ < length_ / 2; }

  // Shrink this Bisect down to the range last returned by Next() and restart
  // at index zero.
  void Keep() {
    assert(window_ != 0);
    int remainder_adjustment = remainder_ >= 0 ? 1 : 0;
    length_ = window_ + remainder_adjustment;
    window_ = std::min(window_ + remainder_adjustment, length_ / 2);
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
    int remainder_adjustment = remainder_ >= 0 ? 1 : 0;
    length_ -= window_ + remainder_adjustment;
    window_ = std::min(window_ + remainder_adjustment, length_ / 2);
    pos_ = 0;
    remainder_ = 0;
    if (window_ != 0) {
      remainder_ = length_ % window_;
    }
  }

  unsigned Size() const { return length_; }

 private:
  unsigned length_;
  unsigned pos_;
  unsigned window_;
  int remainder_;
};
constexpr std::pair<unsigned, unsigned> Bisect::kEndBisect;

class BisectWithFixedLinearRange {
 public:
  explicit BisectWithFixedLinearRange(unsigned length) : bisect_(length) {
    map_.reserve(length);
    for (unsigned i = 0; i != length; ++i) {
      map_.push_back(i);
    }
  }

  std::pair<unsigned, unsigned> Next() {
    range_ = bisect_.Next();
    return range_;
  }

  bool ShouldConsiderInverse() const { return bisect_.ShouldConsiderInverse(); }

  void Keep() {
    unsigned window = range_.second - range_.first;
    std::vector<int> new_map;
    new_map.reserve(window);
    for (unsigned i = 0; i != window; ++i) {
      new_map.push_back(map_[range_.first + i]);
    }
    bisect_.Keep();
    assert(new_map.size() == bisect_.Size());
    std::swap(map_, new_map);
  }

  void KeepInverse() {
    unsigned window = range_.second - range_.first;
    std::vector<int> new_map;
    new_map.reserve(map_.size() - window);
    for (unsigned i = 0, e = map_.size(); i != e; ++i) {
      if (i < range_.first || i >= range_.second) {
        new_map.push_back(map_[i]);
      }
    }
    bisect_.KeepInverse();
    assert(new_map.size() == bisect_.Size());
    std::swap(map_, new_map);
  }

  unsigned Map(unsigned i) const {
    assert(i < map_.size());
    return map_[i];
  }

  unsigned Size() const { return bisect_.Size(); }

 private:
  Bisect bisect_;
  std::vector<int> map_;
  std::pair<unsigned, unsigned> range_;
};

static Module CopyModule(const Module* orig) {
  // TODO: build a copy without involving streams
  MemoryStream stream;
  Result result = WriteBinaryModule(&stream, orig, s_write_binary_options);
  if (Failed(result)) {
    abort();
  }

  Errors errors;
  Module copy;
  ReadBinaryOptions options(s_write_binary_options.features, nullptr, true,
                            true, false);
  result = ReadBinaryIr("memory stream", stream.output_buffer().data.data(),
                        stream.output_buffer().size(), options, &errors, &copy);
  if (Failed(result)) {
    abort();
  }
  return copy;
}

static bool IsModuleInteresting(const Module* module) {
  // TODO: enforced timeouts.
  // TODO: unique temp filename selection so multiple bugpoints can run.
  {
    // TODO: FileWriter::MoveData not implemented!
    // FileStream stream("wasm-bugpoint.tmp.wasm");
    MemoryStream stream;
    Result result = WriteBinaryModule(&stream, module, s_write_binary_options);
    if (Failed(result)) {
      abort();
    }
    stream.WriteToFile("wasm-bugpoint.tmp.wasm");
  }
  int interesting = system((s_scriptfile + " wasm-bugpoint.tmp.wasm").c_str());
  remove("wasm-bugpoint.tmp.wasm");
  return interesting != 0;
}

static void AbsolutePath(std::string* path) {
  if (char* abs_path = realpath(path->c_str(), nullptr)) {
    *path = std::string(abs_path);
    free(abs_path);
  }
}

static void AddSomeValueOfType(ExprList* exprs, Type type) {
  switch (type) {
    case Type::I32:
      exprs->push_back(MakeUnique<ConstExpr>(Const::I32()));
      break;
    case Type::I64:
      exprs->push_back(MakeUnique<ConstExpr>(Const::I64()));
      break;
    case Type::F32:
      exprs->push_back(MakeUnique<ConstExpr>(Const::F32()));
      break;
    case Type::F64:
      exprs->push_back(MakeUnique<ConstExpr>(Const::F64()));
      break;
    case Type::V128:
      exprs->push_back(MakeUnique<ConstExpr>(Const::V128({0, 0, 0, 0})));
      break;
    default:
      abort();
  }
}

static bool TryRemovingBodyFromFunctions(Module* module) {
  bool modified = false;
  BisectWithFixedLinearRange bisect(module->funcs.size());

  // Instead of deleting the functions, we remove their expressions,
  // replacing them with constants as needed to meet the function signature.
  // Because they aren't deleted, we need to map the contiguous space of
  // not-deleted functions to the function indexes in the Module.
  do {
    auto range = bisect.Next();
    if (range == Bisect::kEndBisect) {
      break;
    }

    Module copy = CopyModule(module);

    for (unsigned i = 0, e = bisect.Size(); i != e; ++i) {
      if (i == range.first) {
        i = range.second - 1;
        continue;
      }
      Func* func = copy.funcs[bisect.Map(i)];
      func->exprs = ExprList();
      for (int j = 0, je = func->GetNumResults(); j != je; ++j) {
        AddSomeValueOfType(&func->exprs, func->GetResultType(j));
      }
    }
    if (IsModuleInteresting(&copy)) {
      bisect.Keep();
      *module = CopyModule(&copy);
      modified = true;
    }
  } while (1);

  // TODO: we don't know if we actually modified the function, we could be
  // replacing (i32.const 0) with (i32.const 0) and thinking we modified it.
  // The return value truly means "should we rerun this", so conservatively we
  // can answer "false". If we always answer true, bugpoint will have an
  // infinite loop as it keeps thinking it's making progress.
  // return modified;
  (void)modified;
  return false;
}

static bool TryRemovingBlocksFromFunction(Module* module, Index func) {
  // Find only blocks at a specified depth. In this way, we produce a list of
  // blocks where deleting one doesn't delete another in our list. For one block
  // to be inside another one, they would necessarily be at different depths.
  auto ScanForBlocks = [](ExprList* exprs, unsigned desired_depth) {
    std::vector<Block*> blocks;
    std::vector<std::pair<ExprList*, unsigned>> worklist;
    unsigned depth = 0;
    worklist.emplace_back(exprs, depth);
    do {
      auto entry = worklist.back();
      ExprList* exprs = entry.first;
      depth = entry.second;
      worklist.pop_back();
      for (auto& expr : *exprs) {
        switch (expr.type()) {
          case ExprType::Block:
            if (depth == desired_depth) {
              blocks.push_back(&cast<BlockExpr>(&expr)->block);
            } else {
              worklist.emplace_back(&cast<BlockExpr>(&expr)->block.exprs,
                                    depth + 1);
            }
            break;
          case ExprType::If: {
            IfExpr* if_expr = cast<IfExpr>(&expr);
            if (depth == desired_depth) {
              blocks.push_back(&if_expr->true_);
            } else {
              worklist.emplace_back(&if_expr->true_.exprs, depth + 1);
            }
            worklist.emplace_back(&if_expr->false_, depth);
            break;
          }
          case ExprType::Try: {
            TryExpr* try_expr = cast<TryExpr>(&expr);
            if (depth == desired_depth) {
              blocks.push_back(&try_expr->block);
            } else {
              worklist.emplace_back(&try_expr->block.exprs, depth + 1);
            }
            worklist.emplace_back(&try_expr->catch_, depth);
            break;
          }
          case ExprType::Loop:
            if (depth == desired_depth) {
              blocks.push_back(&cast<LoopExpr>(&expr)->block);
            } else {
              worklist.emplace_back(&cast<LoopExpr>(&expr)->block.exprs,
                                    depth + 1);
            }
            break;
          default:
            break;
        }
      }
    } while (!worklist.empty());
    return blocks;
  };

  bool modified = false;
  unsigned depth = 0;
  // Start at depth zero, the outermost blocks and proceed toward nested blocks.
  // Stop when there are no more blocks.
  do {
    auto blocks = ScanForBlocks(&module->funcs[func]->exprs, depth);
    if (blocks.empty()) {
      break;
    }

    BisectWithFixedLinearRange bisect(blocks.size());
    do {
      auto range = bisect.Next();
      if (range == Bisect::kEndBisect) {
        break;
      }

      Module copy = CopyModule(module);
      auto blocks = ScanForBlocks(&copy.funcs[func]->exprs, depth);

      for (unsigned i = range.first; i != range.second; ++i) {
        Block* block = blocks[bisect.Map(i)];
        block->exprs = ExprList();
        for (int j = 0, je = block->decl.GetNumResults(); j != je; ++j) {
          AddSomeValueOfType(&block->exprs, block->decl.GetResultType(j));
        }
      }

      if (IsModuleInteresting(&copy)) {
        bisect.Keep();
        *module = CopyModule(&copy);
        modified = true;
      }
    } while (1);
    ++depth;
  } while (1);

  // return modified;
  (void)modified;
  return false;
}

static int ProgramMain(int argc, char** argv) {
  InitStdio();

  std::string outfile = "wasm-bugpoint.wasm";
  std::string wasmfile;
  Features features;
  {
    const char s_description[] =
        "  WebAssembly testcase reducer.\n"
        "\n"
        "  Given a script that determines whether a file in WebAssembly "
        "  binary format is interesting, indicated by returning exit code 0,\n"
        "  and an interesting file, we attempt to produce a smaller variation\n"
        "  of the file which is still interesting.\n"
        "\n"
        "example:\n"
        "  $ wasm-bugpoint delta.sh test.wasm\n";
    OptionParser parser("wasm-bugpoint", s_description);
    parser.AddHelpOption();
    parser.AddOption('o', "output", "FILENAME",
                     "Output file for the reduced testcase, defaults to "
                     "\"wasm-bugpoint.wasm\"",
                     [&](const char* argument) {
                       outfile = argument;
                       ConvertBackslashToSlash(&outfile);
                     });
    features.AddOptions(&parser);
    parser.AddArgument("scriptfilename", OptionParser::ArgumentCount::One,
                       [&](const char* argument) {
                         s_scriptfile = argument;
                         ConvertBackslashToSlash(&s_scriptfile);
                         AbsolutePath(&s_scriptfile);
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
      if (!IsModuleInteresting(&module)) {
        // TODO: inform the user. This can happen because the parse and
        // reserialization removed the interestingness.
        abort();
      }

      bool did_modify;
      do {
        did_modify = false;
        did_modify |= TryRemovingBodyFromFunctions(&module);
        // TODO:
        for (int i = 0, e = module.funcs.size(); i != e; ++i) {
          did_modify |= TryRemovingBlocksFromFunction(&module, i);
          // TryRemovingExprsFromFunction();
        }
        // TryRemovingTableElements();
        // TryRemovingFunctions();
        // TryRemovingTypeSignature();
      } while (did_modify);

      // Reduction complete.
      {
        MemoryStream stream;
        Result result =
            WriteBinaryModule(&stream, &module, s_write_binary_options);
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

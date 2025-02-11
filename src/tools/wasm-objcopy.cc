/*
 * Copyright 2025 WebAssembly Community Group participants
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
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "wabt/binary-reader-nop.h"
#include "wabt/binary-reader.h"
#include "wabt/binary-writer.h"
#include "wabt/binary.h"
#include "wabt/common.h"
#include "wabt/error-formatter.h"
#include "wabt/leb128.h"
#include "wabt/option-parser.h"
#include "wabt/result.h"
#include "wabt/stream.h"

using namespace wabt;

static std::string s_filename;
static std::string s_outfile;
static std::set<std::string_view> v_sections_to_remove{};
static std::map<std::string_view, std::string_view> v_sections_to_add{};
static std::map<std::string_view, std::string_view> v_sections_to_update{};

static const char s_description[] =
    R"(  Copy and translate sections of a WebAssembly binary file.

examples:
  # Add test_section with data from test.bin file
  $ wasm-objcopy --add-section=test_section=test.bin test.wasm
)";

static void ParseOptions(int argc, char** argv) {
  OptionParser parser("wasm-objcopy", s_description);

  parser.AddArgument("filename", OptionParser::ArgumentCount::One,
                     [](const char* argument) {
                       s_filename = argument;
                       ConvertBackslashToSlash(&s_filename);
                     });
  parser.AddOption('o', "output", "FILE", "output wasm binary file",
                   [](const char* argument) { s_outfile = argument; });
  parser.AddOption('R', "remove-section", "SECTION NAME", "Section to remove",
                   [](const char* value) {
                     v_sections_to_remove.insert(std::string_view{value});
                   });
  parser.AddOption('a', "add-section", "SECTION NAME=FILENAME",
                   "Add section with contents from the file _FILENAME_",
                   [](const char* value) {
                     auto eq_sign = strchr(value, '=');
                     if (eq_sign == NULL) {
                       v_sections_to_add[std::string_view(value)] = "";
                       return;
                     }

                     auto k = std::string_view(value, eq_sign - value);
                     auto v = std::string_view(eq_sign + 1);

                     v_sections_to_add[k] = v;
                   });
  parser.AddOption('U', "update-section", "SECTION NAME=FILENAME",
                   "Update section with contents from the file _FILENAME_",
                   [](const char* value) {
                     auto eq_sign = strchr(value, '=');
                     if (eq_sign == NULL) {
                       v_sections_to_update[std::string_view(value)] = "";
                       return;
                     }

                     auto k = std::string_view(value, eq_sign - value);
                     auto v = std::string_view(eq_sign + 1);

                     v_sections_to_update[k] = v;
                   });
  parser.Parse(argc, argv);
}

class BinaryReaderObjcopy : public BinaryReaderNop {
 public:
  explicit BinaryReaderObjcopy(
      std::set<std::string_view> sections_to_remove,
      std::map<std::string_view, std::string_view> sections_to_add,
      std::map<std::string_view, std::string_view> sections_to_update,
      Errors* errors)
      : errors_(errors),
        sections_to_remove_(sections_to_remove),
        sections_to_add_(sections_to_add),
        sections_to_update_(sections_to_update),
        section_start_(0) {
    stream_.WriteU32(WABT_BINARY_MAGIC, "WASM_BINARY_MAGIC");
    stream_.WriteU32(WABT_BINARY_VERSION, "WASM_BINARY_VERSION");
  }

  bool OnError(const Error& error) override {
    errors_->push_back(error);
    return true;
  }

  Result BeginSection(Index section_index,
                      BinarySection section_type,
                      Offset size) override {
    section_start_ = state->offset;
    if (section_type == BinarySection::Custom) {
      return Result::Ok;
    }
    stream_.WriteU8Enum(section_type, "section code");
    WriteU32Leb128(&stream_, size, "section size");
    stream_.WriteData(state->data + state->offset, size, "section data");
    return Result::Ok;
  }

  Result WriteToFile(std::string_view filename) {
    Result result;
    for (auto section : sections_to_add_) {
      auto section_name = section.first;
      auto filename = section.second;

      std::vector<uint8_t> file_data;
      result = ReadFile(filename, &file_data);
      if (Failed(result)) {
        return result;
      }

      WriteCustomSection(&stream_, section_name, file_data);
    }

    return stream_.WriteToFile(filename);
  }

  Result BeginCustomSection(Index section_index,
                            Offset size,
                            std::string_view section_name) override {
    if (sections_to_remove_.count(section_name) > 0) {
      return Result::Ok;
    }

    if (sections_to_add_.count(section_name) > 0) {
      std::string section_name_str{section_name};
      fprintf(stderr,
              "add-section: section %s already present in %s. Use "
              "--update-section/-U instead.\n",
              section_name_str.c_str(), s_filename.c_str());
      return Result::Ok;
    }

    if (sections_to_update_.count(section_name) > 0) {
      auto filename = sections_to_update_[section_name];

      std::vector<uint8_t> file_data;
      Result result = ReadFile(filename, &file_data);
      if (Failed(result)) {
        return result;
      }

      WriteCustomSection(&stream_, section_name, file_data);
      return Result::Ok;
    }

    stream_.WriteU8Enum(BinarySection::Custom, "section code");
    WriteU32Leb128(&stream_, size, "section size");
    stream_.WriteData(state->data + section_start_, size, "section data");
    return Result::Ok;
  }

 private:
  MemoryStream stream_;
  Errors* errors_;
  std::set<std::string_view> sections_to_remove_;
  std::map<std::string_view, std::string_view> sections_to_add_;
  std::map<std::string_view, std::string_view> sections_to_update_;
  Offset section_start_;

  void WriteCustomSection(Stream* stream,
                          std::string_view name,
                          std::vector<uint8_t> file_data) {
    MemoryStream data_stream;

    WriteStr(&data_stream, name, "section name");
    data_stream.WriteData(file_data, "section data");

    stream_.WriteU8Enum(BinarySection::Custom, "section code");
    WriteU32Leb128(stream, data_stream.output_buffer().data.size(),
                   "section size");
    stream_.WriteData(data_stream.output_buffer().data, "section data");
  }
};

int ProgramMain(int argc, char** argv) {
  Result result;

  InitStdio();
  ParseOptions(argc, argv);

  std::vector<uint8_t> file_data;
  result = ReadFile(s_filename.c_str(), &file_data);
  if (Failed(result)) {
    return Result::Error;
  }

  Errors errors;
  Features features;
  features.EnableAll();
  const bool kReadDebugNames = false;
  const bool kStopOnFirstError = true;
  const bool kFailOnCustomSectionError = false;
  ReadBinaryOptions options(features, nullptr, kReadDebugNames,
                            kStopOnFirstError, kFailOnCustomSectionError);

  BinaryReaderObjcopy reader(v_sections_to_remove, v_sections_to_add,
                             v_sections_to_update, &errors);
  result = ReadBinary(file_data.data(), file_data.size(), &reader, options);
  FormatErrorsToFile(errors, Location::Type::Binary);
  if (Failed(result)) {
    return Result::Error;
  }

  if (s_outfile.empty()) {
    s_outfile = s_filename;
  }
  return reader.WriteToFile(s_outfile);
}

int main(int argc, char** argv) {
  WABT_TRY
  return ProgramMain(argc, argv);
  WABT_CATCH_BAD_ALLOC_AND_EXIT
}

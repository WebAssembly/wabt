#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "wabt/config.h"

#include "wabt/binary-writer.h"
#include "wabt/common.h"
#include "wabt/error-formatter.h"
#include "wabt/feature.h"
#include "wabt/filenames.h"
#include "wabt/ir.h"
#include "wabt/option-parser.h"
#include "wabt/resolve-names.h"
#include "wabt/stream.h"
#include "wabt/validator.h"
#include "wabt/wast-parser.h"

using namespace wabt;

static const char* s_infile;
static std::string s_outfile;
static bool s_dump_module;
static int s_verbose;
static WriteBinaryOptions s_write_binary_options;
static bool s_validate = true;
static bool s_debug_parsing;
static Features s_features;

static std::unique_ptr<FileStream> s_log_stream;

static const char s_description[] =
    R"(  read a file in the wasm text format, check it for errors, and
  convert it to the wasm binary format.

examples:
  # parse test.wat and write to .wasm binary file with the same name
  $ wat2wasm test.wat

  # parse test.wat and write to binary file test.wasm
  $ wat2wasm test.wat -o test.wasm

  # parse spec-test.wast, and write verbose output to stdout (including
  # the meaning of every byte)
  $ wat2wasm spec-test.wast -v
)";

static void ParseOptions(int argc, char* argv[]) {
  OptionParser parser("wat2wasm", s_description);

  parser.AddOption('v', "verbose", "Use multiple times for more info", []() {
    s_verbose++;
    s_log_stream = FileStream::CreateStderr();
  });
  parser.AddOption("debug-parser", "Turn on debugging the parser of wat files",
                   []() { s_debug_parsing = true; });
  parser.AddOption('d', "dump-module",
                   "Print a hexdump of the module to stdout",
                   []() { s_dump_module = true; });
  s_features.AddOptions(&parser);
  parser.AddOption('o', "output", "FILE",
                   "Output wasm binary file. Use \"-\" to write to stdout.",
                   [](const char* argument) { s_outfile = argument; });
  parser.AddOption(
      'r', "relocatable",
      "Create a relocatable wasm binary (suitable for linking with e.g. lld)",
      []() { s_write_binary_options.relocatable = true; });
  parser.AddOption(
      "no-canonicalize-leb128s",
      "Write all LEB128 sizes as 5-bytes instead of their minimal size",
      []() { s_write_binary_options.canonicalize_lebs = false; });
  parser.AddOption("debug-names",
                   "Write debug names to the generated binary file",
                   []() { s_write_binary_options.write_debug_names = true; });
  parser.AddOption("no-check", "Don't check for invalid modules",
                   []() { s_validate = false; });
  parser.AddArgument("filename", OptionParser::ArgumentCount::One,
                     [](const char* argument) { s_infile = argument; });

  parser.Parse(argc, argv);
}

static void WriteBufferToFile(std::string_view filename,
                              const OutputBuffer& buffer) {
  if (s_dump_module) {
    std::unique_ptr<FileStream> stream = FileStream::CreateStdout();
    if (s_verbose) {
      stream->Writef(";; dump\n");
    }
    if (!buffer.data.empty()) {
      stream->WriteMemoryDump(buffer.data.data(), buffer.data.size());
    }
  }

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

int ProgramMain(int argc, char** argv) {
  InitStdio();

  ParseOptions(argc, argv);

  std::vector<uint8_t> file_data;
  Result result = ReadFile(s_infile, &file_data);
  Errors errors;
  // 处理filedata 里面的缩写
  for (size_t i = 0; i < file_data.size(); ++i) {
    // 前面应该至少有一个空格
    // 后面应该是空格/换行/回车/制表符/)
    // anyref
    if (file_data[i] == ' ' && file_data[i + 1] == 'a' &&
        file_data[i + 2] == 'n' && file_data[i + 3] == 'y' &&
        file_data[i + 4] == 'r' && file_data[i + 5] == 'e' &&
        file_data[i + 6] == 'f' &&
        (file_data[i + 7] == ')' || file_data[i + 7] == ' ' ||
         file_data[i + 7] == '\n' || file_data[i + 7] == '\r' ||
         file_data[i + 7] == '\t')) {
      // 删除原来的anyref
      file_data.erase(file_data.begin() + i, file_data.begin() + i + 7);
      // 不可以使用赋值，使用insert
      file_data.insert(file_data.begin() + i, '(');
      file_data.insert(file_data.begin() + i + 1, 'r');
      file_data.insert(file_data.begin() + i + 2, 'e');
      file_data.insert(file_data.begin() + i + 3, 'f');
      file_data.insert(file_data.begin() + i + 4, ' ');
      file_data.insert(file_data.begin() + i + 5, 'n');
      file_data.insert(file_data.begin() + i + 6, 'u');
      file_data.insert(file_data.begin() + i + 7, 'l');
      file_data.insert(file_data.begin() + i + 8, 'l');
      file_data.insert(file_data.begin() + i + 9, ' ');
      file_data.insert(file_data.begin() + i + 10, 'a');
      file_data.insert(file_data.begin() + i + 11, 'n');
      file_data.insert(file_data.begin() + i + 12, 'y');
      file_data.insert(file_data.begin() + i + 13, ')');

      i += 13;
    }

    // eqref
    if (file_data[i] == ' ' && file_data[i + 1] == 'e' &&
        file_data[i + 2] == 'q' && file_data[i + 3] == 'r' &&
        file_data[i + 4] == 'e' && file_data[i + 5] == 'f' &&
        (file_data[i + 6] == ')' || file_data[i + 6] == ' ' ||
         file_data[i + 6] == '\n' || file_data[i + 6] == '\r' ||
         file_data[i + 6] == '\t')) {
      // 删除原来的structref
      file_data.erase(file_data.begin() + i, file_data.begin() + i + 6);
      // 不可以使用赋值，使用insert
      file_data.insert(file_data.begin() + i, '(');
      file_data.insert(file_data.begin() + i + 1, 'r');
      file_data.insert(file_data.begin() + i + 2, 'e');
      file_data.insert(file_data.begin() + i + 3, 'f');
      file_data.insert(file_data.begin() + i + 4, ' ');
      file_data.insert(file_data.begin() + i + 5, 'n');
      file_data.insert(file_data.begin() + i + 6, 'u');
      file_data.insert(file_data.begin() + i + 7, 'l');
      file_data.insert(file_data.begin() + i + 8, 'l');
      file_data.insert(file_data.begin() + i + 9, ' ');
      file_data.insert(file_data.begin() + i + 10, 'e');
      file_data.insert(file_data.begin() + i + 11, 'q');
      file_data.insert(file_data.begin() + i + 12, ')');
      i += 12;
    }

    // i31ref
    if (file_data[i] == ' ' && file_data[i + 1] == 'i' &&
        file_data[i + 2] == '3' && file_data[i + 3] == '1' &&
        file_data[i + 4] == 'r' && file_data[i + 5] == 'e' &&
        file_data[i + 6] == 'f' &&
        (file_data[i + 7] == ')' || file_data[i + 7] == ' ' ||
         file_data[i + 7] == '\n' || file_data[i + 7] == '\r' ||
         file_data[i + 7] == '\t')) {
      // 删除原来的i31ref
      file_data.erase(file_data.begin() + i, file_data.begin() + i + 7);
      // 不可以使用赋值，使用insert
      file_data.insert(file_data.begin() + i, '(');
      file_data.insert(file_data.begin() + i + 1, 'r');
      file_data.insert(file_data.begin() + i + 2, 'e');
      file_data.insert(file_data.begin() + i + 3, 'f');
      file_data.insert(file_data.begin() + i + 4, ' ');
      file_data.insert(file_data.begin() + i + 5, 'i');
      file_data.insert(file_data.begin() + i + 6, '3');
      file_data.insert(file_data.begin() + i + 7, '1');
      file_data.insert(file_data.begin() + i + 8, ')');
      i += 8;
    }

    // structref
    if (file_data[i] == ' ' && file_data[i + 1] == 's' &&
        file_data[i + 2] == 't' && file_data[i + 3] == 'r' &&
        file_data[i + 4] == 'u' && file_data[i + 5] == 'c' &&
        file_data[i + 6] == 't' && file_data[i + 7] == 'r' &&
        file_data[i + 8] == 'e' && file_data[i + 9] == 'f' &&
        (file_data[i + 10] == ')' || file_data[i + 10] == ' ' ||
         file_data[i + 10] == '\n' || file_data[i + 10] == '\r' ||
         file_data[i + 10] == '\t')) {
      // 删除原来的structref
      file_data.erase(file_data.begin() + i, file_data.begin() + i + 10);
      // 不可以使用赋值，使用insert
      file_data.insert(file_data.begin() + i, '(');
      file_data.insert(file_data.begin() + i + 1, 'r');
      file_data.insert(file_data.begin() + i + 2, 'e');
      file_data.insert(file_data.begin() + i + 3, 'f');
      file_data.insert(file_data.begin() + i + 4, ' ');
      file_data.insert(file_data.begin() + i + 5, 'n');
      file_data.insert(file_data.begin() + i + 6, 'u');
      file_data.insert(file_data.begin() + i + 7, 'l');
      file_data.insert(file_data.begin() + i + 8, 'l');
      file_data.insert(file_data.begin() + i + 9, ' ');
      file_data.insert(file_data.begin() + i + 10, 's');
      file_data.insert(file_data.begin() + i + 11, 't');
      file_data.insert(file_data.begin() + i + 12, 'r');
      file_data.insert(file_data.begin() + i + 13, 'u');
      file_data.insert(file_data.begin() + i + 14, 'c');
      file_data.insert(file_data.begin() + i + 15, 't');
      file_data.insert(file_data.begin() + i + 16, ')');
      i += 16;
    }

    // arrayref
    if (file_data[i] == ' ' && file_data[i + 1] == 'a' &&
        file_data[i + 2] == 'r' && file_data[i + 3] == 'r' &&
        file_data[i + 4] == 'a' && file_data[i + 5] == 'y' &&
        file_data[i + 6] == 'r' && file_data[i + 7] == 'e' &&
        file_data[i + 8] == 'f' &&
        (file_data[i + 9] == ')' || file_data[i + 9] == ' ' ||
         file_data[i + 9] == '\n' || file_data[i + 9] == '\r' ||
         file_data[i + 9] == '\t')) {
      // 删除原来的arrayref
      file_data.erase(file_data.begin() + i, file_data.begin() + i + 9);
      // 不可以使用赋值，使用insert
      file_data.insert(file_data.begin() + i, '(');
      file_data.insert(file_data.begin() + i + 1, 'r');
      file_data.insert(file_data.begin() + i + 2, 'e');
      file_data.insert(file_data.begin() + i + 3, 'f');
      file_data.insert(file_data.begin() + i + 4, ' ');
      file_data.insert(file_data.begin() + i + 5, 'n');
      file_data.insert(file_data.begin() + i + 6, 'u');
      file_data.insert(file_data.begin() + i + 7, 'l');
      file_data.insert(file_data.begin() + i + 8, 'l');
      file_data.insert(file_data.begin() + i + 9, ' ');
      file_data.insert(file_data.begin() + i + 10, 'a');
      file_data.insert(file_data.begin() + i + 11, 'r');
      file_data.insert(file_data.begin() + i + 12, 'r');
      file_data.insert(file_data.begin() + i + 13, 'a');
      file_data.insert(file_data.begin() + i + 14, 'y');
      file_data.insert(file_data.begin() + i + 15, ')');
      i += 15;
    }

    // nullref -> (ref null none)
    if (file_data[i] == ' ' && file_data[i + 1] == 'n' &&
        file_data[i + 2] == 'u' && file_data[i + 3] == 'l' &&
        file_data[i + 4] == 'l' && file_data[i + 5] == 'r' &&
        file_data[i + 6] == 'e' && file_data[i + 7] == 'f' &&
        (file_data[i + 8] == ')' || file_data[i + 8] == ' ' ||
         file_data[i + 8] == '\n' || file_data[i + 8] == '\r' ||
         file_data[i + 8] == '\t')) {
      // 删除原来的nullref
      file_data.erase(file_data.begin() + i, file_data.begin() + i + 8);
      // 不可以使用赋值，使用insert
      file_data.insert(file_data.begin() + i, '(');
      file_data.insert(file_data.begin() + i + 1, 'r');
      file_data.insert(file_data.begin() + i + 2, 'e');
      file_data.insert(file_data.begin() + i + 3, 'f');
      file_data.insert(file_data.begin() + i + 4, ' ');
      file_data.insert(file_data.begin() + i + 5, 'n');
      file_data.insert(file_data.begin() + i + 6, 'u');
      file_data.insert(file_data.begin() + i + 7, 'l');
      file_data.insert(file_data.begin() + i + 8, 'l');
      file_data.insert(file_data.begin() + i + 9, ' ');
      file_data.insert(file_data.begin() + i + 10, 'n');
      file_data.insert(file_data.begin() + i + 11, 'o');
      file_data.insert(file_data.begin() + i + 12, 'n');
      file_data.insert(file_data.begin() + i + 13, 'e');
      file_data.insert(file_data.begin() + i + 14, ')');
      i += 14;
    }
    // nullfuncref -> (ref null nofunc)
    if (file_data[i] == ' ' && file_data[i + 1] == 'n' &&
        file_data[i + 2] == 'u' && file_data[i + 3] == 'l' &&
        file_data[i + 4] == 'l' && file_data[i + 5] == 'f' &&
        file_data[i + 6] == 'u' && file_data[i + 7] == 'n' &&
        file_data[i + 8] == 'c' && file_data[i + 9] == 'r' &&
        file_data[i + 10] == 'e' && file_data[i + 11] == 'f' &&
        (file_data[i + 12] == ')' || file_data[i + 12] == ' ' ||
         file_data[i + 12] == '\n' || file_data[i + 12] == '\r' ||
         file_data[i + 12] == '\t')) {
      // 删除原来的nullfuncref
      file_data.erase(file_data.begin() + i, file_data.begin() + i + 12);
      // 不可以使用赋值，使用insert
      file_data.insert(file_data.begin() + i, '(');
      file_data.insert(file_data.begin() + i + 1, 'r');
      file_data.insert(file_data.begin() + i + 2, 'e');
      file_data.insert(file_data.begin() + i + 3, 'f');
      file_data.insert(file_data.begin() + i + 4, ' ');
      file_data.insert(file_data.begin() + i + 5, 'n');
      file_data.insert(file_data.begin() + i + 6, 'u');
      file_data.insert(file_data.begin() + i + 7, 'l');
      file_data.insert(file_data.begin() + i + 8, 'l');
      file_data.insert(file_data.begin() + i + 9, ' ');
      file_data.insert(file_data.begin() + i + 10, 'n');
      file_data.insert(file_data.begin() + i + 11, 'o');
      file_data.insert(file_data.begin() + i + 12, 'f');
      file_data.insert(file_data.begin() + i + 13, 'u');
      file_data.insert(file_data.begin() + i + 14, 'n');
      file_data.insert(file_data.begin() + i + 15, 'c');
      file_data.insert(file_data.begin() + i + 16, ')');
      i += 16;
    }
    // nullexternref -> (ref null noextern)
    if (file_data[i] == ' ' && file_data[i + 1] == 'n' &&
        file_data[i + 2] == 'u' && file_data[i + 3] == 'l' &&
        file_data[i + 4] == 'l' && file_data[i + 5] == 'e' &&
        file_data[i + 6] == 'x' && file_data[i + 7] == 't' &&
        file_data[i + 8] == 'e' && file_data[i + 9] == 'r' &&
        file_data[i + 10] == 'n' && file_data[i + 11] == 'r' &&
        file_data[i + 12] == 'e' && file_data[i + 13] == 'f' &&
        (file_data[i + 14] == ')' || file_data[i + 14] == ' ' ||
         file_data[i + 14] == '\n' || file_data[i + 14] == '\r' ||
         file_data[i + 14] == '\t')) {
      // 删除原来的nullexternref
      file_data.erase(file_data.begin() + i, file_data.begin() + i + 14);
      // 不可以使用赋值，使用insert
      file_data.insert(file_data.begin() + i, '(');
      file_data.insert(file_data.begin() + i + 1, 'r');
      file_data.insert(file_data.begin() + i + 2, 'e');
      file_data.insert(file_data.begin() + i + 3, 'f');
      file_data.insert(file_data.begin() + i + 4, ' ');
      file_data.insert(file_data.begin() + i + 5, 'n');
      file_data.insert(file_data.begin() + i + 6, 'u');
      file_data.insert(file_data.begin() + i + 7, 'l');
      file_data.insert(file_data.begin() + i + 8, 'l');
      file_data.insert(file_data.begin() + i + 9, ' ');
      file_data.insert(file_data.begin() + i + 10, 'n');
      file_data.insert(file_data.begin() + i + 11, 'o');
      file_data.insert(file_data.begin() + i + 12, 'e');
      file_data.insert(file_data.begin() + i + 13, 'x');
      file_data.insert(file_data.begin() + i + 14, 't');
      file_data.insert(file_data.begin() + i + 15, 'e');
      file_data.insert(file_data.begin() + i + 16, 'r');
      file_data.insert(file_data.begin() + i + 17, 'n');
      file_data.insert(file_data.begin() + i + 18, ')');
      i += 18;
    }
  }
  std::unordered_map<std::string, int> moduletypesname;
  // 处理类型声明
  size_t type_index = -1;
  size_t lparsize = 0;
  size_t i = 0, m = file_data.size();
  while (i < m) {
    // 处理;;
    if (file_data[i] == ';' && file_data[i + 1] == ';') {
      while (i < m && file_data[i] != '\n' && file_data[i] != '\r')
        i++;
    }
    if (lparsize == 2) {
      // 判断是不是（type）
      std::string str;
      while (i < m && file_data[i] == ' ')
        i++;
      while (i < m && file_data[i] != ' ' && file_data[i] != '\n' &&
             file_data[i] != '\r' && file_data[i] != '\t' &&
             file_data[i] != '(' && file_data[i] != ')') {
        str += file_data[i];
        i++;
      }
      if (str == "type") {
        type_index++;
        std::string name;
        while (i < m && file_data[i] == ' ')
          i++;
        if (file_data[i] == '(') {
          continue;
        } else if (file_data[i] == '$') {
          name += file_data[i];
          i++;
          while (i < m && file_data[i] != ' ' && file_data[i] != '\n' &&
                 file_data[i] != '\r' && file_data[i] != '\t' &&
                 file_data[i] != '(' && file_data[i] != ')') {
            name += file_data[i++];
          }
        }
        moduletypesname.insert({name, type_index});
      } else if (str == "rec") {
        size_t Lpar = 0;
        while (i < m) {
          while (i < m && file_data[i] == ' ')
            i++;
          if (Lpar == 1) {
            std::string instruction;
            while (i < m && file_data[i] == ' ')
              i++;
            while (i < m && file_data[i] != ' ' && file_data[i] != '\n' &&
                   file_data[i] != '\r' && file_data[i] != '\t' &&
                   file_data[i] != '(' && file_data[i] != ')') {
              instruction += file_data[i];
              i++;
            }
            if (instruction == "type") {
              type_index++;
              std::string name;
              while (i < m && file_data[i] == ' ')
                i++;
              if (file_data[i] == '(') {
                continue;
              } else if (file_data[i] == '$') {
                name += file_data[i];
                i++;
                while (i < m && file_data[i] != ' ' && file_data[i] != '\n' &&
                       file_data[i] != '\r' && file_data[i] != '\t' &&
                       file_data[i] != '(' && file_data[i] != ')') {
                  name += file_data[i++];
                }
              }
              moduletypesname.insert({name, type_index});
            }
          }
          if (file_data[i] == '(') {
            Lpar++;
          } else if (file_data[i] == ')') {
            if (Lpar == 0) {
              break;
            } else {
              Lpar--;
            }
          }
          i++;
        }
      }
    }
    if (file_data[i] == '(')
      lparsize++;
    if (file_data[i] == ')')
      lparsize--;
    i++;
  }
  std::unique_ptr<WastLexer> lexer = WastLexer::CreateBufferLexer(
      s_infile, file_data.data(), file_data.size(), &errors);
  if (Failed(result)) {
    WABT_FATAL("unable to read file: %s\n", s_infile);
  }

  std::unique_ptr<Module> module = std::make_unique<Module>();
  WastParseOptions parse_wast_options(s_features);
  module->moduletypesname = moduletypesname;
  result = ParseWatModule(lexer.get(), &module, &errors, &parse_wast_options);

  if (Succeeded(result) && s_validate) {
    ValidateOptions options(s_features);
    result = ValidateModule(module.get(), &errors, options);
  }

  if (Succeeded(result)) {
    MemoryStream stream(s_log_stream.get());
    s_write_binary_options.features = s_features;
    result = WriteBinaryModule(&stream, module.get(), s_write_binary_options);

    if (Succeeded(result)) {
      if (s_outfile.empty()) {
        s_outfile = DefaultOuputName(s_infile);
      }
      WriteBufferToFile(s_outfile.c_str(), stream.output_buffer());
    }
  }

  auto line_finder = lexer->MakeLineFinder();
  FormatErrorsToFile(errors, Location::Type::Text, line_finder.get());

  return result != Result::Ok;
}

int main(int argc, char** argv) {
  WABT_TRY
  return ProgramMain(argc, argv);
  WABT_CATCH_BAD_ALLOC_AND_EXIT
}

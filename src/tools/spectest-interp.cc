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
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "src/binary-reader.h"
#include "src/cast.h"
#include "src/common.h"
#include "src/error-formatter.h"
#include "src/feature.h"
#include "src/interp/binary-reader-interp.h"
#include "src/interp/interp-util.h"
#include "src/interp/interp.h"
#include "src/literal.h"
#include "src/option-parser.h"
#include "src/stream.h"
#include "src/validator.h"
#include "src/wast-lexer.h"
#include "src/wast-parser.h"

using namespace wabt;
using namespace wabt::interp;

static int s_verbose;
static const char* s_infile;
static Thread::Options s_thread_options;
static Stream* s_trace_stream;
static Features s_features;

static std::unique_ptr<FileStream> s_log_stream;
static std::unique_ptr<FileStream> s_stdout_stream;

enum class RunVerbosity {
  Quiet = 0,
  Verbose = 1,
};

static const char s_description[] =
    R"(  read a Spectest JSON file, and run its tests in the interpreter.

examples:
  # parse test.json and run the spec tests
  $ spectest-interp test.json
)";

static void ParseOptions(int argc, char** argv) {
  OptionParser parser("spectest-interp", s_description);

  parser.AddOption('v', "verbose", "Use multiple times for more info", []() {
    s_verbose++;
    s_log_stream = FileStream::CreateStdout();
  });
  s_features.AddOptions(&parser);
  parser.AddOption('V', "value-stack-size", "SIZE",
                   "Size in elements of the value stack",
                   [](const std::string& argument) {
                     // TODO(binji): validate.
                     s_thread_options.value_stack_size = atoi(argument.c_str());
                   });
  parser.AddOption('C', "call-stack-size", "SIZE",
                   "Size in elements of the call stack",
                   [](const std::string& argument) {
                     // TODO(binji): validate.
                     s_thread_options.call_stack_size = atoi(argument.c_str());
                   });
  parser.AddOption('t', "trace", "Trace execution",
                   []() { s_trace_stream = s_stdout_stream.get(); });

  parser.AddArgument("filename", OptionParser::ArgumentCount::One,
                     [](const char* argument) { s_infile = argument; });
  parser.Parse(argc, argv);
}

namespace spectest {

class Command;
typedef std::unique_ptr<Command> CommandPtr;
typedef std::vector<CommandPtr> CommandPtrVector;

class Script {
 public:
  std::string filename;
  CommandPtrVector commands;
};

class Command {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(Command);
  Command() = delete;
  virtual ~Command() = default;

  CommandType type;
  uint32_t line = 0;

 protected:
  explicit Command(CommandType type) : type(type) {}
};

template <CommandType TypeEnum>
class CommandMixin : public Command {
 public:
  static bool classof(const Command* cmd) { return cmd->type == TypeEnum; }
  CommandMixin() : Command(TypeEnum) {}
};

enum class ModuleType {
  Text,
  Binary,
};

class ModuleCommand : public CommandMixin<CommandType::Module> {
 public:
  ModuleType module = ModuleType::Binary;
  std::string filename;
  std::string name;
};

class Action {
 public:
  ActionType type = ActionType::Invoke;
  std::string module_name;
  std::string field_name;
  ValueTypes types;
  Values args;
};

template <CommandType TypeEnum>
class ActionCommandBase : public CommandMixin<TypeEnum> {
 public:
  Action action;
};

typedef ActionCommandBase<CommandType::Action> ActionCommand;

class RegisterCommand : public CommandMixin<CommandType::Register> {
 public:
  std::string as;
  std::string name;
};

struct ExpectedValue {
  bool is_expected_nan;
  TypedValue value;
  ExpectedNan expectedNan;
};

class AssertReturnCommand : public CommandMixin<CommandType::AssertReturn> {
 public:
  Action action;
  std::vector<ExpectedValue> expected;
};

template <CommandType TypeEnum>
class AssertTrapCommandBase : public CommandMixin<TypeEnum> {
 public:
  Action action;
  std::string text;
};

typedef AssertTrapCommandBase<CommandType::AssertTrap> AssertTrapCommand;
typedef AssertTrapCommandBase<CommandType::AssertExhaustion>
    AssertExhaustionCommand;

template <CommandType TypeEnum>
class AssertModuleCommand : public CommandMixin<TypeEnum> {
 public:
  ModuleType type = ModuleType::Binary;
  std::string filename;
  std::string text;
};

typedef AssertModuleCommand<CommandType::AssertMalformed>
    AssertMalformedCommand;
typedef AssertModuleCommand<CommandType::AssertInvalid> AssertInvalidCommand;
typedef AssertModuleCommand<CommandType::AssertUnlinkable>
    AssertUnlinkableCommand;
typedef AssertModuleCommand<CommandType::AssertUninstantiable>
    AssertUninstantiableCommand;

// An extremely simple JSON parser that only knows how to parse the expected
// format from wat2wasm.
class JSONParser {
 public:
  JSONParser() {}

  wabt::Result ReadFile(string_view spec_json_filename);
  wabt::Result ParseScript(Script* out_script);

 private:
  void WABT_PRINTF_FORMAT(2, 3) PrintError(const char* format, ...);

  void PutbackChar();
  int ReadChar();
  void SkipWhitespace();
  bool Match(const char* s);
  wabt::Result Expect(const char* s);
  wabt::Result ExpectKey(const char* key);
  wabt::Result ParseUint32(uint32_t* out_int);
  wabt::Result ParseString(std::string* out_string);
  wabt::Result ParseKeyStringValue(const char* key, std::string* out_string);
  wabt::Result ParseOptNameStringValue(std::string* out_string);
  wabt::Result ParseLine(uint32_t* out_line_number);
  wabt::Result ParseTypeObject(Type* out_type);
  wabt::Result ParseTypeVector(TypeVector* out_types);
  wabt::Result ParseConst(TypedValue* out_value);
  wabt::Result ParseConstValue(TypedValue* out_value,
                         string_view type_str,
                         string_view value_str);
  wabt::Result ParseConstVector(ValueTypes* out_types, Values* out_values);
  wabt::Result ParseExpectedValue(ExpectedValue* out_value);
  wabt::Result ParseExpectedValues(std::vector<ExpectedValue>* out_values);
  wabt::Result ParseAction(Action* out_action);
  wabt::Result ParseActionResult();
  wabt::Result ParseModuleType(ModuleType* out_type);

  std::string CreateModulePath(string_view filename);
  wabt::Result ParseFilename(std::string* out_filename);
  wabt::Result ParseCommand(CommandPtr* out_command);

  // Parsing info.
  std::vector<uint8_t> json_data_;
  size_t json_offset_ = 0;
  Location loc_;
  Location prev_loc_;
  bool has_prev_loc_ = false;
};

#define EXPECT(x) CHECK_RESULT(Expect(x))
#define EXPECT_KEY(x) CHECK_RESULT(ExpectKey(x))
#define PARSE_KEY_STRING_VALUE(key, value) \
  CHECK_RESULT(ParseKeyStringValue(key, value))

wabt::Result JSONParser::ReadFile(string_view spec_json_filename) {
  loc_.filename = spec_json_filename;
  loc_.line = 1;
  loc_.first_column = 1;

  return wabt::ReadFile(spec_json_filename, &json_data_);
}

void JSONParser::PrintError(const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  fprintf(stderr, "%s:%d:%d: %s\n", loc_.filename.to_string().c_str(),
          loc_.line, loc_.first_column, buffer);
}

void JSONParser::PutbackChar() {
  assert(has_prev_loc_);
  json_offset_--;
  loc_ = prev_loc_;
  has_prev_loc_ = false;
}

int JSONParser::ReadChar() {
  if (json_offset_ >= json_data_.size()) {
    return -1;
  }
  prev_loc_ = loc_;
  char c = json_data_[json_offset_++];
  if (c == '\n') {
    loc_.line++;
    loc_.first_column = 1;
  } else {
    loc_.first_column++;
  }
  has_prev_loc_ = true;
  return c;
}

void JSONParser::SkipWhitespace() {
  while (1) {
    switch (ReadChar()) {
      case -1:
        return;

      case ' ':
      case '\t':
      case '\n':
      case '\r':
        break;

      default:
        PutbackChar();
        return;
    }
  }
}

bool JSONParser::Match(const char* s) {
  SkipWhitespace();
  Location start_loc = loc_;
  size_t start_offset = json_offset_;
  while (*s && *s == ReadChar())
    s++;

  if (*s == 0) {
    return true;
  } else {
    json_offset_ = start_offset;
    loc_ = start_loc;
    return false;
  }
}

wabt::Result JSONParser::Expect(const char* s) {
  if (Match(s)) {
    return wabt::Result::Ok;
  } else {
    PrintError("expected %s", s);
    return wabt::Result::Error;
  }
}

wabt::Result JSONParser::ExpectKey(const char* key) {
  size_t keylen = strlen(key);
  size_t quoted_len = keylen + 2 + 1;
  char* quoted = static_cast<char*>(alloca(quoted_len));
  snprintf(quoted, quoted_len, "\"%s\"", key);
  EXPECT(quoted);
  EXPECT(":");
  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseUint32(uint32_t* out_int) {
  uint32_t result = 0;
  SkipWhitespace();
  while (1) {
    int c = ReadChar();
    if (c >= '0' && c <= '9') {
      uint32_t last_result = result;
      result = result * 10 + static_cast<uint32_t>(c - '0');
      if (result < last_result) {
        PrintError("uint32 overflow");
        return wabt::Result::Error;
      }
    } else {
      PutbackChar();
      break;
    }
  }
  *out_int = result;
  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseString(std::string* out_string) {
  out_string->clear();

  SkipWhitespace();
  if (ReadChar() != '"') {
    PrintError("expected string");
    return wabt::Result::Error;
  }

  while (1) {
    int c = ReadChar();
    if (c == '"') {
      break;
    } else if (c == '\\') {
      /* The only escape supported is \uxxxx. */
      c = ReadChar();
      if (c != 'u') {
        PrintError("expected escape: \\uxxxx");
        return wabt::Result::Error;
      }
      uint16_t code = 0;
      for (int i = 0; i < 4; ++i) {
        c = ReadChar();
        int cval;
        if (c >= '0' && c <= '9') {
          cval = c - '0';
        } else if (c >= 'a' && c <= 'f') {
          cval = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
          cval = c - 'A' + 10;
        } else {
          PrintError("expected hex char");
          return wabt::Result::Error;
        }
        code = (code << 4) + cval;
      }

      if (code < 256) {
        *out_string += code;
      } else {
        PrintError("only escape codes < 256 allowed, got %u\n", code);
      }
    } else {
      *out_string += c;
    }
  }
  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseKeyStringValue(const char* key,
                                             std::string* out_string) {
  out_string->clear();
  EXPECT_KEY(key);
  return ParseString(out_string);
}

wabt::Result JSONParser::ParseOptNameStringValue(std::string* out_string) {
  out_string->clear();
  if (Match("\"name\"")) {
    EXPECT(":");
    CHECK_RESULT(ParseString(out_string));
    EXPECT(",");
  }
  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseLine(uint32_t* out_line_number) {
  EXPECT_KEY("line");
  CHECK_RESULT(ParseUint32(out_line_number));
  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseTypeObject(Type* out_type) {
  std::string type_str;
  EXPECT("{");
  PARSE_KEY_STRING_VALUE("type", &type_str);
  EXPECT("}");

  if (type_str == "i32") {
    *out_type = Type::I32;
    return wabt::Result::Ok;
  } else if (type_str == "f32") {
    *out_type = Type::F32;
    return wabt::Result::Ok;
  } else if (type_str == "i64") {
    *out_type = Type::I64;
    return wabt::Result::Ok;
  } else if (type_str == "f64") {
    *out_type = Type::F64;
    return wabt::Result::Ok;
  } else if (type_str == "v128") {
    *out_type = Type::V128;
    return wabt::Result::Ok;
  } else if (type_str == "funcref") {
    *out_type = Type::Funcref;
    return wabt::Result::Ok;
  } else if (type_str == "anyref") {
    *out_type = Type::Anyref;
    return wabt::Result::Ok;
  } else if (type_str == "nullref") {
    *out_type = Type::Nullref;
    return wabt::Result::Ok;
  } else if (type_str == "exnref") {
    *out_type = Type::Exnref;
    return wabt::Result::Ok;
  } else {
    PrintError("unknown type: \"%s\"", type_str.c_str());
    return wabt::Result::Error;
  }
}

wabt::Result JSONParser::ParseTypeVector(TypeVector* out_types) {
  out_types->clear();
  EXPECT("[");
  bool first = true;
  while (!Match("]")) {
    if (!first) {
      EXPECT(",");
    }
    Type type;
    CHECK_RESULT(ParseTypeObject(&type));
    first = false;
    out_types->push_back(type);
  }
  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseConst(TypedValue* out_value) {
  std::string type_str;
  std::string value_str;
  EXPECT("{");
  PARSE_KEY_STRING_VALUE("type", &type_str);
  EXPECT(",");
  PARSE_KEY_STRING_VALUE("value", &value_str);
  EXPECT("}");

  return ParseConstValue(out_value, type_str, value_str);
}

wabt::Result JSONParser::ParseConstValue(TypedValue* out_value,
                                   string_view type_str,
                                   string_view value_str) {
  const char* value_start = value_str.data();
  const char* value_end = value_str.data() + value_str.size();
  if (type_str == "i32") {
    uint32_t value;
    if (Failed((ParseInt32(value_start, value_end, &value,
                           ParseIntType::UnsignedOnly)))) {
      PrintError("invalid i32 literal");
      return wabt::Result::Error;
    }
    out_value->type = ValueType::I32;
    out_value->value.Set(value);
  } else if (type_str == "f32") {
    uint32_t value_bits;
    if (Failed(ParseInt32(value_start, value_end, &value_bits,
                            ParseIntType::UnsignedOnly))) {
      PrintError("invalid f32 literal");
      return wabt::Result::Error;
    }
    out_value->type = ValueType::F32;
    out_value->value.Set(Bitcast<f32>(value_bits));
  } else if (type_str == "i64") {
    uint64_t value;
    if (Failed(ParseInt64(value_start, value_end, &value,
                          ParseIntType::UnsignedOnly))) {
      PrintError("invalid i64 literal");
      return wabt::Result::Error;
    }
    out_value->type = ValueType::I64;
    out_value->value.Set(value);
  } else if (type_str == "f64") {
    uint64_t value_bits;
    if (Failed((ParseInt64(value_start, value_end, &value_bits,
                           ParseIntType::UnsignedOnly)))) {
      PrintError("invalid f64 literal");
      return wabt::Result::Error;
    }
    out_value->type = ValueType::F64;
    out_value->value.Set(Bitcast<f64>(value_bits));
  } else if (type_str == "v128") {
    v128 value_bits;
    if (Failed(ParseUint128(value_start, value_end, &value_bits))) {
      PrintError("invalid v128 literal");
      return wabt::Result::Error;
    }
    out_value->type = ValueType::V128;
    out_value->value.Set(value_bits);
  } else if (type_str == "nullref") {
    out_value->type = ValueType::Nullref;
    out_value->value.Set(Ref::Null);
  } else if (type_str == "hostref") {
    uint32_t value;
    if (Failed(ParseInt32(value_start, value_end, &value,
                          ParseIntType::UnsignedOnly))) {
      PrintError("invalid hostref literal");
      return wabt::Result::Error;
    }
    out_value->type = ValueType::Hostref;
    // TODO: hack, just whatever ref is at this index; but skip null (which is
    // always 0).
    out_value->value.Set(Ref{value + 1});
  } else if (type_str == "funcref") {
    uint32_t value;
    if (Failed(ParseInt32(value_start, value_end, &value,
                          ParseIntType::UnsignedOnly))) {
      PrintError("invalid funcref literal");
      return wabt::Result::Error;
    }
    out_value->type = ValueType::Funcref;
    out_value->value.Set(Ref{value});
  } else {
    PrintError("unknown concrete type: \"%s\"", type_str.to_string().c_str());
    return wabt::Result::Error;
  }

  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseExpectedValue(ExpectedValue* out_value) {
  std::string type_str;
  std::string value_str;
  EXPECT("{");
  PARSE_KEY_STRING_VALUE("type", &type_str);
  EXPECT(",");
  PARSE_KEY_STRING_VALUE("value", &value_str);
  EXPECT("}");

  if (type_str == "f32" || type_str == "f64") {
    out_value->value.type = type_str == "f32" ? ValueType::F32 : ValueType::F64;
    if (value_str == "nan:canonical") {
      out_value->is_expected_nan = true;
      out_value->expectedNan = ExpectedNan::Canonical;
      return wabt::Result::Ok;
    } else if (value_str == "nan:arithmetic") {
      out_value->is_expected_nan = true;
      out_value->expectedNan = ExpectedNan::Arithmetic;
      return wabt::Result::Ok;
    }
  }

  out_value->is_expected_nan = false;
  return ParseConstValue(&out_value->value, type_str, value_str);
}

wabt::Result JSONParser::ParseExpectedValues(
    std::vector<ExpectedValue>* out_values) {
  out_values->clear();
  EXPECT("[");
  bool first = true;
  while (!Match("]")) {
    if (!first) {
      EXPECT(",");
    }
    ExpectedValue value;
    CHECK_RESULT(ParseExpectedValue(&value));
    out_values->push_back(value);
    first = false;
  }
  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseConstVector(ValueTypes* out_types, Values* out_values) {
  out_values->clear();
  EXPECT("[");
  bool first = true;
  while (!Match("]")) {
    if (!first) {
      EXPECT(",");
    }
    TypedValue tv;
    CHECK_RESULT(ParseConst(&tv));
    out_types->push_back(tv.type);
    out_values->push_back(tv.value);
    first = false;
  }
  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseAction(Action* out_action) {
  EXPECT_KEY("action");
  EXPECT("{");
  EXPECT_KEY("type");
  if (Match("\"invoke\"")) {
    out_action->type = ActionType::Invoke;
  } else {
    EXPECT("\"get\"");
    out_action->type = ActionType::Get;
  }
  EXPECT(",");
  if (Match("\"module\"")) {
    EXPECT(":");
    CHECK_RESULT(ParseString(&out_action->module_name));
    EXPECT(",");
  }
  PARSE_KEY_STRING_VALUE("field", &out_action->field_name);
  if (out_action->type == ActionType::Invoke) {
    EXPECT(",");
    EXPECT_KEY("args");
    CHECK_RESULT(ParseConstVector(&out_action->types, &out_action->args));
  }
  EXPECT("}");
  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseActionResult() {
  // Not needed for wabt-interp, but useful for other parsers.
  EXPECT_KEY("expected");
  TypeVector expected;
  CHECK_RESULT(ParseTypeVector(&expected));
  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseModuleType(ModuleType* out_type) {
  std::string module_type_str;

  PARSE_KEY_STRING_VALUE("module_type", &module_type_str);
  if (module_type_str == "text") {
    *out_type = ModuleType::Text;
    return wabt::Result::Ok;
  } else if (module_type_str == "binary") {
    *out_type = ModuleType::Binary;
    return wabt::Result::Ok;
  } else {
    PrintError("unknown module type: \"%s\"", module_type_str.c_str());
    return wabt::Result::Error;
  }
}

static string_view GetDirname(string_view path) {
  // Strip everything after and including the last slash (or backslash), e.g.:
  //
  // s = "foo/bar/baz", => "foo/bar"
  // s = "/usr/local/include/stdio.h", => "/usr/local/include"
  // s = "foo.bar", => ""
  // s = "some\windows\directory", => "some\windows"
  size_t last_slash = path.find_last_of('/');
  size_t last_backslash = path.find_last_of('\\');
  if (last_slash == string_view::npos) {
    last_slash = 0;
  }
  if (last_backslash == string_view::npos) {
    last_backslash = 0;
  }

  return path.substr(0, std::max(last_slash, last_backslash));
}

std::string JSONParser::CreateModulePath(string_view filename) {
  string_view spec_json_filename = loc_.filename;
  string_view dirname = GetDirname(spec_json_filename);
  std::string path;

  if (dirname.size() == 0) {
    path = filename.to_string();
  } else {
    path = dirname.to_string();
    path += '/';
    path += filename.to_string();
  }

  ConvertBackslashToSlash(&path);
  return path;
}

wabt::Result JSONParser::ParseFilename(std::string* out_filename) {
  PARSE_KEY_STRING_VALUE("filename", out_filename);
  *out_filename = CreateModulePath(*out_filename);
  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseCommand(CommandPtr* out_command) {
  EXPECT("{");
  EXPECT_KEY("type");
  if (Match("\"module\"")) {
    auto command = MakeUnique<ModuleCommand>();
    EXPECT(",");
    CHECK_RESULT(ParseLine(&command->line));
    EXPECT(",");
    CHECK_RESULT(ParseOptNameStringValue(&command->name));
    CHECK_RESULT(ParseFilename(&command->filename));
    *out_command = std::move(command);
  } else if (Match("\"action\"")) {
    auto command = MakeUnique<ActionCommand>();
    EXPECT(",");
    CHECK_RESULT(ParseLine(&command->line));
    EXPECT(",");
    CHECK_RESULT(ParseAction(&command->action));
    EXPECT(",");
    CHECK_RESULT(ParseActionResult());
    *out_command = std::move(command);
  } else if (Match("\"register\"")) {
    auto command = MakeUnique<RegisterCommand>();
    EXPECT(",");
    CHECK_RESULT(ParseLine(&command->line));
    EXPECT(",");
    CHECK_RESULT(ParseOptNameStringValue(&command->name));
    PARSE_KEY_STRING_VALUE("as", &command->as);
    *out_command = std::move(command);
  } else if (Match("\"assert_malformed\"")) {
    auto command = MakeUnique<AssertMalformedCommand>();
    EXPECT(",");
    CHECK_RESULT(ParseLine(&command->line));
    EXPECT(",");
    CHECK_RESULT(ParseFilename(&command->filename));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &command->text);
    EXPECT(",");
    CHECK_RESULT(ParseModuleType(&command->type));
    *out_command = std::move(command);
  } else if (Match("\"assert_invalid\"")) {
    auto command = MakeUnique<AssertInvalidCommand>();
    EXPECT(",");
    CHECK_RESULT(ParseLine(&command->line));
    EXPECT(",");
    CHECK_RESULT(ParseFilename(&command->filename));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &command->text);
    EXPECT(",");
    CHECK_RESULT(ParseModuleType(&command->type));
    *out_command = std::move(command);
  } else if (Match("\"assert_unlinkable\"")) {
    auto command = MakeUnique<AssertUnlinkableCommand>();
    EXPECT(",");
    CHECK_RESULT(ParseLine(&command->line));
    EXPECT(",");
    CHECK_RESULT(ParseFilename(&command->filename));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &command->text);
    EXPECT(",");
    CHECK_RESULT(ParseModuleType(&command->type));
    *out_command = std::move(command);
  } else if (Match("\"assert_uninstantiable\"")) {
    auto command = MakeUnique<AssertUninstantiableCommand>();
    EXPECT(",");
    CHECK_RESULT(ParseLine(&command->line));
    EXPECT(",");
    CHECK_RESULT(ParseFilename(&command->filename));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &command->text);
    EXPECT(",");
    CHECK_RESULT(ParseModuleType(&command->type));
    *out_command = std::move(command);
  } else if (Match("\"assert_return\"")) {
    auto command = MakeUnique<AssertReturnCommand>();
    EXPECT(",");
    CHECK_RESULT(ParseLine(&command->line));
    EXPECT(",");
    CHECK_RESULT(ParseAction(&command->action));
    EXPECT(",");
    EXPECT_KEY("expected");
    CHECK_RESULT(ParseExpectedValues(&command->expected));
    *out_command = std::move(command);
  } else if (Match("\"assert_trap\"")) {
    auto command = MakeUnique<AssertTrapCommand>();
    EXPECT(",");
    CHECK_RESULT(ParseLine(&command->line));
    EXPECT(",");
    CHECK_RESULT(ParseAction(&command->action));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &command->text);
    EXPECT(",");
    CHECK_RESULT(ParseActionResult());
    *out_command = std::move(command);
  } else if (Match("\"assert_exhaustion\"")) {
    auto command = MakeUnique<AssertExhaustionCommand>();
    EXPECT(",");
    CHECK_RESULT(ParseLine(&command->line));
    EXPECT(",");
    CHECK_RESULT(ParseAction(&command->action));
    EXPECT(",");
    PARSE_KEY_STRING_VALUE("text", &command->text);
    EXPECT(",");
    CHECK_RESULT(ParseActionResult());
    *out_command = std::move(command);
  } else {
    PrintError("unknown command type");
    return wabt::Result::Error;
  }
  EXPECT("}");
  return wabt::Result::Ok;
}

wabt::Result JSONParser::ParseScript(Script* out_script) {
  EXPECT("{");
  PARSE_KEY_STRING_VALUE("source_filename", &out_script->filename);
  EXPECT(",");
  EXPECT_KEY("commands");
  EXPECT("[");
  bool first = true;
  while (!Match("]")) {
    CommandPtr command;
    if (!first) {
      EXPECT(",");
    }
    CHECK_RESULT(ParseCommand(&command));
    out_script->commands.push_back(std::move(command));
    first = false;
  }
  EXPECT("}");
  return wabt::Result::Ok;
}

struct ActionResult {
  ValueTypes types;
  Values values;
  Trap::Ptr trap;
};

class CommandRunner {
 public:
  CommandRunner();
  wabt::Result Run(const Script& script);

  int passed() const { return passed_; }
  int total() const { return total_; }

 private:
  using ExportMap = std::map<std::string, Extern::Ptr>;
  using Registry = std::map<std::string, ExportMap>;

  void WABT_PRINTF_FORMAT(3, 4)
      PrintError(uint32_t line_number, const char* format, ...);
  ActionResult RunAction(int line_number,
                         const Action* action,
                         RunVerbosity verbose);

  interp::Module::Ptr ReadModule(string_view module_filename, Errors* errors);
  Extern::Ptr GetImport(const std::string&, const std::string&);
  void PopulateImports(const interp::Module::Ptr&, RefVec*);
  void PopulateExports(const Instance::Ptr&, ExportMap*);

  wabt::Result OnModuleCommand(const ModuleCommand*);
  wabt::Result OnActionCommand(const ActionCommand*);
  wabt::Result OnRegisterCommand(const RegisterCommand*);
  wabt::Result OnAssertMalformedCommand(const AssertMalformedCommand*);
  wabt::Result OnAssertUnlinkableCommand(const AssertUnlinkableCommand*);
  wabt::Result OnAssertInvalidCommand(const AssertInvalidCommand*);
  wabt::Result OnAssertUninstantiableCommand(
      const AssertUninstantiableCommand*);
  wabt::Result OnAssertReturnCommand(const AssertReturnCommand*);
  wabt::Result OnAssertTrapCommand(const AssertTrapCommand*);
  wabt::Result OnAssertExhaustionCommand(const AssertExhaustionCommand*);

  void TallyCommand(wabt::Result);

  wabt::Result ReadInvalidTextModule(string_view module_filename,
                                     const std::string& header);
  wabt::Result ReadInvalidModule(int line_number,
                           string_view module_filename,
                           ModuleType module_type,
                           const char* desc);
  wabt::Result ReadUnlinkableModule(int line_number,
                              string_view module_filename,
                              ModuleType module_type,
                              const char* desc);

  Store store_;
  Registry registry_;  // Used when importing.
  Registry instances_;  // Used when referencing module by name in invoke.
  ExportMap last_instance_;
  int passed_ = 0;
  int total_ = 0;

  std::string source_filename_;
};

CommandRunner::CommandRunner() : store_(s_features) {
  auto&& spectest = registry_["spectest"];

  // Initialize print functions for the spec test.
  struct {
    const char* name;
    interp::FuncType type;
  } const print_funcs[] = {
      {"print", interp::FuncType{{}, {}}},
      {"print_i32", interp::FuncType{{ValueType::I32}, {}}},
      {"print_f32", interp::FuncType{{ValueType::F32}, {}}},
      {"print_f64", interp::FuncType{{ValueType::F64}, {}}},
      {"print_i32_f32", interp::FuncType{{ValueType::I32, ValueType::F32}, {}}},
      {"print_f64_f64", interp::FuncType{{ValueType::F64, ValueType::F64}, {}}},
  };

  for (auto&& print : print_funcs) {
    auto import_name = StringPrintf("spectest.%s", print.name);
    spectest[print.name] = HostFunc::New(
        store_, print.type,
        [=](const Values& params, Values& results, Trap::Ptr* trap) -> wabt::Result {
          printf("called host ");
          WriteCall(s_stdout_stream.get(), import_name, print.type, params,
                    results, *trap);
          return wabt::Result::Ok;
        });
  }

  spectest["table"] =
      interp::Table::New(store_, TableType{ValueType::Funcref, Limits{10, 20}});

  spectest["memory"] = interp::Memory::New(store_, MemoryType{Limits{1, 2}});

  spectest["global_i32"] = interp::Global::New(
      store_, GlobalType{ValueType::I32, Mutability::Const}, Value::Make(u32{666}));
  spectest["global_i64"] = interp::Global::New(
      store_, GlobalType{ValueType::I64, Mutability::Const}, Value::Make(u64{666}));
  spectest["global_f32"] = interp::Global::New(
      store_, GlobalType{ValueType::F32, Mutability::Const}, Value::Make(f32{666}));
  spectest["global_f64"] = interp::Global::New(
      store_, GlobalType{ValueType::F64, Mutability::Const}, Value::Make(f64{666}));
}

wabt::Result CommandRunner::Run(const Script& script) {
  source_filename_ = script.filename;

  for (const CommandPtr& command : script.commands) {
    switch (command->type) {
      case CommandType::Module:
        OnModuleCommand(cast<ModuleCommand>(command.get()));
        break;

      case CommandType::Action:
        TallyCommand(OnActionCommand(cast<ActionCommand>(command.get())));
        break;

      case CommandType::Register:
        OnRegisterCommand(cast<RegisterCommand>(command.get()));
        break;

      case CommandType::AssertMalformed:
        TallyCommand(OnAssertMalformedCommand(
            cast<AssertMalformedCommand>(command.get())));
        break;

      case CommandType::AssertInvalid:
        TallyCommand(
            OnAssertInvalidCommand(cast<AssertInvalidCommand>(command.get())));
        break;

      case CommandType::AssertUnlinkable:
        TallyCommand(OnAssertUnlinkableCommand(
            cast<AssertUnlinkableCommand>(command.get())));
        break;

      case CommandType::AssertUninstantiable:
        TallyCommand(OnAssertUninstantiableCommand(
            cast<AssertUninstantiableCommand>(command.get())));
        break;

      case CommandType::AssertReturn:
        TallyCommand(
            OnAssertReturnCommand(cast<AssertReturnCommand>(command.get())));
        break;

      case CommandType::AssertTrap:
        TallyCommand(
            OnAssertTrapCommand(cast<AssertTrapCommand>(command.get())));
        break;

      case CommandType::AssertExhaustion:
        TallyCommand(OnAssertExhaustionCommand(
            cast<AssertExhaustionCommand>(command.get())));
        break;
    }
  }

  return wabt::Result::Ok;
}

void CommandRunner::PrintError(uint32_t line_number, const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  printf("%s:%u: %s\n", source_filename_.c_str(), line_number, buffer);
}

ActionResult CommandRunner::RunAction(int line_number,
                                      const Action* action,
                                      RunVerbosity verbose) {
  ExportMap& module = !action->module_name.empty()
                          ? instances_[action->module_name]
                          : last_instance_;
  Extern::Ptr extern_ = module[action->field_name];
  if (!extern_) {
    PrintError(line_number, "unknown invoke \"%s.%s\"",
               action->module_name.c_str(), action->field_name.c_str());
    return {};
  }

  ActionResult result;

  switch (action->type) {
    case ActionType::Invoke: {
      auto* func = cast<interp::Func>(extern_.get());
      func->Call(store_, action->args, result.values, &result.trap,
                 s_trace_stream);
      result.types = func->type().results;
      if (verbose == RunVerbosity::Verbose) {
        WriteCall(s_stdout_stream.get(), action->field_name, func->type(),
                  action->args, result.values, result.trap);
      }
      break;
    }

    case ActionType::Get: {
      auto* global = cast<interp::Global>(extern_.get());
      result.values.push_back(global->Get());
      result.types.push_back(global->type().type);
      break;
    }

    default:
      WABT_UNREACHABLE;
  }

  return result;
}

wabt::Result CommandRunner::ReadInvalidTextModule(string_view module_filename,
                                            const std::string& header) {
  std::vector<uint8_t> file_data;
  wabt::Result result = ReadFile(module_filename, &file_data);
  std::unique_ptr<WastLexer> lexer = WastLexer::CreateBufferLexer(
      module_filename, file_data.data(), file_data.size());
  Errors errors;
  if (Succeeded(result)) {
    std::unique_ptr<wabt::Module> module;
    WastParseOptions options(s_features);
    result = ParseWatModule(lexer.get(), &module, &errors, &options);
  }

  auto line_finder = lexer->MakeLineFinder();
  FormatErrorsToFile(errors, Location::Type::Text, line_finder.get(), stdout,
                     header, PrintHeader::Once);
  return result;
}

interp::Module::Ptr CommandRunner::ReadModule(string_view module_filename,
                                               Errors* errors) {
  std::vector<uint8_t> file_data;

  if (Failed(ReadFile(module_filename, &file_data))) {
    return {};
  }

  const bool kReadDebugNames = true;
  const bool kStopOnFirstError = true;
  const bool kFailOnCustomSectionError = true;
  ReadBinaryOptions options(s_features, s_log_stream.get(), kReadDebugNames,
                            kStopOnFirstError, kFailOnCustomSectionError);
  ModuleDesc module_desc;
  if (Failed(ReadBinaryInterp(file_data.data(), file_data.size(), options,
                              errors, &module_desc))) {
    return {};
  }

  if (s_verbose) {
    module_desc.istream.Disassemble(s_stdout_stream.get());
  }

  return interp::Module::New(store_, module_desc);
}

wabt::Result CommandRunner::ReadInvalidModule(int line_number,
                                        string_view module_filename,
                                        ModuleType module_type,
                                        const char* desc) {
  std::string header = StringPrintf(
      "%s:%d: %s passed", source_filename_.c_str(), line_number, desc);

  switch (module_type) {
    case ModuleType::Text: {
      return ReadInvalidTextModule(module_filename, header);
    }

    case ModuleType::Binary: {
      Errors errors;
      auto module = ReadModule(module_filename, &errors);
      if (!module) {
        FormatErrorsToFile(errors, Location::Type::Binary, {}, stdout, header,
                           PrintHeader::Once);
        return wabt::Result::Error;
      } else {
        return wabt::Result::Ok;
      }
    }
  }

  WABT_UNREACHABLE;
}

Extern::Ptr CommandRunner::GetImport(const std::string& module,
                                     const std::string& name) {
  auto mod_iter = registry_.find(module);
  if (mod_iter != registry_.end()) {
    auto extern_iter = mod_iter->second.find(name);
    if (extern_iter != mod_iter->second.end()) {
      return extern_iter->second;
    }
  }
  return {};
}

void CommandRunner::PopulateImports(const interp::Module::Ptr& module,
                                    RefVec* imports) {
  for (auto&& import : module->desc().imports) {
    auto extern_ = GetImport(import.type.module, import.type.name);
    imports->push_back(extern_ ? extern_.ref() : Ref::Null);
  }
}

void CommandRunner::PopulateExports(const Instance::Ptr& instance,
                                    ExportMap* map) {
  map->clear();
  interp::Module::Ptr module{store_, instance->module()};
  for (size_t i = 0; i < module->export_types().size(); ++i) {
    const ExportType& export_type = module->export_types()[i];
    (*map)[export_type.name] = store_.UnsafeGet<Extern>(instance->exports()[i]);
  }
}

wabt::Result CommandRunner::OnModuleCommand(const ModuleCommand* command) {
  Errors errors;
  auto module = ReadModule(command->filename, &errors);
  FormatErrorsToFile(errors, Location::Type::Binary);

  if (!module) {
    PrintError(command->line, "error reading module: \"%s\"",
               command->filename.c_str());
    return wabt::Result::Error;
  }

  RefVec imports;
  PopulateImports(module, &imports);

  Trap::Ptr trap;
  auto instance = Instance::Instantiate(store_, module.ref(), imports, &trap);
  if (trap) {
    assert(!instance);
    PrintError(command->line, "error instantiating module: \"%s\"",
               trap->message().c_str());
    return wabt::Result::Error;
  }

  PopulateExports(instance, &last_instance_);
  if (!command->name.empty()) {
    instances_[command->name] = last_instance_;
  }

  return wabt::Result::Ok;
}

wabt::Result CommandRunner::OnActionCommand(const ActionCommand* command) {
  ActionResult result =
      RunAction(command->line, &command->action, RunVerbosity::Verbose);

  if (result.trap) {
    PrintError(command->line, "unexpected trap: %s",
               result.trap->message().c_str());
    return wabt::Result::Error;
  }

  return wabt::Result::Ok;
}

wabt::Result CommandRunner::OnAssertMalformedCommand(
    const AssertMalformedCommand* command) {
  wabt::Result result = ReadInvalidModule(command->line, command->filename,
                                    command->type, "assert_malformed");
  if (Succeeded(result)) {
    PrintError(command->line, "expected module to be malformed: \"%s\"",
               command->filename.c_str());
    return wabt::Result::Error;
  }

  return wabt::Result::Ok;
}

wabt::Result CommandRunner::OnRegisterCommand(const RegisterCommand* command) {
  if (!command->name.empty()) {
    auto instance_iter = instances_.find(command->name);
    if (instance_iter == instances_.end()) {
      PrintError(command->line, "unknown module in register");
      return wabt::Result::Error;
    }
    registry_[command->as] = instance_iter->second;
  } else {
    registry_[command->as] = last_instance_;
  }

  return wabt::Result::Ok;
}

wabt::Result CommandRunner::OnAssertUnlinkableCommand(
    const AssertUnlinkableCommand* command) {
  Errors errors;
  auto module = ReadModule(command->filename, &errors);

  if (!module) {
    PrintError(command->line, "unable to compile unlinkable module: \"%s\"",
               command->filename.c_str());
    return wabt::Result::Error;
  }

  RefVec imports;
  PopulateImports(module, &imports);

  Trap::Ptr trap;
  auto instance = Instance::Instantiate(store_, module.ref(), imports, &trap);
  if (!trap) {
    PrintError(command->line, "expected module to be unlinkable: \"%s\"",
               command->filename.c_str());
    return wabt::Result::Error;
  }

  // TODO: Change to one-line error.
  PrintError(command->line, "assert_unlinkable passed:\n  error: %s",
             trap->message().c_str());
  return wabt::Result::Ok;
}

wabt::Result CommandRunner::OnAssertInvalidCommand(
    const AssertInvalidCommand* command) {
  wabt::Result result = ReadInvalidModule(command->line, command->filename,
                                    command->type, "assert_invalid");
  if (Succeeded(result)) {
    PrintError(command->line, "expected module to be invalid: \"%s\"",
               command->filename.c_str());
    return wabt::Result::Error;
  }

  return wabt::Result::Ok;
}

wabt::Result CommandRunner::OnAssertUninstantiableCommand(
    const AssertUninstantiableCommand* command) {
  Errors errors;
  auto module = ReadModule(command->filename, &errors);

  if (!module) {
    PrintError(command->line, "unable to compile uninstantiable module: \"%s\"",
               command->filename.c_str());
    return wabt::Result::Error;
  }

  RefVec imports;
  PopulateImports(module, &imports);

  Trap::Ptr trap;
  auto instance = Instance::Instantiate(store_, module.ref(), imports, &trap);
  if (!trap) {
    PrintError(command->line, "expected module to be uninstantiable: \"%s\"",
               command->filename.c_str());
    return wabt::Result::Error;
  }

  // TODO: print error when assertion passes.
#if 0
  PrintError(command->line, "assert_uninstantiable passed: %s",
             trap->message().c_str());
#endif
  return wabt::Result::Ok;
}

static bool TypedValuesAreEqual(const TypedValue& expected,
                                const TypedValue& actual) {
  assert(expected.type == actual.type || IsReference(expected.type));
  switch (expected.type) {
    case Type::I32:
      return expected.value.Get<u32>() == actual.value.Get<u32>();

    case Type::F32:
      return Bitcast<u32>(expected.value.Get<f32>()) ==
             Bitcast<u32>(actual.value.Get<f32>());

    case Type::I64:
      return expected.value.Get<u64>() == actual.value.Get<u64>();

    case Type::F64:
      return Bitcast<u64>(expected.value.Get<f64>()) ==
             Bitcast<u64>(actual.value.Get<f64>());

    case Type::V128:
      return expected.value.Get<v128>() == actual.value.Get<v128>();

    case Type::Nullref:
      return actual.value.Get<Ref>() == Ref::Null;

    case Type::Funcref:
    case Type::Hostref:
      return expected.value.Get<Ref>() == actual.value.Get<Ref>();

    default:
      WABT_UNREACHABLE;
  }
}

static bool WABT_VECTORCALL IsCanonicalNan(f32 val) {
  const u32 kQuietNan = 0x7fc00000U;
  const u32 kQuietNegNan = 0xffc00000U;
  u32 bits = Bitcast<u32>(val);
  return bits == kQuietNan || bits == kQuietNegNan;
}

static bool WABT_VECTORCALL IsCanonicalNan(f64 val) {
  const u64 kQuietNan = 0x7ff8000000000000ULL;
  const u64 kQuietNegNan = 0xfff8000000000000ULL;
  u64 bits = Bitcast<u64>(val);
  return bits == kQuietNan || bits == kQuietNegNan;
}

static bool WABT_VECTORCALL IsArithmeticNan(f32 val) {
  const u32 kQuietNan = 0x7fc00000U;
  return (Bitcast<u32>(val) & kQuietNan) == kQuietNan;
}

static bool WABT_VECTORCALL IsArithmeticNan(f64 val) {
  const u64 kQuietNan = 0x7ff8000000000000ULL;
  return (Bitcast<u64>(val) & kQuietNan) == kQuietNan;
}

wabt::Result CommandRunner::OnAssertReturnCommand(
    const AssertReturnCommand* command) {
  ActionResult action_result =
      RunAction(command->line, &command->action, RunVerbosity::Quiet);

  if (action_result.trap) {
    PrintError(command->line, "unexpected trap: %s",
               action_result.trap->message().c_str());
    return wabt::Result::Error;
  }

  if (action_result.values.size() != command->expected.size()) {
    PrintError(command->line,
               "result length mismatch in assert_return: expected %" PRIzd
               ", got %" PRIzd,
               command->expected.size(), action_result.values.size());
    return wabt::Result::Error;
  }

  wabt::Result result = wabt::Result::Ok;
  for (size_t i = 0; i < action_result.values.size(); ++i) {
    const ExpectedValue& expected = command->expected[i];
    TypedValue actual{action_result.types[i], action_result.values[i]};

    if (expected.is_expected_nan) {
      bool is_nan;
      if (expected.expectedNan == ExpectedNan::Arithmetic) {
        if (expected.value.type == Type::F64) {
          is_nan = IsArithmeticNan(actual.value.Get<f64>());
        } else {
          is_nan = IsArithmeticNan(actual.value.Get<f32>());
        }
      } else if (expected.expectedNan == ExpectedNan::Canonical) {
        if (expected.value.type == Type::F64) {
          is_nan = IsCanonicalNan(actual.value.Get<f64>());
        } else {
          is_nan = IsCanonicalNan(actual.value.Get<f32>());
        }
      } else {
        WABT_UNREACHABLE;
      }
      if (!is_nan) {
        PrintError(command->line, "expected result to be nan, got %s",
                   TypedValueToString(actual).c_str());
        result = wabt::Result::Error;
      }
    } else if (expected.value.type == Type::Funcref) {
      if (!store_.HasValueType(actual.value.Get<Ref>(), Type::Funcref)) {
        PrintError(command->line,
                   "mismatch in result %" PRIzd
                   " of assert_return: expected funcref, got %s",
                   i, TypedValueToString(actual).c_str());
      }
    } else {
      if (!TypedValuesAreEqual(expected.value, actual)) {
        PrintError(command->line,
                   "mismatch in result %" PRIzd
                   " of assert_return: expected %s, got %s",
                   i, TypedValueToString(expected.value).c_str(),
                   TypedValueToString(actual).c_str());
        result = wabt::Result::Error;
      }
    }
  }

  return result;
}

wabt::Result CommandRunner::OnAssertTrapCommand(
    const AssertTrapCommand* command) {
  ActionResult result =
      RunAction(command->line, &command->action, RunVerbosity::Quiet);
  if (!result.trap) {
    PrintError(command->line, "expected trap: \"%s\"", command->text.c_str());
    return wabt::Result::Error;
  }

  PrintError(command->line, "assert_trap passed: %s",
             result.trap->message().c_str());
  return wabt::Result::Ok;
}

wabt::Result CommandRunner::OnAssertExhaustionCommand(
    const AssertExhaustionCommand* command) {
  ActionResult result =
      RunAction(command->line, &command->action, RunVerbosity::Quiet);
  if (!result.trap || result.trap->message() != "call stack exhausted") {
    PrintError(command->line, "expected trap: \"%s\"", command->text.c_str());
    return wabt::Result::Error;
  }

  // TODO: print message when assertion passes.
#if 0
  PrintError(command->line, "assert_exhaustion passed: %s",
             result.trap->message().c_str());
#endif
  return wabt::Result::Ok;
}

void CommandRunner::TallyCommand(wabt::Result result) {
  if (Succeeded(result)) {
    passed_++;
  }
  total_++;
}

static int ReadAndRunSpecJSON(string_view spec_json_filename) {
  JSONParser parser;
  if (parser.ReadFile(spec_json_filename) == wabt::Result::Error) {
    return 1;
  }

  Script script;
  if (parser.ParseScript(&script) == wabt::Result::Error) {
    return 1;
  }

  CommandRunner runner;
  if (runner.Run(script) == wabt::Result::Error) {
    return 1;
  }

  printf("%d/%d tests passed.\n", runner.passed(), runner.total());
  const int failed = runner.total() - runner.passed();
  return failed;
}

}  // namespace spectest

int ProgramMain(int argc, char** argv) {
  InitStdio();
  s_stdout_stream = FileStream::CreateStdout();

  ParseOptions(argc, argv);
  return spectest::ReadAndRunSpecJSON(s_infile);
}

int main(int argc, char** argv) {
  WABT_TRY
  return ProgramMain(argc, argv);
  WABT_CATCH_BAD_ALLOC_AND_EXIT
}

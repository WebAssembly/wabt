/*
 * Copyright 2017 WebAssembly Community Group participants
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

#include "gtest/gtest.h"

#include <memory>

#include "wabt/cast.h"
#include "wabt/wast-lexer.h"
#include "wabt/wast-parser.h"

using namespace wabt;

namespace {

std::string repeat(std::string s, size_t count) {
  std::string result;
  for (size_t i = 0; i < count; ++i) {
    result += s;
  }
  return result;
}

Errors ParseInvalidModule(std::string text) {
  Errors errors;
  auto lexer =
      WastLexer::CreateBufferLexer("test", text.c_str(), text.size(), &errors);
  std::unique_ptr<Module> module;
  Features features;
  WastParseOptions options(features);
  Result result = ParseWatModule(lexer.get(), &module, &errors, &options);
  EXPECT_EQ(Result::Error, result);

  return errors;
}

}  // end of anonymous namespace

TEST(WastParser, LongToken) {
  std::string text;
  text = "(module (memory ";
  text += repeat("a", 0x5000);
  text += "))";

  Errors errors = ParseInvalidModule(text);
  ASSERT_EQ(1u, errors.size());

  ASSERT_EQ(ErrorLevel::Error, errors[0].error_level);
  ASSERT_EQ(1, errors[0].loc.line);
  ASSERT_EQ(17, errors[0].loc.first_column);
  ASSERT_STREQ(
      R"(unexpected token "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa...", expected a natural number (e.g. 123).)",
      errors[0].message.c_str());
}

TEST(WastParser, LongTokenSpace) {
  std::string text;
  text = "notparen";
  text += repeat(" ", 0x10000);
  text += "notmodule";

  Errors errors = ParseInvalidModule(text);
  ASSERT_EQ(2u, errors.size());

  ASSERT_EQ(ErrorLevel::Error, errors[0].error_level);
  ASSERT_EQ(1, errors[0].loc.line);
  ASSERT_EQ(1, errors[0].loc.first_column);
  ASSERT_STREQ(
      R"(unexpected token "notparen", expected a module field or a module.)",
      errors[0].message.c_str());

  ASSERT_EQ(1, errors[1].loc.line);
  ASSERT_EQ(65545, errors[1].loc.first_column);
  ASSERT_STREQ(R"(unexpected token notmodule, expected EOF.)",
               errors[1].message.c_str());
}

TEST(WastParser, InvalidBinaryModule) {
  std::string text = R"((module binary "\00asm\bc\0a\00\00"))";
  std::vector<uint8_t> expected_data = {
      0, 'a', 's', 'm', static_cast<uint8_t>('\xbc'), '\x0a', 0, 0};

  Errors errors;
  auto lexer =
      WastLexer::CreateBufferLexer("test", text.c_str(), text.size(), &errors);
  std::unique_ptr<Script> script;
  Features features;
  WastParseOptions options(features);
  options.parse_binary_modules = false;
  Result result = ParseWastScript(lexer.get(), &script, &errors, &options);
  EXPECT_EQ(result, Result::Ok);
  EXPECT_EQ(0u, errors.size());
  EXPECT_EQ(1u, script->commands.size());

  Command* cmd = script->commands[0].get();
  EXPECT_TRUE(isa<ScriptModuleCommand>(cmd));

  auto* mod_cmd = cast<ScriptModuleCommand>(cmd);
  ScriptModule* script_mod = mod_cmd->script_module.get();
  EXPECT_NE(script_mod, nullptr);

  EXPECT_TRUE(isa<BinaryScriptModule>(script_mod));
  auto* binary_mod = cast<BinaryScriptModule>(script_mod);

  EXPECT_EQ(expected_data, binary_mod->data);
}

TEST(WastParser, ModuleDefinition) {
  std::string text = "(module definition)";

  Errors errors;
  auto lexer =
      WastLexer::CreateBufferLexer("test", text.c_str(), text.size(), &errors);
  Features features;
  WastParseOptions options(features);

  std::unique_ptr<Script> script;
  Result result = ParseWastScript(lexer.get(), &script, &errors, &options);
  ASSERT_EQ(result, Result::Ok);
  ASSERT_EQ(script->commands.size(), 1U);
  Command* cmd = script->commands[0].get();
  auto* mod_cmd = cast<ScriptModuleCommand>(cmd);
  ASSERT_NE(mod_cmd, nullptr);
  ASSERT_TRUE(mod_cmd->script_module->is_definition);
}

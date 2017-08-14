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

#ifndef WABT_WAST_PARSER_H_
#define WABT_WAST_PARSER_H_

#include "circular-array.h"
#include "ir.h"
#include "intrusive-list.h"
#include "wast-lexer.h"

namespace wabt {

class ErrorHandler;

struct WastParseOptions {
  bool allow_future_exceptions = false;
  bool debug_parsing = false;
};

class WastParser {
 public:
  WastParser(WastLexer*, ErrorHandler*, WastParseOptions*);

  void WABT_PRINTF_FORMAT(3, 4) Error(Location, const char* format, ...);
  Result ParseScript(Script*);

 private:
  void ErrorUnlessExceptionsAllowed();
  Result ErrorExpected(const std::vector<std::string>& expected,
                       const char* example = nullptr);
  Result ErrorIfLpar(const std::vector<std::string>& expected,
                     const char* example = nullptr);

  Token GetToken();
  Location GetLocation();
  TokenType Peek();
  TokenType PeekAfter();
  bool PeekMatch(TokenType);
  bool PeekMatchLpar(TokenType);
  bool PeekMatchExpr();
  bool Match(TokenType);
  bool MatchLpar(TokenType);
  Result Expect(TokenType);
  Token Consume();
  // Give the Match() function a clearer name when used to optionally consume a
  // token (used for printing better error messages).
  void ConsumeIfLpar() { Match(TokenType::Lpar); }

  typedef bool SynchronizeFunc(TokenType peek, TokenType peek_after);
  Result Synchronize(SynchronizeFunc);

  void ParseBindVarOpt(std::string* name);
  Result ParseVar(Var* out_var);
  bool ParseVarOpt(Var* out_var, Var default_var = Var());
  Result ParseOffsetExpr(ExprList* out_expr_list);
  Result ParseTextList(std::vector<uint8_t>* out_data);
  bool ParseTextListOpt(std::vector<uint8_t>* out_data);
  Result ParseVarList(VarVector* out_var_list);
  bool ParseVarListOpt(VarVector* out_var_list);
  Result ParseValueType(Type* out_type);
  Result ParseValueTypeList(TypeVector* out_type_list);
  Result ParseQuotedText(std::string* text);
  bool ParseOffsetOpt(uint32_t* offset);
  bool ParseAlignOpt(uint32_t* align);
  Result ParseLimits(Limits*);
  Result ParseNat(uint64_t*);

  Result ParseModuleFieldList(Module*);
  Result ParseModuleField(Module*);
  Result ParseDataModuleField(Module*);
  Result ParseElemModuleField(Module*);
  Result ParseExceptModuleField(Module*);
  Result ParseExportModuleField(Module*);
  Result ParseFuncModuleField(Module*);
  Result ParseTypeModuleField(Module*);
  Result ParseGlobalModuleField(Module*);
  Result ParseImportModuleField(Module*);
  Result ParseMemoryModuleField(Module*);
  Result ParseStartModuleField(Module*);
  Result ParseTableModuleField(Module*);

  Result ParseExportDesc(Export*);
  Result ParseInlineExports(ModuleFieldList*, ExternalKind);
  Result ParseInlineImport(Import*);
  Result ParseTypeUseOpt(FuncDeclaration*);
  Result ParseFuncSignature(FuncSignature*, BindingHash* param_bindings);
  Result ParseBoundValueTypeList(TokenType, TypeVector*, BindingHash*);
  Result ParseResultList(TypeVector*);
  Result ParseInstrList(ExprList*);
  Result ParseTerminatingInstrList(ExprList*);
  Result ParseInstr(ExprList*);
  Result ParsePlainInstr(std::unique_ptr<Expr>*);
  Result ParseConst(Const*);
  Result ParseConstList(ConstVector*);
  Result ParseBlockInstr(std::unique_ptr<Expr>*);
  Result ParseLabelOpt(std::string*);
  Result ParseEndLabelOpt(const std::string&);
  Result ParseBlock(Block*);
  Result ParseExprList(ExprList*);
  Result ParseExpr(ExprList*);
  Result ParseCatchInstrList(CatchVector* catches);
  Result ParseCatchExprList(CatchVector* catches);
  Result ParseGlobalType(Global*);

  template <typename T>
  Result ParsePlainInstrVar(Location, std::unique_ptr<Expr>*);

  Result ParseCommandList(CommandPtrVector*);
  Result ParseCommand(CommandPtr*);
  Result ParseAssertExhaustionCommand(CommandPtr*);
  Result ParseAssertInvalidCommand(CommandPtr*);
  Result ParseAssertMalformedCommand(CommandPtr*);
  Result ParseAssertReturnCommand(CommandPtr*);
  Result ParseAssertReturnArithmeticNanCommand(CommandPtr*);
  Result ParseAssertReturnCanonicalNanCommand(CommandPtr*);
  Result ParseAssertTrapCommand(CommandPtr*);
  Result ParseAssertUnlinkableCommand(CommandPtr*);
  Result ParseActionCommand(CommandPtr*);
  Result ParseModuleCommand(CommandPtr*);
  Result ParseRegisterCommand(CommandPtr*);

  Result ParseAction(Action*);
  Result ParseScriptModule(std::unique_ptr<ScriptModule>*);

  template <typename T>
  Result ParseActionCommand(TokenType, CommandPtr*);
  template <typename T>
  Result ParseAssertActionCommand(TokenType, CommandPtr*);
  template <typename T>
  Result ParseAssertActionTextCommand(TokenType, CommandPtr*);
  template <typename T>
  Result ParseAssertScriptModuleCommand(TokenType, CommandPtr*);

  void CheckImportOrdering(Module*);

  WastLexer* lexer_;
  Script* script_ = nullptr;
  Index last_module_index_ = kInvalidIndex;
  ErrorHandler* error_handler_;
  int errors_ = 0;
  WastParseOptions* options_;

  CircularArray<Token, 2> tokens_;
};

Result ParseWast(WastLexer* lexer,
                 Script** out_script,
                 ErrorHandler*,
                 WastParseOptions* options = nullptr);

}  // namespace wabt

#endif /* WABT_WAST_PARSER_H_ */

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

#include <array>
#include <memory>
#include <optional>
#include <unordered_map>

#include "wabt/error.h"
#include "wabt/feature.h"
#include "wabt/intrusive-list.h"
#include "wabt/ir.h"
#include "wabt/wast-lexer.h"

namespace wabt {

struct WastParseOptions {
  WastParseOptions(const Features& features) : features(features) {}

  Features features;
  bool debug_parsing = false;
};

using TokenTypePair = std::array<TokenType, 2>;

class WastParser {
 public:
  WastParser(WastLexer*, Errors*, WastParseOptions*);

  void WABT_PRINTF_FORMAT(3, 4) Error(Location, const char* format, ...);
  Result ParseModule(std::unique_ptr<Module>* out_module);
  Result ParseScript(std::unique_ptr<Script>* out_script);

  std::unique_ptr<Script> ReleaseScript();

 private:
  enum class ConstType {
    Normal,
    Expectation,
  };

  void ErrorUnlessOpcodeEnabled(const Token&);

  // Print an error message listing the expected tokens, as well as an example
  // of expected input.
  Result ErrorExpected(const std::vector<std::string>& expected,
                       const char* example = nullptr);

  // Print an error message, and and return Result::Error if the next token is
  // '('. This is commonly used after parsing a sequence of s-expressions -- if
  // no more can be parsed, we know that a following '(' is invalid. This
  // function consumes the '(' so a better error message can be provided
  // (assuming the following token was unexpected).
  Result ErrorIfLpar(const std::vector<std::string>& expected,
                     const char* example = nullptr);

  // Returns the next token without consuming it.
  Token GetToken();

  // Returns the location of the next token.
  Location GetLocation();

  // Returns the type of the next token.
  TokenType Peek(size_t n = 0);

  // Returns the types of the next two tokens.
  TokenTypePair PeekPair();

  // Returns true if the next token's type is equal to the parameter.
  bool PeekMatch(TokenType, size_t n = 0);

  // Returns true if the next token's type is '(' and the following token is
  // equal to the parameter.
  bool PeekMatchLpar(TokenType);

  // Returns true if the next two tokens can start an Expr. This allows for
  // folded expressions, plain instructions and block instructions.
  bool PeekMatchExpr();

  // Returns true if the next two tokens are form reference type - (ref $t)
  bool PeekMatchRefType();

  // Returns true if the next token's type is equal to the parameter. If so,
  // then the token is consumed.
  bool Match(TokenType);

  // Returns true if the next token's type is equal to '(' and the following
  // token is equal to the parameter. If so, then the token is consumed.
  bool MatchLpar(TokenType);

  // Like Match(), but prints an error message if the token doesn't match, and
  // returns Result::Error.
  Result Expect(TokenType);

  // Consume one token and return it.
  Token Consume();

  // Give the Match() function a clearer name when used to optionally consume a
  // token (used for printing better error messages).
  void ConsumeIfLpar() { Match(TokenType::Lpar); }

  using SynchronizeFunc = bool(*)(TokenTypePair pair);

  // Attempt to synchronize the token stream by dropping tokens until the
  // SynchronizeFunc returns true, or until a token limit is reached. This
  // function returns Result::Error if the stream was not able to be
  // synchronized.
  Result Synchronize(SynchronizeFunc);

  bool ParseBindVarOpt(std::string* name);
  Result ParseVar(Var* out_var);
  bool ParseVarOpt(Var* out_var, Var default_var = Var());
  Result ParseOffsetExpr(ExprList* out_expr_list);
  bool ParseOffsetExprOpt(ExprList* out_expr_list);
  Result ParseTextList(std::vector<uint8_t>* out_data);
  bool ParseTextListOpt(std::vector<uint8_t>* out_data);
  Result ParseVarList(VarVector* out_var_list);
  bool ParseElemExprOpt(ExprList* out_elem_expr);
  bool ParseElemExprListOpt(ExprListVector* out_list);
  bool ParseElemExprVarListOpt(ExprListVector* out_list);
  Result ParseValueType(Var* out_type);
  Result ParseValueTypeList(
      TypeVector* out_type_list,
      std::unordered_map<uint32_t, std::string>* type_names);
  Result ParseRefKind(Type* out_type);
  Result ParseRefType(Type* out_type);
  bool ParseRefTypeOpt(Type* out_type);
  Result ParseQuotedText(std::string* text, bool check_utf8 = true);
  bool ParseOffsetOpt(Address* offset);
  bool ParseAlignOpt(Address* align);
  Result ParseMemidx(Location loc, Var* memidx);
  Result ParseLimitsIndex(Limits*);
  Result ParseLimits(Limits*);
  Result ParsePageSize(uint32_t*);
  Result ParseNat(uint64_t*, bool is_64);

  Result ParseModuleFieldList(Module*);
  Result ParseModuleField(Module*);
  Result ParseDataModuleField(Module*);
  Result ParseElemModuleField(Module*);
  Result ParseTagModuleField(Module*);
  Result ParseExportModuleField(Module*);
  Result ParseFuncModuleField(Module*);
  Result ParseTypeModuleField(Module*);
  Result ParseGlobalModuleField(Module*);
  Result ParseImportModuleField(Module*);
  Result ParseMemoryModuleField(Module*);
  Result ParseStartModuleField(Module*);
  Result ParseTableModuleField(Module*);

  Result ParseCustomSectionAnnotation(Module*);
  bool PeekIsAnnotation(const char* name);

  Result ParseExportDesc(Export*);
  Result ParseInlineExports(ModuleFieldList*, ExternalKind);
  Result ParseInlineImport(Import*);
  Result ParseTypeUseOpt(FuncDeclaration*);
  Result ParseFuncSignature(FuncSignature*, BindingHash* param_bindings);
  Result ParseUnboundFuncSignature(FuncSignature*);
  Result ParseBoundValueTypeList(TokenType,
                                 TypeVector*,
                                 BindingHash*,
                                 std::unordered_map<uint32_t, std::string>*,
                                 Index binding_index_offset = 0);
  Result ParseUnboundValueTypeList(TokenType,
                                   TypeVector*,
                                   std::unordered_map<uint32_t, std::string>*);
  Result ParseResultList(TypeVector*,
                         std::unordered_map<uint32_t, std::string>*);
  Result ParseInstrList(ExprList*);
  Result ParseTerminatingInstrList(ExprList*);
  Result ParseInstr(ExprList*);
  Result ParseCodeMetadataAnnotation(ExprList*);
  Result ParsePlainInstr(std::unique_ptr<Expr>*);
  Result ParseF32(Const*, ConstType type);
  Result ParseF64(Const*, ConstType type);
  Result ParseConst(Const*, ConstType type);
  Result ParseExpectedValues(ExpectationPtr*);
  Result ParseEither(ConstVector*);
  Result ParseExternref(Const*);
  Result ParseExpectedNan(ExpectedNan* expected);
  Result ParseConstList(ConstVector*, ConstType type);
  Result ParseBlockInstr(std::unique_ptr<Expr>*);
  Result ParseLabelOpt(std::string*);
  Result ParseEndLabelOpt(const std::string&);
  Result ParseBlockDeclaration(BlockDeclaration*);
  Result ParseBlock(Block*);
  Result ParseExprList(ExprList*);
  Result ParseExpr(ExprList*);
  Result ParseTryTableCatches(TryTableVector* catches);
  Result ParseCatchInstrList(CatchVector* catches);
  Result ParseCatchExprList(CatchVector* catches);
  Result ParseGlobalType(Global*);
  Result ParseField(Field*);
  Result ParseFieldList(std::vector<Field>*);

  template <typename T>
  Result ParsePlainInstrVar(Location, std::unique_ptr<Expr>*);
  template <typename T>
  Result ParseMemoryInstrVar(Location, std::unique_ptr<Expr>*);
  template <typename T>
  Result ParseLoadStoreInstr(Location, Token, std::unique_ptr<Expr>*);
  template <typename T>
  Result ParseSIMDLoadStoreInstr(Location loc,
                                 Token token,
                                 std::unique_ptr<Expr>* out_expr);
  template <typename T>
  Result ParseMemoryExpr(Location, std::unique_ptr<Expr>*);
  template <typename T>
  Result ParseMemoryBinaryExpr(Location, std::unique_ptr<Expr>*);
  Result ParseSimdLane(Location, uint64_t*);

  Result ParseCommandList(Script*, CommandPtrVector*);
  Result ParseCommand(Script*, CommandPtr*);
  Result ParseAssertExceptionCommand(CommandPtr*);
  Result ParseAssertExhaustionCommand(CommandPtr*);
  Result ParseAssertInvalidCommand(CommandPtr*);
  Result ParseAssertMalformedCommand(CommandPtr*);
  Result ParseAssertReturnCommand(CommandPtr*);
  Result ParseAssertReturnFuncCommand(CommandPtr*);
  Result ParseAssertTrapCommand(CommandPtr*);
  Result ParseAssertUnlinkableCommand(CommandPtr*);
  Result ParseActionCommand(CommandPtr*);
  Result ParseModuleCommand(Script*, CommandPtr*);
  Result ParseRegisterCommand(CommandPtr*);
  Result ParseInputCommand(CommandPtr*);
  Result ParseOutputCommand(CommandPtr*);

  Result ParseAction(ActionPtr*);
  Result ParseScriptModule(std::unique_ptr<ScriptModule>*);

  template <typename T>
  Result ParseActionCommand(TokenType, CommandPtr*);
  template <typename T>
  Result ParseAssertActionCommand(TokenType, CommandPtr*);
  template <typename T>
  Result ParseAssertActionTextCommand(TokenType, CommandPtr*);
  template <typename T>
  Result ParseAssertScriptModuleCommand(TokenType, CommandPtr*);

  Result ParseSimdV128Const(Const*, TokenType, ConstType);

  void CheckImportOrdering(Module*);
  bool HasError() const;

  WastLexer* lexer_;
  Index last_module_index_ = kInvalidIndex;
  Errors* errors_;
  WastParseOptions* options_;

  // two-element queue of upcoming tokens
  class TokenQueue {
    std::array<std::optional<Token>, 2> tokens{};
    bool i{};

   public:
    void push_back(Token t);
    void pop_front();
    const Token& at(size_t n) const;
    const Token& front() const;
    bool empty() const;
    size_t size() const;
  };

  TokenQueue tokens_{};
};

Result ParseWatModule(WastLexer* lexer,
                      std::unique_ptr<Module>* out_module,
                      Errors*,
                      WastParseOptions* options);

Result ParseWastScript(WastLexer* lexer,
                       std::unique_ptr<Script>* out_script,
                       Errors*,
                       WastParseOptions* options);

}  // namespace wabt

#endif /* WABT_WAST_PARSER_H_ */

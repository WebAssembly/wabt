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

#ifndef WABT_WAST_LEXER_H_
#define WABT_WAST_LEXER_H_

#include <cstddef>
#include <cstdio>
#include <memory>

#include "src/common.h"
#include "src/lexer-source-line-finder.h"
#include "src/literal.h"
#include "src/make-unique.h"
#include "src/opcode.h"
#include "src/token.h"

namespace wabt {

class ErrorHandler;
class LexerSource;
class WastParser;

class WastLexer {
 public:
  WABT_DISALLOW_COPY_AND_ASSIGN(WastLexer);

  WastLexer(std::unique_ptr<LexerSource> source, string_view filename);

  // Convenience functions.
  static std::unique_ptr<WastLexer> CreateBufferLexer(string_view filename,
                                                      const void* data,
                                                      size_t size);

  Token GetToken(WastParser* parser);
  Result Fill(size_t need);

  // TODO(binji): Move this out of the lexer.
  std::unique_ptr<LexerSourceLineFinder> MakeLineFinder() {
    return MakeUnique<LexerSourceLineFinder>(source_->Clone());
  }

 private:
  static const int kEof = -1;

  Location GetLocation();
  Literal MakeLiteral(LiteralType);
  std::string GetText(size_t at = 0);

  int PeekChar();
  int ReadChar();
  bool MatchChar(char c);

  int ReadDigits();
  int ReadHexDigits();
  int ReadReservedChars();
  void ReadSign();
  Token ReadString(WastParser*);
  Token ReadNumber(WastParser*, TokenType);
  Token ReadHexNumber(WastParser*, TokenType);
  Token ReadInf(WastParser*);
  Token ReadNan(WastParser*);
  Token ReadName(WastParser*);
  Token ReadKeyword(WastParser*);

  std::unique_ptr<LexerSource> source_;
  std::string filename_;
  int line_;
  const char* buffer_;
  const char* buffer_end_;
  size_t line_file_offset_;
  const char* token_start_;
  const char* cursor_;
};

}  // namespace wabt

#endif /* WABT_WAST_LEXER_H_ */

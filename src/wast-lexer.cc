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

#include "src/wast-lexer.h"

#include <cassert>
#include <cstdio>

#include "config.h"

#include "src/lexer-source.h"
#include "src/wast-parser.h"

#define ERROR(...) parser->Error(GetLocation(), __VA_ARGS__)

/* p must be a pointer somewhere in the lexer buffer */
#define FILE_OFFSET(p) ((p) - (buffer_))
#define COLUMN(p) (FILE_OFFSET(p) - line_file_offset_ + 1)

#define NEWLINE                               \
  do {                                        \
    line_++;                                  \
    line_file_offset_ = FILE_OFFSET(cursor_); \
  } while (0)

#define RETURN(token) return Token(GetLocation(), token);

#define RETURN_LITERAL(token, literal) \
  return Token(GetLocation(), token, Literal(LiteralType::literal, GetText()))

#define RETURN_TYPE(token, type) return Token(GetLocation(), token, type)
#define RETURN_OPCODE(token, opcode) return Token(GetLocation(), token, opcode)
#define RETURN_TEXT(token) return Token(GetLocation(), token, GetText())

#define RETURN_TEXT_AT(token, at) \
  return Token(GetLocation(), token, GetText(at))

namespace wabt {

namespace {

#include "src/prebuilt/lexer-keywords.cc"

}  // namespace

WastLexer::WastLexer(std::unique_ptr<LexerSource> source, string_view filename)
    : source_(std::move(source)),
      filename_(filename),
      line_(1),
      buffer_(static_cast<const char*>(source_->data())),
      buffer_end_(buffer_ + source_->size()),
      line_file_offset_(0),
      token_start_(buffer_),
      cursor_(buffer_) {}

// static
std::unique_ptr<WastLexer> WastLexer::CreateBufferLexer(string_view filename,
                                                        const void* data,
                                                        size_t size) {
  return MakeUnique<WastLexer>(MakeUnique<LexerSource>(data, size), filename);
}

Location WastLexer::GetLocation() {
  return Location(filename_, line_, COLUMN(token_start_), COLUMN(cursor_));
}

std::string WastLexer::GetText(size_t offset) {
  return std::string(token_start_ + offset, (cursor_ - token_start_) - offset);
}

int WastLexer::PeekChar() {
  return cursor_ < buffer_end_ ? *cursor_ : kEof;
}

int WastLexer::ReadChar() {
  return cursor_ < buffer_end_ ? *cursor_++ : kEof;
}

bool WastLexer::MatchChar(char c) {
  if (PeekChar() == c) {
    ReadChar();
    return true;
  }
  return false;
}

bool WastLexer::MatchString(string_view s) {
  const char* saved_cursor = cursor_;
  for (char c : s) {
    if (ReadChar() != c) {
      cursor_ = saved_cursor;
      return false;
    }
  }
  return true;
}

Token WastLexer::GetToken(WastParser* parser) {
  while (1) {
    token_start_ = cursor_;
    switch (ReadChar()) {
      case kEof: RETURN(TokenType::Eof);

      case '(':
        if (MatchChar(';')) {
          // Block comment.
          int nesting = 1;
          bool in_comment = true;
          while (in_comment) {
            switch (ReadChar()) {
              case kEof:
                ERROR("EOF in block comment");
                RETURN(TokenType::Eof);

              case ';':
                if (MatchChar(')') && --nesting == 0) {
                  in_comment = false;
                  break;
                }
                break;

              case '(':
                if (MatchChar(';')) {
                  nesting++;
                }
                break;

              case '\n': NEWLINE; break;
            }
          }
          continue;
        } else {
          RETURN(TokenType::Lpar);
        }
        break;

      case ')': RETURN(TokenType::Rpar);

      case ';':
        if (MatchChar(';')) {
          // Line comment.
          bool in_comment = true;
          while (in_comment) {
            switch (ReadChar()) {
              case kEof: RETURN(TokenType::Eof);
              case '\n': NEWLINE; in_comment = false; break;
            }
          }
          continue;
        } else {
          ERROR("unexpected char");
          continue;
        }
        break;

      case ' ':
      case '\t':
      case '\r':  // Whitespace.
        continue;

      case '\n': NEWLINE; continue;

      case '"': // String.
        return ReadString(parser);

      case '+':
      case '-':
        switch (PeekChar()) {
          case 'i':
            ReadChar();
            return ReadInf();

          case 'n':
            ReadChar();
            return ReadNan();

          case '0':
            ReadChar();
            return MatchChar('x') ? ReadHexNumber(TokenType::Int)
                                  : ReadNumber(TokenType::Int);
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            ReadChar();
            return ReadNumber(TokenType::Int);

          default: RETURN_TEXT(TokenType::Reserved);
        }
        break;

      case '0':
        return MatchChar('x') ? ReadHexNumber(TokenType::Nat)
                              : ReadNumber(TokenType::Nat);

      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':  // Number.
        return ReadNumber(TokenType::Nat);

      case '$':  // Name.
        return ReadName();

      case 'a':
        return ReadNameEqNum("lign=", TokenType::AlignEqNat);

      case 'i':
        return ReadInf();

      case 'n':
        return ReadNan();

      case 'o':
        return ReadNameEqNum("ffset=", TokenType::OffsetEqNat);

      default:
        if (IsCharClass(cursor_[-1], CharClass::Reserved)) {
          return ReadKeyword();
        }
        ERROR("unexpected char");
        continue;
    }
  }
}

Token WastLexer::ReadString(WastParser* parser) {
  bool in_string = true;
  while (in_string) {
    switch (ReadChar()) {
      case kEof: RETURN(TokenType::Eof);
      case '\n':
        ERROR("newline in string");
        NEWLINE;
        continue;
      case '\\':
        switch (ReadChar()) {
          case 't':
          case 'n':
          case 'r':
          case '"':
          case '\'':
          case '\\':
            // Valid escape.
            break;

          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
          case 'f':
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
          case 'F':  // Hex byte escape.
            // TODO
            break;

          default:
            ERROR("bad escape");
            break;
        }
        break;
      case '"':
        in_string = false;
        break;
      default:
        break;  // TODO Check for utf-8.
    }
  }
  RETURN_TEXT(TokenType::Text);
}

// static
bool WastLexer::IsCharClass(int c, CharClass bit) {
  // Generated by the following python script:
  //
  //   def Range(c, lo, hi): return lo <= c.lower() <= hi
  //   def IsDigit(c): return Range(c, '0', '9') or c == '_'
  //   def IsHexDigit(c): return IsDigit(c) or Range(c, 'a', 'f')
  //   def IsReserved(c): return Range(c, '!', '~') and c not in '"(),;[]{}'
  //
  //   print [
  //     0,  # For EOF==-1.
  //     (1 if IsDigit(c) else 0) |
  //     (2 if IsHexDigit(c) else 0) |
  //     (4 if IsReserved(c) else 0)
  //     for c in map(chr, range(0, 128))
  //   ]
  static const char kCharClasses[257] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 4, 4, 4, 4, 4, 0, 0, 4,
      4, 0, 4, 4, 4, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 0, 4, 4, 4, 4, 4,
      6, 6, 6, 6, 6, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 0, 4, 0, 4, 7, 4, 6, 6, 6, 6, 6, 6, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 0, 4, 0,
  };

  assert(c >= -1 && c < 256);
  return (kCharClasses[c + 1] & static_cast<int>(bit)) != 0;
}

template <WastLexer::CharClass cc>
int WastLexer::ReadCharClass() {
  int count = 0;
  while (IsCharClass(PeekChar(), cc)) {
    ReadChar();
    ++count;
  }
  return count;
}

void WastLexer::ReadSign() {
  if (PeekChar() == '+' || PeekChar() == '-') {
    ReadChar();
  }
}

Token WastLexer::ReadNumber(TokenType token_type) {
  // We've already read the first digit in GetToken above.
  ReadDigits();
  if (MatchChar('.')) {
    token_type = TokenType::Float;
    ReadDigits();
  }
  if (tolower(PeekChar()) == 'e') {
    token_type = TokenType::Float;
    ReadChar();
    ReadSign();
    if (ReadDigits() == 0) {
      RETURN_TEXT(TokenType::Reserved);
    }
  }
  if (ReadReservedChars() != 0) {
    RETURN_TEXT(TokenType::Reserved);
  }
  if (token_type == TokenType::Float) {
    RETURN_LITERAL(token_type, Float);
  } else {
    RETURN_LITERAL(token_type, Int);
  }
}

Token WastLexer::ReadHexNumber(TokenType token_type) {
  // We've only read the '0x' part, we need to ensure that there's at least one
  // hex digit.
  if (ReadHexDigits() == 0) {
    RETURN_TEXT(TokenType::Reserved);
  }
  if (MatchChar('.')) {
    token_type = TokenType::Float;
    ReadHexDigits();
  }
  if (tolower(PeekChar()) == 'p') {
    token_type = TokenType::Float;
    ReadChar();
    ReadSign();
    if (ReadHexDigits() == 0) {
      RETURN_TEXT(TokenType::Reserved);
    }
  }
  if (ReadReservedChars() != 0) {
    RETURN_TEXT(TokenType::Reserved);
  }
  if (token_type == TokenType::Float) {
    RETURN_LITERAL(token_type, Hexfloat);
  } else {
    RETURN_LITERAL(token_type, Int);
  }
}

Token WastLexer::ReadInf() {
  // We've already read the `i`.
  if (MatchString("nf")) {
    if (ReadReservedChars() != 0) {
      RETURN_TEXT(TokenType::Reserved);
    }
    RETURN_LITERAL(TokenType::Float, Infinity);
  }
  return ReadKeyword();
}

Token WastLexer::ReadNan() {
  // We've already read the `n`.
  if (MatchString("an")) {
    if (MatchChar(':')) {
      if (MatchString("0x") && ReadHexDigits() != 0 &&
          ReadReservedChars() == 0) {
        RETURN_LITERAL(TokenType::Float, Nan);
      }
    } else if (ReadReservedChars() == 0) {
      RETURN_LITERAL(TokenType::Float, Nan);
    }
  }
  return ReadKeyword();
}

Token WastLexer::ReadNameEqNum(string_view name, TokenType token_type) {
  if (MatchString(name)) {
    if (MatchString("0x")) {
      if (ReadHexDigits() != 0 && ReadReservedChars() == 0) {
        RETURN_TEXT_AT(token_type, name.size() + 1);
      }
    } else if (ReadDigits() != 0 && ReadReservedChars() == 0) {
      RETURN_TEXT_AT(token_type, name.size() + 1);
    }
  }
  return ReadKeyword();
}

Token WastLexer::ReadName() {
  // We've already read the leading `$`, but we need at least one additional
  // character.
  if (ReadReservedChars() == 0) {
    RETURN_TEXT(TokenType::Reserved);
  }
  RETURN_TEXT(TokenType::Var);
}

Token WastLexer::ReadKeyword() {
  ReadReservedChars();
  TokenInfo* info =
      Perfect_Hash::InWordSet(token_start_, cursor_ - token_start_);
  if (!info) {
    RETURN_TEXT(TokenType::Reserved);
  }
  if (IsTokenTypeBare(info->token_type)) {
    RETURN(info->token_type);
  } else if (IsTokenTypeType(info->token_type)) {
    RETURN_TYPE(info->token_type, info->value_type);
  } else {
    assert(IsTokenTypeOpcode(info->token_type));
    RETURN_OPCODE(info->token_type, info->opcode);
  }
}

}  // namespace wabt

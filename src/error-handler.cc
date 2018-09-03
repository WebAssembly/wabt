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

#include "src/error-handler.h"

#include <algorithm>

namespace wabt {

ErrorHandler::ErrorHandler(Location::Type location_type)
    : location_type_(location_type) {}

ErrorHandler::ErrorHandler(Location::Type location_type,
                           std::unique_ptr<LexerSourceLineFinder> line_finder)
    : location_type_(location_type), line_finder_(std::move(line_finder)) {}

bool ErrorHandler::OnError(ErrorLevel error_level,
                           const Location& loc,
                           const char* format,
                           va_list args) {
  va_list args_copy;
  va_copy(args_copy, args);
  char fixed_buf[WABT_DEFAULT_SNPRINTF_ALLOCA_BUFSIZE];
  char* buffer = fixed_buf;
  size_t len = wabt_vsnprintf(fixed_buf, sizeof(fixed_buf), format, args);
  if (len + 1 > sizeof(fixed_buf)) {
    buffer = static_cast<char*>(alloca(len + 1));
    len = wabt_vsnprintf(buffer, len + 1, format, args_copy);
  }

  LexerSourceLineFinder::SourceLine source_line;
  if (line_finder_) {
    Result result = line_finder_->GetSourceLine(loc, source_line_max_length(),
                                                &source_line);
    if (Failed(result)) {
      // If this fails, it means that we've probably screwed up the lexer. Blow
      // up.
      WABT_FATAL("error getting the source line.\n");
    }
  }

  return OnError(ErrorLevel::Error, loc, std::string(buffer), source_line.line,
                 source_line.column_offset);
}

std::string ErrorHandler::DefaultErrorMessage(ErrorLevel error_level,
                                              const Color& color,
                                              const Location& loc,
                                              const std::string& error,
                                              const std::string& source_line,
                                              size_t source_line_column_offset,
                                              int indent) {
  std::string indent_str(indent, ' ');
  std::string result = indent_str;

  result += color.MaybeBoldCode();

  if (!loc.filename.empty()) {
    result += loc.filename.to_string();
    result += ":";
  }

  if (location_type_ == Location::Type::Text) {
    result += StringPrintf("%d:%d: ", loc.line, loc.first_column);
  } else if (loc.offset != kInvalidOffset) {
    result += StringPrintf("%07" PRIzx ": ", loc.offset);
  }

  result += color.MaybeRedCode();
  result += GetErrorLevelName(error_level);
  result += ": ";
  result += color.MaybeDefaultCode();

  result += error;
  result += '\n';

  if (!source_line.empty()) {
    result += indent_str;
    result += source_line;
    result += '\n';
    result += indent_str;

    size_t num_spaces = (loc.first_column - 1) - source_line_column_offset;
    size_t num_carets = loc.last_column - loc.first_column;
    num_carets = std::min(num_carets, source_line.size() - num_spaces);
    num_carets = std::max<size_t>(num_carets, 1);
    result.append(num_spaces, ' ');
    result += color.MaybeBoldCode();
    result += color.MaybeGreenCode();
    result.append(num_carets, '^');
    result += color.MaybeDefaultCode();
    result += '\n';
  }

  return result;
}

ErrorHandlerNop::ErrorHandlerNop()
    // The Location::Type is irrelevant since we never display an error.
    : ErrorHandler(Location::Type::Text) {}

ErrorHandlerFile::ErrorHandlerFile(
    Location::Type location_type,
    std::unique_ptr<LexerSourceLineFinder> line_finder,
    FILE* file,
    const std::string& header,
    PrintHeader print_header,
    size_t source_line_max_length)
    : ErrorHandler(location_type, std::move(line_finder)),
      file_(file),
      header_(header),
      print_header_(print_header),
      source_line_max_length_(source_line_max_length),
      color_(file) {}

bool ErrorHandlerFile::OnError(ErrorLevel level,
                               const Location& loc,
                               const std::string& error,
                               const std::string& source_line,
                               size_t source_line_column_offset) {
  PrintErrorHeader();
  int indent = header_.empty() ? 0 : 2;

  std::string message =
      DefaultErrorMessage(level, color_, loc, error, source_line,
                          source_line_column_offset, indent);
  fwrite(message.data(), 1, message.size(), file_);
  return true;
}

void ErrorHandlerFile::PrintErrorHeader() {
  if (header_.empty()) {
    return;
  }

  switch (print_header_) {
    case PrintHeader::Never:
      break;

    case PrintHeader::Once:
      print_header_ = PrintHeader::Never;
    // Fallthrough.

    case PrintHeader::Always:
      fprintf(file_, "%s:\n", header_.c_str());
      break;
  }
}

ErrorHandlerBuffer::ErrorHandlerBuffer(
    Location::Type location_type,
    std::unique_ptr<LexerSourceLineFinder> line_finder,
    size_t source_line_max_length)
    : ErrorHandler(location_type, std::move(line_finder)),
      source_line_max_length_(source_line_max_length),
      color_(nullptr, false) {}

bool ErrorHandlerBuffer::OnError(ErrorLevel level,
                                 const Location& loc,
                                 const std::string& error,
                                 const std::string& source_line,
                                 size_t source_line_column_offset) {
  buffer_ += DefaultErrorMessage(level, color_, loc, error, source_line,
                                 source_line_column_offset, 0);
  return true;
}

}  // namespace wabt

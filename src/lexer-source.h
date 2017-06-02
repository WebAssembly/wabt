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

#ifndef WABT_LEXER_SOURCE_H_
#define WABT_LEXER_SOURCE_H_

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "common.h"

namespace wabt {

template <typename T>
struct Range {
  Range() : start(0), end(0) {}
  Range(T start, T end) : start(start), end(end) {}
  T start;
  T end;

  T size() const { return end - start; }
};

typedef Range<Offset> OffsetRange;
typedef Range<int> ColumnRange;

class LexerSource {
 public:
  LexerSource() = default;
  virtual ~LexerSource() {}
  virtual std::unique_ptr<LexerSource> Clone() = 0;
  virtual Result Tell(Offset* out_offset) = 0;
  virtual size_t Fill(void* dest, size_t size) = 0;
  virtual Result ReadRange(OffsetRange, std::vector<char>* out_data) = 0;

  WABT_DISALLOW_COPY_AND_ASSIGN(LexerSource);
};

class LexerSourceFile : public LexerSource {
 public:
  explicit LexerSourceFile(const std::string& filename);
  ~LexerSourceFile();

  bool IsOpen() const { return file_ != nullptr; }

  std::unique_ptr<LexerSource> Clone() override;
  Result Tell(Offset* out_offset) override;
  size_t Fill(void* dest, size_t size) override;
  Result ReadRange(OffsetRange, std::vector<char>* out_data) override;

 private:
  Result Seek(Offset offset);

  std::string filename_;
  FILE* file_;
};

class LexerSourceBuffer : public LexerSource {
 public:
  LexerSourceBuffer(const void* data, Offset size);

  std::unique_ptr<LexerSource> Clone() override;
  Result Tell(Offset* out_offset) override;
  size_t Fill(void* dest, size_t size) override;
  Result ReadRange(OffsetRange, std::vector<char>* out_data) override;

 private:
  const void* data_;
  Offset size_;
  Offset read_offset_;
};

class LexerSourceLineFinder {
 public:
  struct SourceLine {
    std::string line;
    int column_offset;
  };

  explicit LexerSourceLineFinder(std::unique_ptr<LexerSource>);

  Result GetLine(int line,
                 ColumnRange column_range,
                 Offset max_line_length,
                 SourceLine* out_source_line);
  Result GetLineOffsets(int line, OffsetRange* out_offsets);

 private:
  static OffsetRange ClampSourceLineOffsets(OffsetRange line_offset_range,
                                            ColumnRange column_range,
                                            Offset max_line_length);

  bool IsLineCached(int) const;
  OffsetRange GetCachedLine(int) const;

  std::unique_ptr<LexerSource> source_;
  std::vector<OffsetRange> line_ranges_;
  Offset next_line_start_;
  bool last_cr_;  // Last read character was a '\r' (carriage return).
  bool eof_;
};

}  // namespace wabt

#endif  // WABT_LEXER_SOURCE_H_

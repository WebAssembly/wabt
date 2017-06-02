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

#include "lexer-source.h"

#include <algorithm>

#define CHECK_RESULT(expr)  \
  do {                      \
    if (WABT_FAILED(expr))  \
      return Result::Error; \
  } while (0)

namespace wabt {

LexerSourceFile::LexerSourceFile(const std::string& filename)
  : filename_(filename) {
  file_ = fopen(filename.c_str(), "rb");
}

LexerSourceFile::~LexerSourceFile() {
  if (file_)
    fclose(file_);
}

std::unique_ptr<LexerSource> LexerSourceFile::Clone() {
  std::unique_ptr<LexerSourceFile> result(new LexerSourceFile(filename_));

  Offset offset = 0;
  if (WABT_FAILED(Tell(&offset)) || WABT_FAILED(result->Seek(offset)))
    result.reset();

  return std::move(result);
}

Result LexerSourceFile::Tell(Offset* out_offset) {
  if (!file_)
    return Result::Error;

  long offset = ftell(file_);
  if (offset < 0)
    return Result::Error;

  *out_offset = offset;
  return Result::Ok;
}

size_t LexerSourceFile::Fill(void* dest, size_t size)  {
  if (!file_)
    return 0;
  return fread(dest, 1, size, file_);
}

Result LexerSourceFile::ReadRange(OffsetRange range,
                                  std::vector<char>* out_data) {
  Offset old_offset = 0;
  CHECK_RESULT(Tell(&old_offset));
  CHECK_RESULT(Seek(range.start));

  std::vector<char> result(range.size());
  size_t read_size = Fill(result.data(), range.size());
  if (read_size < range.size())
    result.resize(read_size);

  CHECK_RESULT(Seek(old_offset));

  *out_data = std::move(result);
  return Result::Ok;
}

Result LexerSourceFile::Seek(Offset offset) {
  if (!file_)
    return Result::Error;

  int result = fseek(file_, offset, SEEK_SET);
  return result < 0 ? Result::Error : Result::Ok;
}

LexerSourceBuffer::LexerSourceBuffer(const void* data, Offset size)
    : data_(data), size_(size), read_offset_(0) {}

std::unique_ptr<LexerSource> LexerSourceBuffer::Clone() {
  LexerSourceBuffer* result = new LexerSourceBuffer(data_, size_);
  result->read_offset_ = read_offset_;
  return std::unique_ptr<LexerSource>(result);
}

Result LexerSourceBuffer::Tell(Offset* out_offset) {
  *out_offset = read_offset_;
  return Result::Ok;
}

size_t LexerSourceBuffer::Fill(void* dest, Offset size) {
  Offset read_size = std::min(size, size_ - read_offset_);
  const void* src = static_cast<const char*>(data_) + read_offset_;
  memcpy(dest, src, read_size);
  read_offset_ += read_size;
  return read_size;
}

Result LexerSourceBuffer::ReadRange(OffsetRange range,
                                    std::vector<char>* out_data) {
  OffsetRange clamped = range;
  clamped.start = std::min(clamped.start, size_);
  clamped.end = std::min(clamped.end, size_);
  std::vector<char> result(clamped.size());
  const void* src = static_cast<const char*>(data_) + clamped.start;
  memcpy(result.data(), src, clamped.size());
  return Result::Ok;
}

LexerSourceLineFinder::LexerSourceLineFinder(
    std::unique_ptr<LexerSource> source)
    : source_(std::move(source)),
      next_line_start_(0),
      last_cr_(false),
      eof_(false) {
  // Line 0 should not be used; but it makes indexing simpler.
  line_ranges_.emplace_back(0, 0);
}

Result LexerSourceLineFinder::GetLine(int line,
                                      ColumnRange column_range,
                                      Offset max_line_length,
                                      SourceLine* out_source_line) {
  OffsetRange original;
  CHECK_RESULT(GetLineOffsets(line, &original));

  OffsetRange clamped =
      ClampSourceLineOffsets(original, column_range, max_line_length);
  bool has_start_ellipsis = original.start != clamped.start;
  bool has_end_ellipsis = original.end != clamped.end;

  out_source_line->column_offset = clamped.start - original.start;

  if (has_start_ellipsis) {
    out_source_line->line += "...";
    clamped.start += 3;
  }
  if (has_end_ellipsis)
    clamped.end -= 3;

  std::vector<char> read_line;
  CHECK_RESULT(source_->ReadRange(clamped, &read_line));
  out_source_line->line.append(read_line.begin(), read_line.end());

  if (has_end_ellipsis)
    out_source_line->line += "...";

  return Result::Ok;
}

bool LexerSourceLineFinder::IsLineCached(int line) const {
  return static_cast<size_t>(line) < line_ranges_.size();
}

OffsetRange LexerSourceLineFinder::GetCachedLine(int line) const {
  assert(IsLineCached(line));
  return line_ranges_[line];
}

Result LexerSourceLineFinder::GetLineOffsets(int find_line,
                                             OffsetRange* out_range) {
  if (IsLineCached(find_line)) {
    *out_range = GetCachedLine(find_line);
    return Result::Ok;
  }

  const size_t kBufferSize = 1 << 16;
  std::vector<char> buffer(kBufferSize);

  assert(!line_ranges_.empty());
  Offset buffer_file_offset = 0;
  CHECK_RESULT(source_->Tell(&buffer_file_offset));
  while (!IsLineCached(find_line) && !eof_) {
    size_t read_size = source_->Fill(buffer.data(), buffer.size());
    if (read_size < buffer.size())
      eof_ = true;

    for (auto iter = buffer.begin(), end = iter + read_size; iter < end;
         ++iter) {
      if (*iter == '\n') {
        // Don't include \n or \r in the line range.
        Offset line_offset =
            buffer_file_offset + (iter - buffer.begin()) - last_cr_;
        line_ranges_.emplace_back(next_line_start_, line_offset);
        next_line_start_ = line_offset + last_cr_ + 1;
      }
      last_cr_ = *iter == '\r';
    }

    if (eof_) {
      // Add the final line as an empty range.
      Offset end = buffer_file_offset + read_size;
      line_ranges_.emplace_back(end, end);
    }
  }

  if (IsLineCached(find_line)) {
    *out_range = GetCachedLine(find_line);
    return Result::Ok;
  } else {
    assert(eof_);
    return Result::Error;
  }
}

// static
OffsetRange LexerSourceLineFinder::ClampSourceLineOffsets(
    OffsetRange offset_range,
    ColumnRange column_range,
    Offset max_line_length) {
  Offset line_length = offset_range.size();
  if (line_length > max_line_length) {
    size_t column_count = column_range.size();
    size_t center_on;
    if (column_count > max_line_length) {
      // The column range doesn't fit, just center on first_column.
      center_on = column_range.start - 1;
    } else {
      // the entire range fits, display it all in the center.
      center_on = (column_range.start + column_range.end) / 2 - 1;
    }
    if (center_on > max_line_length / 2)
      offset_range.start += center_on - max_line_length / 2;
    offset_range.start =
        std::min(offset_range.start, offset_range.end - max_line_length);
    offset_range.end = offset_range.start + max_line_length;
  }

  return offset_range;
}

}  // namespace wabt

/*
 * Copyright 2026 WebAssembly Community Group participants
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

#include <algorithm>
#include <array>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

#include "wabt/stream.h"

#if COMPILER_IS_MSVC && defined(_WIN64)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <io.h>
#include <windows.h>
#include <winioctl.h>
#endif

using namespace wabt;

namespace {

struct FileCloser {
  void operator()(FILE* file) const { fclose(file); }
};
}  // namespace

#if SIZE_MAX > UINT32_MAX
TEST(FileStream, SeekAndPatchAbove4GB) {
  std::unique_ptr<FILE, FileCloser> file(tmpfile());
  ASSERT_NE(nullptr, file);
#if COMPILER_IS_MSVC
  // Keep the disk footprint small when seeking beyond the end on NTFS.
  HANDLE handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file.get())));
  DWORD bytes_returned;
  if (!DeviceIoControl(handle, FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0,
                       &bytes_returned, nullptr)) {
    GTEST_SKIP() << "This large-file test requires sparse-file support";
  }
#endif
  constexpr size_t kOffset = size_t{1} << 32;
  FileStream stream(file.get());
  const std::array<uint8_t, 1> initial{0x12};
  stream.WriteDataAt(kOffset, initial);
  ASSERT_EQ(Result::Ok, stream.result());
  const std::array<uint8_t, 1> patch{0xab};
  stream.WriteDataAt(kOffset, patch);
  ASSERT_EQ(Result::Ok, stream.result());
  stream.Flush();
#if COMPILER_IS_MSVC
  ASSERT_EQ(0, _fseeki64(file.get(), 0, SEEK_END));
  EXPECT_EQ(kOffset + 1, static_cast<size_t>(_ftelli64(file.get())));
  ASSERT_EQ(0, _fseeki64(file.get(), kOffset, SEEK_SET));
#else
  ASSERT_EQ(0, fseek(file.get(), 0, SEEK_END));
  EXPECT_EQ(kOffset + 1, static_cast<size_t>(ftell(file.get())));
  ASSERT_EQ(0, fseek(file.get(), kOffset, SEEK_SET));
#endif
  EXPECT_EQ(0xab, fgetc(file.get()));
  EXPECT_EQ(EOF, fgetc(file.get()));
  EXPECT_EQ(0, ferror(file.get()));
}
#endif

TEST(FileStream, LargeWriteAndPatch) {
  std::unique_ptr<FILE, FileCloser> file(tmpfile());
  ASSERT_NE(nullptr, file);
  constexpr size_t kChunkSize = 64 * 1024 * 1024;
  std::vector<uint8_t> data(kChunkSize + 17, 0x5a);
  data.front() = 0x12;
  data.back() = 0x34;
  FileStream stream(file.get());
  stream.WriteData(data);
  ASSERT_EQ(Result::Ok, stream.result());

  // Patch inside the large output, exercising the file-seek path.
  const std::array<uint8_t, 3> patch{0xab, 0xcd, 0xef};
  stream.WriteDataAt(kChunkSize - 1, patch);
  ASSERT_EQ(Result::Ok, stream.result());
  std::copy(patch.begin(), patch.end(), data.begin() + kChunkSize - 1);
  stream.Flush();
  ASSERT_EQ(0, fseek(file.get(), 0, SEEK_SET));

  std::array<uint8_t, 4096> buffer;
  size_t offset = 0;
  while (offset < data.size()) {
    size_t count = std::min(buffer.size(), data.size() - offset);
    ASSERT_EQ(count, fread(buffer.data(), 1, count, file.get()));
    ASSERT_TRUE(std::equal(buffer.begin(), buffer.begin() + count,
                           data.begin() + offset));
    offset += count;
  }
  EXPECT_EQ(EOF, fgetc(file.get()));
  EXPECT_EQ(0, ferror(file.get()));
}

#if !COMPILER_IS_MSVC
TEST(FileStream, RejectUnrepresentableSeekOffset) {
  std::unique_ptr<FILE, FileCloser> file(tmpfile());
  ASSERT_NE(nullptr, file);
  FileStream stream(file.get());
  const std::array<uint8_t, 1> data{0xab};
  const size_t offset = static_cast<size_t>(LONG_MAX) + 1;
  stream.WriteDataAt(offset, data);
  EXPECT_EQ(Result::Error, stream.result());
  stream.Flush();
  rewind(file.get());
  EXPECT_EQ(EOF, fgetc(file.get()));
}
#endif

#if COMPILER_IS_MSVC && defined(_WIN64)
TEST(FileStream, ReadAndPatchAbove2GBOnWindows64) {
  const char* filename = "large_file.dat";
  struct TempFile {
    const char* path = nullptr;
    ~TempFile() {
      if (path) {
        remove(path);
      }
    }
  } temp;
  // Exclusive creation avoids overwriting an existing file in the test dir.
  std::unique_ptr<FILE, FileCloser> file(fopen(filename, "w+bx"));
  ASSERT_NE(nullptr, file);
  temp.path = filename;

  // Only sparse-file setup needs the Windows API; stdio handles the file IO.
  HANDLE handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file.get())));
  DWORD bytes_returned;
  if (!DeviceIoControl(handle, FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0,
                       &bytes_returned, nullptr)) {
    GTEST_SKIP() << "This large-file test requires sparse-file support";
  }

  // A sparse file avoids allocating 2GB of disk space. Only Windows x64 runs
  // this test because ReadFile still needs a buffer of roughly 2GB.
  constexpr size_t kOffset = static_cast<size_t>(INT_MAX) + 1;
  ASSERT_EQ(0, _fseeki64(file.get(), kOffset, SEEK_SET));
  ASSERT_NE(EOF, fputc(0x12, file.get()));
  ASSERT_EQ(0, fflush(file.get()));

  {
    FileStream stream(file.get());
    const std::array<uint8_t, 1> patch{0xab};
    stream.WriteDataAt(kOffset, patch);
    ASSERT_EQ(Result::Ok, stream.result());
    stream.Flush();
  }
  file.reset();

  std::vector<uint8_t> data;
  ASSERT_EQ(Result::Ok, ReadFile(filename, &data));
  ASSERT_EQ(kOffset + 1, data.size());
  EXPECT_EQ(0, data.front());
  EXPECT_EQ(0, data[kOffset - 1]);
  EXPECT_EQ(0xab, data.back());
}
#endif

/*
 * Copyright 2018 WebAssembly Community Group participants
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

#include "wabt/binary-reader-ir.h"
#include "wabt/binary-reader-nop.h"
#include "wabt/binary-reader.h"
#include "wabt/error.h"
#include "wabt/ir.h"
#include "wabt/leb128.h"
#include "wabt/opcode.h"
#include "wabt/stream.h"

using namespace wabt;

namespace {

struct BinaryReaderError : BinaryReaderNop {
  bool OnError(const Error& error) override {
    first_error = error;
    return true;  // Error handled.
  }

  Error first_error;
};

}  // End of anonymous namespace

TEST(BinaryReader, DisabledOpcodes) {
  // Use the default features.
  ReadBinaryOptions options;

  // Loop through all opcodes.
  for (uint32_t i = 0; i < static_cast<uint32_t>(Opcode::Invalid); ++i) {
    Opcode opcode(static_cast<Opcode::Enum>(i));
    if (opcode.IsEnabled(options.features)) {
      continue;
    }

    // Use a shorter name to make the clang-formatted table below look nicer.
    std::vector<uint8_t> b = opcode.GetBytes();
    assert(b.size() <= 3);
    b.resize(3);

    uint8_t data[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,  // magic + version
        0x01, 0x04, 0x01, 0x60, 0x00, 0x00,  // type section: 1 type, (func)
        0x03, 0x02, 0x01, 0x00,              // func section: 1 func, type 0
        0x0a, 0x07, 0x01, 0x05, 0x00,        // code section: 1 func, 0 locals
        b[0], b[1], b[2],  // The instruction, padded with zeroes
        0x0b,              // end
    };
    BinaryReaderError reader;
    Result result = ReadBinary(data, &reader, options);
    EXPECT_EQ(Result::Error, result);

    // This relies on the binary reader checking whether the opcode is allowed
    // before reading any further data needed by the instruction.
    const std::string& message = reader.first_error.message;
    EXPECT_TRUE(message.starts_with("unexpected opcode"))
        << "Got error message: " << message;
  }
}

TEST(BinaryReader, InvalidFunctionBodySize) {
  // A wasm module where the function body size extends past the end of the
  // code section.  Without the bounds check this would allow the binary reader
  // to read past the section boundary.
  // TODO: Move this test upstream into the spec repo.

  uint8_t data[] = {
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,  // magic + version
      0x01, 0x04, 0x01, 0x60, 0x00, 0x00,  // type section: 1 type, (func)
      0x03, 0x02, 0x01, 0x00,              // func section: 1 func, type 0
      // Code section: 1 func, but body_size claims 0xFF bytes
      0x0a, 0x04,  // code section, size=4
      0x01,        // 1 function body
      0xff, 0x01,  // body size = 255 (LEB128), far exceeds section
      0x00,        // would be local decl count, but body_size is invalid
  };

  BinaryReaderError reader;
  ReadBinaryOptions options;
  Result result = ReadBinary(data, &reader, options);
  EXPECT_EQ(Result::Error, result);
  EXPECT_NE(std::string::npos,
            reader.first_error.message.find("invalid function body size"))
      << "Got: " << reader.first_error.message;
}

TEST(BinaryReader, OversizedSectionSize) {
  // A module whose section size extends past the end of the data.  The
  // subtraction-based overflow check must reject this before computing
  // read_end_ = offset + section_size, which would overflow on platforms
  // where size_t is 32-bit.
  // TODO: Move this test upstream into the spec repo.

  uint8_t data[] = {
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,  // magic + version
      0x01,                                            // section code: Type
      0x80, 0x80, 0x80, 0x80, 0x08,  // section size: 0x80000000 (LEB128)
  };

  BinaryReaderError reader;
  ReadBinaryOptions options;
  Result result = ReadBinary(data, &reader, options);
  EXPECT_EQ(Result::Error, result);
  EXPECT_NE(std::string::npos,
            reader.first_error.message.find("invalid section size"))
      << "Got: " << reader.first_error.message;
}

TEST(BinaryReader, OversizedSubsectionSize) {
  // A module with a name section containing a subsection whose size extends
  // past the section boundary.
  // TODO: Move this test upstream into the spec repo.

  uint8_t data[] = {
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,  // magic + version
      // Custom section (name section)
      0x00,  // section code: custom
      0x09,  // section size: 9 bytes
      0x04,  // name length
      'n', 'a', 'm', 'e',
      0x01,              // subsection type: function names
      0x80, 0x80, 0x04,  // subsection size: 65536 (LEB128), exceeds section
  };

  BinaryReaderError reader;
  ReadBinaryOptions options;
  Result result = ReadBinary(data, &reader, options);
  // Custom section errors are not fatal by default, but ensure no crash.
  (void)result;
}

TEST(BinaryReader, NameSectionLocalFunctionIndexOutOfBounds) {
  // With stop_on_first_error disabled the reader continues after a bad section.
  // The function section here declares two functions but the second signature
  // index runs past the section end, so only one Func is appended while
  // NumTotalFuncs() still counts two.  A following name section then names a
  // local of function index 1, which passes the reader's `index <
  // NumTotalFuncs()` guard but is out of range for module_->funcs.  Without the
  // bounds check in the local-name handlers this reads module_->funcs[1] out of
  // bounds.
  // TODO: Move this test upstream into the spec repo.

  uint8_t data[] = {
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,  // magic + version
      0x01, 0x04, 0x01, 0x60, 0x00, 0x00,  // type section: 1 type, (func)
      // Function section: count=2, but only one signature index fits; the
      // second read runs past the section end and fails.
      0x03, 0x03, 0x02, 0x80, 0x00,
      // Custom "name" section with a local-names subsection for func index 1.
      0x00, 0x0d,                // custom section, size=13
      0x04, 'n', 'a', 'm', 'e',  // section name "name"
      0x02, 0x06,                // subsection: local names, size=6
      0x01,                      // function count = 1
      0x01,                      // function index = 1 (past module_->funcs)
      0x01,                      // local count = 1
      0x00,                      // local index = 0
      0x01, 'x',                 // local name "x"
  };

  Errors errors;
  Module module;
  ReadBinaryOptions options(Features{}, nullptr, /*read_debug_names=*/true,
                            /*stop_on_first_error=*/false,
                            /*fail_on_custom_section_error=*/false);
  Result result = ReadBinaryIr("test", data, options, &errors, &module);
  EXPECT_EQ(Result::Error, result);
}

TEST(BinaryReaderLogging, DeepIndentDoesNotOverflowBuffer) {
  // With verbose logging on (e.g. `wasm-objdump --debug`) every section Begin
  // callback bumps the log indent.  A custom "name" section whose sub-section
  // size runs past the section end fails after BeginNamesSection, so with
  // stop_on_first_error / fail_on_custom_section_error both disabled the reader
  // continues and the matching End callbacks are skipped, leaking indent.  Once
  // the indent passed the fixed 142-byte indent buffer, WriteIndent's remainder
  // write used the whole indent count instead of the leftover, reading off the
  // end of the buffer (ASan: global-buffer-overflow READ).
  // TODO: Move this test upstream into the spec repo.

  std::vector<uint8_t> data = {
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,  // magic + version
  };
  // One malformed "name" custom section whose sub-section size overflows the
  // section; each one leaks four columns of indent.
  const uint8_t section[] = {
      0x00,                        // custom section
      0x09,                        // section size = 9
      0x04, 'n',  'a',  'm', 'e',  // section name "name"
      0x01,                        // subsection: function names
      0x80, 0x80, 0x04,  // subsection size 65536, extends past section
  };
  // 60 sections drives the indent well past the buffer.
  for (int i = 0; i < 60; i++) {
    data.insert(data.end(), std::begin(section), std::end(section));
  }

  MemoryStream log_stream;
  BinaryReaderError reader;
  ReadBinaryOptions options(Features{}, &log_stream, /*read_debug_names=*/true,
                            /*stop_on_first_error=*/false,
                            /*fail_on_custom_section_error=*/false);
  // Just ensure the deep indent is written without overrunning the buffer.
  Result result = ReadBinary(data, &reader, options);
  (void)result;
}

TEST(Opcode, DecodeInvalidOpcode) {
  Opcode opcode = Opcode::FromCode(0xfd, 0x13f);
  EXPECT_TRUE(opcode.IsInvalid());
  std::vector<uint8_t> bytes = opcode.GetBytes();
  ASSERT_EQ(3u, bytes.size());
  EXPECT_EQ(0xfdu, bytes[0]);
  // 0x13f encoded as unsigned LEB128 is 0xbf 0x02.
  EXPECT_EQ(0xbfu, bytes[1]);
  EXPECT_EQ(0x02u, bytes[2]);
}

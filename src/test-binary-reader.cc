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

#include "wabt/binary-reader-nop.h"
#include "wabt/binary-reader-objdump.h"
#include "wabt/binary-reader.h"
#include "wabt/leb128.h"
#include "wabt/opcode.h"

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
    const size_t size = sizeof(data);

    BinaryReaderError reader;
    Result result = ReadBinary(data, size, &reader, options);
    EXPECT_EQ(Result::Error, result);

    // This relies on the binary reader checking whether the opcode is allowed
    // before reading any further data needed by the instruction.
    const std::string& message = reader.first_error.message;
    EXPECT_EQ(0u, message.find("unexpected opcode"))
        << "Got error message: " << message;
  }
}

TEST(BinaryReaderObjdump, RelocInvalidSectionIndex) {
  // Minimal wasm with a reloc section referencing a section_index that exceeds
  // the actual number of sections.  Before the fix, the derived-class
  // OnRelocCount ignored the error returned by the base class and proceeded
  // to call GetSectionName with BinarySection::Invalid, causing an
  // out-of-bounds read on the section_starts_ array.
  //
  // The fix propagates the base class error via CHECK_RESULT so that the
  // out-of-bounds GetSectionName call is never reached.  The overall result
  // is still Ok because custom section errors are not fatal by default.

  uint8_t data[] = {
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,  // magic + version

      // Custom section pretending to be "reloc." (section id 0)
      0x00,  // section code: custom
      0x0a,  // section size: 10 bytes
      // section name "reloc." (length-prefixed)
      0x06,  // name length
      'r', 'e', 'l', 'o', 'c', '.', 0xff,
      0x01,  // section_index = 255 (invalid, LEB128)
      0x00,  // relocation count = 0
  };

  ObjdumpOptions options;
  memset(&options, 0, sizeof(options));
  options.mode = ObjdumpMode::Details;
  options.details = true;
  ObjdumpState state;

  // Should not crash.  Custom section errors are suppressed, so the overall
  // result is Ok even though the reloc section itself fails.
  Result result = ReadBinaryObjdump(data, sizeof(data), &options, &state);
  EXPECT_EQ(Result::Ok, result);
}

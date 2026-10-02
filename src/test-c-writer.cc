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

#include <clocale>
#include <string>
#include <vector>

#include "wabt/apply-names.h"
#include "wabt/binary-reader-ir.h"
#include "wabt/binary-reader.h"
#include "wabt/c-writer.h"
#include "wabt/error.h"
#include "wabt/generate-names.h"
#include "wabt/ir.h"
#include "wabt/stream.h"
#include "wabt/validator.h"

using namespace wabt;

namespace {

// (module
//   (global (export "g_f64") f64 (f64.const 1.5))
//   (global (export "g_f32") f32 (f32.const 0.5)))
const uint8_t s_float_globals[] = {
    0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x06, 0x15,
    0x02, 0x7c, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf8, 0x3f, 0x0b, 0x7d, 0x00, 0x43, 0x00, 0x00, 0x00, 0x3f,
    0x0b, 0x07, 0x11, 0x02, 0x05, 0x67, 0x5f, 0x66, 0x36, 0x34,
    0x03, 0x00, 0x05, 0x67, 0x5f, 0x66, 0x33, 0x32, 0x03, 0x01,
};

std::string WriteCSource(const uint8_t* data, size_t size) {
  Errors errors;
  Module module;
  ReadBinaryOptions read_options;
  EXPECT_EQ(Result::Ok,
            ReadBinaryIr("test", data, size, read_options, &errors, &module));
  Features features;
  ValidateOptions validate_options(features);
  EXPECT_EQ(Result::Ok, ValidateModule(&module, &errors, validate_options));
  EXPECT_EQ(Result::Ok, GenerateNames(&module));
  EXPECT_EQ(Result::Ok, ApplyNames(&module));

  MemoryStream c_stream, h_stream, h_impl_stream;
  std::vector<Stream*> c_streams{&c_stream};
  WriteCOptions options;
  options.module_name = "test";
  options.features = features;
  EXPECT_EQ(Result::Ok, WriteC(std::move(c_streams), &h_stream, &h_impl_stream,
                               "test.h", "test-impl.h", &module, options));
  const OutputBuffer& buf = c_stream.output_buffer();
  return std::string(buf.data.begin(), buf.data.end());
}

}  // namespace

// The generated C source must use '.' as the radix for float constants.
// snprintf renders the radix according to the current LC_NUMERIC locale, so
// under a locale whose decimal point is ',' wasm2c used to emit "1,5" (invalid
// C, or silently a comma expression). Output must not depend on the host
// locale.
TEST(CWriter, FloatConstLocaleIndependent) {
  std::string saved = setlocale(LC_NUMERIC, nullptr);

  const char* comma_locales[] = {"de_DE.UTF-8", "de_DE.utf8", "de_DE",
                                 "fr_FR.UTF-8", "nl_NL.UTF-8"};
  bool have_comma_locale = false;
  for (const char* loc : comma_locales) {
    if (setlocale(LC_NUMERIC, loc) && *localeconv()->decimal_point == ',') {
      have_comma_locale = true;
      break;
    }
  }
  if (!have_comma_locale) {
    setlocale(LC_NUMERIC, saved.c_str());
    GTEST_SKIP() << "no comma-decimal locale installed";
  }

  std::string source = WriteCSource(s_float_globals, sizeof(s_float_globals));

  setlocale(LC_NUMERIC, saved.c_str());

  EXPECT_NE(source.find("1.5"), std::string::npos);
  EXPECT_NE(source.find("0.5"), std::string::npos);
  EXPECT_EQ(source.find("1,5"), std::string::npos);
  EXPECT_EQ(source.find("0,5"), std::string::npos);
}

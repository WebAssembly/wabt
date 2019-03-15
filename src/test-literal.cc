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

#include <cstdio>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

#include "src/literal.h"

using namespace wabt;

namespace {

enum ParseIntTypeCombo {
  UnsignedOnly,
  SignedAndUnsigned,
  Both,
};

template <typename T, typename F>
void AssertIntEquals(T expected,
                     const char* s,
                     F&& parse_int,
                     ParseIntTypeCombo parse_type = Both) {
  const char* const end = s + strlen(s);
  T actual;
  if (parse_type == UnsignedOnly || parse_type == Both) {
    ASSERT_EQ(Result::Ok,
              parse_int(s, end, &actual, ParseIntType::UnsignedOnly))
        << s;
    ASSERT_EQ(expected, actual);
  } else {
    ASSERT_EQ(Result::Error,
              parse_int(s, end, &actual, ParseIntType::UnsignedOnly))
        << s;
  }

  if (parse_type == SignedAndUnsigned || parse_type == Both) {
    ASSERT_EQ(Result::Ok,
              parse_int(s, end, &actual, ParseIntType::SignedAndUnsigned))
        << s;
    ASSERT_EQ(expected, actual);
  } else {
    ASSERT_EQ(Result::Error,
              parse_int(s, end, &actual, ParseIntType::SignedAndUnsigned))
        << s;
  }
}

void AssertInt32Equals(uint32_t expected,
                       const char* s,
                       ParseIntTypeCombo parse_type = Both) {
  AssertIntEquals(expected, s, ParseInt32, parse_type);
}

void AssertInt64Equals(uint64_t expected,
                       const char* s,
                       ParseIntTypeCombo parse_type = Both) {
  AssertIntEquals(expected, s, ParseInt64, parse_type);
}

void AssertInt32Fails(const char* s) {
  const char* const end = s + strlen(s);
  uint32_t actual;
  ASSERT_EQ(Result::Error,
            ParseInt32(s, end, &actual, ParseIntType::SignedAndUnsigned))
      << s;
  ASSERT_EQ(Result::Error,
            ParseInt32(s, end, &actual, ParseIntType::UnsignedOnly))
      << s;
}

void AssertInt64Fails(const char* s) {
  const char* const end = s + strlen(s);
  uint64_t actual;
  ASSERT_EQ(Result::Error,
            ParseInt64(s, end, &actual, ParseIntType::SignedAndUnsigned))
      << s;
  ASSERT_EQ(Result::Error,
            ParseInt64(s, end, &actual, ParseIntType::UnsignedOnly))
      << s;
}

void AssertUint64Equals(uint64_t expected, const char* s) {
  uint64_t actual;
  ASSERT_EQ(Result::Ok, ParseUint64(s, s + strlen(s), &actual)) << s;
  ASSERT_EQ(expected, actual);
}

void AssertUint64Fails(const char* s) {
  uint64_t actual_bits;
  ASSERT_EQ(Result::Error, ParseUint64(s, s + strlen(s), &actual_bits)) << s;
}

void AssertHexFloatEquals(uint32_t expected_bits, const char* s) {
  uint32_t actual_bits;
  ASSERT_EQ(Result::Ok,
            ParseFloat(LiteralType::Hexfloat, s, s + strlen(s), &actual_bits))
      << s;
  ASSERT_EQ(expected_bits, actual_bits);
}

void AssertHexFloatFails(const char* s) {
  uint32_t actual_bits;
  ASSERT_EQ(Result::Error,
            ParseFloat(LiteralType::Hexfloat, s, s + strlen(s), &actual_bits))
      << s;
}

void AssertHexDoubleEquals(uint64_t expected_bits, const char* s) {
  uint64_t actual_bits;
  ASSERT_EQ(Result::Ok,
            ParseDouble(LiteralType::Hexfloat, s, s + strlen(s), &actual_bits))
      << s;
  ASSERT_EQ(expected_bits, actual_bits);
}

void AssertHexDoubleFails(const char* s) {
  uint64_t actual_bits;
  ASSERT_EQ(Result::Error,
            ParseDouble(LiteralType::Hexfloat, s, s + strlen(s), &actual_bits))
      << s;
}

}  // end anonymous namespace

TEST(ParseInt32, Both) {
  AssertInt32Equals(0, "0");
  AssertInt32Equals(1000, "1000");
  AssertInt32Equals(123456789, "123456789");
  AssertInt32Equals(2147483647, "2147483647");
  AssertInt32Equals(4294967295u, "4294967295");
  AssertInt32Equals(0xcafef00du, "0xcafef00d");
  AssertInt32Equals(0x7fffffff, "0x7fffffff");
  AssertInt32Equals(0x80000000u, "0x80000000");
  AssertInt32Equals(0xffffffffu, "0xffffffff");
}

TEST(ParseInt32, SignedAndUnsigned) {
  AssertInt32Equals(2147483648, "-2147483648", SignedAndUnsigned);
  AssertInt32Equals(-0x80000000u, "-0x80000000", SignedAndUnsigned);
  AssertInt32Equals(4294967295u, "-1", SignedAndUnsigned);
  AssertInt32Equals(-1, "-0x1", SignedAndUnsigned);
  AssertInt32Equals(1, "+1", SignedAndUnsigned);
  AssertInt32Equals(-0xabcd, "-0xABCD", SignedAndUnsigned);
  AssertInt32Equals(0xabcd, "+0xabcd", SignedAndUnsigned);
}

TEST(ParseInt32, Invalid) {
  AssertInt32Fails("");
  AssertInt32Fails("-100hello");
  AssertInt32Fails("0XABCDEF");
  AssertInt32Fails("0xgabba");
  AssertInt32Fails("two");
}

TEST(ParseInt32, Underscores) {
  AssertInt32Equals(123456789, "12_345_6789", Both);
  AssertInt32Equals(123456789, "+12_345_6789", SignedAndUnsigned);
  AssertInt32Equals(-123456789, "-12345_6789", SignedAndUnsigned);
  AssertInt32Equals(19, "1______9", Both);
  AssertInt32Equals(0xabcd, "0xa_b_c_d", Both);
  AssertInt32Equals(0xabcd, "+0xa_b_c_d", SignedAndUnsigned);
  AssertInt32Equals(-0xabcd, "-0xa_b_c_d", SignedAndUnsigned);
}

TEST(ParseInt32, Overflow) {
  AssertInt32Fails("4294967296");
  AssertInt32Fails("-2147483649");
  AssertInt32Fails("0x100000000");
  AssertInt32Fails("-0x80000001");
  AssertInt32Fails("1231231231231231231231");
}

TEST(ParseInt64, Both) {
  AssertInt64Equals(0, "0");
  AssertInt64Equals(1000, "1000");
  AssertInt64Equals(123456789, "123456789");
  AssertInt64Equals(9223372036854775807ull, "9223372036854775807");
  AssertInt64Equals(18446744073709551615ull, "18446744073709551615");
  AssertInt64Equals(0x7fffffffffffffffull, "0x7fffffffffffffff");
  AssertInt64Equals(0x8000000000000000ull, "0x8000000000000000");
  AssertInt64Equals(0xffffffffffffffffull, "0xffffffffffffffff");
}

TEST(ParseInt64, SignedAndUnsigned) {
  AssertInt64Equals(9223372036854775808ull, "-9223372036854775808",
                    SignedAndUnsigned);
  AssertInt64Equals(18446744073709551615ull, "-1", SignedAndUnsigned);
  AssertInt64Equals(-1, "-0x1", SignedAndUnsigned);
  AssertInt64Equals(1, "+1", SignedAndUnsigned);
  AssertInt64Equals(-0x0bcdefabcdefabcdull, "-0x0BCDEFABCDEFABCD",
                    SignedAndUnsigned);
  AssertInt64Equals(0xabcdefabcdefabcdull, "+0xabcdefabcdefabcd",
                    SignedAndUnsigned);
}

TEST(ParseInt64, Invalid) {
  AssertInt64Fails("");
  AssertInt64Fails("-100hello");
  AssertInt64Fails("0XABCDEF");
  AssertInt64Fails("0xgabba");
  AssertInt64Fails("two");
}

TEST(ParseInt64, Underscores) {
  AssertInt64Equals(123456789, "12_345_6789", Both);
  AssertInt64Equals(123456789, "+12_345_6789", SignedAndUnsigned);
  AssertInt64Equals(-123456789, "-12345_6789", SignedAndUnsigned);
  AssertInt64Equals(19, "1______9", Both);
  AssertInt64Equals(0xabcd, "0xa_b_c_d", Both);
  AssertInt64Equals(0xabcd, "+0xa_b_c_d", SignedAndUnsigned);
  AssertInt64Equals(-0xabcd, "-0xa_b_c_d", SignedAndUnsigned);
}

TEST(ParseInt64, Overflow) {
  AssertInt64Fails("18446744073709551616");
  AssertInt64Fails("-9223372036854775809");
  AssertInt32Fails("0x10000000000000000");
  AssertInt32Fails("-0x80000000000000001");
  AssertInt64Fails("1231231231231231231231");
}

TEST(ParseUint64, Basic) {
  AssertUint64Equals(0, "0");
  AssertUint64Equals(1000, "1000");
  AssertUint64Equals(123456789, "123456789");
  AssertUint64Equals(1844674407370955161ull, "1844674407370955161");
  AssertUint64Equals(18446744073709551615ull, "18446744073709551615");

  AssertUint64Equals(0, "0x0");
  AssertUint64Equals(0x1000, "0x1000");
  AssertUint64Equals(0x123456789, "0x123456789");
  AssertUint64Equals(0xabcdef, "0xabcdef");
  AssertUint64Equals(0xffffffffffffffull, "0xffffffffffffff");
  AssertUint64Equals(0xfffffffffffffffull, "0xfffffffffffffff");

  AssertUint64Equals(0xabcdefabcdefabcdull, "0xabcdefabcdefabcd");
}

TEST(ParseUint64, NoOctal) {
  AssertUint64Equals(100, "0100");
  AssertUint64Equals(888, "0000888");
}

TEST(ParseUint64, Invalid) {
  AssertUint64Fails("");
  AssertUint64Fails("-100");
  AssertUint64Fails("0XABCDEF");
  AssertUint64Fails("0xgabba");
  AssertUint64Fails("two");
}

TEST(ParseUint64, Underscores) {
  AssertUint64Equals(123456789, "12_345_6789");
  AssertUint64Equals(19, "1______9");
  AssertUint64Equals(0xabcd, "0xa_b_c_d");
}

TEST(ParseUint64, Overflow) {
  AssertUint64Fails("0x10000000000000000");
  AssertUint64Fails("18446744073709551616");
  AssertUint64Fails("62857453058642199420");
  AssertUint64Fails("82000999361882825820");
  AssertUint64Fails("126539114687237086210");
  AssertUint64Fails("10000000000000000000000000000000000000000");
}

TEST(ParseFloat, NonCanonical) {
  AssertHexFloatEquals(0x3f800000, "0x00000000000000000000001.0p0");
  AssertHexFloatEquals(0x3f800000, "0x1.00000000000000000000000p0");
  AssertHexFloatEquals(0x3f800000, "0x0.0000000000000000000001p88");
}

TEST(ParseFloat, Rounding) {
  // |------- 23 bits -----| V-- extra bit
  //
  // 11111111111111111111101 0  ==> no rounding
  AssertHexFloatEquals(0x7f7ffffd, "0x1.fffffap127");
  // 11111111111111111111101 1  ==> round up
  AssertHexFloatEquals(0x7f7ffffe, "0x1.fffffbp127");
  // 11111111111111111111110 0  ==> no rounding
  AssertHexFloatEquals(0x7f7ffffe, "0x1.fffffcp127");
  // 11111111111111111111110 1  ==> round down
  AssertHexFloatEquals(0x7f7ffffe, "0x1.fffffdp127");
  // 11111111111111111111111 0  ==> no rounding
  AssertHexFloatEquals(0x7f7fffff, "0x1.fffffep127");
}

TEST(ParseFloat, OutOfRange) {
  AssertHexFloatFails("0x1p128");
  AssertHexFloatFails("-0x1p128");
  AssertHexFloatFails("0x1.ffffffp127");
  AssertHexFloatFails("-0x1.ffffffp127");
}

TEST(ParseDouble, NonCanonical) {
  AssertHexDoubleEquals(0x3ff0000000000000, "0x00000000000000000000001.0p0");
  AssertHexDoubleEquals(0x3ff0000000000000, "0x1.00000000000000000000000p0");
  AssertHexDoubleEquals(0x3ff0000000000000, "0x0.0000000000000000000001p88");
}

TEST(ParseDouble, Rounding) {
  // |-------------------- 52 bits ---------------------| V-- extra bit
  //
  // 1111111111111111111111111111111111111111111111111101 0  ==> no rounding
  AssertHexDoubleEquals(0x7feffffffffffffd, "0x1.ffffffffffffd0p1023");
  // 1111111111111111111111111111111111111111111111111101 1  ==> round up
  AssertHexDoubleEquals(0x7feffffffffffffe, "0x1.ffffffffffffd8p1023");
  // 1111111111111111111111111111111111111111111111111110 0  ==> no rounding
  AssertHexDoubleEquals(0x7feffffffffffffe, "0x1.ffffffffffffe0p1023");
  // 1111111111111111111111111111111111111111111111111110 1  ==> round down
  AssertHexDoubleEquals(0x7feffffffffffffe, "0x1.ffffffffffffe8p1023");
  // 1111111111111111111111111111111111111111111111111111 0  ==> no rounding
  AssertHexDoubleEquals(0x7fefffffffffffff, "0x1.fffffffffffff0p1023");
}

TEST(ParseDouble, OutOfRange) {
  AssertHexDoubleFails("0x1p1024");
  AssertHexDoubleFails("-0x1p1024");
  AssertHexDoubleFails("0x1.fffffffffffff8p1023");
  AssertHexDoubleFails("-0x1.fffffffffffff8p1023");
}

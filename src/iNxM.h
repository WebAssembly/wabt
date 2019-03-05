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

#ifndef WABT_INXM_H_
#define WABT_INXM_H_

#include <array>
#include <cassert>
#include <cstdint>
#include <climits>

namespace iNxM {

/// Array containing `Length` `IntBits`-bit integers
template <uint8_t IntBits, ptrdiff_t Length>
struct iNxM {
  /// 64-bit integers are used to represent parsed natural numbers;
  using text_t = uint64_t;
  /// Internally the iN integers are stored in a byte:
  using bin_t = uint8_t;
  static_assert(
    IntBits <= sizeof(bin_t) * CHAR_BIT,
    "bin_t is not large enough to fit iN"
  );

  /// Largest value that an N-bit integer can have: [0, 2^N)
  static constexpr text_t max_value() {
    return (1 << IntBits) - 1;
  }

  /// Array length
  static constexpr ptrdiff_t size() {
    return Length;
  }

  /// In the textual encoding we treat the integers as Nat.
  using TextValue = std::array<text_t, Length>;

  static_assert(
    IntBits * Length % (sizeof(bin_t) * CHAR_BIT) == 0,
    "The array length in bits must be a multiple of sizeof(bin_t) * CHAR_BIT"
  );
  /// In the binary encoding we pack the iN integers together:
  using BinaryValue
    = std::array<bin_t, IntBits * Length / (sizeof(bin_t) * CHAR_BIT)>;

  enum class Discriminant : uint8_t { Binary, Text };

  union Value {
    BinaryValue binary;
    TextValue text;
  };

  Discriminant discriminant;
  Value value;

  iNxM() = default;
  iNxM(iNxM const&) = default;
  iNxM(iNxM&&) = default;
  iNxM& operator=(iNxM&&) = default;
  iNxM& operator=(iNxM const&) = default;

  iNxM(TextValue const& val) : discriminant(Discriminant::Text) {
    // We don't validate anything here - that's done:
    // * in type checking: returning a proper error
    // * when attempting to convert into a binary representation
    value.text = val;
  }
  iNxM(BinaryValue const& val) : discriminant(Discriminant::Binary) {
    value.binary = val;
  }

  /// Is the current stored representation binary or text ?
  Discriminant repr() const {
    return discriminant;
  };

  /// Converts the internal representation from text to binary.
  ///
  /// Pre-condition: assert(repr == iNxM::Discriminant::Text);
  void text_to_binary();
  /// Converts the internal representation from binary to text.
  ///
  /// Pre-condition: assert(repr == iNxM::Discriminant::Binary);
  void binary_to_text();

  /// Reads the iN at index
  ///
  /// Pre-condition: assert(repr == iNxM::Discriminant::Binary);
  bin_t read_binary(int32_t index) const;
  /// Reads the iN at index
  ///
  /// Pre-condition: assert(repr == iNxM::Discriminant::Binary);
  text_t read_text(int32_t index) const {
    assert(repr() == Discriminant::Text);
    assert(index >= 0);
    assert(index < Length);
    return value.text[index];
  }
};

template <uint8_t IntBits, ptrdiff_t Length>
using binary_v = typename iNxM<IntBits, Length>::BinaryValue;

template <uint8_t IntBits, ptrdiff_t Length>
using text_v = typename iNxM<IntBits, Length>::TextValue;

template <uint8_t IntBits, ptrdiff_t Length>
using bin_t = typename iNxM<IntBits, Length>::bin_t;

template <uint8_t IntBits, ptrdiff_t Length>
using text_t = typename iNxM<IntBits, Length>::text_t;

/// Reads the N-bit integer at the position `index` of the Mx iN array `binary`.
template <uint8_t N, ptrdiff_t L>
typename iNxM<N, L>::bin_t
read_i(
  typename iNxM<N, L>::BinaryValue const& binary,
  int32_t index
) {
  assert(index >= 0);
  assert(index < L);
  uint8_t value = 0;
  for (int32_t value_bit = 0; value_bit < N; ++value_bit) {
    // Index of the bit within the array:
    int32_t array_bit = index * N + value_bit;
    // Index to the byte of the array containing the bit:
    int32_t array_byte = array_bit / CHAR_BIT;
    // Relative index to the bit within the byte:
    int32_t array_byte_rel_bit = array_bit % CHAR_BIT;

    // Bitmaks for the relative bit:
    bin_t<N, L> flag = 1 << array_byte_rel_bit;

    // Test the bit:
    bin_t<N, L> bit_value = binary[array_byte] & flag ? 1 : 0;

    // Set the bit in value
    value |= bit_value << value_bit;
  }

  // the result must always be in bounds since we only read N
  assert(value <= (iNxM<N, L>::max_value()));

  return value;
}

/// Write the `N`-bit integer `value` at the position `index` within
/// `binary`
template <uint8_t N, ptrdiff_t L>
void write_i(
  typename iNxM<N, L>::BinaryValue& binary,
  typename iNxM<N, L>::bin_t iN,
  int32_t index
) {
  static_assert(N <= sizeof(bin_t<N, L>) * CHAR_BIT, "bin_t overflows");
  assert(iN <= (iNxM<N, L>::max_value()));
  assert(index >= 0);
  assert(index < L);

  for (int32_t value_bit = 0; value_bit < N; ++value_bit) {
    // Index of the bit within the array:
    int32_t array_bit = index * N + value_bit;
    // Index to the byte of the array containing the bit:
    int32_t array_byte = array_bit / CHAR_BIT;
    // Relative index to the bit within the byte:
    int32_t array_byte_rel_bit = array_bit % CHAR_BIT;

    // Bitmaks for the relative bit:
    bin_t<N, L> test_flag = 1 << value_bit;

    // Test the bit:
    bin_t<N, L> bit_value = iN & test_flag ? 1 : 0;

    if (bit_value == 1) {
      // Bitmaks for the relative bit:
      bin_t<N, L> set_flag = bit_value << array_byte_rel_bit;

      // Set the bit:
      binary[array_byte] |= set_flag;
    }
  }
}

template <uint8_t N, ptrdiff_t L>
typename iNxM<N, L>::BinaryValue
text_to_binary_impl(
  typename iNxM<N, L>::TextValue const& text
) {
  binary_v<N, L> binary;
  std::fill(binary.begin(), binary.end(), 0);

  for (int32_t index = 0; index < L; ++index) {
    write_i<N, L>(binary, text[index], index);
  }

  return binary;
}

template <uint8_t N, ptrdiff_t L>
typename iNxM<N, L>::TextValue
binary_to_text_impl(
  typename iNxM<N, L>::BinaryValue const& binary
) {
  text_v<N, L> text;
  std::fill(text.begin(), text.end(), 0);

  for (int32_t index = 0; index < L; ++index) {
    text[index] = read_i<N, L>(binary, index);
    assert(text[index] <= (iNxM<N, L>::max_value()));
  }

  return text;
}

template <uint8_t N, ptrdiff_t L>
void iNxM<N, L>::text_to_binary() {
   assert(repr() == Discriminant::Text);
   value.binary = text_to_binary_impl<N, L>(value.text);
   discriminant = Discriminant::Binary;
   assert(repr() == Discriminant::Binary);
}

template <uint8_t N, ptrdiff_t L>
void iNxM<N, L>::binary_to_text() {
   assert(repr() == Discriminant::Binary);
   value.text = binary_to_text_impl<N, L>(value.binary);
   discriminant = Discriminant::Text;
   assert(repr() == Discriminant::Text);
}

template <uint8_t N, ptrdiff_t L>
typename iNxM<N, L>::bin_t
iNxM<N, L>::read_binary(int32_t index) const {
   assert(repr() == Discriminant::Binary);
   assert(index >= 0);
   assert(index < L);
   return read_i<N, L>(value.binary, index);
}

} // namespace iNxM

/// Immediate mode argument of i8x16.shuffle
using i5x16 = iNxM::iNxM<5, 16>;

#endif // WABT_INXM_H_

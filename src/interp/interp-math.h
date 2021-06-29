/*
 * Copyright 2020 WebAssembly Community Group participants
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

// clang-format off

#ifndef WABT_INTERP_MATH_H_
#define WABT_INTERP_MATH_H_

#include <cmath>
#include <limits>
#include <string>
#include <type_traits>

#if COMPILER_IS_MSVC
#include <emmintrin.h>
#include <immintrin.h>
#endif

#include "src/common.h"
#include "src/interp/interp.h"

namespace wabt {
namespace interp {

template <
    typename T,
    typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
bool WABT_VECTORCALL IsNaN(T val) {
  return false;
}

template <
    typename T,
    typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
bool WABT_VECTORCALL IsNaN(T val) {
  return std::isnan(val);
}

template <
    typename T,
    typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
T WABT_VECTORCALL CanonNaN(T val) {
  return val;
}

template <
    typename T,
    typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
T WABT_VECTORCALL CanonNaN(T val) {
  if (WABT_UNLIKELY(std::isnan(val))) {
    return std::numeric_limits<f32>::quiet_NaN();
  }
  return val;
}

template <typename T> T ShiftMask(T val) { return val & (sizeof(T)*8-1); }

template <typename T> bool WABT_VECTORCALL IntEqz(T val) { return val == 0; }
template <typename T> bool WABT_VECTORCALL Eq(T lhs, T rhs) { return lhs == rhs; }
template <typename T> bool WABT_VECTORCALL Ne(T lhs, T rhs) { return lhs != rhs; }
template <typename T> bool WABT_VECTORCALL Lt(T lhs, T rhs) { return lhs < rhs; }
template <typename T> bool WABT_VECTORCALL Le(T lhs, T rhs) { return lhs <= rhs; }
template <typename T> bool WABT_VECTORCALL Gt(T lhs, T rhs) { return lhs > rhs; }
template <typename T> bool WABT_VECTORCALL Ge(T lhs, T rhs) { return lhs >= rhs; }
template <typename T> T WABT_VECTORCALL IntClz(T val) { return Clz(val); }
template <typename T> T WABT_VECTORCALL IntCtz(T val) { return Ctz(val); }
template <typename T> T WABT_VECTORCALL IntPopcnt(T val) { return Popcount(val); }
template <typename T> T WABT_VECTORCALL IntNot(T val) { return ~val; }
template <typename T> T WABT_VECTORCALL IntNeg(T val) { return ~val + 1; }
template <typename T> T WABT_VECTORCALL Add(T lhs, T rhs) { return CanonNaN(lhs + rhs); }
template <typename T> T WABT_VECTORCALL Sub(T lhs, T rhs) { return CanonNaN(lhs - rhs); }
template <typename T> T WABT_VECTORCALL IntAnd(T lhs, T rhs) { return lhs & rhs; }
template <typename T> T WABT_VECTORCALL IntOr(T lhs, T rhs) { return lhs | rhs; }
template <typename T> T WABT_VECTORCALL IntXor(T lhs, T rhs) { return lhs ^ rhs; }
template <typename T> T WABT_VECTORCALL IntShl(T lhs, T rhs) { return lhs << ShiftMask(rhs); }
template <typename T> T WABT_VECTORCALL IntShr(T lhs, T rhs) { return lhs >> ShiftMask(rhs); }
template <typename T> T WABT_VECTORCALL IntMin(T lhs, T rhs) { return std::min(lhs, rhs); }
template <typename T> T WABT_VECTORCALL IntMax(T lhs, T rhs) { return std::max(lhs, rhs); }
template <typename T> T WABT_VECTORCALL IntAndNot(T lhs, T rhs) { return lhs & ~rhs; }
template <typename T> T WABT_VECTORCALL IntAvgr(T lhs, T rhs) { return (lhs + rhs + 1) / 2; }
template <typename T> T WABT_VECTORCALL Xchg(T lhs, T rhs) { return rhs; }

// This is a wrapping absolute value function, so a negative number that is not
// representable as a positive number will be unchanged (e.g. abs(-128) = 128).
//
// Note that std::abs() does not have this behavior (e.g. abs(-128) is UB).
// Similarly, using unary minus is also UB.
template <typename T>
T WABT_VECTORCALL IntAbs(T val) {
  static_assert(std::is_unsigned<T>::value, "T must be unsigned.");
  const auto signbit = T(-1) << (sizeof(T) * 8 - 1);
  return (val & signbit) ? ~val + 1 : val;
}

// Because of the integer promotion rules [1], any value of a type T which is
// smaller than `int` will be converted to an `int`, as long as `int` can hold
// any value of type T.
//
// So type `u16` will be promoted to `int`, since all values can be stored in
// an int. Unfortunately, the product of two `u16` values cannot always be
// stored in an `int` (e.g. 65535 * 65535). This triggers an error in UBSan.
//
// As a result, we make sure to promote the type ahead of time for `u16`. Note
// that this isn't a problem for any other unsigned types.
//
// [1]; https://en.cppreference.com/w/cpp/language/implicit_conversion#Integral_promotion
template <typename T> struct PromoteMul { using type = T; };
template <> struct PromoteMul<u16> { using type = u32; };

template <typename T>
T WABT_VECTORCALL Mul(T lhs, T rhs) {
  using U = typename PromoteMul<T>::type;
  return CanonNaN(U(lhs) * U(rhs));
}

template <typename T> struct Mask { using Type = T; };
template <> struct Mask<f32> { using Type = u32; };
template <> struct Mask<f64> { using Type = u64; };

template <typename T> typename Mask<T>::Type WABT_VECTORCALL EqMask(T lhs, T rhs) { return lhs == rhs ? -1 : 0; }
template <typename T> typename Mask<T>::Type WABT_VECTORCALL NeMask(T lhs, T rhs) { return lhs != rhs ? -1 : 0; }
template <typename T> typename Mask<T>::Type WABT_VECTORCALL LtMask(T lhs, T rhs) { return lhs < rhs ? -1 : 0; }
template <typename T> typename Mask<T>::Type WABT_VECTORCALL LeMask(T lhs, T rhs) { return lhs <= rhs ? -1 : 0; }
template <typename T> typename Mask<T>::Type WABT_VECTORCALL GtMask(T lhs, T rhs) { return lhs > rhs ? -1 : 0; }
template <typename T> typename Mask<T>::Type WABT_VECTORCALL GeMask(T lhs, T rhs) { return lhs >= rhs ? -1 : 0; }

template <typename T>
T WABT_VECTORCALL IntRotl(T lhs, T rhs) {
  return (lhs << ShiftMask(rhs)) | (lhs >> ShiftMask<T>(0 - rhs));
}

template <typename T>
T WABT_VECTORCALL IntRotr(T lhs, T rhs) {
  return (lhs >> ShiftMask(rhs)) | (lhs << ShiftMask<T>(0 - rhs));
}

// i{32,64}.{div,rem}_s are special-cased because they trap when dividing the
// max signed value by -1. The modulo operation on x86 uses the same
// instruction to generate the quotient and the remainder.
template <typename T,
          typename std::enable_if<std::is_signed<T>::value, int>::type = 0>
bool IsNormalDivRem(T lhs, T rhs) {
  return !(lhs == std::numeric_limits<T>::min() && rhs == -1);
}

template <typename T,
          typename std::enable_if<!std::is_signed<T>::value, int>::type = 0>
bool IsNormalDivRem(T lhs, T rhs) {
  return true;
}

template <typename T>
RunResult WABT_VECTORCALL IntDiv(T lhs, T rhs, T* out, std::string* out_msg) {
  if (WABT_UNLIKELY(rhs == 0)) {
    *out_msg = "integer divide by zero";
    return RunResult::Trap;
  }
  if (WABT_LIKELY(IsNormalDivRem(lhs, rhs))) {
    *out = lhs / rhs;
    return RunResult::Ok;
  } else {
    *out_msg = "integer overflow";
    return RunResult::Trap;
  }
}

template <typename T>
RunResult WABT_VECTORCALL IntRem(T lhs, T rhs, T* out, std::string* out_msg) {
  if (WABT_UNLIKELY(rhs == 0)) {
    *out_msg = "integer divide by zero";
    return RunResult::Trap;
  }
  if (WABT_LIKELY(IsNormalDivRem(lhs, rhs))) {
    *out = lhs % rhs;
  } else {
    *out = 0;
  }
  return RunResult::Ok;
}

#if COMPILER_IS_MSVC
template <typename T> T WABT_VECTORCALL FloatAbs(T val);
template <typename T> T WABT_VECTORCALL FloatCopysign(T lhs, T rhs);

// Don't use std::{abs,copysign} directly on MSVC, since that seems to lose
// the NaN tag.
template <>
inline f32 WABT_VECTORCALL FloatAbs(f32 val) {
  return _mm_cvtss_f32(_mm_and_ps(
      _mm_set1_ps(val), _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff))));
}

template <>
inline f64 WABT_VECTORCALL FloatAbs(f64 val) {
  return _mm_cvtsd_f64(
      _mm_and_pd(_mm_set1_pd(val),
                 _mm_castsi128_pd(_mm_set1_epi64x(0x7fffffffffffffffull))));
}

template <>
inline f32 WABT_VECTORCALL FloatCopysign(f32 lhs, f32 rhs) {
  return _mm_cvtss_f32(
    _mm_or_ps(
      _mm_and_ps(_mm_set1_ps(lhs), _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff))),
      _mm_and_ps(_mm_set1_ps(rhs), _mm_castsi128_ps(_mm_set1_epi32(0x80000000)))));
}

template <>
inline f64 WABT_VECTORCALL FloatCopysign(f64 lhs, f64 rhs) {
  return _mm_cvtsd_f64(
    _mm_or_pd(
      _mm_and_pd(_mm_set1_pd(lhs), _mm_castsi128_pd(_mm_set1_epi64x(0x7fffffffffffffffull))),
      _mm_and_pd(_mm_set1_pd(rhs), _mm_castsi128_pd(_mm_set1_epi64x(0x8000000000000000ull)))));
}

#else
template <typename T>
T WABT_VECTORCALL FloatAbs(T val) {
  return std::abs(val);
}

template <typename T>
T WABT_VECTORCALL FloatCopysign(T lhs, T rhs) {
  return std::copysign(lhs, rhs);
}
#endif

#if COMPILER_IS_MSVC
#else
#endif

template <typename T> T WABT_VECTORCALL FloatNeg(T val) { return -val; }
template <typename T> T WABT_VECTORCALL FloatCeil(T val) { return CanonNaN(std::ceil(val)); }
template <typename T> T WABT_VECTORCALL FloatFloor(T val) { return CanonNaN(std::floor(val)); }
template <typename T> T WABT_VECTORCALL FloatTrunc(T val) { return CanonNaN(std::trunc(val)); }
template <typename T> T WABT_VECTORCALL FloatNearest(T val) { return CanonNaN(std::nearbyint(val)); }
template <typename T> T WABT_VECTORCALL FloatSqrt(T val) { return CanonNaN(std::sqrt(val)); }

template <typename T>
T WABT_VECTORCALL FloatDiv(T lhs, T rhs) {
  // IEE754 specifies what should happen when dividing a float by zero, but
  // C/C++ says it is undefined behavior.
  if (WABT_UNLIKELY(rhs == 0)) {
    return std::isnan(lhs) || lhs == 0
               ? std::numeric_limits<T>::quiet_NaN()
               : ((std::signbit(lhs) ^ std::signbit(rhs))
                      ? -std::numeric_limits<T>::infinity()
                      : std::numeric_limits<T>::infinity());
  }
  return CanonNaN(lhs / rhs);
}

template <typename T>
T WABT_VECTORCALL FloatMin(T lhs, T rhs) {
  if (WABT_UNLIKELY(std::isnan(lhs) || std::isnan(rhs))) {
    return std::numeric_limits<T>::quiet_NaN();
  } else if (WABT_UNLIKELY(lhs == 0 && rhs == 0)) {
    return std::signbit(lhs) ? lhs : rhs;
  } else {
    return std::min(lhs, rhs);
  }
}

template <typename T>
T WABT_VECTORCALL FloatPMin(T lhs, T rhs) {
  return std::min(lhs, rhs);
}

template <typename T>
T WABT_VECTORCALL FloatMax(T lhs, T rhs) {
  if (WABT_UNLIKELY(std::isnan(lhs) || std::isnan(rhs))) {
    return std::numeric_limits<T>::quiet_NaN();
  } else if (WABT_UNLIKELY(lhs == 0 && rhs == 0)) {
    return std::signbit(lhs) ? rhs : lhs;
  } else {
    return std::max(lhs, rhs);
  }
}

template <typename T>
T WABT_VECTORCALL FloatPMax(T lhs, T rhs) {
  return std::max(lhs, rhs);
}

template <typename R, typename T> bool WABT_VECTORCALL CanConvert(T val) { return true; }
template <> inline bool WABT_VECTORCALL CanConvert<s32, f32>(f32 val) { return val >= -2147483648.f && val < 2147483648.f; }
template <> inline bool WABT_VECTORCALL CanConvert<s32, f64>(f64 val) { return val > -2147483649. && val < 2147483648.; }
template <> inline bool WABT_VECTORCALL CanConvert<u32, f32>(f32 val) { return val > -1.f && val < 4294967296.f; }
template <> inline bool WABT_VECTORCALL CanConvert<u32, f64>(f64 val) { return val > -1. && val < 4294967296.; }
template <> inline bool WABT_VECTORCALL CanConvert<s64, f32>(f32 val) { return val >= -9223372036854775808.f && val < 9223372036854775808.f; }
template <> inline bool WABT_VECTORCALL CanConvert<s64, f64>(f64 val) { return val >= -9223372036854775808. && val < 9223372036854775808.; }
template <> inline bool WABT_VECTORCALL CanConvert<u64, f32>(f32 val) { return val > -1.f && val < 18446744073709551616.f; }
template <> inline bool WABT_VECTORCALL CanConvert<u64, f64>(f64 val) { return val > -1. && val < 18446744073709551616.; }

template <typename R, typename T>
R WABT_VECTORCALL Convert(T val) {
  assert((CanConvert<R, T>(val)));
  return static_cast<R>(val);
}

template <>
inline f32 WABT_VECTORCALL Convert(f64 val) {
  // The WebAssembly rounding mode means that these values (which are > F32_MAX)
  // should be rounded to F32_MAX and not set to infinity. Unfortunately, UBSAN
  // complains that the value is not representable as a float, so we'll special
  // case them.
  const f64 kMin = 3.4028234663852886e38;
  const f64 kMax = 3.4028235677973366e38;
  if (WABT_LIKELY(val >= -kMin && val <= kMin)) {
    return val;
  } else if (WABT_UNLIKELY(val > kMin && val < kMax)) {
    return std::numeric_limits<f32>::max();
  } else if (WABT_UNLIKELY(val > -kMax && val < -kMin)) {
    return -std::numeric_limits<f32>::max();
  } else if (WABT_UNLIKELY(std::isnan(val))) {
    return std::numeric_limits<f32>::quiet_NaN();
  } else {
    return std::copysign(std::numeric_limits<f32>::infinity(), val);
  }
}

template <>
inline f32 WABT_VECTORCALL Convert(u64 val) {
  return wabt_convert_uint64_to_float(val);
}

template <>
inline f64 WABT_VECTORCALL Convert(u64 val) {
  return wabt_convert_uint64_to_double(val);
}

template <>
inline f32 WABT_VECTORCALL Convert(s64 val) {
  return wabt_convert_int64_to_float(val);
}

template <>
inline f64 WABT_VECTORCALL Convert(s64 val) {
  return wabt_convert_int64_to_double(val);
}

template <typename T, int N>
T WABT_VECTORCALL IntExtend(T val) {
  // Hacker's delight 2.6 - sign extension
  auto bit = T{1} << N;
  auto mask = (bit << 1) - 1;
  return ((val & mask) ^ bit) - bit;
}

template <typename R, typename T>
R WABT_VECTORCALL IntTruncSat(T val) {
  if (WABT_UNLIKELY(std::isnan(val))) {
    return 0;
  } else if (WABT_UNLIKELY(!CanConvert<R>(val))) {
    return std::signbit(val) ? std::numeric_limits<R>::min()
                             : std::numeric_limits<R>::max();
  } else {
    return static_cast<R>(val);
  }
}

template <typename T> struct SatPromote;
template <> struct SatPromote<s8> { using type = s32; };
template <> struct SatPromote<s16> { using type = s32; };
template <> struct SatPromote<u8> { using type = s32; };
template <> struct SatPromote<u16> { using type = s32; };

template <typename R, typename T>
R WABT_VECTORCALL Saturate(T val) {
  static_assert(sizeof(R) < sizeof(T), "Incorrect types for Saturate");
  const T min = std::numeric_limits<R>::min();
  const T max = std::numeric_limits<R>::max();
  return val > max ? max : val < min ? min : val;
}

template <typename T, typename U = typename SatPromote<T>::type>
T WABT_VECTORCALL IntAddSat(T lhs, T rhs) {
  return Saturate<T, U>(lhs + rhs);
}

template <typename T, typename U = typename SatPromote<T>::type>
T WABT_VECTORCALL IntSubSat(T lhs, T rhs) {
  return Saturate<T, U>(lhs - rhs);
}

template <typename T>
T WABT_VECTORCALL SaturatingRoundingQMul(T lhs, T rhs) {
  constexpr int size_in_bits = sizeof(T) * 8;
  int round_const = 1 << (size_in_bits - 2);
  int64_t product = lhs * rhs;
  product += round_const;
  product >>= (size_in_bits - 1);
  return Saturate<T, int64_t>(product);
}

}  // namespace interp
}  // namespace wabt

// clang-format off

#endif  // WABT_INTERP_MATH_H_

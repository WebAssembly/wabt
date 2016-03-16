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

#include "wasm-parse-literal.h"

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define HEX_DIGIT_BITS 4

/* The PLUS_ONE values are used because normal IEEE floats have an implicit
 * leading one, so they have an additional bit of precision. */

#define F32_SIGN_SHIFT 31
#define F32_SIG_BITS 23
#define F32_SIG_MASK 0x7fffff
#define F32_SIG_PLUS_ONE_BITS 24
#define F32_SIG_PLUS_ONE_MASK 0xffffff
#define F32_MIN_EXP -127
#define F32_MAX_EXP 128
#define F32_EXP_BIAS 127

#define F64_SIGN_SHIFT 63
#define F64_SIG_BITS 52
#define F64_SIG_MASK 0xfffffffffffffULL
#define F64_SIG_PLUS_ONE_BITS 53
#define F64_SIG_PLUS_ONE_MASK 0x1fffffffffffffULL
#define F64_MIN_EXP -1023
#define F64_MAX_EXP 1024
#define F64_EXP_BIAS 1023

int wasm_parse_hexdigit(char c, uint32_t* out) {
  if ((unsigned int)(c - '0') <= 9) {
    *out = c - '0';
    return 1;
  } else if ((unsigned int)(c - 'a') <= 6) {
    *out = 10 + (c - 'a');
    return 1;
  } else if ((unsigned int)(c - 'A') <= 6) {
    *out = 10 + (c - 'A');
    return 1;
  }
  return 0;
}

/* return 1 if the non-NULL-terminated string starting with |start| and ending
 with |end| starts with the NULL-terminated string |prefix|. */
static int string_starts_with(const char* start,
                              const char* end,
                              const char* prefix) {
  while (start < end && *prefix) {
    if (*start != *prefix)
      return 0;
    start++;
    prefix++;
  }
  return *prefix == 0;
}

int wasm_parse_uint64(const char* s, const char* end, uint64_t* out) {
  if (s == end)
    return 0;
  uint64_t value = 0;
  if (*s == '0' && s + 1 < end && s[1] == 'x') {
    s += 2;
    if (s == end)
      return 0;
    for (; s < end; ++s) {
      uint32_t digit;
      if (!wasm_parse_hexdigit(*s, &digit))
        return 0;
      uint64_t old_value = value;
      value = value * 16 + digit;
      /* check for overflow */
      if (old_value > value)
        return 0;
    }
  } else {
    for (; s < end; ++s) {
      uint32_t digit = (*s - '0');
      if (digit > 9)
        return 0;
      uint64_t old_value = value;
      value = value * 10 + digit;
      /* check for overflow */
      if (old_value > value)
        return 0;
    }
  }
  if (s != end)
    return 0;
  *out = value;
  return 1;
}

int wasm_parse_int64(const char* s, const char* end, uint64_t* out) {
  int has_sign = 0;
  if (*s == '-') {
    has_sign = 1;
    s++;
  }
  uint64_t value = 0;
  int result = wasm_parse_uint64(s, end, &value);
  if (has_sign) {
    if (value > (uint64_t)INT64_MAX + 1) /* abs(INT64_MIN) == INT64_MAX + 1 */
      return 0;
    value = UINT64_MAX - value + 1;
  }
  *out = value;
  return result;
}

int wasm_parse_int32(const char* s,
                     const char* end,
                     uint32_t* out,
                     int allow_signed) {
  uint64_t value;
  int has_sign = 0;
  if (*s == '-') {
    if (!allow_signed)
      return 0;
    has_sign = 1;
    s++;
  }
  if (!wasm_parse_uint64(s, end, &value))
    return 0;

  if (has_sign) {
    if (value > (uint64_t)INT32_MAX + 1) /* abs(INT32_MIN) == INT32_MAX + 1 */
      return 0;
    value = UINT32_MAX - value + 1;
  } else {
    if (value > (uint64_t)UINT32_MAX)
      return 0;
  }
  *out = (uint32_t)value;
  return 1;
}

/* floats */
static uint32_t make_float(int sign, int exp, uint32_t sig) {
  assert(sign == 0 || sign == 1);
  assert(exp >= F32_MIN_EXP && exp <= F32_MAX_EXP);
  assert(sig <= F32_SIG_MASK);
  return ((uint32_t)sign << F32_SIGN_SHIFT) |
         ((uint32_t)(exp + F32_EXP_BIAS) << F32_SIG_BITS) | sig;
}

static uint32_t shift_float_and_round_to_nearest(uint32_t significand,
                                                 int shift) {
  assert(shift > 0);
  /* round ties to even */
  if (significand & ((uint32_t)1 << shift))
    significand += (uint32_t)1 << (shift - 1);
  significand >>= shift;
  return significand;
}

static int parse_float_nan(const char* s, const char* end, uint32_t* out_bits) {
  int is_neg = 0;
  if (*s == '-') {
    is_neg = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }
  assert(string_starts_with(s, end, "nan"));
  s += 3;

  uint32_t tag;
  if (s != end) {
    tag = 0;
    assert(string_starts_with(s, end, ":0x"));
    s += 3;

    for (; s < end; ++s) {
      uint32_t digit;
      if (!wasm_parse_hexdigit(*s, &digit))
        return 0;
      tag = tag * 16 + digit;
      /* check for overflow */
      if (tag > F32_SIG_MASK)
        return 0;
    }

    /* NaN cannot have a zero tag, that is reserved for infinity */
    if (tag == 0)
      return 0;
  } else {
    /* normal quiet NaN */
    tag = 0x400000;
  }

  *out_bits = make_float(is_neg, F32_MAX_EXP, tag);
  return 1;
}

static void parse_float_hex(const char* s,
                            const char* end,
                            uint32_t* out_bits) {
  int is_neg = 0;
  if (*s == '-') {
    is_neg = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }
  assert(string_starts_with(s, end, "0x"));
  s += 2;

  /* loop over the significand; everything up to the 'p'.
   this code is a bit nasty because we want to support extra zeroes anywhere
   without having to use many significand bits.
   e.g.
   0x00000001.0p0 => significand = 1, significand_exponent = 0
   0x10000000.0p0 => significand = 1, significand_exponent = 28
   0x0.000001p0 => significand = 1, significand_exponent = -24
   */
  int seen_dot = 0;
  uint32_t significand = 0;
  /* how much to shift |significand| if a non-zero value is appended */
  int significand_shift = 0;
  int significand_bits = 0; /* bits of |significand| */
  int significand_exponent = 0;  /* exponent adjustment due to dot placement */
  for (; s < end; ++s) {
    uint32_t digit;
    if (*s == '.') {
      if (significand != 0)
        significand_exponent += significand_shift;
      significand_shift = 0;
      seen_dot = 1;
      continue;
    } else if (!wasm_parse_hexdigit(*s, &digit)) {
      break;
    }
    significand_shift += HEX_DIGIT_BITS;
    if (digit != 0 && (significand == 0 ||
                       significand_bits + significand_shift <=
                           F32_SIG_BITS + 1 + HEX_DIGIT_BITS)) {
      significand <<= significand_shift;
      if (seen_dot)
        significand_exponent -= significand_shift;
      significand += digit;
      significand_shift = 0;
      significand_bits += HEX_DIGIT_BITS;
    }
  }

  if (!seen_dot)
    significand_exponent += significand_shift;

  assert(s < end && *s == 'p');
  s++;

  if (significand == 0) {
    /* 0 or -0 */
    *out_bits = make_float(is_neg, F32_MIN_EXP, 0);
    return;
  }

  assert(s < end);
  int exponent = 0;
  int exponent_is_neg = 0;
  /* exponent is always positive, but significand_exponent is signed.
   significand_exponent_add is negated if exponent will be negative, so it  can
   be easily summed to see if the exponent is too large (see below) */
  int significand_exponent_add = 0;
  if (*s == '-') {
    exponent_is_neg = 1;
    significand_exponent_add = -significand_exponent;
    s++;
  } else if (*s == '+') {
    s++;
    significand_exponent_add = significand_exponent;
  }

  for (; s < end; ++s) {
    uint32_t digit = (*s - '0');
    assert(digit <= 9);
    exponent = exponent * 10 + digit;
    if (exponent + significand_exponent_add >= F32_MAX_EXP)
      break;
  }

  if (exponent_is_neg)
    exponent = -exponent;

  significand_bits = sizeof(uint32_t) * 8 - wasm_clz_u32(significand);
  /* -1 for the implicit 1 bit of the significand */
  exponent += significand_exponent + significand_bits - 1;

  if (exponent >= F32_MAX_EXP) {
    /* inf or -inf */
    *out_bits = make_float(is_neg, F32_MAX_EXP, 0);
  } else if (exponent <= F32_MIN_EXP) {
    /* maybe subnormal */
    if (significand_bits > F32_SIG_BITS) {
      significand = shift_float_and_round_to_nearest(
          significand, significand_bits - F32_SIG_BITS);
    } else if (significand_bits < F32_SIG_BITS) {
      significand <<= (F32_SIG_BITS - significand_bits);
    }

    int shift = F32_MIN_EXP - exponent;
    if (shift < F32_SIG_BITS) {
      if (shift) {
        significand =
            shift_float_and_round_to_nearest(significand, shift) & F32_SIG_MASK;
      }
      exponent = F32_MIN_EXP;

      if (significand != 0) {
        *out_bits = make_float(is_neg, exponent, significand);
        return;
      }
    }

    /* not subnormal, too small; return 0 or -0 */
    *out_bits = make_float(is_neg, F32_MIN_EXP, 0);
  } else {
    /* normal value */
    if (significand_bits > F32_SIG_PLUS_ONE_BITS) {
      significand = shift_float_and_round_to_nearest(
          significand, significand_bits - F32_SIG_PLUS_ONE_BITS);
      if (significand > F32_SIG_PLUS_ONE_MASK)
        exponent++;
    } else if (significand_bits < F32_SIG_PLUS_ONE_BITS) {
      significand <<= (F32_SIG_PLUS_ONE_BITS - significand_bits);
    }

    *out_bits = make_float(is_neg, exponent, significand & F32_SIG_MASK);
  }
}

static void parse_float_infinity(const char* s,
                                 const char* end,
                                 uint32_t* out_bits) {
  int is_neg = 0;
  if (*s == '-') {
    is_neg = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }
  assert(string_starts_with(s, end, "infinity"));
  *out_bits = make_float(is_neg, F32_MAX_EXP, 0);
}

int wasm_parse_float(WasmLiteralType literal_type,
                     const char* s,
                     const char* end,
                     uint32_t* out_bits) {
  switch (literal_type) {
    case WASM_LITERAL_TYPE_INT:
    case WASM_LITERAL_TYPE_FLOAT: {
      errno = 0;
      char* endptr;
      float value;
      value = strtof(s, &endptr);
      if (endptr != end ||
          ((value == 0 || value == HUGE_VALF || value == -HUGE_VALF) &&
           errno != 0))
        return 0;

      memcpy(out_bits, &value, sizeof(value));
      return 1;
    }

    case WASM_LITERAL_TYPE_HEXFLOAT:
      parse_float_hex(s, end, out_bits);
      return 1;

    case WASM_LITERAL_TYPE_INFINITY:
      parse_float_infinity(s, end, out_bits);
      return 1;

    case WASM_LITERAL_TYPE_NAN:
      return parse_float_nan(s, end, out_bits);

    default:
      assert(0);
      return 0;
  }
}

/* doubles */
static uint64_t make_double(int sign, int exp, uint64_t sig) {
  assert(sign == 0 || sign == 1);
  assert(exp >= F64_MIN_EXP && exp <= F64_MAX_EXP);
  assert(sig <= F64_SIG_MASK);
  return ((uint64_t)sign << F64_SIGN_SHIFT) |
         ((uint64_t)(exp + F64_EXP_BIAS) << F64_SIG_BITS) | sig;
}

static uint64_t shift_double_and_round_to_nearest(uint64_t significand,
                                                  int shift) {
  assert(shift > 0);
  /* round ties to even */
  if (significand & ((uint64_t)1 << shift))
    significand += (uint64_t)1 << (shift - 1);
  significand >>= shift;
  return significand;
}

static int parse_double_nan(const char* s,
                            const char* end,
                            uint64_t* out_bits) {
  int is_neg = 0;
  if (*s == '-') {
    is_neg = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }
  assert(string_starts_with(s, end, "nan"));
  s += 3;

  uint64_t tag;
  if (s != end) {
    tag = 0;
    if (!string_starts_with(s, end, ":0x"))
      return 0;
    s += 3;

    for (; s < end; ++s) {
      uint32_t digit;
      if (!wasm_parse_hexdigit(*s, &digit))
        return 0;
      tag = tag * 16 + digit;
      /* check for overflow */
      if (tag > F64_SIG_MASK)
        return 0;
    }

    /* NaN cannot have a zero tag, that is reserved for infinity */
    if (tag == 0)
      return 0;
  } else {
    /* normal quiet NaN */
    tag = 0x8000000000000ULL;
  }

  *out_bits = make_double(is_neg, F64_MAX_EXP, tag);
  return 1;
}

static void parse_double_hex(const char* s,
                             const char* end,
                             uint64_t* out_bits) {
  int is_neg = 0;
  if (*s == '-') {
    is_neg = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }
  assert(string_starts_with(s, end, "0x"));
  s += 2;

  /* see the similar comment in parse_float_hex */
  int seen_dot = 0;
  uint64_t significand = 0;
  /* how much to shift |significand| if a non-zero value is appended */
  int significand_shift = 0;
  int significand_bits = 0; /* bits of |significand| */
  int significand_exponent = 0;  /* exponent adjustment due to dot placement */
  for (; s < end; ++s) {
    uint32_t digit;
    if (*s == '.') {
      if (significand != 0)
        significand_exponent += significand_shift;
      significand_shift = 0;
      seen_dot = 1;
      continue;
    } else if (!wasm_parse_hexdigit(*s, &digit)) {
      break;
    }
    significand_shift += HEX_DIGIT_BITS;
    if (digit != 0 && (significand == 0 ||
                       significand_bits + significand_shift <=
                           F64_SIG_BITS + 1 + HEX_DIGIT_BITS)) {
      significand <<= significand_shift;
      if (seen_dot)
        significand_exponent -= (int)significand_shift;
      significand += digit;
      significand_shift = 0;
      significand_bits += HEX_DIGIT_BITS;
    }
  }

  if (!seen_dot)
    significand_exponent += significand_shift;

  assert(s < end && *s == 'p');
  s++;

  if (significand == 0) {
    /* 0 or -0 */
    *out_bits = make_double(is_neg, F64_MIN_EXP, 0);
    return;
  }

  assert(s < end);
  int exponent = 0;
  int exponent_is_neg = 0;
  /* exponent is always positive, but significand_exponent is signed.
   significand_exponent_add is negated if exponent will be negative, so it  can
   be easily summed to see if the exponent is too large (see below) */
  int significand_exponent_add = 0;
  if (*s == '-') {
    exponent_is_neg = 1;
    significand_exponent_add = -significand_exponent;
    s++;
  } else if (*s == '+') {
    s++;
    significand_exponent_add = significand_exponent;
  }

  for (; s < end; ++s) {
    uint32_t digit = (*s - '0');
    assert(digit <= 9);
    exponent = exponent * 10 + digit;
    if (exponent + significand_exponent_add >= F64_MAX_EXP)
      break;
  }

  if (exponent_is_neg)
    exponent = -exponent;

  significand_bits = sizeof(uint64_t) * 8 - wasm_clz_u64(significand);
  /* -1 for the implicit 1 bit of the significand */
  exponent += significand_exponent + significand_bits - 1;

  if (exponent >= F64_MAX_EXP) {
    /* inf or -inf */
    *out_bits = make_double(is_neg, F64_MAX_EXP, 0);
  } else if (exponent <= F64_MIN_EXP) {
    /* maybe subnormal */
    if (significand_bits > F64_SIG_BITS) {
      significand = shift_double_and_round_to_nearest(
          significand, significand_bits - F64_SIG_BITS);
    } else if (significand_bits < F64_SIG_BITS) {
      significand <<= (F64_SIG_BITS - significand_bits);
    }

    int shift = F64_MIN_EXP - exponent;
    if (shift < F64_SIG_BITS) {
      if (shift) {
        significand = shift_double_and_round_to_nearest(significand, shift) &
                      F64_SIG_MASK;
      }
      exponent = F64_MIN_EXP;

      if (significand != 0) {
        *out_bits = make_double(is_neg, exponent, significand);
        return;
      }
    }

    /* not subnormal, too small; return 0 or -0 */
    *out_bits = make_double(is_neg, F64_MIN_EXP, 0);
  } else {
    /* normal value */
    if (significand_bits > F64_SIG_PLUS_ONE_BITS) {
      significand = shift_double_and_round_to_nearest(
          significand, significand_bits - F64_SIG_PLUS_ONE_BITS);
      if (significand > F64_SIG_PLUS_ONE_MASK)
        exponent++;
    } else if (significand_bits < F64_SIG_PLUS_ONE_BITS) {
      significand <<= (F64_SIG_PLUS_ONE_BITS - significand_bits);
    }

    *out_bits = make_double(is_neg, exponent, significand & F64_SIG_MASK);
  }
}

static void parse_double_infinity(const char* s,
                                  const char* end,
                                  uint64_t* out_bits) {
  int is_neg = 0;
  if (*s == '-') {
    is_neg = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }
  assert(string_starts_with(s, end, "infinity"));
  *out_bits = make_double(is_neg, F64_MAX_EXP, 0);
}

int wasm_parse_double(WasmLiteralType literal_type,
                      const char* s,
                      const char* end,
                      uint64_t* out_bits) {
  switch (literal_type) {
    case WASM_LITERAL_TYPE_INT:
    case WASM_LITERAL_TYPE_FLOAT: {
      errno = 0;
      char* endptr;
      double value;
      value = strtod(s, &endptr);
      if (endptr != end ||
          ((value == 0 || value == HUGE_VAL || value == -HUGE_VAL) &&
           errno != 0))
        return 0;

      memcpy(out_bits, &value, sizeof(value));
      return 1;
    }

    case WASM_LITERAL_TYPE_HEXFLOAT:
      parse_double_hex(s, end, out_bits);
      return 1;

    case WASM_LITERAL_TYPE_INFINITY:
      parse_double_infinity(s, end, out_bits);
      return 1;

    case WASM_LITERAL_TYPE_NAN:
      return parse_double_nan(s, end, out_bits);

    default:
      assert(0);
      return 0;
  }
}


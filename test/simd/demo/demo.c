#include <wasm_simd128.h>


void all_the_i8x16(int* in_a, int* in_b, int* out) {
  v128_t a = wasm_v128_load(in_a);
  v128_t b = wasm_v128_load(in_b);

  v128_t res = wasm_i8x16_abs(a);
  res = wasm_i8x16_add(res, a);
  res = wasm_i8x16_add_sat(res, a);
  bool alltrue = wasm_i8x16_all_true(res);
  int bitmask = wasm_i8x16_bitmask(res);
  res = wasm_i8x16_const(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  res = wasm_i8x16_const_splat(99);
  res = wasm_i8x16_eq(res, a);
  int extracted = wasm_i8x16_extract_lane(res, 7);
  res = wasm_i8x16_ge(res, a);
  res = wasm_i8x16_gt(res, a);
  res = wasm_i8x16_le(res, a);
  res = wasm_i8x16_lt(res, a);
  res = wasm_i8x16_make(1, 2, 3, 4, 5, 6, 7, 8, extracted, 10, bitmask, 12, 13, 14, 15, 16);
  res = wasm_i8x16_max(res, a);
  res = wasm_i8x16_min(res, a);
  res = wasm_i8x16_narrow_i16x8(res, a);
  res = wasm_i8x16_ne(res, a);
  res = wasm_i8x16_neg(res);
  res = wasm_i8x16_popcnt(res);
  res = wasm_i8x16_replace_lane(res, 3, 100);
  res = wasm_i8x16_shl(res, 2);
  res = wasm_i8x16_shr(res, 7);
  res = wasm_i8x16_shuffle(res, a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0);
  res = wasm_i8x16_splat(22);
  res = wasm_i8x16_sub(res, a);
  res = wasm_i8x16_sub_sat(res, a);
  res = wasm_i8x16_swizzle(res, a);

  wasm_v128_store(out, res);
}

void all_the_u8x16(int* in_a, int* in_b, int* out) {
  v128_t a = wasm_v128_load(in_a);
  v128_t b = wasm_v128_load(in_b);

  v128_t res = wasm_u8x16_add_sat(b, a);
  int extracted = wasm_u8x16_extract_lane(res, 7);
  res = wasm_u8x16_ge(res, a);
  res = wasm_u8x16_avgr(res, a);
  res = wasm_u8x16_gt(res, a);
  res = wasm_u8x16_le(res, a);
  res = wasm_u8x16_lt(res, a);
  res = wasm_u8x16_max(res, a);
  res = wasm_u8x16_min(res, a);
  res = wasm_u8x16_narrow_i16x8(res, a);
  res = wasm_u8x16_shr(res, 7);
  res = wasm_u8x16_sub_sat(res, a);

  wasm_v128_store(out, res);
}

void all_the_f32x4(int* in_a, int* in_b, int* out) {
  v128_t a = wasm_v128_load(in_a);
  v128_t b = wasm_v128_load(in_b);

  float f0 = 2.1;
  float f1 = 3.7;
  float f2 = 12.0;
  float f3 = 0.4;

  v128_t res = wasm_f32x4_abs(a);
  res = wasm_f32x4_add(a, res);
  res = wasm_f32x4_ceil(res);
  res = wasm_f32x4_const(2, 4, 8, 16);
  res = wasm_f32x4_const_splat(2);
  res = wasm_f32x4_convert_i32x4(res);
  res = wasm_f32x4_convert_u32x4(res);
  res = wasm_f32x4_demote_f64x2_zero(res);
  res = wasm_f32x4_div(res, b);
  res = wasm_f32x4_eq(a, res);
  f3 = wasm_f32x4_extract_lane(res, 3);
  res = wasm_f32x4_floor(res);
  res = wasm_f32x4_ge(a, res);
  res = wasm_f32x4_gt(a, res);
  res = wasm_f32x4_le(a, res);
  res = wasm_f32x4_lt(a, res);
  res = wasm_f32x4_make(f0, f1, f2, f3);
  res = wasm_f32x4_max(a, res);
  res = wasm_f32x4_min(res, b);
  res = wasm_f32x4_mul(a, res);
  res = wasm_f32x4_ne(a, res);

  // this instruction had some issues with linking to roundf
  res = wasm_f32x4_nearest(a);

  res = wasm_f32x4_neg(a);
  res = wasm_f32x4_pmax(res, b);
  res = wasm_f32x4_pmin(res, b);
  res = wasm_f32x4_replace_lane(res, 0, f3);
  
  res = wasm_f32x4_splat(f0);
  res = wasm_f32x4_sqrt(res);
  res = wasm_f32x4_sub(a, res);
  res = wasm_f32x4_trunc(res);

  wasm_v128_store(out, res);
}

void all_the_f64x2(int* in_a, int* in_b, int* out) {
  v128_t a = wasm_v128_load(in_a);
  v128_t b = wasm_v128_load(in_b);

  float f0 = 2.1;
  float f1 = 3.7;
  float f2 = 12.0;
  float f3 = 0.4;

  v128_t res = wasm_f64x2_abs(a);
  res = wasm_f64x2_add(a, res);
  res = wasm_f64x2_ceil(res);
  res = wasm_f64x2_const(2.3, 13.37);
  res = wasm_f64x2_const_splat(2);
  res = wasm_f64x2_convert_low_i32x4(res);
  res = wasm_f64x2_convert_low_u32x4(res);
  res = wasm_f64x2_div(res, b);
  res = wasm_f64x2_eq(a, res);
  f3 = wasm_f64x2_extract_lane(res, 3);
  res = wasm_f64x2_floor(res);
  res = wasm_f64x2_ge(a, res);
  res = wasm_f64x2_gt(a, res);
  res = wasm_f64x2_le(a, res);
  res = wasm_f64x2_lt(a, res);
  res = wasm_f64x2_make(f0, f1);
  res = wasm_f64x2_max(a, res);
  res = wasm_f64x2_min(res, b);
  res = wasm_f64x2_mul(a, res);
  res = wasm_f64x2_ne(a, res);
  res = wasm_f64x2_nearest(a);
  res = wasm_f64x2_neg(a);
  res = wasm_f64x2_pmax(res, b);
  res = wasm_f64x2_pmin(res, b);
  res = wasm_f64x2_promote_low_f32x4(res);
  res = wasm_f64x2_replace_lane(res, 0, f3);
  res = wasm_f64x2_splat(f0);
  res = wasm_f64x2_sqrt(res);
  res = wasm_f64x2_sub(a, res);
  res = wasm_f64x2_trunc(res);

  wasm_v128_store(out, res);
}

void all_the_i16x8(int* in_a, int* in_b, int* out) {
  v128_t a = wasm_v128_load(in_a);
  v128_t b = wasm_v128_load(in_b);

  v128_t res = wasm_i16x8_abs(a);
  res = wasm_i16x8_add(res, a);
  res = wasm_i16x8_add_sat(res, a);
  bool alltrue = wasm_i16x8_all_true(res);
  int mask = wasm_i16x8_bitmask(res);
  res = wasm_i16x8_const(1, 2, 4, 8, 16, 32, 64, 128);
  res = wasm_i16x8_const_splat(22);
  res = wasm_i16x8_eq(res, a);
  res = wasm_i16x8_extadd_pairwise_i8x16(res);
  res = wasm_i16x8_extend_high_i8x16(res);
  res = wasm_i16x8_extend_low_i8x16(res);
  res = wasm_i16x8_extmul_high_i8x16(res, b);
  res = wasm_i16x8_extmul_low_i8x16(res, b);
  int extracted = wasm_i16x8_extract_lane(res, 2);
  res = wasm_i16x8_ge(res, a);
  res = wasm_i16x8_gt(res, a);
  res = wasm_i16x8_le(res, a);
  res = wasm_i16x8_load8x8(in_a);
  res = wasm_i16x8_lt(res, b);
  res = wasm_i16x8_make(1, 2, 4, 8, 16, 32, 64, 128);
  res = wasm_i16x8_max(res, a);
  res = wasm_i16x8_min(res, a);
  res = wasm_i16x8_mul(res, a);
  res = wasm_i16x8_narrow_i32x4(res, a);
  res = wasm_i16x8_ne(res, a);
  res = wasm_i16x8_neg(res);
  res = wasm_i16x8_q15mulr_sat(res, a);
  res = wasm_i16x8_replace_lane(res, 0, mask);
  res = wasm_i16x8_shl(res, extracted);
  res = wasm_i16x8_shr(res, extracted);
  res = wasm_i16x8_shuffle(res, a, 1, 2, 3, 4, 5, 6, 7, 0);
  res = wasm_i16x8_splat(22);
  res = wasm_i16x8_sub(res, a);
  res = wasm_i16x8_sub_sat(res, a);

  wasm_v128_store(out, res);
}

void all_the_u16x8(int* in_a, int* in_b, int* out) {
  v128_t a = wasm_v128_load(in_a);
  v128_t b = wasm_v128_load(in_b);

  v128_t res = wasm_u16x8_add_sat(b, a);
  res = wasm_u16x8_avgr(res, b);
  res = wasm_u16x8_extadd_pairwise_u8x16(res);
  res = wasm_u16x8_extend_high_u8x16(res);
  res = wasm_u16x8_extend_low_u8x16(res);
  res = wasm_u16x8_extmul_high_u8x16(res, b);
  res = wasm_u16x8_extmul_low_u8x16(res, b);
  int extracted = wasm_u16x8_extract_lane(res, 2);
  res = wasm_u16x8_ge(res, a);
  res = wasm_u16x8_gt(res, a);
  res = wasm_u16x8_le(res, a);
  res = wasm_u16x8_load8x8(in_a);
  res = wasm_u16x8_lt(res, b);
  res = wasm_u16x8_max(res, a);
  res = wasm_u16x8_min(res, a);
  res = wasm_u16x8_narrow_i32x4(res, a);
  res = wasm_u16x8_shr(res, extracted);
  res = wasm_u16x8_sub_sat(res, a);

  wasm_v128_store(out, res);
}

void all_the_i32x4(int* in_a, int* in_b, int* out) {
  v128_t a = wasm_v128_load(in_a);
  v128_t b = wasm_v128_load(in_b);

  v128_t res = wasm_i32x4_abs(a);
  res = wasm_i32x4_add(b, res);
  bool alltrue = wasm_i32x4_all_true(res);
  int bitmask = wasm_i32x4_bitmask(res);
  res = wasm_i32x4_const(1, 2, 4, 8);
  res = wasm_i32x4_const_splat(1337);
  res = wasm_i32x4_dot_i16x8(a, res);
  res = wasm_i32x4_eq(a, res);
  res = wasm_i32x4_extadd_pairwise_i16x8(res);
  res = wasm_i32x4_extend_high_i16x8(res);
  res = wasm_i32x4_extend_low_i16x8(res);
  res = wasm_i32x4_extmul_high_i16x8(a, res);
  res = wasm_i32x4_extmul_low_i16x8(a, res);
  int extracted = wasm_i32x4_extract_lane(a, 3);
  res = wasm_i32x4_ge(a, res);
  res = wasm_i32x4_gt(a, res);
  res = wasm_i32x4_le(a, res);
  res = wasm_i32x4_load16x4(in_b);
  res = wasm_i32x4_lt(a, res);
  res = wasm_i32x4_make(bitmask, extracted, 23, 19);
  res = wasm_i32x4_max(a, res);
  res = wasm_i32x4_min(a, res);
  res = wasm_i32x4_mul(a, res);
  res = wasm_i32x4_ne(a, res);
  res = wasm_i32x4_neg(res);
  res = wasm_i32x4_replace_lane(res, 3, extracted);
  res = wasm_i32x4_shl(a, 2);
  res = wasm_i32x4_shr(a, 3);
  res = wasm_i32x4_shuffle(a, res, 1, 2, 3, 0);
  res = wasm_i32x4_splat(1337);
  res = wasm_i32x4_sub(a, res);
  res = wasm_i32x4_trunc_sat_f32x4(res);
  res = wasm_i32x4_trunc_sat_f64x2_zero(res);

  wasm_v128_store(out, res);
}

void all_the_u32x4(int* in_a, int* in_b, int* out) {
  v128_t a = wasm_v128_load(in_a);
  v128_t b = wasm_v128_load(in_b);

  v128_t res = wasm_u32x4_extadd_pairwise_u16x8(a);
  res = wasm_u32x4_extend_high_u16x8(res);
  res = wasm_u32x4_extend_low_u16x8(res);
  res = wasm_u32x4_extmul_high_u16x8(a, res);
  res = wasm_u32x4_extmul_low_u16x8(a, res);
  res = wasm_u32x4_ge(a, res);
  res = wasm_u32x4_gt(a, res);
  res = wasm_u32x4_le(a, res);
  res = wasm_u32x4_load16x4(in_b);
  res = wasm_u32x4_lt(a, res);
  res = wasm_u32x4_max(a, res);
  res = wasm_u32x4_min(a, res);
  res = wasm_u32x4_shr(a, 3);
  res = wasm_u32x4_trunc_sat_f32x4(res);
  res = wasm_u32x4_trunc_sat_f64x2_zero(res);

  wasm_v128_store(out, res);
}

void all_the_i64x2(int* in_a, int* in_b, int* out) {
  v128_t a = wasm_v128_load(in_a);
  v128_t b = wasm_v128_load(in_b);

  v128_t res = wasm_i64x2_abs(a);
  res = wasm_i64x2_add(b, res);
  bool alltrue = wasm_i64x2_all_true(res);
  int bitmask = wasm_i64x2_bitmask(res);
  res = wasm_i64x2_const(23, 78);
  res = wasm_i64x2_const_splat(22);
  res = wasm_i64x2_eq(a, res);
  res = wasm_i64x2_extend_high_i32x4(res);
  res = wasm_i64x2_extend_low_i32x4(res);
  res = wasm_i64x2_extmul_high_i32x4(b, res);
  res = wasm_i64x2_extmul_low_i32x4(a, res);
  int extracted = wasm_i64x2_extract_lane(res, 0);
  res = wasm_i64x2_ge(a, res);
  res = wasm_i64x2_gt(a, res);
  res = wasm_i64x2_le(a, res);
  res = wasm_i64x2_load32x2(in_b);
  res = wasm_i64x2_lt(a, res);
  res = wasm_i64x2_make(bitmask, extracted);
  res = wasm_i64x2_mul(a, res);
  res = wasm_i64x2_ne(a, res);
  res = wasm_i64x2_neg(res);
  res = wasm_i64x2_replace_lane(res, 1, extracted);
  res = wasm_i64x2_shl(res, 1);
  res = wasm_i64x2_shr(res, 1);
  res = wasm_i64x2_shuffle(a, res, 1, 0);
  res = wasm_i64x2_splat(223);
  res = wasm_i64x2_sub(a, res);

  wasm_v128_store(out, res);
}


void all_the_u64x2(int* in_a, int* in_b, int* out) {
  v128_t a = wasm_v128_load(in_a);
  v128_t b = wasm_v128_load(in_b);

  v128_t res = wasm_u64x2_load32x2(in_b);
  res = wasm_u64x2_extend_high_u32x4(res);
  res = wasm_u64x2_extend_low_u32x4(res);
  res = wasm_u64x2_extmul_high_u32x4(b, res);
  res = wasm_u64x2_extmul_low_u32x4(a, res);
  res = wasm_u64x2_shr(res, 1);

  wasm_v128_store(out, res);
}


void all_the_v128(int* in_a, int* in_b, int* out) {
  v128_t a = wasm_v128_load(in_a);
  v128_t b = wasm_v128_load(in_b);

  v128_t res = wasm_v128_and(a, b);
  res = wasm_v128_andnot(res, a);
  bool anytrue = wasm_v128_any_true(res);
  res = wasm_v128_bitselect(res, a, b);
  //res = wasm_v128_load(res, a);
  res = wasm_v128_load16_lane(in_a, res, 1);
  res = wasm_v128_load16_splat(in_b);

  res = wasm_v128_load32_lane(in_a, res, 2);
  res = wasm_v128_load32_splat(in_b);
  res = wasm_v128_load32_zero(in_b);

  res = wasm_v128_load64_lane(in_a, res, 1);
  res = wasm_v128_load64_splat(in_b);
  res = wasm_v128_load64_zero(in_b);

  res = wasm_v128_load8_lane(in_a, res, 2);
  res = wasm_v128_load8_splat(in_b);

  res = wasm_v128_not(res);
  res = wasm_v128_or(res, a);
  //res = wasm_v128_store(res, a);

  wasm_v128_store16_lane(in_a, res, 2);
  wasm_v128_store32_lane(in_a, res, 1);
  wasm_v128_store64_lane(in_a, res, 0);
  wasm_v128_store8_lane(in_a, res, 2);
  res = wasm_v128_xor(res, a);

  wasm_v128_store(out, res);
}
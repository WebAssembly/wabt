;;; TOOL: run-interp
(module
  (func $f32_is_nan (param f32) (result i32)
    get_local 0
    get_local 0
    f32.ne)
  (func $f64_is_nan (param f64) (result i32)
    get_local 0
    get_local 0
    f64.ne)

  ;; i32
  (func (export "i32_eqz_100") (result i32) 
    i32.const 100
    i32.eqz)
  (func (export "i32_eqz_0") (result i32)
    i32.const 0 
    i32.eqz)
  (func (export "i32_clz") (result i32)
    i32.const 128 
    i32.clz)
  (func (export "i32_ctz") (result i32)
    i32.const 128 
    i32.ctz)
  (func (export "i32_popcnt") (result i32) 
    i32.const 128
    i32.popcnt)

  ;; i64
  (func (export "i64_eqz_100") (result i32)
    i64.const 100 
    i64.eqz)
  (func (export "i64_eqz_0") (result i32)
    i64.const 0 
    i64.eqz)
  (func (export "i64_clz") (result i64) 
    i64.const 128
    i64.clz)
  (func (export "i64_ctz") (result i64) 
    i64.const 128
    i64.ctz)
  (func (export "i64_popcnt") (result i64)
    i64.const 128 
    i64.popcnt)

  ;; f32
  (func (export "f32_neg") (result f32)
    f32.const 100 
    f32.neg)
  (func (export "f32_abs") (result f32)
    f32.const -100 
    f32.abs)
  (func (export "f32_sqrt_neg_is_nan") (result i32)
    f32.const -100
    f32.sqrt
    call $f32_is_nan)
  (func (export "f32_sqrt_100") (result f32)
    f32.const 100
    f32.sqrt)
  (func (export "f32_ceil") (result f32)
    f32.const -0.75 
    f32.ceil)
  (func (export "f32_floor") (result f32)
    f32.const -0.75 
    f32.floor)
  (func (export "f32_trunc") (result f32)
    f32.const -0.75 
    f32.trunc)
  (func (export "f32_nearest_lo") (result f32)
    f32.const 1.25 
    f32.nearest)
  (func (export "f32_nearest_hi") (result f32)
    f32.const 1.75 
    f32.nearest)

  ;; f64
  (func (export "f64_neg") (result f64) 
    f64.const 100
    f64.neg)
  (func (export "f64_abs") (result f64) 
    f64.const -100
    f64.abs)
  (func (export "f64_sqrt_neg_is_nan") (result i32)
    f64.const -100
    f64.sqrt
    call $f64_is_nan)
  (func (export "f64_sqrt_100") (result f64)
   f64.const 100 
   f64.sqrt)
  (func (export "f64_ceil") (result f64)
   f64.const -0.75 
   f64.ceil)
  (func (export "f64_floor") (result f64)
    f64.const -0.75 
    f64.floor)
  (func (export "f64_trunc") (result f64)
    f64.const -0.75 
    f64.trunc)
  (func (export "f64_nearest_lo") (result f64) 
    f64.const 1.25
    f64.nearest)
  (func (export "f64_nearest_hi") (result f64)
    f64.const 1.75 
    f64.nearest)
)
(;; STDOUT ;;;
i32_eqz_100() => i32:0
i32_eqz_0() => i32:1
i32_clz() => i32:24
i32_ctz() => i32:7
i32_popcnt() => i32:1
i64_eqz_100() => i32:0
i64_eqz_0() => i32:1
i64_clz() => i64:56
i64_ctz() => i64:7
i64_popcnt() => i64:1
f32_neg() => f32:-100
f32_abs() => f32:100
f32_sqrt_neg_is_nan() => i32:1
f32_sqrt_100() => f32:10
f32_ceil() => f32:-0
f32_floor() => f32:-1
f32_trunc() => f32:-0
f32_nearest_lo() => f32:1
f32_nearest_hi() => f32:2
f64_neg() => f64:-100
f64_abs() => f64:100
f64_sqrt_neg_is_nan() => i32:1
f64_sqrt_100() => f64:10
f64_ceil() => f64:-0
f64_floor() => f64:-1
f64_trunc() => f64:-0
f64_nearest_lo() => f64:1
f64_nearest_hi() => f64:2
;;; STDOUT ;;)

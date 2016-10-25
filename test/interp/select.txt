;;; TOOL: run-interp
(module
  (func $i32 (param i32) (result i32)
    i32.const 1
    i32.const 2
    get_local 0
    select)
  (func $i64 (param i32) (result i64)
    i64.const 1
    i64.const 2
    get_local 0
    select)
  (func $f32 (param i32) (result f32)
    f32.const 1
    f32.const 2
    get_local 0
    select)
  (func $f64 (param i32) (result f64)
    f64.const 1
    f64.const 2
    get_local 0
    select)

  (func (export "test_i32_l") (result i32)
    i32.const 0 
    call $i32)
  (func (export "test_i32_r") (result i32) 
    i32.const 1
    call $i32)
  (func (export "test_i64_l") (result i64) 
    i32.const 0
    call $i64)
  (func (export "test_i64_r") (result i64) 
    i32.const 1
    call $i64)
  (func (export "test_f32_l") (result f32) 
    i32.const 0
    call $f32)
  (func (export "test_f32_r") (result f32) 
   i32.const 1
   call $f32)
  (func (export "test_f64_l") (result f64) 
   i32.const 0
   call $f64)
  (func (export "test_f64_r") (result f64) 
   i32.const 1
   call $f64))
(;; STDOUT ;;;
test_i32_l() => i32:2
test_i32_r() => i32:1
test_i64_l() => i64:2
test_i64_r() => i64:1
test_f32_l() => f32:2
test_f32_r() => f32:1
test_f64_l() => f64:2
test_f64_r() => f64:1
;;; STDOUT ;;)

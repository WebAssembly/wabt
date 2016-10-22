;;; TOOL: run-interp
(module
  (type $v_i (func (result i32)))
  (func $zero (type $v_i) 
     i32.const 0)
  (func $one (type $v_i) 
     i32.const 1)

  (func $nullary (param i32) (result i32)
    get_local 0
    call_indirect $v_i)

  (type $ii_i (func (param i32 i32) (result i32)))
  (func $add (type $ii_i)
    get_local 0 
    get_local 1
    i32.add)
  (func $sub (type $ii_i)
     get_local 0
     get_local 1 
     i32.sub)

  (func $binary (param i32 i32 i32) (result i32)
    get_local 0
    get_local 1
    get_local 2
    call_indirect $ii_i)

  (table anyfunc (elem $zero $one $add $sub))

  (func (export "test_zero") (result i32)
    i32.const 0
    call $nullary)

  (func (export "test_one") (result i32)
    i32.const 1
    call $nullary)

  (func (export "test_add") (result i32)
    i32.const 10
    i32.const 4
    i32.const 2
    call $binary)

  (func (export "test_sub") (result i32)
    i32.const 10
    i32.const 4
    i32.const 3
    call $binary)

  (func (export "trap_oob") (result i32)
    i32.const 10
    i32.const 4
    i32.const 4
    call $binary)

  (func (export "trap_sig_mismatch") (result i32)
    i32.const 10
    i32.const 4
    i32.const 0
    call $binary))
(;; STDOUT ;;;
test_zero() => i32:0
test_one() => i32:1
test_add() => i32:14
test_sub() => i32:6
trap_oob() => error: undefined table index
trap_sig_mismatch() => error: indirect call signature mismatch
;;; STDOUT ;;)

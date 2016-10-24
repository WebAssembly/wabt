;;; TOOL: run-interp
(module
  (memory 1)
  (func $store_unless (param i32)
    get_local 0
    i32.const 0
    i32.eq
    if
      return
    end
    i32.const 0
    i32.const 1
    i32.store)

  (func (export "test1")
    i32.const 0
    call $store_unless)

  (func (export "check1") (result i32)
    i32.const 0
    i32.load)

  (func (export "test2")
    i32.const 1
    call $store_unless)

  (func (export "check2") (result i32)
    i32.const 0
    i32.load))
(;; STDOUT ;;;
test1() =>
check1() => i32:0
test2() =>
check2() => i32:1
;;; STDOUT ;;)

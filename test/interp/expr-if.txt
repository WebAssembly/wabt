;;; TOOL: run-interp
(module
  (func (param i32) (result i32)
    get_local 0
    i32.const 0
    i32.eq
    if i32
      i32.const 1
    else
      i32.const 2
    end)

  (func (export "test1") (result i32)
    i32.const 0
    call 0)

  (func (export "test2") (result i32)
    i32.const 1
    call 0))
(;; STDOUT ;;;
test1() => i32:1
test2() => i32:2
;;; STDOUT ;;)

;;; TOOL: run-interp
(module
  (func $f (param i32) (result i32)
    block $exit
      get_local 0
      br_if $exit
      i32.const 1
      return
    end
    i32.const 2
    return)

  (func (export "test1") (result i32)
    i32.const 0
    call $f)

  (func (export "test2") (result i32)
    i32.const 1
    call $f))
(;; STDOUT ;;;
test1() => i32:1
test2() => i32:2
;;; STDOUT ;;)

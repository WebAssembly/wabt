;;; TOOL: run-interp
(module
  (func $f (param i32) (result i32)
    (local i32)
    loop $cont
      get_local 1
      i32.const 1
      i32.add
      set_local 1
      get_local 1
      get_local 0
      i32.lt_s
      br_if $cont
    end
    get_local 1
    return)

  (func (export "test1") (result i32)
    i32.const 3
    call $f)

  (func (export "test2") (result i32)
    i32.const 10
    call $f))
(;; STDOUT ;;;
test1() => i32:3
test2() => i32:10
;;; STDOUT ;;)

;;; TOOL: run-interp
(module
  (func $f (result i32)
    i32.const 42)

  (func $g (param i32 i32) (result i32)
    get_local 0
    get_local 1
    i32.add)

  (func (export "h") (result i32)
    i32.const 1
    call $f
    call $g))
(;; STDOUT ;;;
h() => i32:43
;;; STDOUT ;;)

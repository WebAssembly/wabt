;;; TOOL: run-interp
(module
  (func $trap
    i32.const 1
    i32.const 0
    i32.div_s
    drop)

  (func $f 
    call $trap)
  (func $g 
    call $f)
  (func (export "h") 
    call $g)

  ;; this function should run properly, even after h traps.
  (func (export "i") (result i32)
    i32.const 22))
(;; STDOUT ;;;
h() => error: integer divide by zero
i() => i32:22
;;; STDOUT ;;)

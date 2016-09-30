;;; TOOL: run-interp
(module
  (func (param i32) (result i32)
    (block $exit i32
      (i32.sub
        (br_if $exit (i32.const 42) (get_local 0))
        (i32.const 13))))

  (func (export "test1") (result i32)
    (call 0 (i32.const 0)))

  (func (export "test2") (result i32)
    (call 0 (i32.const 1))))
(;; STDOUT ;;;
test1() => i32:29
test2() => i32:42
;;; STDOUT ;;)

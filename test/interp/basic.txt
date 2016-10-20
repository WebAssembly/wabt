;;; TOOL: run-interp
(module
  (func (export "main") (result i32)
    i32.const 42
    return))
(;; STDOUT ;;;
main() => i32:42
;;; STDOUT ;;)

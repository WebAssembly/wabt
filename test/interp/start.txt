;;; TOOL: run-interp
(module
  (memory 1)
  (func $start
    i32.const 0 
    i32.const 42
    i32.store)
  (start $start)
  (func (export "get") (result i32) 
    i32.const 0
    i32.load))
(;; STDOUT ;;;
get() => i32:42
;;; STDOUT ;;)

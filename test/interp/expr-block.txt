;;; TOOL: run-interp
(module
  (func (export "test") (result i32)
    block i32
      i32.const 10
      drop 
      i32.const 1
    end
  )
)
(;; STDOUT ;;;
test() => i32:1
;;; STDOUT ;;)

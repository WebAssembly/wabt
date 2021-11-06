;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (global i32 
    i32.const 1
    i32.const 2
    i32.add))
(;; STDERR ;;;
out/test/parse/module/bad-global-invalid-expr.txt:4:4: error: invalid global initializer expression, must be a constant expression
  (global i32 
   ^^^^^^
;;; STDERR ;;)

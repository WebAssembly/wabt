;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (memory 1)
  (func
    i32.const 0
    i32.load align=3 
    drop))
(;; STDERR ;;;
out/test/parse/expr/bad-load-align-not-pot.txt:7:14: error: alignment must be power-of-two
    i32.load align=3 
             ^^^^^^^
;;; STDERR ;;)

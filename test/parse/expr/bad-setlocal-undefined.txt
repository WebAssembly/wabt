;;; TOOL: wat2wasm
;;; ERROR: 1
(module (func
          i32.const 0 
          set_local 0))
(;; STDERR ;;;
out/test/parse/expr/bad-setlocal-undefined.txt:5:21: error: local variable out of range (max 0)
          set_local 0))
                    ^
;;; STDERR ;;)

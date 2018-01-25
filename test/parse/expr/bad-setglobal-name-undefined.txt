;;; TOOL: wat2wasm
;;; ERROR: 1
(module (func
          i32.const 1 
          set_global $f))
(;; STDERR ;;;
out/test/parse/expr/bad-setglobal-name-undefined.txt:5:22: error: undefined global variable "$f"
          set_global $f))
                     ^^
;;; STDERR ;;)

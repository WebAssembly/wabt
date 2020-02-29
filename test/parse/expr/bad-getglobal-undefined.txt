;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (func
    get_global 0
    drop))
(;; STDERR ;;;
out/test/parse/expr/bad-getglobal-undefined.txt:5:16: error: global variable out of range: 0 (max 0)
    get_global 0
               ^
;;; STDERR ;;)

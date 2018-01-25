;;; TOOL: wat2wasm
;;; ERROR: 1
(module (func get_global $n))
(;; STDERR ;;;
out/test/parse/expr/bad-getglobal-name-undefined.txt:3:26: error: undefined global variable "$n"
(module (func get_global $n))
                         ^^
;;; STDERR ;;)

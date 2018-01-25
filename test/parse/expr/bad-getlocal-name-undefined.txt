;;; TOOL: wat2wasm
;;; ERROR: 1
(module (func get_local $n))
(;; STDERR ;;;
out/test/parse/expr/bad-getlocal-name-undefined.txt:3:25: error: undefined local variable "$n"
(module (func get_local $n))
                        ^^
;;; STDERR ;;)

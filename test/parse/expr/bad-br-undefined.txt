;;; TOOL: wat2wasm
;;; ERROR: 1
(module (func br 1))
(;; STDERR ;;;
out/test/parse/expr/bad-br-undefined.txt:3:15: error: invalid depth: 1 (max 0)
(module (func br 1))
              ^^
;;; STDERR ;;)

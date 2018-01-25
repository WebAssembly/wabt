;;; TOOL: wat2wasm
;;; ERROR: 1
(module (func))

;; Can't have two modules in a .wat file.
(module)
(;; STDERR ;;;
out/test/parse/module/bad-module-multi.txt:6:1: error: unexpected token (, expected EOF.
(module)
^
;;; STDERR ;;)

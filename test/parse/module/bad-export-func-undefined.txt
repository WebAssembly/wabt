;;; TOOL: wat2wasm
;;; ERROR: 1
(module (export "foo" (func 0)))
(;; STDERR ;;;
out/test/parse/module/bad-export-func-undefined.txt:3:29: error: function variable out of range: 0 (max 0)
(module (export "foo" (func 0)))
                            ^
;;; STDERR ;;)

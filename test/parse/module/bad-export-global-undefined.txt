;;; TOOL: wat2wasm
;;; ERROR: 1
(module (export "foo" (global 0)))
(;; STDERR ;;;
out/test/parse/module/bad-export-global-undefined.txt:3:31: error: global variable out of range: 0 (max 0)
(module (export "foo" (global 0)))
                              ^
;;; STDERR ;;)

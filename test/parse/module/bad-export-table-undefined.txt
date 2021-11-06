;;; TOOL: wat2wasm
;;; ERROR: 1
(module (export "foo" (table 0)))
(;; STDERR ;;;
out/test/parse/module/bad-export-table-undefined.txt:3:30: error: table variable out of range: 0 (max 0)
(module (export "foo" (table 0)))
                             ^
;;; STDERR ;;)

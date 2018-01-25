;;; TOOL: wat2wasm
;;; ERROR: 1
(module (export "foo" (table $bar)))
(;; STDERR ;;;
out/test/parse/module/bad-export-table-name-undefined.txt:3:30: error: undefined table variable "$bar"
(module (export "foo" (table $bar)))
                             ^^^^
;;; STDERR ;;)

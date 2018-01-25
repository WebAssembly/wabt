;;; TOOL: wat2wasm
;;; ERROR: 1
(module (export "foo" (memory $bar)))
(;; STDERR ;;;
out/test/parse/module/bad-export-memory-name-undefined.txt:3:31: error: undefined memory variable "$bar"
(module (export "foo" (memory $bar)))
                              ^^^^
;;; STDERR ;;)

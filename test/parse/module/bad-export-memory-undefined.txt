;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (export "mem" (memory 0)))
(;; STDERR ;;;
out/test/parse/module/bad-export-memory-undefined.txt:4:25: error: memory variable out of range: 0 (max 0)
  (export "mem" (memory 0)))
                        ^
;;; STDERR ;;)

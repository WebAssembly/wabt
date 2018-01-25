;;; TOOL: wat2wasm
;;; ERROR: 1
(module (memory 2 1))
(;; STDERR ;;;
out/test/parse/module/bad-memory-max-less-than-init.txt:3:10: error: max pages (1) must be >= initial pages (2)
(module (memory 2 1))
         ^^^^^^
;;; STDERR ;;)

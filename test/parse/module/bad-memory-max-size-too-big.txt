;;; TOOL: wat2wasm
;;; ERROR: 1
(module (memory 0 65537))
(;; STDERR ;;;
out/test/parse/module/bad-memory-max-size-too-big.txt:3:10: error: max pages (65537) must be <= (65536)
(module (memory 0 65537))
         ^^^^^^
;;; STDERR ;;)

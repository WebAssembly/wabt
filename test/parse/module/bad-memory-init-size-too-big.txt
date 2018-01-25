;;; TOOL: wat2wasm
;;; ERROR: 1
(module (memory 65537))
(;; STDERR ;;;
out/test/parse/module/bad-memory-init-size-too-big.txt:3:10: error: initial pages (65537) must be <= (65536)
(module (memory 65537))
         ^^^^^^
;;; STDERR ;;)

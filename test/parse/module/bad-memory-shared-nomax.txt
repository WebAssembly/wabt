;;; TOOL: wat2wasm
;;; ARGS: --enable-threads
;;; ERROR: 1
(module (memory 1 shared))

(;; STDERR ;;;
out/test/parse/module/bad-memory-shared-nomax.txt:4:10: error: shared memories must have max sizes
(module (memory 1 shared))
         ^^^^^^
;;; STDERR ;;)

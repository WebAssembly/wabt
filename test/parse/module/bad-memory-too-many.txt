;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (memory 1)
  (memory 2))
(;; STDERR ;;;
out/test/parse/module/bad-memory-too-many.txt:5:4: error: only one memory block allowed
  (memory 2))
   ^^^^^^
;;; STDERR ;;)

;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (start 0)
  (func (param i32)))
(;; STDERR ;;;
out/test/parse/module/bad-start-not-nullary.txt:4:4: error: start function must be nullary
  (start 0)
   ^^^^^
;;; STDERR ;;)

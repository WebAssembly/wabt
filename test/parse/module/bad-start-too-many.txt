;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (start 0)
  (start 1)
  (func)
  (func))
(;; STDERR ;;;
out/test/parse/module/bad-start-too-many.txt:5:4: error: multiple start sections
  (start 1)
   ^^^^^
;;; STDERR ;;)

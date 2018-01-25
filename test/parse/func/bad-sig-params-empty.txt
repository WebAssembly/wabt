;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (type $t (func))
  (func (type $t) (param i32)))
(;; STDERR ;;;
out/test/parse/func/bad-sig-params-empty.txt:5:4: error: expected 0 arguments, got 1
  (func (type $t) (param i32)))
   ^^^^
;;; STDERR ;;)

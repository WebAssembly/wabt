;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (type $t (func (param i32 f32)))
  (func (type $t) (param i32)))
(;; STDERR ;;;
out/test/parse/func/bad-sig-too-few-params.txt:5:4: error: expected 2 arguments, got 1
  (func (type $t) (param i32)))
   ^^^^
;;; STDERR ;;)

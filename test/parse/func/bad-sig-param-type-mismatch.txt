;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (type $t (func (param i32 f32)))
  (func (type $t) (param f32 f32)))
(;; STDERR ;;;
out/test/parse/func/bad-sig-param-type-mismatch.txt:5:4: error: type mismatch for argument 0 of function. got f32, expected i32
  (func (type $t) (param f32 f32)))
   ^^^^
;;; STDERR ;;)

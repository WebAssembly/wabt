;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (type $t (func (param i32)))
  (func (type $t) (param i32) (result f32)))
(;; STDERR ;;;
out/test/parse/func/bad-sig-result-type-not-void.txt:5:4: error: expected 0 results, got 1
  (func (type $t) (param i32) (result f32)))
   ^^^^
;;; STDERR ;;)

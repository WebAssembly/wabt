;;; TOOL: wat2wasm
;;; ARGS: --enable-exceptions
(module
  (type $t (func (param f64)))
  (event)
  (event $e1 (param i32))
  (event $e2 (param f32 f32 f32))
  (event $e3 (type $t)))

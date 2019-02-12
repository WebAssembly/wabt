;;; TOOL: wat2wasm
;;; ARGS: --enable-exceptions
(module
  (import "foo" "1" (event (param f64 f32))))

;;; TOOL: wat2wasm
(module
  (import "foo" "bar" (func $bar (param f32)))
  (func
    f32.const 0
    call $bar))

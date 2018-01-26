;;; TOOL: wat2wasm
(module
  (import "foo" "bar" (func (param i32) (result i32)))
  (func (param i32) (result i32)
    i32.const 0
    call 0))

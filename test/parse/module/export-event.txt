;;; TOOL: wat2wasm
;;; ARGS: --enable-exceptions
(module
  (event (param i32 i32))
  (export "my_event" (event 0)))

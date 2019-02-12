;;; TOOL: wat2wasm
;;; ARGS: --enable-exceptions
(module
  (event $e1)
  (func throw $e1))

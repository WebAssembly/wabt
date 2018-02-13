;;; TOOL: wat2wasm
;;; ARGS: --enable-threads
(module
  (import "foo" "2" (memory 0 2 shared)))

;;; TOOL: wat2wasm
;;; ARGS: --enable-exceptions
(module
  (event $e1)
  (func
    try
    catch
      rethrow
    end))

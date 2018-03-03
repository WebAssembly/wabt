;;; TOOL: wat2wasm
;;; ARGS: --enable-exceptions
(module
  ;; bare
  (func
    try
      nop
    catch
      drop
    end)

  ;; with label
  (func
    try $foo
      nop
    catch
      drop
    end)

  ;; with result type
  (func
    try (result i32)
      i32.const 1
    catch
      drop
      i32.const 2
    end
    drop))

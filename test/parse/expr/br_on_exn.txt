;;; TOOL: wat2wasm
;;; ARGS: --enable-exceptions
(module
  (event $e (param i32))
  (func
    block $b (result i32)
      try
      catch
        br_on_exn $b $e
        drop
      end
      i32.const 0
    end
    drop))

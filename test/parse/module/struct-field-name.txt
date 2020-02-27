;;; TOOL: wat2wasm
;;; ARGS: --enable-gc
(type
  (struct (field $a f32)
          (field $b (mut f64))))

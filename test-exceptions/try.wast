;; out/clang/Debug/wast2wasm --except test-exceptions/try.wast
(module
  (func (result i32)
    (try (result i32)
      (nop)
      (i32.const 7)
    )
    (catch 1
      (i32.const 7)
    )
    (catch_all
       (i32.const 7))
    )
  )
)

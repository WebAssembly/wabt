;; Test throw instruction.

(module
  (tag $e0)
  (tag $e-i32 (param i32))
  (tag $e-f32 (param f32))
  (tag $e-i64 (param i64))
  (tag $e-f64 (param f64))
  (tag $e-i32-i32 (param i32 i32))

  (func $throw-if (export "throw-if") (param i32) (result i32)
    (local.get 0)
    (i32.const 0) (if (i32.ne) (then (throw $e0)))
    (i32.const 0)
  )

  (func (export "throw-param-f32") (param f32) (local.get 0) (throw $e-f32))

  (func (export "throw-param-i64") (param i64) (local.get 0) (throw $e-i64))

  (func (export "throw-param-f64") (param f64) (local.get 0) (throw $e-f64))

  (func $throw-1-2 (i32.const 1) (i32.const 2) (throw $e-i32-i32))
  (func (export "test-throw-1-2")
    (try
      (do (call $throw-1-2))
      (catch $e-i32-i32
        (i32.const 2)
        (if (i32.ne) (then (unreachable)))
        (i32.const 1)
        (if (i32.ne) (then (unreachable)))
      )
    )
  )
)

(assert_return (invoke "throw-if" (i32.const 0)) (i32.const 0))
(assert_exception (invoke "throw-if" (i32.const 10)))
(assert_exception (invoke "throw-if" (i32.const -1)))

(assert_exception (invoke "throw-param-f32" (f32.const 5.0)))
(assert_exception (invoke "throw-param-i64" (i64.const 5)))
(assert_exception (invoke "throw-param-f64" (f64.const 5.0)))

(assert_return (invoke "test-throw-1-2"))

(assert_invalid (module (func (throw 0))) "unknown tag 0")
(assert_invalid (module (tag (param i32)) (func (throw 0)))
                "type mismatch: instruction requires [i32] but stack has []")
(assert_invalid (module (tag (param i32)) (func (i64.const 5) (throw 0)))
                "type mismatch: instruction requires [i32] but stack has [i64]")

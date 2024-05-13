;; Test rethrow instruction.

(module
  (tag $e0)
  (tag $e1)

  (func (export "catch-rethrow-0")
    (try
      (do (throw $e0))
      (catch $e0 (rethrow 0))
    )
  )

  (func (export "catch-rethrow-1") (param i32) (result i32)
    (try (result i32)
      (do (throw $e0))
      (catch $e0
        (if (i32.eqz (local.get 0)) (then (rethrow 1))) (i32.const 23)
      )
    )
  )

  (func (export "catchall-rethrow-0")
    (try
      (do (throw $e0))
      (catch_all (rethrow 0))
    )
  )

  (func (export "catchall-rethrow-1") (param i32) (result i32)
    (try (result i32)
      (do (throw $e0))
      (catch_all
        (if (i32.eqz (local.get 0)) (then (rethrow 1))) (i32.const 23)
      )
    )
  )

  (func (export "rethrow-nested") (param i32) (result i32)
    (try (result i32)
      (do (throw $e1))
      (catch $e1
        (try (result i32)
          (do (throw $e0))
          (catch $e0
            (if (i32.eq (local.get 0) (i32.const 0)) (then (rethrow 1)))
            (if (i32.eq (local.get 0) (i32.const 1)) (then (rethrow 2)))
            (i32.const 23)
          )
        )
      )
    )
  )

  (func (export "rethrow-recatch") (param i32) (result i32)
    (try (result i32)
      (do (throw $e0))
      (catch $e0
        (try (result i32)
         (do (if (i32.eqz (local.get 0)) (then (rethrow 2))) (i32.const 42))
         (catch $e0 (i32.const 23))
        )
      )
    )
  )

  (func (export "rethrow-stack-polymorphism")
    (try
      (do (throw $e0))
      (catch $e0 (i32.const 1) (rethrow 0))
    )
  )
)

(assert_exception (invoke "catch-rethrow-0"))

(assert_exception (invoke "catch-rethrow-1" (i32.const 0)))
(assert_return (invoke "catch-rethrow-1" (i32.const 1)) (i32.const 23))

(assert_exception (invoke "catchall-rethrow-0"))

(assert_exception (invoke "catchall-rethrow-1" (i32.const 0)))
(assert_return (invoke "catchall-rethrow-1" (i32.const 1)) (i32.const 23))
(assert_exception (invoke "rethrow-nested" (i32.const 0)))
(assert_exception (invoke "rethrow-nested" (i32.const 1)))
(assert_return (invoke "rethrow-nested" (i32.const 2)) (i32.const 23))

(assert_return (invoke "rethrow-recatch" (i32.const 0)) (i32.const 23))
(assert_return (invoke "rethrow-recatch" (i32.const 1)) (i32.const 42))

(assert_exception (invoke "rethrow-stack-polymorphism"))

(assert_invalid (module (func (rethrow 0))) "invalid rethrow label")
(assert_invalid (module (func (block (rethrow 0)))) "invalid rethrow label")
(assert_invalid (module (func (try (do (rethrow 0)) (delegate 0))))
                "invalid rethrow label")

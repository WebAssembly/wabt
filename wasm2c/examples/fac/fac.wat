(func (export "fac") (param i32) (result i32)
  (if (result i32) (i32.eq (get_local 0) (i32.const 0))
    (then (i32.const 1))
    (else
      (i32.mul (get_local 0) (call 0 (i32.sub (get_local 0) (i32.const 1))))
    )
  )
)

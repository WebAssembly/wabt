(module
  (func (export "x") (param $x i32) (result i32) (unreachable))
)
(assert_return (invoke "x" (i32.const 1))
                           (i32.const 1))

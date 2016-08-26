(module
  (func $f (param i32))
  (func $g (param i32 i64))
  (func $h (result i64) (i64.const 0))
  (table anyfunc (elem $f $f $g $h)))

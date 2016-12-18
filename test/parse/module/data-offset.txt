(module
  (import "foo" "bar" (global i32))
  (memory 1)
  (global i32 i32.const 1)
  (data (get_global 0) "hi"))

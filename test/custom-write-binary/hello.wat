(module
  (type (;0;) (func (param i32 i32) (result i32)))
  (type (;1;) (func (param i32) (result i32)))
  (import "env" "printf" (func (;0;) (type 0)))
  (import "env" "get_const" (func (;1;) (type 1)))
  (func (;2;) (type 0) (param i32 i32) (result i32)
    i32.const 0
    call 1
    i32.const 0
    call 0
    drop
    i32.const 0)
  (memory (;0;) 1)
  (global (;0;) (mut i32) (i32.const 3072))
  (global (;1;) i32 (i32.const 1024))
  (global (;2;) i32 (i32.const 3072))
  (export "memory" (memory 0))
  (export "__main_argc_argv" (func 2))
  (export "__data_end" (global 1))
  (export "__heap_base" (global 2)))

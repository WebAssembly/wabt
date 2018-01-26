;;; TOOL: wat2wasm
(module
  (func
    i32.const 2
    i32.const 3
    i32.const 1
    select
    drop
    i64.const 2
    i64.const 3
    i32.const 1
    select
    drop
    f32.const 2
    f32.const 3
    i32.const 1
    select
    drop
    f64.const 2
    f64.const 3
    i32.const 1
    select
    drop))

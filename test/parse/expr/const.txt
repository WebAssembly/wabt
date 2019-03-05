;;; TOOL: wat2wasm
;;; ARGS: --enable-simd
(module
  (func
    i32.const 0
    drop
    i32.const -2147483648
    drop
    i32.const 4294967295
    drop
    i32.const -0x80000000
    drop
    i32.const 0xffffffff
    drop
    i64.const 0
    drop
    i64.const -9223372036854775808
    drop
    i64.const 18446744073709551615
    drop
    i64.const -0x8000000000000000
    drop
    i64.const 0xffffffffffffffff
    drop
    f32.const 0
    drop
    f32.const 1.e15
    drop
    f32.const 1e23
    drop
    f32.const 1.23457e-5
    drop
    f32.const nan
    drop
    f32.const -nan
    drop
    f32.const +nan
    drop
    f32.const nan:0xabc
    drop
    f32.const -nan:0xabc
    drop
    f32.const +nan:0xabc
    drop
    f32.const inf
    drop
    f32.const -inf
    drop
    f32.const +inf
    drop
    f32.const -0x1p-1
    drop
    f32.const 0x1.921fb6p+2
    drop
    f64.const 0.0
    drop
    f64.const 1.e300
    drop
    f64.const -0.987654321
    drop
    f64.const 6.283185307179586
    drop
    f64.const nan
    drop
    f64.const -nan
    drop
    f64.const +nan
    drop
    f64.const nan:0xabc
    drop
    f64.const -nan:0xabc
    drop
    f64.const +nan:0xabc
    drop
    f64.const inf
    drop
    f64.const -inf
    drop
    f64.const +inf
    drop
    f64.const -0x1p-1
    drop
    f64.const 0x1.921fb54442d18p+2
    drop
    v128.const i32x4 0x11223344 0x55667788 0x99aabbcc 0xddeeff00
    drop
    v128.const i32x4 0x11223344 1432778632 2578103244 0xddeeff00
    drop))

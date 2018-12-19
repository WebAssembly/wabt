;;; TOOL: wat2wasm
;;; ARGS: --enable-threads
(module
  (memory 1 1 shared)
  (func
    i32.const 0 i32.const 0 atomic.notify align=4 drop
    i32.const 0 i32.const 0 i64.const 0 i32.atomic.wait align=4 drop
    i32.const 0 i64.const 0 i64.const 0 i64.atomic.wait align=8 drop

    i32.const 0 i32.atomic.load align=4 drop
    i32.const 0 i64.atomic.load align=8 drop
    i32.const 0 i32.atomic.load8_u align=1 drop
    i32.const 0 i32.atomic.load16_u align=2 drop
    i32.const 0 i64.atomic.load8_u align=1 drop
    i32.const 0 i64.atomic.load16_u align=2 drop
    i32.const 0 i64.atomic.load32_u align=4 drop

    i32.const 0 i32.const 0 i32.atomic.store align=4
    i32.const 0 i64.const 0 i64.atomic.store align=8
    i32.const 0 i32.const 0 i32.atomic.store8 align=1
    i32.const 0 i32.const 0 i32.atomic.store16 align=2
    i32.const 0 i64.const 0 i64.atomic.store8 align=1
    i32.const 0 i64.const 0 i64.atomic.store16 align=2
    i32.const 0 i64.const 0 i64.atomic.store32 align=4

    i32.const 0 i32.const 0 i32.atomic.rmw.add align=4 drop
    i32.const 0 i64.const 0 i64.atomic.rmw.add align=8 drop
    i32.const 0 i32.const 0 i32.atomic.rmw8.add_u align=1 drop
    i32.const 0 i32.const 0 i32.atomic.rmw16.add_u align=2 drop
    i32.const 0 i64.const 0 i64.atomic.rmw8.add_u align=1 drop
    i32.const 0 i64.const 0 i64.atomic.rmw16.add_u align=2 drop
    i32.const 0 i64.const 0 i64.atomic.rmw32.add_u align=4 drop

    i32.const 0 i32.const 0 i32.atomic.rmw.sub align=4 drop
    i32.const 0 i64.const 0 i64.atomic.rmw.sub align=8 drop
    i32.const 0 i32.const 0 i32.atomic.rmw8.sub_u align=1 drop
    i32.const 0 i32.const 0 i32.atomic.rmw16.sub_u align=2 drop
    i32.const 0 i64.const 0 i64.atomic.rmw8.sub_u align=1 drop
    i32.const 0 i64.const 0 i64.atomic.rmw16.sub_u align=2 drop
    i32.const 0 i64.const 0 i64.atomic.rmw32.sub_u align=4 drop

    i32.const 0 i32.const 0 i32.atomic.rmw.and align=4 drop
    i32.const 0 i64.const 0 i64.atomic.rmw.and align=8 drop
    i32.const 0 i32.const 0 i32.atomic.rmw8.and_u align=1 drop
    i32.const 0 i32.const 0 i32.atomic.rmw16.and_u align=2 drop
    i32.const 0 i64.const 0 i64.atomic.rmw8.and_u align=1 drop
    i32.const 0 i64.const 0 i64.atomic.rmw16.and_u align=2 drop
    i32.const 0 i64.const 0 i64.atomic.rmw32.and_u align=4 drop

    i32.const 0 i32.const 0 i32.atomic.rmw.or align=4 drop
    i32.const 0 i64.const 0 i64.atomic.rmw.or align=8 drop
    i32.const 0 i32.const 0 i32.atomic.rmw8.or_u align=1 drop
    i32.const 0 i32.const 0 i32.atomic.rmw16.or_u align=2 drop
    i32.const 0 i64.const 0 i64.atomic.rmw8.or_u align=1 drop
    i32.const 0 i64.const 0 i64.atomic.rmw16.or_u align=2 drop
    i32.const 0 i64.const 0 i64.atomic.rmw32.or_u align=4 drop

    i32.const 0 i32.const 0 i32.atomic.rmw.xor align=4 drop
    i32.const 0 i64.const 0 i64.atomic.rmw.xor align=8 drop
    i32.const 0 i32.const 0 i32.atomic.rmw8.xor_u align=1 drop
    i32.const 0 i32.const 0 i32.atomic.rmw16.xor_u align=2 drop
    i32.const 0 i64.const 0 i64.atomic.rmw8.xor_u align=1 drop
    i32.const 0 i64.const 0 i64.atomic.rmw16.xor_u align=2 drop
    i32.const 0 i64.const 0 i64.atomic.rmw32.xor_u align=4 drop

    i32.const 0 i32.const 0 i32.atomic.rmw.xchg align=4 drop
    i32.const 0 i64.const 0 i64.atomic.rmw.xchg align=8 drop
    i32.const 0 i32.const 0 i32.atomic.rmw8.xchg_u align=1 drop
    i32.const 0 i32.const 0 i32.atomic.rmw16.xchg_u align=2 drop
    i32.const 0 i64.const 0 i64.atomic.rmw8.xchg_u align=1 drop
    i32.const 0 i64.const 0 i64.atomic.rmw16.xchg_u align=2 drop
    i32.const 0 i64.const 0 i64.atomic.rmw32.xchg_u align=4 drop

    i32.const 0 i32.const 0 i32.const 0 i32.atomic.rmw.cmpxchg align=4 drop
    i32.const 0 i64.const 0 i64.const 0 i64.atomic.rmw.cmpxchg align=8 drop
    i32.const 0 i32.const 0 i32.const 0 i32.atomic.rmw8.cmpxchg_u align=1 drop
    i32.const 0 i32.const 0 i32.const 0 i32.atomic.rmw16.cmpxchg_u align=2 drop
    i32.const 0 i64.const 0 i64.const 0 i64.atomic.rmw8.cmpxchg_u align=1 drop
    i32.const 0 i64.const 0 i64.const 0 i64.atomic.rmw16.cmpxchg_u align=2 drop
    i32.const 0 i64.const 0 i64.const 0 i64.atomic.rmw32.cmpxchg_u align=4 drop

))

(module
  (type (;0;) (func (param i64 i32)))
  (type (;1;) (func (param i32 i32) (result i32)))
  (func $store_char*__char_ (type 0) (param i64 i32)
    local.get 0
    local.get 1
    i32.store8)
  (func (export "get_from_stack") (type 1) (param i32 i32) (result i32)
    (local i64 i64 i64 i64)
    global.get $__stack_pointer
    local.tee 2
    local.set 3
    local.get 2
    local.get 0
    i64.extend_i32_s
    i64.const 15
    i64.add
    i64.const -16
    i64.and
    i64.sub
    local.tee 4
    global.set $__stack_pointer
    block  ;; label = @1
      local.get 0
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      i64.extend_i32_u
      local.set 5
      i64.const 0
      local.set 2
      loop  ;; label = @2
        local.get 4
        local.get 2
        i64.add
        local.get 2
        i64.const 1
        i64.add
        local.tee 2
        i32.wrap_i64
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $store_char*__char_
        local.get 5
        local.get 2
        i64.ne
        br_if 0 (;@2;)
      end
    end
    local.get 4
    local.get 1
    i64.extend_i32_s
    i64.add
    i32.load8_s
    local.set 0
    local.get 3
    global.set $__stack_pointer
    local.get 0)
  (memory (;0;) i64 2)
  (global $__stack_pointer (mut i64) (i64.const 66560))
  (export "memory" (memory 0))
)

(assert_return (invoke "get_from_stack"(i32.const 16)(i32.const 0)) (i32.const 1))
(assert_return (invoke "get_from_stack"(i32.const 16)(i32.const 5)) (i32.const 6))

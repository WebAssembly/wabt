(module
  (type (;0;) (func (param i32 i32)))
  (type (;1;) (func (param i32 i32) (result i32)))
  (func $store_char*__char_ (type 0) (param i32 i32)
    local.get 0
    local.get 1
    i32.store8)
  (func (export "get_from_stack") (type 1) (param i32 i32) (result i32)
    (local i32 i32 i32)
    global.get $__stack_pointer
    local.tee 2
    local.set 3
    local.get 2
    local.get 1
    i32.const 15
    i32.add
    i32.const -16
    i32.and
    i32.sub
    local.tee 4
    global.set $__stack_pointer
    block  ;; label = @1
      local.get 1
      i32.eqz
      br_if 0 (;@1;)
      i32.const 0
      local.set 2
      loop  ;; label = @2
        local.get 4
        local.get 2
        i32.add
        local.get 2
        i32.const 1
        i32.add
        local.tee 2
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $store_char*__char_
        local.get 1
        local.get 2
        i32.ne
        br_if 0 (;@2;)
      end
    end
    local.get 4
    local.get 0
    i32.add
    i32.load8_s
    local.set 2
    local.get 3
    global.set $__stack_pointer
    local.get 2)
  (memory (;0;) 2)
  (global $__stack_pointer (mut i32) (i32.const 66560))
  (export "memory" (memory 0))
)

(assert_return (invoke "get_from_stack"(i32.const 16)(i32.const 0)) (i32.const 1))
(assert_return (invoke "get_from_stack"(i32.const 16)(i32.const 5)) (i32.const 6))

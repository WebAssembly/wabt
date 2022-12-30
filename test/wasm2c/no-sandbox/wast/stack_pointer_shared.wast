(module
  (type (;0;) (func))
  (type (;1;) (func (param i64 i32)))
  (type (;2;) (func (param i32 i32) (result i32)))
  (import "env" "memory" (memory (;0;) i64 0))
  (import "env" "__indirect_function_table" (table (;0;) 0 funcref))
  (import "env" "__stack_pointer" (global $__stack_pointer (mut i64)))
  (import "env" "__memory_base" (global $__memory_base i64))
  (import "env" "__table_base" (global $__table_base i64))
  (import "env" "__table_base32" (global $__table_base32 i32))
  (func $__wasm_call_ctors (type 0)
    call $__wasm_apply_data_relocs)
  (func $__wasm_apply_data_relocs (type 0))
  (func $store_char*__char_ (type 1) (param i64 i32)
    local.get 0
    local.get 1
    i32.store8)
  (func $get_from_stack (type 2) (param i32 i32) (result i32)
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
      block  ;; label = @2
        local.get 0
        br_if 0 (;@2;)
        i32.const 0
        local.set 0
        br 1 (;@1;)
      end
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
      local.get 4
      local.get 1
      i64.extend_i32_s
      i64.add
      i32.load8_s
      local.set 0
    end
    local.get 3
    global.set $__stack_pointer
    local.get 0)
  (export "get_from_stack" (func $get_from_stack)))

(assert_return (invoke "get_from_stack"(i32.const 16)(i32.const 0)) (i32.const 1))
(assert_return (invoke "get_from_stack"(i32.const 16)(i32.const 5)) (i32.const 6))

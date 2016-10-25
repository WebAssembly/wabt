;;; TOOL: run-interp
(module
  (func (export "if1") (result i32) (local i32)
    i32.const 0
    set_local 0
    i32.const 1
    if
      get_local 0
      i32.const 1
      i32.add
      set_local 0 
    end
    i32.const 0
    if
      get_local 0
      i32.const 1
      i32.add
      set_local 0 
    end
    get_local 0
    return)

  (func (export "if2") (result i32) (local i32 i32)
    i32.const 1
    if
      i32.const 1 
      set_local 0
    else
      i32.const 2
      set_local 0
    end
    i32.const 0
    if
      i32.const 4
      set_local 1
    else
      i32.const 8
      set_local 1
    end
    get_local 0
    get_local 1
    i32.add
    return)
)
(;; STDOUT ;;;
if1() => i32:1
if2() => i32:9
;;; STDOUT ;;)

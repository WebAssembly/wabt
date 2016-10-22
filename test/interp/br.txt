;;; TOOL: run-interp
(module
  ;; basic br test
  (func (export "br0") (result i32)
    (local i32 i32)
    block $exit
      i32.const 1
      if 
        br 1     ;; if branches introduce blocks
      end
      i32.const 1
      set_local 0    ;; not executed
    end
    i32.const 1  
    set_local 1
    get_local 0
    i32.const 0
    i32.eq
    get_local 1
    i32.const 1
    i32.eq
    i32.add
    return)

  ;; test br-ing with a depth > 0
  (func (export "br1") (result i32)
    (local i32 i32 i32)
    block $outer
      block $inner
        i32.const 1
        if  
          br 2
        end     ;; if branches introduce blocks
        i32.const 1
        set_local 0    ;; not executed
      end
      i32.const 1
      set_local 1    ;; not executed
    end
    i32.const 1
    set_local 2
    get_local 0
    i32.const 0
    i32.eq
    get_local 1
    i32.const 0
    i32.eq
    i32.add
    get_local 2
    i32.const 1
    i32.eq
    i32.add
    return)

  ;; test br-ing to a label
  (func (export "br2") (result i32)
    block $exit
      block
        i32.const 1
        if
          br $exit
        end
        i32.const 1
        return      ;; not executed
      end
    end
    i32.const 2  
    return)

  ;; test br-ing in a loop with an exit and continue continuation
  (func (export "br3") (result i32)
    (local i32 i32)
    block $exit
      loop $cont
        get_local 0
        i32.const 1
        i32.add
        set_local 0
        get_local 0
        i32.const 5
        i32.ge_s
        if
          br $exit
        end
        get_local 0
        i32.const 4
        i32.eq 
        if 
          (br $cont)
        end
        get_local 0
        set_local 1
        br $cont
      end
    end
    get_local 1
    return)
)
(;; STDOUT ;;;
br0() => i32:2
br1() => i32:3
br2() => i32:2
br3() => i32:3
;;; STDOUT ;;)

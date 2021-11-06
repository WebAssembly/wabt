(import "host" "mem" (memory $mem 1))
(import "host" "fill_buf" (func $fill_buf (param i32 i32) (result i32)))
(import "host" "buf_done" (func $buf_done (param i32 i32)))

(func $rot13c (param $c i32) (result i32)
  (local $uc i32)

  ;; No change if < 'A'.
  (if (i32.lt_u (get_local $c) (i32.const 65))
    (return (get_local $c)))

  ;; Clear 5th bit of c, to force uppercase. 0xdf = 0b11011111
  (set_local $uc (i32.and (get_local $c) (i32.const 0xdf)))

  ;; In range ['A', 'M'] return |c| + 13.
  (if (i32.le_u (get_local $uc) (i32.const 77))
    (return (i32.add (get_local $c) (i32.const 13))))

  ;; In range ['N', 'Z'] return |c| - 13.
  (if (i32.le_u (get_local $uc) (i32.const 90))
    (return (i32.sub (get_local $c) (i32.const 13))))

  ;; No change for everything else.
  (return (get_local $c))
)

(func (export "rot13")
  (local $size i32)
  (local $i i32)

  ;; Ask host to fill memory [0, 1024) with data.
  (call $fill_buf (i32.const 0) (i32.const 1024))

  ;; The host returns the size filled.
  (set_local $size)

  ;; Loop over all bytes and rot13 them.
  (block $exit
    (loop $top
      ;; if (i >= size) break
      (if (i32.ge_u (get_local $i) (get_local $size)) (br $exit))

      ;; mem[i] = rot13c(mem[i])
      (i32.store8
        (get_local $i)
        (call $rot13c
          (i32.load8_u (get_local $i))))

      ;; i++
      (set_local $i (i32.add (get_local $i) (i32.const 1)))
      (br $top)
    )
  )

  (call $buf_done (i32.const 0) (get_local $size))
)

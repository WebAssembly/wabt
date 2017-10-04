(module
  (memory (shared 1 1))

  (func (export "worker") (param $id i32)
    (i32.atomic.store (get_local $id) (i32.const 1)))

  (func (export "main") (result i32)
    (local $workers_done i32)
    (local $id i32)

    (loop $not_finished
      (set_local $id (i32.const 0))

      (loop $each_worker
        (block $worker_not_finished
          (br_if $worker_not_finished
            (i32.ne
              (i32.atomic.rmw8_u.cmpxchg
                (get_local $id) (i32.const 1) (i32.const -1))
              (i32.const 1)))

          (set_local $workers_done
            (i32.add (get_local $workers_done) (i32.const 1)))
        )

        (br_if $each_worker
          (i32.lt_u
            (tee_local $id (i32.add (get_local $id) (i32.const 1)))
            (i32.const 4)))
      )

      (br_if $not_finished
        (i32.lt_u
          (get_local $workers_done)
          (i32.const 4)))
    )

    (get_local $workers_done)
  )
)

(threads
  (assert_return (invoke "main")  (i32.const 4))
  (invoke "worker" (i32.const 0))
  (invoke "worker" (i32.const 1))
  (invoke "worker" (i32.const 2))
  (invoke "worker" (i32.const 3))
)

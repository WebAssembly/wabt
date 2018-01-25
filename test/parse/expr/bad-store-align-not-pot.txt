;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (memory 1)
  (func
    i32.const 0
    f32.const 0.0
    f32.store align=3))
(;; STDERR ;;;
out/test/parse/expr/bad-store-align-not-pot.txt:8:15: error: alignment must be power-of-two
    f32.store align=3))
              ^^^^^^^
;;; STDERR ;;)

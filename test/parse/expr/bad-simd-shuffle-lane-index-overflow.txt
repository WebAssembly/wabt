;;; TOOL: wat2wasm
;;; ARGS: --enable-simd
;;; ERROR: 1
(module
  (func
    v128.const i32x4 0xff00ff01 0xff00ff0f 0xff00ffff 0xff00ff7f
    v128.const i32x4 0x00550055 0x00550055 0x00550055 0x00550155
    v8x16.shuffle 0x01020304 05060708 0x090a0b0c 0x00000000
    ))

(;; STDERR ;;;
out/test/parse/expr/bad-simd-shuffle-lane-index-overflow.txt:8:19: error: shuffle index "0x01020304" out-of-range [0, 32)
    v8x16.shuffle 0x01020304 05060708 0x090a0b0c 0x00000000
                  ^^^^^^^^^^
;;; STDERR ;;)

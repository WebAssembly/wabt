;;; TOOL: wat2wasm
;;; ARGS: --enable-simd
;;; ERROR: 1
(module
  (func v128.const i8x16 -127 -128 128 255 256 0 0 0 0 0 0 0 0 0 0 0)
  (func v128.const i8x16 -127 -129 128 255 256 0 0 0 0 0 0 0 0 0 0 0)
)
(;; STDERR ;;;
out/test/parse/expr/bad-const-v128-i8x16-overflow.txt:5:44: error: invalid literal "256"
  (func v128.const i8x16 -127 -128 128 255 256 0 0 0 0 0 0 0 0 0 0 0)
                                           ^^^
out/test/parse/expr/bad-const-v128-i8x16-overflow.txt:6:31: error: invalid literal "-129"
  (func v128.const i8x16 -127 -129 128 255 256 0 0 0 0 0 0 0 0 0 0 0)
                              ^^^^
;;; STDERR ;;)

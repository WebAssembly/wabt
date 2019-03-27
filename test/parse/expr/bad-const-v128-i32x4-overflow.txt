;;; TOOL: wat2wasm
;;; ARGS: --enable-simd
;;; ERROR: 1
(module
  (func v128.const i32x4 -2147483648 2147483648 4294967295 4294967296)
  (func v128.const i32x4 -2147483648 -2147483649 4294967295 4294967296)
)
(;; STDERR ;;;
out/test/parse/expr/bad-const-v128-i32x4-overflow.txt:5:60: error: invalid literal "4294967296"
  (func v128.const i32x4 -2147483648 2147483648 4294967295 4294967296)
                                                           ^^^^^^^^^^
out/test/parse/expr/bad-const-v128-i32x4-overflow.txt:6:38: error: invalid literal "-2147483649"
  (func v128.const i32x4 -2147483648 -2147483649 4294967295 4294967296)
                                     ^^^^^^^^^^^
;;; STDERR ;;)

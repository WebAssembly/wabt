;;; TOOL: wat2wasm
;;; ARGS: --enable-simd
;;; ERROR: 1
(module
  (func v128.const i16x8 -32768 -32767 32767 65535 65536 0 0 0)
  (func v128.const i16x8 -32768 -32769 32767 65535 65536 0 0 0)
)
(;; STDERR ;;;
out/test/parse/expr/bad-const-v128-i16x8-overflow.txt:5:52: error: invalid literal "65536"
  (func v128.const i16x8 -32768 -32767 32767 65535 65536 0 0 0)
                                                   ^^^^^
out/test/parse/expr/bad-const-v128-i16x8-overflow.txt:6:33: error: invalid literal "-32769"
  (func v128.const i16x8 -32768 -32769 32767 65535 65536 0 0 0)
                                ^^^^^^
;;; STDERR ;;)

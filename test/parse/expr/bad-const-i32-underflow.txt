;;; TOOL: wat2wasm
;;; ERROR: 1
(module (func i32.const -2147483649))
(;; STDERR ;;;
out/test/parse/expr/bad-const-i32-underflow.txt:3:25: error: invalid literal "-2147483649"
(module (func i32.const -2147483649))
                        ^^^^^^^^^^^
;;; STDERR ;;)

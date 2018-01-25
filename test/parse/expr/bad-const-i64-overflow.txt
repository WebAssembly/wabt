;;; TOOL: wat2wasm
;;; ERROR: 1
(module (func i64.const 18446744073709551616))
(;; STDERR ;;;
out/test/parse/expr/bad-const-i64-overflow.txt:3:25: error: invalid literal "18446744073709551616"
(module (func i64.const 18446744073709551616))
                        ^^^^^^^^^^^^^^^^^^^^
;;; STDERR ;;)

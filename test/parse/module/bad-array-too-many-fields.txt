;;; TOOL: wat2wasm
;;; ARGS: --enable-gc
;;; ERROR: 1
(type (array i32 i32))
(;; STDERR ;;;
out/test/parse/module/bad-array-too-many-fields.txt:4:18: error: unexpected token i32, expected ).
(type (array i32 i32))
                 ^^^
;;; STDERR ;;)

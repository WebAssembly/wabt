;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (import "foo" "bar" (table 0 3 shared anyfunc))
)
(;; STDERR ;;;
out/test/parse/module/bad-import-table-shared.txt:4:4: error: tables may not be shared
  (import "foo" "bar" (table 0 3 shared anyfunc))
   ^^^^^^
;;; STDERR ;;)

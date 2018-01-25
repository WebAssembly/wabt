;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (import "foo" "bar" (global $baz i32))
  (import "oof" "rab" (global $baz i32)))
(;; STDERR ;;;
out/test/parse/module/bad-import-global-redefinition.txt:5:4: error: redefinition of global "$baz"
  (import "oof" "rab" (global $baz i32)))
   ^^^^^^
;;; STDERR ;;)

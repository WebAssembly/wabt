;;; TOOL: wat2wasm
;;; ERROR: 1
(foo bar)
(;; STDERR ;;;
out/test/parse/bad-toplevel.txt:3:2: error: unexpected token "foo", expected a module field or a module.
(foo bar)
 ^^^
out/test/parse/bad-toplevel.txt:3:6: error: unexpected token bar, expected EOF.
(foo bar)
     ^^^
;;; STDERR ;;)

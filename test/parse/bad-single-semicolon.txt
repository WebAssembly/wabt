;;; TOOL: wat2wasm
;;; ERROR: 1
; foo bar
(module)
(;; STDERR ;;;
out/test/parse/bad-single-semicolon.txt:3:1: error: unexpected char
; foo bar
^
out/test/parse/bad-single-semicolon.txt:3:3: error: unexpected token "foo", expected a module field or a module.
; foo bar
  ^^^
out/test/parse/bad-single-semicolon.txt:3:7: error: unexpected token bar, expected EOF.
; foo bar
      ^^^
;;; STDERR ;;)

;;; TOOL: wat2wasm
;;; ERROR: 1
modulez
funcz
(;; STDERR ;;;
out/test/parse/bad-crlf.txt:3:1: error: unexpected token "modulez", expected a module field or a module.
modulez
^^^^^^^
out/test/parse/bad-crlf.txt:4:1: error: unexpected token funcz, expected EOF.
funcz
^^^^^
;;; STDERR ;;)

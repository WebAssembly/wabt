;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (func (@name "some func") (result i32)
    i32.const 42
    return)
  (@custom section)
  (@custom (@nested section))
  (@custom (section) (@with "other") nested-subsections))
(;; STDERR ;;;
out/test/parse/bad-annotations.txt:4:9: error: annotations not enabled: name
  (func (@name "some func") (result i32)
        ^^^^^^
out/test/parse/bad-annotations.txt:4:9: error: unexpected token Invalid, expected ).
  (func (@name "some func") (result i32)
        ^^^^^^
out/test/parse/bad-annotations.txt:7:3: error: annotations not enabled: custom
  (@custom section)
  ^^^^^^^^
;;; STDERR ;;)

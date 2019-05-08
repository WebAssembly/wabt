;;; TOOL: wat2wasm
;;; ARGS: --enable-annotations
(module
  (func (@name "some func") (result i32)
    i32.const 42
    return)
  (@custom section)
  (@custom (@nested section))
  (@custom (section) (@with "other") nested-subsections))

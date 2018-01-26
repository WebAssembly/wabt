;;; TOOL: wat2wasm
(module
  (func
    i32.const 1
    if $exit 
      br $exit
    end))

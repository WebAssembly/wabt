;;; TOOL: wat2wasm
;;; ARGS0: --enable-tail-call
(module
  (type $t (func))
  (table 1 anyfunc)
  (elem (i32.const 0) 0)
  (func $f return_call $f)
  (func i32.const 0 return_call_indirect (type $t))
)

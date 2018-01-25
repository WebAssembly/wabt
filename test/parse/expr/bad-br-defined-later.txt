;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (func
    br 1
    block 
      nop
    end))
(;; STDERR ;;;
out/test/parse/expr/bad-br-defined-later.txt:5:5: error: invalid depth: 1 (max 0)
    br 1
    ^^
;;; STDERR ;;)

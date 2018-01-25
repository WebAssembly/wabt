;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (func
    loop $foo
    end $bar))
(;; STDERR ;;;
out/test/parse/expr/bad-loop-mismatch-label.txt:6:9: error: mismatching label "$foo" != "$bar"
    end $bar))
        ^^^^
;;; STDERR ;;)

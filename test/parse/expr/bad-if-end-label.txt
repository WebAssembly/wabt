;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (func
    i32.const 0
    if
    else $foo
    end $bar))
(;; STDERR ;;;
out/test/parse/expr/bad-if-end-label.txt:7:10: error: unexpected label "$foo"
    else $foo
         ^^^^
out/test/parse/expr/bad-if-end-label.txt:8:9: error: unexpected label "$bar"
    end $bar))
        ^^^^
;;; STDERR ;;)

;;; TOOL: wat2wasm
;;; ERROR: 1
(module
  (func
    i32.const 1
    if $foo
    else $bar
    end $baz))
(;; STDERR ;;;
out/test/parse/expr/bad-if-mismatch-label.txt:7:10: error: mismatching label "$foo" != "$bar"
    else $bar
         ^^^^
out/test/parse/expr/bad-if-mismatch-label.txt:8:9: error: mismatching label "$foo" != "$baz"
    end $baz))
        ^^^^
;;; STDERR ;;)

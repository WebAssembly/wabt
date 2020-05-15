;;; TOOL: wat2wasm
;;; ARGS: --enable-exceptions
;;; ERROR: 1
(module
  ;; flat
  (func
    try
      nop
    catch
      nop
    catch
      nop
    end)

  ;; folded
  (func
    (try
      (do
        (nop))
      (catch
        (nop))
      (catch
        (nop))))
(;; STDERR ;;;
out/test/parse/expr/bad-try-multiple-catch.txt:11:5: error: unexpected token catch, expected end.
    catch
    ^^^^^
out/test/parse/expr/bad-try-multiple-catch.txt:13:5: error: unexpected token end, expected ).
    end)
    ^^^
out/test/parse/expr/bad-try-multiple-catch.txt:22:7: error: unexpected token (, expected ).
      (catch
      ^
out/test/parse/expr/bad-try-multiple-catch.txt:23:16: error: unexpected token ), expected EOF.
        (nop))))
               ^
;;; STDERR ;;)

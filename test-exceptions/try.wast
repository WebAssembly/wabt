;; Test 'try' block


(try (block (nop))
  (catch 1 (nop))
  (catch-all (nop))
)

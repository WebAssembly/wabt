;;; RUN: %(wasm-objdump)s -d third_party/
;;; ERROR: 1
(;; STDERR ;;;
third_party/ is not a regular file.
;;; STDERR ;;)

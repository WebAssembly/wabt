;;; TOOL: wast2json
;;; ERROR: 1
;; syntax is (input name? text)
(input "hello")
(input $var "hello")
(;; STDERR ;;;
out/test/parse/bad-input-command.txt:4:2: error: input command is not supported
(input "hello")
 ^^^^^
out/test/parse/bad-input-command.txt:5:2: error: input command is not supported
(input $var "hello")
 ^^^^^
;;; STDERR ;;)

;;; TOOL: wast2json
;;; ERROR: 1
;; syntax is (output name? text?)
(output)
(output "hello")
(output $var)
(output $var "hello")
(;; STDERR ;;;
out/test/parse/bad-output-command.txt:4:2: error: output command is not supported
(output)
 ^^^^^^
out/test/parse/bad-output-command.txt:5:2: error: output command is not supported
(output "hello")
 ^^^^^^
out/test/parse/bad-output-command.txt:6:2: error: output command is not supported
(output $var)
 ^^^^^^
out/test/parse/bad-output-command.txt:7:2: error: output command is not supported
(output $var "hello")
 ^^^^^^
;;; STDERR ;;)

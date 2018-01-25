;;; TOOL: wat2wasm
;;; ERROR: 1
(module (func (param $n i32) (param $n f32)))
(;; STDERR ;;;
out/test/parse/func/bad-param-redefinition.txt:3:37: error: redefinition of parameter "$n"
(module (func (param $n i32) (param $n f32)))
                                    ^^
;;; STDERR ;;)

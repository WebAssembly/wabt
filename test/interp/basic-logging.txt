;;; TOOL: run-interp
;;; FLAGS: -v
(module
  (func (export "main") (result i32)
    i32.const 42
    return))
(;; STDOUT ;;;
0000000: 0061 736d                                 ; WASM_BINARY_MAGIC
0000004: 0100 0000                                 ; WASM_BINARY_VERSION
; section "Type" (1)
0000008: 01                                        ; section code
0000009: 00                                        ; section size (guess)
000000a: 01                                        ; num types
; type 0
000000b: 60                                        ; func
000000c: 00                                        ; num params
000000d: 01                                        ; num results
000000e: 7f                                        ; i32
0000009: 05                                        ; FIXUP section size
; section "Function" (3)
000000f: 03                                        ; section code
0000010: 00                                        ; section size (guess)
0000011: 01                                        ; num functions
0000012: 00                                        ; function 0 signature index
0000010: 02                                        ; FIXUP section size
; section "Export" (7)
0000013: 07                                        ; section code
0000014: 00                                        ; section size (guess)
0000015: 01                                        ; num exports
0000016: 04                                        ; string length
0000017: 6d61 696e                                main  ; export name
000001b: 00                                        ; export kind
000001c: 00                                        ; export func index
0000014: 08                                        ; FIXUP section size
; section "Code" (10)
000001d: 0a                                        ; section code
000001e: 00                                        ; section size (guess)
000001f: 01                                        ; num functions
; function body 0
0000020: 00                                        ; func body size (guess)
0000021: 00                                        ; local decl count
0000022: 41                                        ; i32.const
0000023: 2a                                        ; i32 literal
0000024: 0f                                        ; return
0000025: 0b                                        ; end
0000020: 05                                        ; FIXUP func body size
000001e: 07                                        ; FIXUP section size
begin_module(1)
begin_signature_section(5)
  on_signature_count(1)
  on_signature(index: 0, params: [], results: [i32])
end_signature_section
begin_function_signatures_section(2)
  on_function_signatures_count(1)
  on_function_signature(index: 0, sig_index: 0)
end_function_signatures_section
begin_export_section(8)
  on_export_count(1)
  on_export(index: 0, kind: func, item_index: 0, name: "main")
end_export_section
begin_function_bodies_section(7)
  on_function_bodies_count(1)
  begin_function_body(0)
  on_local_decl_count(0)
  on_i32_const_expr(42 (0x2a))
  on_return_expr
  end_function_body(0)
end_function_bodies_section
end_module
begin_module(1)
begin_signature_section(5)
  on_signature_count(1)
  on_signature(index: 0, params: [], results: [i32])
end_signature_section
begin_function_signatures_section(2)
  on_function_signatures_count(1)
  on_function_signature(index: 0, sig_index: 0)
end_function_signatures_section
begin_export_section(8)
  on_export_count(1)
  on_export(index: 0, kind: func, item_index: 0, name: "main")
end_export_section
begin_function_bodies_section(7)
  on_function_bodies_count(1)
  begin_function_body(0)
  on_local_decl_count(0)
  on_i32_const_expr(42 (0x2a))
  on_return_expr
  end_function_body(0)
end_function_bodies_section
end_module
   0| i32.const $42
   5| return
   6| return
main() => i32:42
;;; STDOUT ;;)

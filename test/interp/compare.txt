;;; TOOL: run-interp
(module
  ;; i32
  (func (export "i32_eq_true") (result i32)
    i32.const -1
    i32.const -1 
    i32.eq)
  (func (export "i32_eq_false") (result i32) 
    i32.const 1
    i32.const -1
    i32.eq)
  (func (export "i32_ne_true") (result i32)
    i32.const 1
    i32.const -1 
    i32.ne)
  (func (export "i32_ne_false") (result i32)
    i32.const -1
    i32.const -1 
    i32.ne)
  (func (export "i32_lt_s_less") (result i32)
    i32.const -1
    i32.const 1 
    i32.lt_s)
  (func (export "i32_lt_s_equal") (result i32)
    i32.const -1
    i32.const -1 
    i32.lt_s)
  (func (export "i32_lt_s_greater") (result i32) 
    i32.const 1
    i32.const -1
    i32.lt_s)
  (func (export "i32_lt_u_less") (result i32)
    i32.const 1
    i32.const -1 
    i32.lt_u )
  (func (export "i32_lt_u_equal") (result i32)
    i32.const 1
    i32.const 1 
    i32.lt_u)
  (func (export "i32_lt_u_greater") (result i32)
    i32.const -1 
    i32.const 1
    i32.lt_u)
  (func (export "i32_le_s_less") (result i32)
    i32.const -1
    i32.const 1 
    i32.le_s)

  (func (export "i32_le_s_equal") (result i32)
    i32.const -1
    i32.const -1
    i32.le_s)
  (func (export "i32_le_s_greater") (result i32)
    i32.const 1 
    i32.const -1
    i32.le_s)

  (func (export "i32_le_u_less") (result i32)
    i32.const 1
    i32.const -1 
    i32.le_u)
  (func (export "i32_le_u_equal") (result i32)
    i32.const 1
    i32.const 1 
    i32.le_u)
  (func (export "i32_le_u_greater") (result i32) 
    i32.const -1
    i32.const 1
    i32.le_u)

  (func (export "i32_gt_s_less") (result i32)
    i32.const -1
    i32.const 1 
    i32.gt_s)
  (func (export "i32_gt_s_equal") (result i32)
    i32.const -1
    i32.const -1
    i32.gt_s)
  (func (export "i32_gt_s_greater") (result i32)
    i32.const 1 
    i32.const -1
    i32.gt_s)

  (func (export "i32_gt_u_less") (result i32)
    i32.const 1
    i32.const -1 
    i32.gt_u)
  (func (export "i32_gt_u_equal") (result i32)
    i32.const 1
    i32.const 1 
    i32.gt_u)
  (func (export "i32_gt_u_greater") (result i32)
    i32.const -1
    i32.const 1 
    i32.gt_u)

  (func (export "i32_ge_s_less") (result i32)
    i32.const -1
    i32.const 1 
    i32.ge_s)
  (func (export "i32_ge_s_equal") (result i32)
    i32.const -1
    i32.const -1 
    i32.ge_s)
  (func (export "i32_ge_s_greater") (result i32)
    i32.const 1
    i32.const -1
    i32.ge_s)

  (func (export "i32_ge_u_less") (result i32)
    i32.const 1 
    i32.const -1
    i32.ge_u)
  (func (export "i32_ge_u_equal") (result i32)
    i32.const 1
    i32.const 1 
    i32.ge_u)
  (func (export "i32_ge_u_greater") (result i32)
    i32.const -1
    i32.const 1 
    i32.ge_u)

  ;; i64
  (func (export "i64_eq_true") (result i32)
    i64.const -1 
    i64.const -1
    i64.eq)
  (func (export "i64_eq_false") (result i32)
    i64.const 1
    i64.const -1 
    i64.eq)

  (func (export "i64_ne_true") (result i32)
    i64.const 1
    i64.const -1 
    i64.ne)
  (func (export "i64_ne_false") (result i32)
    i64.const -1
    i64.const -1 
    i64.ne)

  (func (export "i64_lt_s_less") (result i32)
    i64.const -1
    i64.const 1 
    i64.lt_s)
  (func (export "i64_lt_s_equal") (result i32)
    i64.const -1
    i64.const -1 
    i64.lt_s)
  (func (export "i64_lt_s_greater") (result i32)
    i64.const 1
    i64.const -1 
    i64.lt_s)

  (func (export "i64_lt_u_less") (result i32)
    i64.const 1 
    i64.const -1
    i64.lt_u)
  (func (export "i64_lt_u_equal") (result i32)
    i64.const 1
    i64.const 1 
    i64.lt_u)
  (func (export "i64_lt_u_greater") (result i32)
    i64.const -1
    i64.const 1 
    i64.lt_u)

  (func (export "i64_le_s_less") (result i32)
    i64.const -1
    i64.const 1 
    i64.le_s)
  (func (export "i64_le_s_equal") (result i32)
    i64.const -1
    i64.const -1 
    i64.le_s)
  (func (export "i64_le_s_greater") (result i32)
    i64.const 1
    i64.const -1 
    i64.le_s)

  (func (export "i64_le_u_less") (result i32)
    i64.const 1
    i64.const -1 
    i64.le_u)
  (func (export "i64_le_u_equal") (result i32)
    i64.const 1
    i64.const 1 
    i64.le_u)
  (func (export "i64_le_u_greater") (result i32)
    i64.const -1 
    i64.const 1
    i64.le_u)

  (func (export "i64_gt_s_less") (result i32)
    i64.const -1 
    i64.const 1
    i64.gt_s)
  (func (export "i64_gt_s_equal") (result i32)
    i64.const -1 
    i64.const -1
    i64.gt_s)
  (func (export "i64_gt_s_greater") (result i32)
    i64.const 1
    i64.const -1 
    i64.gt_s)

  (func (export "i64_gt_u_less") (result i32)
    i64.const 1
    i64.const -1 
    i64.gt_u)
  (func (export "i64_gt_u_equal") (result i32)
    i64.const 1 
    i64.const 1
    i64.gt_u)
  (func (export "i64_gt_u_greater") (result i32)
    i64.const -1
    i64.const 1 
    i64.gt_u)

  (func (export "i64_ge_s_less") (result i32)
    i64.const -1
    i64.const 1 
    i64.ge_s)
  (func (export "i64_ge_s_equal") (result i32)
    i64.const -1
    i64.const -1 
    i64.ge_s)
  (func (export "i64_ge_s_greater") (result i32)
    i64.const 1
    i64.const -1 
    i64.ge_s)

  (func (export "i64_ge_u_less") (result i32) 
    i64.const 1
    i64.const -1
    i64.ge_u)
  (func (export "i64_ge_u_equal") (result i32)
    i64.const 1
    i64.const 1 
    i64.ge_u)
  (func (export "i64_ge_u_greater") (result i32) 
     i64.const -1
     i64.const 1
     i64.ge_u)

  ;; f32
  (func (export "f32_eq_true") (result i32)
    f32.const -1
    f32.const -1 
    f32.eq)
  (func (export "f32_eq_false") (result i32)
    f32.const 1 
    f32.const -1
    f32.eq)

  (func (export "f32_ne_true") (result i32)
    f32.const 1
    f32.const -1 
    f32.ne)
  (func (export "f32_ne_false") (result i32)
    f32.const -1
    f32.const -1 
    f32.ne)

  (func (export "f32_lt_less") (result i32)
    f32.const -1
    f32.const 1 
    f32.lt)
  (func (export "f32_lt_equal") (result i32)
    f32.const -1
    f32.const -1 
    f32.lt)
  (func (export "f32_lt_greater") (result i32)
    f32.const 1
    f32.const -1 
    f32.lt)

  (func (export "f32_le_less") (result i32) 
    f32.const -1
    f32.const 1
    f32.le)
  (func (export "f32_le_equal") (result i32)
    f32.const -1
    f32.const -1 
    f32.le)
  (func (export "f32_le_greater") (result i32)
    f32.const 1
    f32.const -1 
    f32.le)

  (func (export "f32_gt_less") (result i32)
    f32.const -1
    f32.const 1 
    f32.gt)
  (func (export "f32_gt_equal") (result i32)
    f32.const -1
    f32.const -1 
    f32.gt)
  (func (export "f32_gt_greater") (result i32)
    f32.const 1
    f32.const -1 
    f32.gt)

  (func (export "f32_ge_less") (result i32)
    f32.const -1
    f32.const 1 
    f32.ge)
  (func (export "f32_ge_equal") (result i32)
    f32.const -1 
    f32.const -1
    f32.ge)
  (func (export "f32_ge_greater") (result i32)
    f32.const 1
    f32.const -1 
    f32.ge)

  ;; f64
  (func (export "f64_eq_true") (result i32) 
    f64.const -1
    f64.const -1
    f64.eq)
  (func (export "f64_eq_false") (result i32)
    f64.const 1 
    f64.const -1
    f64.eq)

  (func (export "f64_ne_true") (result i32)
    f64.const 1
    f64.const -1 
    f64.ne)
  (func (export "f64_ne_false") (result i32)
    f64.const -1
    f64.const -1 
    f64.ne)

  (func (export "f64_lt_less") (result i32)
    f64.const -1
    f64.const 1 
    f64.lt)
  (func (export "f64_lt_equal") (result i32)
    f64.const -1
    f64.const -1 
    f64.lt)
  (func (export "f64_lt_greater") (result i32)
    f64.const 1
    f64.const -1 
    f64.lt)

  (func (export "f64_le_less") (result i32)
    f64.const -1
    f64.const 1 
    f64.le)
  (func (export "f64_le_equal") (result i32) 
    f64.const -1
    f64.const -1
    f64.le)
  (func (export "f64_le_greater") (result i32)
    f64.const 1
    f64.const -1 
    f64.le)

  (func (export "f64_gt_less") (result i32)
    f64.const -1 
    f64.const 1
    f64.gt)
  (func (export "f64_gt_equal") (result i32)
    f64.const -1 
    f64.const -1
    f64.gt)
  (func (export "f64_gt_greater") (result i32)
    f64.const 1
    f64.const -1 
    f64.gt)

  (func (export "f64_ge_less") (result i32)
    f64.const -1
    f64.const 1 
    f64.ge)
  (func (export "f64_ge_equal") (result i32)
    f64.const -1
    f64.const -1 
    f64.ge)
  (func (export "f64_ge_greater") (result i32)
    f64.const 1
    f64.const -1 
    f64.ge)
)
(;; STDOUT ;;;
i32_eq_true() => i32:1
i32_eq_false() => i32:0
i32_ne_true() => i32:1
i32_ne_false() => i32:0
i32_lt_s_less() => i32:1
i32_lt_s_equal() => i32:0
i32_lt_s_greater() => i32:0
i32_lt_u_less() => i32:1
i32_lt_u_equal() => i32:0
i32_lt_u_greater() => i32:0
i32_le_s_less() => i32:1
i32_le_s_equal() => i32:1
i32_le_s_greater() => i32:0
i32_le_u_less() => i32:1
i32_le_u_equal() => i32:1
i32_le_u_greater() => i32:0
i32_gt_s_less() => i32:0
i32_gt_s_equal() => i32:0
i32_gt_s_greater() => i32:1
i32_gt_u_less() => i32:0
i32_gt_u_equal() => i32:0
i32_gt_u_greater() => i32:1
i32_ge_s_less() => i32:0
i32_ge_s_equal() => i32:1
i32_ge_s_greater() => i32:1
i32_ge_u_less() => i32:0
i32_ge_u_equal() => i32:1
i32_ge_u_greater() => i32:1
i64_eq_true() => i32:1
i64_eq_false() => i32:0
i64_ne_true() => i32:1
i64_ne_false() => i32:0
i64_lt_s_less() => i32:1
i64_lt_s_equal() => i32:0
i64_lt_s_greater() => i32:0
i64_lt_u_less() => i32:1
i64_lt_u_equal() => i32:0
i64_lt_u_greater() => i32:0
i64_le_s_less() => i32:1
i64_le_s_equal() => i32:1
i64_le_s_greater() => i32:0
i64_le_u_less() => i32:1
i64_le_u_equal() => i32:1
i64_le_u_greater() => i32:0
i64_gt_s_less() => i32:0
i64_gt_s_equal() => i32:0
i64_gt_s_greater() => i32:1
i64_gt_u_less() => i32:0
i64_gt_u_equal() => i32:0
i64_gt_u_greater() => i32:1
i64_ge_s_less() => i32:0
i64_ge_s_equal() => i32:1
i64_ge_s_greater() => i32:1
i64_ge_u_less() => i32:0
i64_ge_u_equal() => i32:1
i64_ge_u_greater() => i32:1
f32_eq_true() => i32:1
f32_eq_false() => i32:0
f32_ne_true() => i32:1
f32_ne_false() => i32:0
f32_lt_less() => i32:1
f32_lt_equal() => i32:0
f32_lt_greater() => i32:0
f32_le_less() => i32:1
f32_le_equal() => i32:1
f32_le_greater() => i32:0
f32_gt_less() => i32:0
f32_gt_equal() => i32:0
f32_gt_greater() => i32:1
f32_ge_less() => i32:0
f32_ge_equal() => i32:1
f32_ge_greater() => i32:1
f64_eq_true() => i32:1
f64_eq_false() => i32:0
f64_ne_true() => i32:1
f64_ne_false() => i32:0
f64_lt_less() => i32:1
f64_lt_equal() => i32:0
f64_lt_greater() => i32:0
f64_le_less() => i32:1
f64_le_equal() => i32:1
f64_le_greater() => i32:0
f64_gt_less() => i32:0
f64_gt_equal() => i32:0
f64_gt_greater() => i32:1
f64_ge_less() => i32:0
f64_ge_equal() => i32:1
f64_ge_greater() => i32:1
;;; STDOUT ;;)

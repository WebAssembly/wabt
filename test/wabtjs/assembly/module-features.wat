(module
 (type $v128_v128_=>_v128 (func (param v128 v128) (result v128)))
 (memory $0 0)
 (table $0 1 funcref)
 (export "memory" (memory $0))
 (export "add" (func $tests/assembly/module-features/add))
 (func $tests/assembly/module-features/add (; 0 ;) (param $0 v128) (param $1 v128) (result v128)
  local.get $0
  local.get $1
  i32x4.add
 )
)

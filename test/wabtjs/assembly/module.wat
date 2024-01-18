(module
 (type $i32_i32_=>_i32 (func (param i32 i32) (result i32)))
 (memory $0 0)
 (table $0 1 funcref)
 (export "memory" (memory $0))
 (export "add" (func $tests/assembly/module/add))
 (func $tests/assembly/module/add (; 0 ;) (param $0 i32) (param $1 i32) (result i32)
  local.get $0
  local.get $1
  i32.add
 )
)

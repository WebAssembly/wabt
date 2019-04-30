/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

var examples = [
  {
    name: 'empty',
    contents: '(module)',
    js: '',
  },

  {
    name: 'simple',
    contents:
`(module
  (func (export "addTwo") (param i32 i32) (result i32)
    local.get 0
    local.get 1
    i32.add))
`,
    js:
`const wasmInstance =
      new WebAssembly.Instance(wasmModule, {});
const { addTwo } = wasmInstance.exports;
for (let i = 0; i < 10; i++) {
  console.log(addTwo(i, i));
}
`,
  },

  {
    name: 'factorial',
    contents:
`(module
  (func $fac (param f64) (result f64)
    local.get 0
    f64.const 1
    f64.lt
    if (result f64)
      f64.const 1
    else
      local.get 0
      local.get 0
      f64.const 1
      f64.sub
      call $fac
      f64.mul
    end)
	(export "fac" (func $fac)))
`,
    js: `const wasmInstance =
    new WebAssembly.Instance(wasmModule, {});
const { fac } = wasmInstance.exports;
for (let i = 1; i <= 15; i++) {
  console.log(fac(i));
}
`,
  },

  {
    name: 'stuff',
    contents:
`(module
  (import "foo" "bar" (func (param f32)))
  (memory (data "hi"))
  (type (func (param i32) (result i32)))
  (start 1)
  (table 0 1 anyfunc)
  (func)
  (func (type 1)
    i32.const 42
    drop)
  (export "e" (func 1)))
`,
    js: `const wasmInstance = new WebAssembly.Instance(wasmModule, {
  foo: {
    bar() {}
  },
});
`,
  },

  {
    name: 'mutable globals',
    contents:
`(module
  (import "env" "g" (global (mut i32)))
  (func (export "f")
    i32.const 100
    global.set 0))
`,
    js:
`
  const g = new WebAssembly.Global({value: 'i32', mutable: true});
  const wasmInstance = new WebAssembly.Instance(wasmModule, {env: {g}});
  console.log('before: ' + g.value);
  wasmInstance.exports.f();
  console.log('after: ' + g.value);
`
  },

  {
    name: "saturating float-to-int",
    contents:
`(module
  (func (export "f") (param f32) (result i32)
    local.get 0
    i32.trunc_sat_f32_s))`,
    js:
`const wasmInstance = new WebAssembly.Instance(wasmModule);
const {f} = wasmInstance.exports;
console.log(f(Infinity));`
  },

  {
    name: "sign extension",
    contents:
`(module
  (func (export "f") (param i32) (result i32)
    local.get 0
    i32.extend8_s))
`,
    js:
`const wasmInstance = new WebAssembly.Instance(wasmModule);
const {f} = wasmInstance.exports;
console.log(f(0));
console.log(f(127));
console.log(f(128));
console.log(f(255));`
  },

  {
    name: "multi value",
    contents:
`(module
  (func $swap (param i32 i32) (result i32 i32)
    local.get 1
    local.get 0)

  (func (export "reverseSub") (param i32 i32) (result i32)
    local.get 0
    local.get 1
    call $swap
    i32.sub))
`,
    js:
`const wasmInstance = new WebAssembly.Instance(wasmModule);
const {reverseSub} = wasmInstance.exports;
console.log(reverseSub(10, 3));`
  },

  {
    name: "bulk memory",
    contents:
`(module
  (memory (export "mem") 1)
  (func (export "fill") (param i32 i32 i32)
    local.get 0
    local.get 1
    local.get 2
    memory.fill))
`,
    js:
`const wasmInstance = new WebAssembly.Instance(wasmModule);
const {fill, mem} = wasmInstance.exports;
fill(0, 13, 5);
fill(10, 77, 7);
fill(20, 255, 1000);
console.log(new Uint8Array(mem.buffer, 0, 50));
`
  }
];

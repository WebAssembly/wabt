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
  (func $addTwo (param i32 i32) (result i32)
    get_local 0
    get_local 1
    i32.add)
  (export "addTwo" (func $addTwo)))
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
    get_local 0
    f64.const 1
    f64.lt
    if (result f64)
      f64.const 1
    else
      get_local 0
      get_local 0
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
    js: `var wasmInstance = new WebAssembly.Instance(wasmModule, {
  foo: {
    bar() {}
  },
});
`,
  }
];

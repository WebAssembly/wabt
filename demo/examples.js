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
    contents: '(module)'
  },

  {
    name: 'simple',
    contents:
      '(module\n' +
      '  (func $addTwo (param i32 i32) (result i32)\n' +
      '    get_local 0\n' +
      '    get_local 1\n' +
      '    i32.add)\n' +
      '  (export "addTwo" (func $addTwo)))\n'
  },

  {
    name: 'factorial',
    contents:
      '(module\n' +
      '  (func $fac (param i64) (result i64)\n' +
      '    get_local 0\n' +
      '    i64.const 1\n' +
      '    i64.lt_s\n' +
      '    if (result i64)\n' +
      '      i64.const 1\n' +
      '    else\n' +
      '      get_local 0\n' +
      '      get_local 0\n' +
      '      i64.const 1\n' +
      '      i64.sub\n' +
      '      call $fac\n' +
      '      i64.mul\n' +
      '    end)\n' +
      '  (export "fac" (func $fac)))\n'
  },

  {
    name: 'stuff',
    contents:
      '(module\n' +
      '  (import "foo" "bar" (func (param f32)))\n' +
      '  (memory (data "hi"))\n' +
      '  (type (func (param i32) (result i32)))\n' +
      '  (start 1)\n' +
      '  (table 0 1 anyfunc)\n' +
      '  (func)\n' +
      '  (func (type 0)\n' +
      '    i32.const 42\n' +
      '    drop)\n' +
      '  (export "e" (func 1)))\n'
  }
];

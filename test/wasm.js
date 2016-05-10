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

/* polyfill from SM to D8 */
if (typeof arguments == 'undefined') {
  arguments = scriptArgs;
}

if (typeof readbuffer == 'undefined') {
  readbuffer = function(path) { return read(path, 'binary'); };
}

if (arguments.length != 1) {
  print('usage: <exe> wasm.js -- <filename.wasm>');
  print('got: ', arguments);
  quit(0);
}

var ffi = {
  stdio: {print: print},
  test: {has_memory: function() { return typeof module.memory !== undefined; }},
};
var buffer = readbuffer(arguments[0]);
var module = Wasm.instantiateModule(buffer, ffi);

var name;
var f;
var result;

if (typeof module.exports === 'object') {
  var exportedFunctions = Object.getOwnPropertyNames(module.exports);

  for (name of exportedFunctions) {
    f = module.exports[name];
    if (typeof f !== 'function')
      continue;

    if (name.lastIndexOf('trap', 0) === 0) {
      try {
        result = f();
        print('Expected ' + name + '() to trap, instead got: ' + result);
      } catch (e) {
        print(name + '() trapped: ' + e.toString());
      }
    } else {
      print(name + '() = ' + f());
    }
  }
}

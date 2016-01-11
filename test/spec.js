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

var quiet; /* global, defined by output from sexpr-wasm */
var passed = 0;
var failed = 0;

function wasmWrite(memory, offset, count) {
  try {
    var view = new Uint8Array(memory, offset, count);
  } catch (exc) {
    print("wasmWrite(" + offset + ", " + count + ") failed on heap of size " +
          memory.byteLength);
    throw exc;
  }
  var str = "";

  for (var i = 0; i < count; i++)
    str += String.fromCharCode(view[i]);

  write(str);
}

function createModule(a) {
  var memory = null;
  var u8a = new Uint8Array(a);
  var ffi = {
    print: print,
    write: function(offset, count) { wasmWrite(memory, offset, count); }
  };
  var module = _WASMEXP_.instantiateModule(u8a.buffer, ffi);
  memory = module.memory;
  return module;
}

function assertReturn(m, name, file, line) {
  try {
    var result = m[name]();
  } catch(e) {
    print(file + ":" + line + ": " + name + " unexpectedly threw: " + e);
  }

  if (result == 1) {
    passed++;
  } else {
    print(file + ":" + line + ": " + name + " failed.");
    failed++;
  }
}

function assertTrap(m, name, file, line) {
  var threw = false;
  try {
    m[name]();
  } catch (e) {
    threw = true;
  }

  if (threw) {
    passed++;
  } else {
    print(file + ":" + line + ": " + name + " failed, didn't throw");
    failed++;
  }
}

function invoke(m, name) {
  try {
    var invokeResult = m[name]();
  } catch(e) {
    print(file + ":" + line + ": " + name + " unexpectedly threw: " + e);
  }

  if (!quiet)
    print(name + " = " + invokeResult);
}

function end() {
  if ((failed > 0) || !quiet)
    print(passed + "/" + (passed + failed) + " tests passed.");
}

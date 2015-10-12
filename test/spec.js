function wasm_write(memory, offset, count) {
  try {
    var view = new Uint8Array(memory, offset, count);
  } catch (exc) {
    print("wasm_write(" + offset + ", " + count + ") failed on heap of size " + memory.byteLength);
    throw exc;
  }
  var str = "";

  for (var i = 0; i < count; i++)
    str += String.fromCharCode(view[i]);

  write(str);
}

function createModule(a, quiet) {
  var memory = null;
  var u8a = new Uint8Array(a);
  var ffi = {
    print: print, 
    write: function (offset, count) { wasm_write(memory, offset, count); }
  };
  if (!quiet)
    print('instantiating module');
  var module = WASM.instantiateModule(u8a.buffer, ffi);
  memory = module.memory;
  return module;
}

function startsWith(s, prefix) {
  return s.lastIndexOf(prefix, 0) == 0;
}

function runTests(module, quiet) {
  var passed = 0;
  var failed = 0;
  for (var name in module) {
    if (module[name] instanceof Function) {
      var f = module[name];
      if (startsWith(name, "$assert_return")) {
        var result = f();
        if (result == 1) {
          if (!quiet)
            print(name + " OK");

          passed++;
        } else {
          print(name + " failed, expected 1, got " + result);
          failed++;
        }
      } else if (startsWith(name, '$assert_trap')) {
        var threw = false;
        try {
          f();
        } catch (e) {
          threw = true;
        }

        if (threw) {
          if (!quiet)
            print(name + " OK, threw");

          passed++;
        } else {
          print(name + " failed, didn't throw");
          failed++;
        }
      } else if (startsWith(name, '$invoke')) {
        var invokeResult = f();

        if (!quiet)
          print(name + " = " + invokeResult);
      }
    }
  }

  if ((failed > 0) || !quiet)
    print(passed + "/" + (passed + failed) + " tests passed.");
}

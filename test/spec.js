function createModule(a) {
  var ffi = {print: print};
  var u8a = new Uint8Array(a);
  print('instantiating module');
  var module = WASM.instantiateModule(u8a.buffer, ffi);
  return module;
}

function startsWith(s, prefix) {
  return s.lastIndexOf(prefix, 0) == 0;
}

function runTests(module) {
  var passed = 0;
  var failed = 0;
  for (var name in module) {
    if (module[name] instanceof Function) {
      var f = module[name];
      if (startsWith(name, "$assert_return")) {
        var result = f();
        if (result == 1) {
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
          print(name + " OK, threw");
          passed++;
        } else {
          print(name + " failed, didn't throw");
          failed++;
        }
      } else if (startsWith(name, '$invoke')) {
        print(name + " = " + f());
      }
    }
  }
  print(passed + "/" + (passed + failed) + " tests passed.");
}

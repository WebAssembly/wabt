if (arguments.length != 1) {
  print('usage: d8 wasm.js -- <filename>');
  quit(0);
}

var ffi = { print: print };
var buffer = readbuffer(arguments[0]);
var module = WASM.instantiateModule(buffer, ffi);

var name;
var f;
var result;

for (name in module) {
  f = module[name];
  if (!(f instanceof Function))
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

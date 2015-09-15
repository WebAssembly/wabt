if (arguments.length != 1) {
  print('usage: d8 wasm.js -- <filename>');
  quit(0);
}

var ffi = { print: print };
var buffer = readbuffer(arguments[0]);
var module = WASM.instantiateModule(buffer, ffi);
for (var name in module) {
  if (module[name] instanceof Function) {
    print(name + '() = ' + module[name]());
  }
}

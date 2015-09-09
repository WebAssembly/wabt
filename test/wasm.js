if (arguments.length != 1) {
  print('usage: d8 wasm.js -- <filename>');
  quit(0);
}

var buffer = readbuffer(arguments[0]);
var module = WASM.instantiateModule(buffer, {});
print(module.main());

if (arguments.length != 1) {
  print('usage: d8 wasm.js -- <filename>');
  quit(0);
}

var data = read(arguments[0]);
var buffer = new ArrayBuffer(data.length);
var u8a = new Uint8Array(buffer);
for (var i = 0; i < data.length; ++i) {
  u8a[i] = data.charCodeAt(i);
}
var module = WASM.instantiateModule(buffer, {});
print(module.main());

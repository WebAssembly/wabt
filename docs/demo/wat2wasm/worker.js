// This worker runs the generated WebAssembly module and sends console.log
// output back to the main thread in order to prevent infinite loops from
// locking up the main thread.
let wrappedConsole = Object.create(console);

wrappedConsole.log = (...args) => {
  const line = args.map(String).join('') + '\n';
  postMessage({
    type: "log",
    data: line
  });
  console.log(...args);
}

self.onmessage = async function (event) {
  console.log("Running WebAssembly");
  const { binaryBuffer, js } = event.data;
  let wasm;
  try {
    wasm = new WebAssembly.Module(binaryBuffer);
  } catch (e) {
    postMessage({
      type: "log",
      data: String(e)
    });
  }
  const fn = new Function('wasmModule', 'console', js + '//# sourceURL=demo.js');
  fn(wasm, wrappedConsole);
  postMessage({
    type: "done"
  });
}

import { readFile } from 'fs/promises';
import { WABT, WasmFeature } from './input.js';

let contents = await readFile(new URL('../out/wasi/Debug/libwabt.wasm', import.meta.url));
let wabt = new WABT();
await wabt.instantiate(contents, {
    "wasi_snapshot_preview1": {
        "fd_close": function() {throw new Error("fd_close")},
        "fd_seek": function() {throw new Error("fd_seek")},
        "fd_write": function() {throw new Error("fd_write")},
        "environ_get": function() {throw new Error("environ_get")},
        "environ_sizes_get": function() {throw new Error("environ_sizes_get")},
        "fd_fdstat_get": function() {throw new Error("fd_fdstat_get")},
        "proc_exit": function() {throw new Error("proc_exit")},
    }
});

function wat2wasm(contents) {
    console.log(`trying ${contents}`);
    try {
        let result = wabt.wat2wasm(contents, WasmFeature.MutableGlobals);
        console.log(result)    }
    catch (e) {console.log("error", e)}
}

function wasm2wat(binary) {
    console.log(`trying ${binary}`);
    try {
        let result = wabt.wasm2wat(binary, WasmFeature.MutableGlobals);
        console.log(result)
    }
    catch (e) {console.log("error", e)}
}


wat2wasm("(module)");
wat2wasm("xyz");
wasm2wat(new Uint8Array([
    0, 97, 115, 109,   1,   0,   0,   0,   0,
    8,  4, 110,  97, 109, 101,   2,   1,   0,
    0,  9,   7, 108, 105, 110, 107, 105, 110,
  103,  2
]))

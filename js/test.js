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

function letDo(contents) {
    console.log(`trying ${contents}`);
    try {
        let result = wabt.wat2wasm(contents, WasmFeature.MutableGlobals);
        console.log(result)    }
    catch (e) {console.log("error", e)}
}

letDo("(module)");
letDo("xyz");

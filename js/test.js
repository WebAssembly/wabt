import { readFile } from 'fs/promises';
import { WABT, WASM_FEATURE_MUTABLE_GLOBALS } from './input.js';

let contents = await readFile(new URL('../out/wasi/Debug/libwabt.wasm', import.meta.url));
let wabt = new WABT();
await wabt.instantiate(contents, {
    "wasi_snapshot_preview1": {
        "fd_close": function() {throw new Error("error1")},
        "fd_seek": function() {throw new Error("error2")},
        "fd_write": function() {throw new Error("error3")},
    }
});

function letDo(contents) {
    console.log(`trying ${contents}`);
    try {
        let result = wabt.wat2wasm(contents, WASM_FEATURE_MUTABLE_GLOBALS);
        console.log(result)    }
    catch (e) {console.log("error", e)}
}

letDo("(module)");
letDo("(modulex)");
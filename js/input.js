import { data_view, validate_flags, utf8_encode, UTF8_ENCODED_LEN } from './intrinsics.js';
export const WASM_FEATURE_EXCEPTIONS = 1;
export const WASM_FEATURE_MUTABLE_GLOBALS = 2;
export const Errno = Object.freeze({
    0: "Base",
    "Base": 0,
    1: "BaseX",
  "BaseX": 1,
});
export class WABT {
  addToImports(imports) {
  }
  
  async instantiate(module, imports) {
    imports = imports || {};
    this.addToImports(imports);
    
    if (module instanceof WebAssembly.Instance) {
      this.instance = module;
    } else if (module instanceof WebAssembly.Module) {
      this.instance = await WebAssembly.instantiate(module, imports);
    } else if (module instanceof ArrayBuffer || module instanceof Uint8Array) {
      const { instance } = await WebAssembly.instantiate(module, imports);
      this.instance = instance;
    } else {
      const { instance } = await WebAssembly.instantiateStreaming(module, imports);
      this.instance = instance;
    }
    this._exports = this.instance.exports;
    this._exports._initialize();
  }
  wat2wasm(arg0, arg1) {
    const memory = this._exports.memory;
    const realloc = this._exports["canonical_abi_realloc"];
    const free = this._exports["canonical_abi_free"];
    const ptr0 = utf8_encode(arg0, realloc, memory);
    const len0 = UTF8_ENCODED_LEN;
    const ret = this._exports['wat2wasm'](ptr0, len0, validate_flags(arg1, 3));
    let variant3;
    switch (data_view(memory).getInt32(ret + 0, true)) {
      case 0: {
        const ptr1 = data_view(memory).getInt32(ret + 8, true);
        const len1 = data_view(memory).getInt32(ret + 16, true);
        const list1 = new Uint8Array(memory.buffer.slice(ptr1, ptr1 + len1 * 1));
        free(ptr1, len1, 1);
        variant3 = {
          tag: "ok",
          val: list1,
        };
        break;
      }
      case 1: {
        const tag2 = data_view(memory).getInt32(ret + 8, true);
        if (!(tag2 in Errno))
        throw new RangeError("invalid discriminant specified for Errno");
        variant3 = {
          tag: "err",
          val: tag2,
        };
        break;
      }
      default:
      throw new RangeError("invalid variant discriminant for expected");
    }
    return variant3;
  }
}

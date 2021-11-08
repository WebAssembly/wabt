import { data_view, validate_flags, utf8_encode, UTF8_ENCODED_LEN, UTF8_DECODER, UnexpectedError } from './intrinsics.js';
export const WASM_FEATURE_EXCEPTIONS = 1;
export const WASM_FEATURE_MUTABLE_GLOBALS = 2;
export const WASM_FEATURE_SAT_FLOAT_TO_INT = 4;
export const WASM_FEATURE_SIGN_EXTENSION = 8;
export const WASM_FEATURE_SIMD = 16;
export const WASM_FEATURE_THREADS = 32;
export const WASM_FEATURE_MULTI_VALUE = 64;
export const WASM_FEATURE_TAIL_CALL = 128;
export const WASM_FEATURE_BULK_MEMORY = 256;
export const WASM_FEATURE_REFERENCE_TYPES = 512;
export const WASM_FEATURE_ANNOTATIONS = 1024;
export const WASM_FEATURE_GC = 2048;
export const WasmFeature = {
    Exceptions:1,
    MutableGlobals:2,
    SatFloatToInt:4,
    SignExtension:8,
    SIMD:16,
    Threads:32,
    MultiValue:64,
    TailCall:128,
    BulkMemory:256,
    ReferenceTypes:512,
    Annotations:1024,
    GC:2048,
};

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
    switch (data_view(memory).getInt32(ret + 0, true)) {
      case 0: {
        const ptr1 = data_view(memory).getInt32(ret + 8, true);
        const len1 = data_view(memory).getInt32(ret + 16, true);
        const list1 = new Uint8Array(memory.buffer.slice(ptr1, ptr1 + len1 * 1));
        free(ptr1, len1, 1);
        return list1;
      }
      case 1: {
        const ptr2 = data_view(memory).getInt32(ret + 8, true);
        const len2 = data_view(memory).getInt32(ret + 16, true);
        const list2 = UTF8_DECODER.decode(new Uint8Array(memory.buffer, ptr2, len2));
        free(ptr2, len2, 1);
        throw new UnexpectedError(list2);
      }
      default:
      throw new RangeError("invalid variant discriminant for expected");
    }
  }
  wasm2wat(arg0, arg1) {
    const memory = this._exports.memory;
    const realloc = this._exports["canonical_abi_realloc"];
    const free = this._exports["canonical_abi_free"];
    const val0 = arg0;
    const len0 = val0.length;
    const ptr0 = realloc(0, 0, 1, len0 * 1);
    (new Uint8Array(memory.buffer, ptr0, len0 * 1)).set(new Uint8Array(val0.buffer));
    const ret = this._exports['wasm2wat'](ptr0, len0, validate_flags(arg1, 4095));
    let variant3;
    switch (data_view(memory).getInt32(ret + 0, true)) {
      case 0: {
        const ptr1 = data_view(memory).getInt32(ret + 8, true);
        const len1 = data_view(memory).getInt32(ret + 16, true);
        const list1 = UTF8_DECODER.decode(new Uint8Array(memory.buffer, ptr1, len1));
        free(ptr1, len1, 1);
        return list1;
      }
      case 1: {
        const ptr2 = data_view(memory).getInt32(ret + 8, true);
        const len2 = data_view(memory).getInt32(ret + 16, true);
        const list2 = UTF8_DECODER.decode(new Uint8Array(memory.buffer, ptr2, len2));
        free(ptr2, len2, 1);
        throw new UnexpectedError(list2);
      }
      default:
      throw new RangeError("invalid variant discriminant for expected");
    }
    return variant3;
  }

}

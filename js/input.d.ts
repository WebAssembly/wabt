export type Result<T, E> = { tag: "ok", val: T } | { tag: "err", val: E };
export type WasmFeature = number;
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
export enum WasmFeature {
    Exceptions=1,
    MutableGlobals=2,
    SatFloatToInt=4,
    SignExtension=8,
    SIMD=16,
    Threads=32,
    MultiValue=64,
    TailCall=128,
    BulkMemory=256,
    ReferenceTypes=512,
    Annotations=1024,
    GC=2048,
}
export class Input {
  
  // The WebAssembly instance that this class is operating with.
  // This is only available after the `instantiate` method has
  // been called.
  instance: WebAssembly.Instance;
  
  // Constructs a new instance with internal state necessary to
  // manage a wasm instance.
  //
  // Note that this does not actually instantiate the WebAssembly
  // instance or module, you'll need to call the `instantiate`
  // method below to "activate" this class.
  constructor();
  
  // This is a low-level method which can be used to add any
  // intrinsics necessary for this instance to operate to an
  // import object.
  //
  // The `import` object given here is expected to be used later
  // to actually instantiate the module this class corresponds to.
  // If the `instantiate` method below actually does the
  // instantiation then there's no need to call this method, but
  // if you're instantiating manually elsewhere then this can be
  // used to prepare the import object for external instantiation.
  addToImports(imports: any): void;
  
  // Initializes this object with the provided WebAssembly
  // module/instance.
  //
  // This is intended to be a flexible method of instantiating
  // and completion of the initialization of this class. This
  // method must be called before interacting with the
  // WebAssembly object.
  //
  // The first argument to this method is where to get the
  // wasm from. This can be a whole bunch of different types,
  // for example:
  //
  // * A precompiled `WebAssembly.Module`
  // * A typed array buffer containing the wasm bytecode.
  // * A `Promise` of a `Response` which is used with
  //   `instantiateStreaming`
  // * A `Response` itself used with `instantiateStreaming`.
  // * An already instantiated `WebAssembly.Instance`
  //
  // If necessary the module is compiled, and if necessary the
  // module is instantiated. Whether or not it's necessary
  // depends on the type of argument provided to
  // instantiation.
  //
  // If instantiation is performed then the `imports` object
  // passed here is the list of imports used to instantiate
  // the instance. This method may add its own intrinsics to
  // this `imports` object too.
  instantiate(
  module: WebAssembly.Module | BufferSource | Promise<Response> | Response | WebAssembly.Instance,
  imports?: any,
  ): Promise<void>;
  wat2wasm(wat: string, features: WasmFeature): Result<Uint8Array, Errno>;
  wasm2wat(wasm: Uint8Array, features: WasmFeature): Result<string, string>;
}

/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const WABT_OK = 0;
const WABT_ERROR = 1;

/// features and default enabled state
const FEATURES = Object.freeze({
  'exceptions': false,
  'mutable_globals': true,
  'sat_float_to_int': true,
  'sign_extension': true,
  'simd': true,
  'threads': false,
  'function_references': false,
  'multi_value': true,
  'tail_call': false,
  'bulk_memory': true,
  'reference_types': true,
  'annotations': false,
  'code_metadata': false,
  'gc': false,
  'memory64': false,
  'multi_memory': false,
  'extended_const': false,
  'relaxed_simd': false,
  'custom_page_sizes': false,
  'compact_imports': false,
  'wide_arithmetic': false,
});


/// Allocate memory in the Module.
function malloc(size) {
  const addr = Module._malloc(size);
  if (addr == 0) {
    throw new Error('out of memory');
  }
  return addr;
}

/// Convert an ArrayBuffer/TypedArray/string into a buffer that can be
/// used by the Module.
function allocateBuffer(buf) {
  let addr;
  let size;
  if (buf instanceof ArrayBuffer) {
    size = buf.byteLength;
    addr = malloc(size);
    new Uint8Array(HEAPU8.buffer, addr, size).set(new Uint8Array(buf));
  } else if (ArrayBuffer.isView(buf)) {
    size = buf.byteLength;
    addr = malloc(size);
    new Uint8Array(HEAPU8.buffer, addr, size)
        .set(new Uint8Array(buf.buffer, buf.byteOffset, buf.byteLength));
  } else if (typeof buf == 'string') {
    addr = stringToNewUTF8(buf);
    size = lengthBytesUTF8(buf);
  } else {
    throw new Error('unknown buffer type: ' + buf);
  }
  return {addr: addr, size: size};
}


/// Features
class Features {
  constructor(obj = {}) {
    this.addr = Module._wabt_new_features();
    for (const [f, v] of Object.entries(FEATURES)) {
      this[f] = obj[f] ?? v;
    }
  }

  destroy() {
    Module._wabt_destroy_features(this.addr);
  }
}

Object.keys(FEATURES).forEach(function(feature) {
  Object.defineProperty(Features.prototype, feature, {
    enumerable: true,
    get: function() {
      return Module['_wabt_' + feature + '_enabled'](this.addr);
    },
    set: function(newValue) {
      Module['_wabt_set_' + feature + '_enabled'](this.addr, newValue | 0);
    }
  });
});


/// Lexer
class Lexer {
  constructor(filename, buffer, errors) {
    this.filename = stringToNewUTF8(filename);
    this.bufferObj = allocateBuffer(buffer);
    this.addr = Module._wabt_new_wast_buffer_lexer(
        this.filename, this.bufferObj.addr, this.bufferObj.size, errors.addr);
  }

  destroy() {
    Module._wabt_destroy_wast_lexer(this.addr);
    Module._free(this.bufferObj.addr);
    Module._free(this.filename);
  }
}


/// OutputBuffer
class OutputBuffer {
  constructor(addr) {
    this.addr = addr;
  }

  toTypedArray() {
    if (!this.addr) {
      return null;
    }

    const addr = Module._wabt_output_buffer_get_data(this.addr);
    const size = Module._wabt_output_buffer_get_size(this.addr);
    const buffer = new Uint8Array(size);
    buffer.set(new Uint8Array(HEAPU8.buffer, addr, size));
    return buffer;
  }

  toString() {
    if (!this.addr) {
      return '';
    }

    const addr = Module._wabt_output_buffer_get_data(this.addr);
    const size = Module._wabt_output_buffer_get_size(this.addr);
    return UTF8ToString(addr, size);
  }

  destroy() {
    Module._wabt_destroy_output_buffer(this.addr);
  }
}


/// Errors
class Errors {
  constructor(kind) {
    this.kind = kind;
    this.addr = Module._wabt_new_errors();
  }

  format(lexer = null) {
    let buffer;
    switch (this.kind) {
      case 'text':
        buffer = new OutputBuffer(
            Module._wabt_format_text_errors(this.addr, lexer.addr));
        break;
      case 'binary':
        buffer = new OutputBuffer(Module._wabt_format_binary_errors(this.addr));
        break;
      default:
        throw new Error(`Invalid Errors kind: ${this.kind}`);
    }
    const message = buffer.toString();
    buffer.destroy();
    return message;
  }

  destroy() {
    Module._wabt_destroy_errors(this.addr);
  }
}


/// parseWat
function parseWat(filename, buffer, options) {
  let errors = new Errors('text');
  let lexer = new Lexer(filename, buffer, errors);
  const features = new Features(options || {});

  let parseResult_addr;
  try {
    parseResult_addr =
        Module._wabt_parse_wat(lexer.addr, features.addr, errors.addr);

    const result = Module._wabt_parse_wat_result_get_result(parseResult_addr);
    if (result !== WABT_OK) {
      throw new Error('parseWat failed:\n' + errors.format(lexer));
    }

    const module_addr =
        Module._wabt_parse_wat_result_release_module(parseResult_addr);
    const wasmModule = new WasmModule(module_addr, errors, lexer);
    // Clear errors and lexer so they aren't destroyed below.
    errors = null;
    lexer = null;
    return wasmModule;
  } finally {
    Module._wabt_destroy_parse_wat_result(parseResult_addr);
    features.destroy();
    if (errors) {
      errors.destroy();
    }
    if (lexer) {
      lexer.destroy();
    }
  }
}


// readWasm
function readWasm(buffer, options = {}) {
  const bufferObj = allocateBuffer(buffer);
  let errors = new Errors('binary');
  const readDebugNames = options.readDebugNames ?? false;
  const check = options.check ?? true;
  const features = new Features(options);

  let readBinaryResult_addr;
  try {
    readBinaryResult_addr = Module._wabt_read_binary(
        bufferObj.addr, bufferObj.size, readDebugNames, features.addr,
        errors.addr);

    const result =
        Module._wabt_read_binary_result_get_result(readBinaryResult_addr);
    if (check && result !== WABT_OK) {
      throw new Error('readWasm failed:\n' + errors.format());
    }

    const module_addr =
        Module._wabt_read_binary_result_release_module(readBinaryResult_addr);
    const wasmModule = new WasmModule(module_addr, errors);
    // Clear errors so it isn't destroyed below.
    errors = null;
    return wasmModule;
  } finally {
    Module._wabt_destroy_read_binary_result(readBinaryResult_addr);
    features.destroy();
    if (errors) {
      errors.destroy();
    }
    Module._free(bufferObj.addr);
  }
}


// WasmModule (can't call it Module because emscripten has claimed it.)
class WasmModule {
  constructor(module_addr, errors, lexer = null) {
    this.module_addr = module_addr;
    this.errors = errors;
    this.lexer = lexer;
  }

  validate(options) {
    const features = new Features(options || {});
    try {
      const result = Module._wabt_validate_module(
          this.module_addr, features.addr, this.errors.addr);
      if (result !== WABT_OK) {
        throw new Error('validate failed:\n' + this.errors.format(this.lexer));
      }
    } finally {
      features.destroy();
    }
  }

  generateNames() {
    const result = Module._wabt_generate_names_module(this.module_addr);
    if (result !== WABT_OK) {
      throw new Error('generateNames failed.');
    }
  }

  applyNames() {
    const result = Module._wabt_apply_names_module(this.module_addr);
    if (result !== WABT_OK) {
      throw new Error('applyNames failed.');
    }
  }

  toText(options = {}) {
    const foldExprs = options.foldExprs ?? false;
    const inlineExport = options.inlineExport ?? false;

    const writeModuleResult_addr = Module._wabt_write_text_module(
        this.module_addr, foldExprs, inlineExport);

    const result =
        Module._wabt_write_module_result_get_result(writeModuleResult_addr);

    let outputBuffer;
    try {
      if (result !== WABT_OK) {
        throw new Error('toText failed.');
      }

      outputBuffer = new OutputBuffer(
          Module._wabt_write_module_result_release_output_buffer(
              writeModuleResult_addr));

      return outputBuffer.toString();

    } finally {
      if (outputBuffer) {
        outputBuffer.destroy();
      }
      Module._wabt_destroy_write_module_result(writeModuleResult_addr);
    }
  }

  toBinary(options = {}) {
    const log = options.log ?? false;
    const canonicalize_lebs = options.canonicalize_lebs ?? true;
    const relocatable = options.relocatable ?? false;
    const write_debug_names = options.write_debug_names ?? false;

    const writeModuleResult_addr = Module._wabt_write_binary_module(
        this.module_addr, log, canonicalize_lebs, relocatable,
        write_debug_names);

    const result =
        Module._wabt_write_module_result_get_result(writeModuleResult_addr);

    let binaryOutputBuffer;
    let logOutputBuffer;
    try {
      if (result !== WABT_OK) {
        throw new Error('toBinary failed.');
      }

      binaryOutputBuffer = new OutputBuffer(
          Module._wabt_write_module_result_release_output_buffer(
              writeModuleResult_addr));
      logOutputBuffer = new OutputBuffer(
          Module._wabt_write_module_result_release_log_output_buffer(
              writeModuleResult_addr));

      return {
        buffer: binaryOutputBuffer.toTypedArray(),
        log: logOutputBuffer.toString()
      };

    } finally {
      if (binaryOutputBuffer) {
        binaryOutputBuffer.destroy();
      }
      if (logOutputBuffer) {
        logOutputBuffer.destroy();
      }
      Module._wabt_destroy_write_module_result(writeModuleResult_addr);
    }
  }

  destroy() {
    Module._wabt_destroy_module(this.module_addr);
    if (this.errors) {
      this.errors.destroy();
    }
    if (this.lexer) {
      this.lexer.destroy();
    }
  }
}

Module['parseWat'] = parseWat;
Module['readWasm'] = readWasm;
Module['FEATURES'] = FEATURES;

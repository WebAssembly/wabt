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

var WABT_OK = 0;
var WABT_ERROR = 1;

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
});

/// If value is not undefined, return it. Otherwise return default_.
function maybeDefault(value, default_) {
  if (value === undefined) {
    return default_;
  }
  return value;
}

/// Coerce value to boolean if not undefined. Otherwise return default_.
function booleanOrDefault(value, default_) {
  return !!maybeDefault(value, default_);
}

/// Allocate memory in the Module.
function malloc(size) {
  var addr = Module._malloc(size);
  if (addr == 0) {
    throw new Error('out of memory');
  }
  return addr;
}

/// Convert an ArrayBuffer/TypedArray/string into a buffer that can be
/// used by the Module.
function allocateBuffer(buf) {
  var addr;
  var size;
  if (buf instanceof ArrayBuffer) {
    size = buf.byteLength;
    addr = malloc(size);
    (new Uint8Array(HEAP8.buffer, addr, size)).set(new Uint8Array(buf))
  } else if (ArrayBuffer.isView(buf)) {
    size = buf.buffer.byteLength;
    addr = malloc(size);
    (new Uint8Array(HEAP8.buffer, addr, size)).set(buf);
  } else if (typeof buf == 'string') {
    size = buf.length;
    addr = malloc(size);
    writeAsciiToMemory(buf, addr, true);  // don't null-terminate
  } else {
    throw new Error('unknown buffer type: ' + buf);
  }
  return {addr: addr, size: size};
}

function allocateCString(s) {
  var size = s.length;
  var addr = malloc(size);
  writeAsciiToMemory(s, addr);
  return {addr: addr, size: size};
}


/// Features
function Features(obj) {
  this.addr = Module._wabt_new_features();
  for (var [f, v] of Object.entries(FEATURES)) {
    this[f] = booleanOrDefault(obj[f], v);
  }
}
Features.prototype = Object.create(Object.prototype);

Features.prototype.destroy = function() {
  Module._wabt_destroy_features(this.addr);
};

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
function Lexer(filename, buffer, errors) {
  this.filenameObj = allocateCString(filename);
  this.bufferObj = allocateBuffer(buffer);
  this.addr = Module._wabt_new_wast_buffer_lexer(
      this.filenameObj.addr, this.bufferObj.addr, this.bufferObj.size,
      errors.addr);
}
Lexer.prototype = Object.create(Object.prototype);

Lexer.prototype.destroy = function() {
  Module._wabt_destroy_wast_lexer(this.addr);
  Module._free(this.bufferObj.addr);
  Module._free(this.filenameObj.addr);
};


/// OutputBuffer
function OutputBuffer(addr) {
  this.addr = addr;
}
OutputBuffer.prototype = Object.create(Object.prototype);

OutputBuffer.prototype.toTypedArray = function() {
  if (!this.addr) {
    return null;
  }

  var addr = Module._wabt_output_buffer_get_data(this.addr);
  var size = Module._wabt_output_buffer_get_size(this.addr);
  var buffer = new Uint8Array(size);
  buffer.set(new Uint8Array(HEAPU8.buffer, addr, size));
  return buffer;
};

OutputBuffer.prototype.toString = function() {
  if (!this.addr) {
    return '';
  }

  var addr = Module._wabt_output_buffer_get_data(this.addr);
  var size = Module._wabt_output_buffer_get_size(this.addr);
  return UTF8ToString(addr, size);
};

OutputBuffer.prototype.destroy = function() {
  Module._wabt_destroy_output_buffer(this.addr);
};


/// Errors
function Errors(kind) {
  this.kind = kind;
  this.addr = Module._wabt_new_errors();
}
Errors.prototype = Object.create(Object.prototype);

Errors.prototype.format = function() {
  var buffer;
  switch (this.kind) {
    case 'text':
      buffer = new OutputBuffer(
          Module._wabt_format_text_errors(this.addr, this.lexer.addr));
      break;
    case 'binary':
      buffer = new OutputBuffer(Module._wabt_format_binary_errors(this.addr));
      break;
    default:
      throw new Error('Invalid Errors kind: ' + this.kind);
  }
  var message = buffer.toString();
  buffer.destroy();
  return message;
};

Errors.prototype.destroy = function() {
  Module._wabt_destroy_errors(this.addr);
  if (this.lexer) {
    this.lexer.destroy();
  }
};


/// parseWat
function parseWat(filename, buffer, options) {
  var errors = new Errors('text');
  var lexer = new Lexer(filename, buffer, errors);
  errors.lexer = lexer;
  var features = new Features(options || {});

  try {
    var parseResult_addr =
        Module._wabt_parse_wat(lexer.addr, features.addr, errors.addr);

    var result = Module._wabt_parse_wat_result_get_result(parseResult_addr);
    if (result !== WABT_OK) {
      throw new Error('parseWat failed:\n' + errors.format());
    }

    var module_addr =
        Module._wabt_parse_wat_result_release_module(parseResult_addr);
    var result = new WasmModule(module_addr, errors);
    // Clear errors so it isn't destroyed below.
    errors = null;
    return result;
  } finally {
    Module._wabt_destroy_parse_wat_result(parseResult_addr);
    features.destroy();
    if (errors) {
      errors.destroy();
    }
  }
}


// readWasm
function readWasm(buffer, options) {
  var bufferObj = allocateBuffer(buffer);
  var errors = new Errors('binary');
  var readDebugNames = booleanOrDefault(options.readDebugNames, false);
  var check = booleanOrDefault(options.check, true);
  var features = new Features(options);

  try {
    var readBinaryResult_addr = Module._wabt_read_binary(
        bufferObj.addr, bufferObj.size, readDebugNames, features.addr,
        errors.addr);

    var result =
        Module._wabt_read_binary_result_get_result(readBinaryResult_addr);
    if (check && result !== WABT_OK) {
      throw new Error('readWasm failed:\n' + errors.format());
    }

    var module_addr =
        Module._wabt_read_binary_result_release_module(readBinaryResult_addr);
    var result = new WasmModule(module_addr, errors);
    // Clear errors so it isn't destroyed below.
    errors = null;
    return result;
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
function WasmModule(module_addr, errors) {
  this.module_addr = module_addr;
  this.errors = errors;
}
WasmModule.prototype = Object.create(Object.prototype);

WasmModule.prototype.validate = function(options) {
  var features = new Features(options || {});
  try {
    var result = Module._wabt_validate_module(
        this.module_addr, features.addr, this.errors.addr);
    if (result !== WABT_OK) {
      throw new Error('validate failed:\n' + this.errors.format());
    }
  } finally {
    features.destroy();
  }
};

WasmModule.prototype.resolveNames = function() {
  // No-op, this is now part of text parsing.
};

WasmModule.prototype.generateNames = function() {
  var result = Module._wabt_generate_names_module(this.module_addr);
  if (result !== WABT_OK) {
    throw new Error('generateNames failed.');
  }
};

WasmModule.prototype.applyNames = function() {
  var result = Module._wabt_apply_names_module(this.module_addr);
  if (result !== WABT_OK) {
    throw new Error('applyNames failed.');
  }
};

WasmModule.prototype.toText = function(options) {
  var foldExprs = booleanOrDefault(options.foldExprs, false);
  var inlineExport = booleanOrDefault(options.inlineExport, false);

  var writeModuleResult_addr =
      Module._wabt_write_text_module(this.module_addr, foldExprs, inlineExport);

  var result =
      Module._wabt_write_module_result_get_result(writeModuleResult_addr);

  try {
    if (result !== WABT_OK) {
      throw new Error('toText failed.');
    }

    var outputBuffer =
        new OutputBuffer(Module._wabt_write_module_result_release_output_buffer(
            writeModuleResult_addr));

    return outputBuffer.toString();

  } finally {
    if (outputBuffer) {
      outputBuffer.destroy();
    }
    Module._wabt_destroy_write_module_result(writeModuleResult_addr);
  }
};

WasmModule.prototype.toBinary = function(options) {
  var log = booleanOrDefault(options.log, false);
  var canonicalize_lebs = booleanOrDefault(options.canonicalize_lebs, true);
  var relocatable = booleanOrDefault(options.relocatable, false);
  var write_debug_names = booleanOrDefault(options.write_debug_names, false);

  var writeModuleResult_addr = Module._wabt_write_binary_module(
      this.module_addr, log, canonicalize_lebs, relocatable, write_debug_names);

  var result =
      Module._wabt_write_module_result_get_result(writeModuleResult_addr);

  try {
    if (result !== WABT_OK) {
      throw new Error('toBinary failed.');
    }

    var binaryOutputBuffer =
        new OutputBuffer(Module._wabt_write_module_result_release_output_buffer(
            writeModuleResult_addr));
    var logOutputBuffer = new OutputBuffer(
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
};

WasmModule.prototype.destroy = function() {
  Module._wabt_destroy_module(this.module_addr);
  if (this.errors) {
    this.errors.destroy();
  }
};

Module['parseWat'] = parseWat;
Module['readWasm'] = readWasm;
Module['FEATURES'] = FEATURES;

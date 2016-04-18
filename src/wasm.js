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

var wasm = {};

wasm.ready = new Promise(function(resolve, reject) {
  wasm.$resolve = resolve;
  wasm.$reject = reject;
});

var Module = {};
Module.onRuntimeInitialized = function() {

var OK = 0;
var ERROR = 1;

// Helpers /////////////////////////////////////////////////////////////////////
function malloc(size) {
  var addr = Module._malloc(size);
  if (addr == 0)
    throw "malloc failed";
  return addr;
}

function mallocz(size) {
  var addr = malloc(size);
  HEAP8.fill(0, addr, addr + size);
  return addr;
}

function free(p) { Module._free(p); }

function decorateStruct(struct, structName, members) {
  struct.prototype = Object.create(Object.prototype);
  struct.$size = Module['_wasm_sizeof_' + structName]();
  struct.$allocate = function() { return mallocz(this.$size); };
  struct.prototype.$free = function() { return free(this.$addr); };
  struct.prototype.$destroy = function() { return this.$free(); };
  struct.$newAt = function(addr) {
    var this_ = Object.create(this.prototype);
    var args = Array.prototype.slice.call(arguments, 1);
    this_.$addr = addr;
    this_.$init.apply(this_, args);
    return this_;
  };
  defineOffsets(struct.prototype, structName, members);
}

function defineOffsets(proto, structName, members) {
  for (var i = 0; i < members.length; ++i) {
    var offsetName = '$offsetof_' + members[i];
    var funcName = '_wasm_offsetof_' + structName + '_' + members[i];
    proto[offsetName] = Module[funcName]();
  }
}

function loadi8(addr) { return HEAP8[addr]; }
function loadu8(addr) { return HEAPU8[addr]; }
function loadi16(addr) { return HEAP16[addr >> 1]; }
function loadu16(addr) { return HEAPU16[addr >> 1]; }
function loadi32(addr) { return HEAP32[addr >> 2]; }
function loadu32(addr) { return HEAPU32[addr >> 2]; }

function storei8(addr, value) { HEAP8[addr] = value; }
function storeu8(addr, value) { HEAPU8[addr] = value; }
function storei16(addr, value) { HEAP16[addr >> 1] = value; }
function storeu16(addr, value) { HEAPU16[addr >> 1] = value; }
function storei32(addr, value) { HEAP32[addr >> 2] = value; }
function storeu32(addr, value) { HEAPU32[addr >> 2] = value; }

function storeboolopt(addr, value, defaultValue) {
  storeu8(addr, (typeof value === 'undefined') ? defaultValue : value);
}

function allocateString(str) {
  var len = str.length + 1;
  var mem = malloc(len);
  Module.writeAsciiToMemory(str, mem);
  return mem;
}

function freeString(mem) {
  free(mem);
}

// WrappedFunction /////////////////////////////////////////////////////////////
function WrappedFunction(f) {
  this.$index = Runtime.addFunction(f);
}
WrappedFunction.prototype = Object.create(Object.prototype);
WrappedFunction.prototype.store = function(addr) {
  storeu32(addr, this.$index);
};
WrappedFunction.prototype.$destroy = function() {
  Runtime.removeFunction(this.$index);
};


// Allocator ///////////////////////////////////////////////////////////////////
var Allocator = function() {
  throw "Allocator is an abstract base class";
};
decorateStruct(Allocator, 'allocator',
    ['alloc', 'realloc', 'free', 'destroy', 'mark', 'reset_to_mark',
     'print_stats']);
Allocator.prototype.$init = function() {};
Allocator.prototype.alloc = function(size, align) {
  var alloc = loadu32(this.$addr + this.$offsetof_alloc);
  return Runtime.dynCall('iiiiii', alloc, [this.$addr, size, align, 0, 0]);
};
Allocator.prototype.realloc = function(p, size, align) {
  var realloc = loadu32(this.$addr + this.$offsetof_realloc);
  return Runtime.dynCall('iiiiiii', realloc,
                         [this.$addr, p, size, align, 0, 0]);
};
Allocator.prototype.free = function(p) {
  var free = loadu32(this.$addr + this.$offsetof_free);
  Runtime.dynCall('viiii', free, [this.$addr, p, 0, 0]);
};
Allocator.prototype.destroy = function() {
  var destroy = loadu32(this.$addr + this.$offsetof_destroy);
  Runtime.dynCall('vi', destroy, [this.$addr]);
};
Allocator.prototype.mark = function() {
  var mark = loadu32(this.$addr + this.$offsetof_mark);
  Runtime.dynCall('ii', mark, [this.$addr]);
};
Allocator.prototype.resetToMark = function(mark) {
  var reset_to_mark = loadu32(this.$addr + this.$offsetof_reset_to_mark);
  Runtime.dynCall('vii', reset_to_mark, [this.$addr, mark]);
};
Allocator.prototype.printStats = function() {
  var print_stats = loadu32(this.$addr + this.$offsetof_print_stats);
  Runtime.dynCall('vi', print_stats, [this.$addr]);
};

LibcAllocator = Allocator.$newAt(Module._wasm_get_libc_allocator());


// Buffer //////////////////////////////////////////////////////////////////////
var Buffer = function(size) {
  this.$addr = mallocz(size);
  this.$size = size;
};
Buffer.prototype = Object.create(Object.prototype);
Buffer.fromString = function(str) {
  var this_ = Object.create(Buffer.prototype);
  this_.$size = str.length;
  this_.$addr = malloc(this_.$size);
  Module.writeAsciiToMemory(str, this_.$addr, true);  // don't null-terminate
  return this_;
};
Buffer.$newAt = function(addr, size) {
  var this_ = Object.create(Buffer.prototype);
  this_.$addr = addr;
  this_.$size = size;
  return this_;
};
Buffer.prototype.$free = function() { free(this.$addr); };
Buffer.prototype.$destroy = function() { this.$free(); };


// JSWriter ////////////////////////////////////////////////////////////////////
var JSWriter = function(writeData, moveData) {
  var writeDataWrap = function(offset, data, size, userData) {
    return writeData(offset, new Uint8Array(Module.buffer, data, size));
  };
  var moveDataWrap = function(dstOffset, srcOffset, size, userData) {
    return moveData(dstOffset, srcOffset, size);
  };

  this.writer = new Writer();
  this.$writeData = new WrappedFunction(writeDataWrap);
  this.$moveData = new WrappedFunction(moveDataWrap);
  this.$writeData.store(this.writer.$addr + this.writer.$offsetof_write_data);
  this.$moveData.store(this.writer.$addr + this.writer.$offsetof_move_data);
};
JSWriter.prototype = Object.create(Object.prototype);
JSWriter.prototype.$destroy = function() {
  this.$moveData.$destroy();
  this.$writeData.$destroy();
  this.writer.$destroy();
};


// JSStringWriter //////////////////////////////////////////////////////////////
var JSStringWriter = function() {
  this.string = '';

  var splice = function(offset, str) {
    var before = this.string.slice(0, offset);
    var after = this.string.slice(offset + str.length);
    this.string = before + str + after;
  };

  var writeData = function(offset, buf) {
    var str = '';
    for (var i = 0; i < buf.length; ++i)
      str += String.fromCharCode(buf[i]);
    if (offset == this.string.length) {
      // common case, appending data
      this.string += str;
    } else {
      splice(offset, str);
    }
  };

  var moveData = function(dstOffset, srcOffset, size) {
    splice(dstOffset, this.string.slice(srcOffset, size));
  };

  JSWriter.call(this, writeData.bind(this), moveData.bind(this));
};
JSStringWriter.prototype = Object.create(JSWriter.prototype);


// Lexer ///////////////////////////////////////////////////////////////////////
var Lexer = function() {
  throw "Lexer must be created with $fromBuffer";
};
Lexer.prototype = Object.create(Object.prototype);
Lexer.fromBuffer = function(allocator, filename, buffer) {
  var $filename = allocateString(filename);
  var addr = Module._wasm_new_buffer_lexer(allocator.$addr, $filename,
                                           buffer.$addr, buffer.$size);
  if (addr == 0)
    throw "Lexer.fromBuffer failed";
  var this_ = Object.create(Lexer.prototype);
  this_.$addr = addr;
  this_.$filename = $filename;
  return this_;
};
Lexer.prototype.$destroy = function() {
  Module._wasm_destroy_lexer(this.$addr);
  freeString(this.$filename);
};


// Location ////////////////////////////////////////////////////////////////////
var Location = function() {
  this.$addr = Location.$allocate();
  this.$init();
};
decorateStruct(Location, 'location',
               ['filename', 'line', 'first_column', 'last_column']);
Location.prototype.$init = function() {};
Object.defineProperty(Location.prototype, 'filename', {
  get: function() {
    return Module.AsciiToString(loadu32(this.$addr + this.$offsetof_filename));
  }
});
Object.defineProperty(Location.prototype, 'line', {
  get: function() { return loadu32(this.$addr + this.$offsetof_line); }
});
Object.defineProperty(Location.prototype, 'firstColumn', {
  get: function() { return loadu32(this.$addr + this.$offsetof_first_column); }
});
Object.defineProperty(Location.prototype, 'lastColumn', {
  get: function() { return loadu32(this.$addr + this.$offsetof_last_column); }
});


// MemoryWriter ////////////////////////////////////////////////////////////////
var MemoryWriter = function(allocator) {
  this.$addr = MemoryWriter.$allocate();
  this.$init(allocator);
};
decorateStruct(MemoryWriter, 'memory_writer', ['base', 'buf']);
MemoryWriter.prototype.$init = function(allocator) {
  var result = Module._wasm_init_mem_writer(allocator.$addr, this.$addr);
  if (result != OK)
    throw "error initializing MemoryWriter";
  this.base = Writer.$newAt(this.$addr + this.$offsetof_base);
  this.buf = OutputBuffer.$newAt(this.$addr + this.$offsetof_buf);
};
MemoryWriter.prototype.$destroy = function() {
  Module._wasm_close_mem_writer(this.$addr);
  this.$free();
};


// OutputBuffer ////////////////////////////////////////////////////////////////
var OutputBuffer = function() {
  this.$addr = OutputBuffer.$allocate();
  this.$init();
};
decorateStruct(OutputBuffer, 'output_buffer',
               ['allocator', 'start', 'size', 'capacity']);
OutputBuffer.prototype.$init = function() {
  this.$allocator =
      Allocator.$newAt(loadu32(this.$addr + this.$offsetof_allocator)),
  this.$buf = Buffer.$newAt(loadu32(this.$addr + this.$offsetof_start),
                            loadu32(this.$addr + this.$offsetof_size));
};
OutputBuffer.prototype.$destroy = function() {
  Module._wasm_destroy_output_buffer(this.$addr);
  this.$free();
};
Object.defineProperty(OutputBuffer.prototype, 'allocator', {
  get: function() {
    this.$allocator.$addr = loadu32(this.$addr + this.$offsetof_allocator);
    return this.$allocator;
  }
});
Object.defineProperty(OutputBuffer.prototype, 'buf', {
  get: function() {
    this.$buf.$addr = loadu32(this.$addr + this.$offsetof_start);
    this.$buf.$size = loadu32(this.$addr + this.$offsetof_size);
    return this.$buf;
  }
});
Object.defineProperty(OutputBuffer.prototype, 'capacity', {
  get: function() {
    return loadu32(this.$addr + this.$offsetof_capacity);
  }
});


// Script //////////////////////////////////////////////////////////////////////
var Script = function() {
  this.$addr = Script.$allocate();
  this.$init();
};
decorateStruct(Script, 'script', []);
Script.prototype.$init = function() {};
Script.prototype.$destroy = function() {
  Module._wasm_destroy_script(this.$addr);
  this.$free();
};


// SourceErrorHandler //////////////////////////////////////////////////////////
var SourceErrorHandler = function(callback, sourceLineMaxLength) {
  this.$addr = SourceErrorHandler.$allocate();
  this.$init(callback, sourceLineMaxLength);
}
decorateStruct(SourceErrorHandler, 'source_error_handler',
    ['on_error', 'source_line_max_length', 'user_data']);
SourceErrorHandler.prototype.$init =
    function(onError, sourceLineMaxLength) {
  var f;
  if (onError) {
    f = function(loc, error, sourceLine, sourceLineLength,
                 sourceLineColumnOffset, userData) {
      loc = Location.$newAt(loc);
      error = Module.AsciiToString(error);
      sourceLine = Module.Pointer_stringify(sourceLine, sourceLineLength);
      onError(loc, error, sourceLine, sourceLineColumnOffset);
    };
  } else {
    f = function() {
      Module._wasm_default_source_error_callback.apply(null, arguments);
    };
  }
  this.$onError = new WrappedFunction(f);
  this.$onError.store(this.$addr + this.$offsetof_on_error);
  storeu32(this.$addr + this.$offsetof_source_line_max_length,
           sourceLineMaxLength);
};
SourceErrorHandler.prototype.$destroy = function() {
  this.$onError.$destroy();
  this.$free();
};


// StackAllocator //////////////////////////////////////////////////////////////
var StackAllocator = function(fallback) {
  this.$addr = StackAllocator.$allocate();
  this.$init(fallback);
}
decorateStruct(StackAllocator, 'stack_allocator', ['allocator']);
StackAllocator.prototype.$init = function(fallback) {
  Module._wasm_init_stack_allocator(this.$addr, fallback.$addr);
  this.allocator = Allocator.$newAt(this.$addr + this.$offsetof_allocator);
};
StackAllocator.prototype.$destroy = function() {
  Module._wasm_destroy_stack_allocator(this.$addr);
};


// Stream //////////////////////////////////////////////////////////////
var Stream = function(writer, logStream) {
  this.$addr = Stream.$allocate();
  this.$init(writer, logStream);
}
decorateStruct(Stream, 'stream', ['writer', 'result', 'offset', 'log_stream']);
Stream.prototype.$init = function(writer, logStream) {
  var logStreamAddr = logStream ? logStream.$addr : 0;

  Module._wasm_init_stream(this.$addr, writer.$addr, logStreamAddr);
  this.$writer = Writer.$newAt(loadu32(this.$addr + this.$offsetof_writer));

  logStreamAddr = loadu32(this.$addr + this.$offsetof_log_stream);
  this.$logStream = logStreamAddr ? Stream.$newAt(logStreamAddr) : null;
};
Object.defineProperty(Stream.prototype, 'writer', {
  get: function() {
    this.$writer.$addr = loadu32(this.$addr + this.$offsetof_writer);
    return this.$writer;
  }
});
Object.defineProperty(Stream.prototype, 'offset', {
  get: function() { return loadu32(this.$addr + this.$offsetof_offset); }
});
Object.defineProperty(Stream.prototype, 'result', {
  get: function() { return loadu32(this.$addr + this.$offsetof_result); }
});
Object.defineProperty(Stream.prototype, 'logStream', {
  get: function() {
    if (this.$logStream) {
      this.$logStream.$addr = loadu32(this.$addr + this.$offsetof_log_stream);
    }
    return this.$logStream;
  }
});


// WriteBinaryOptions //////////////////////////////////////////////////////////
var WriteBinaryOptions = function(options) {
  this.$addr = WriteBinaryOptions.$allocate();
  this.$init(options);
};
decorateStruct(WriteBinaryOptions, 'write_binary_options',
               ['log_stream', 'canonicalize_lebs', 'remap_locals',
                'write_debug_names']);
WriteBinaryOptions.prototype.$init = function(options) {
  if (!options)
    options = {};
  storeu32(this.$addr + this.$offsetof_log_stream,
           options.logStream ? options.logStream.$addr : 0);
  storeboolopt(this.$addr + this.$offsetof_canonicalize_lebs,
               options.canonicalizeLebs, true);
  storeboolopt(this.$addr + this.$offsetof_remap_locals, options.remapLocals,
               true);
  storeboolopt(this.$addr + this.$offsetof_write_debug_names,
               options.writeDebugNames, false);
};


// Writer //////////////////////////////////////////////////////////////////////
var Writer = function() {
  this.$addr = Writer.$allocate();
};
decorateStruct(Writer, 'writer', ['write_data', 'move_data']);
Writer.prototype.$init = function() {};
Writer.prototype.$writeData = function(offset, data, size, userData) {
  var write_data = loadu32(this.$addr + this.$offsetof_write_data);
  return Runtime.dynCall('iiiii', write_data,
                         [this.$addr, offset, data, size, userData]);
};
Writer.prototype.$moveData = function(dstOffset, srcOffset, size, userData) {
  var move_data = loadu32(this.$addr + this.$offsetof_move_data);
  return Runtime.dynCall('iiiii', move_data,
                         [this.$addr, dstOffset, srcOffset, size, userData]);
};


// Free functions //////////////////////////////////////////////////////////////
var parse = function(lexer, errorHandler) {
  var script = new Script();
  var result =
      Module._wasm_parse(lexer.$addr, script.$addr, errorHandler.$addr);
  if (result != OK)
    throw "parse failed";
  return script;
};

var checkAst = function(lexer, script, errorHandler) {
  var result =
      Module._wasm_check_ast(lexer.$addr, script.$addr, errorHandler.$addr);
  if (result != OK)
    throw "checkAst failed";
};

var markUsedBlocks = function(allocator, script) {
  var result = Module._wasm_mark_used_blocks(allocator.$addr, script.$addr);
  if (result != OK)
    throw "markUsedBlocks failed";
};

var writeBinaryScript = function(allocator, writer, script, options) {
  var result = Module._wasm_write_binary_script(allocator.$addr, writer.$addr,
                                                script.$addr, options.$addr);
  if (result != OK)
    throw "writeBinaryScript failed";
};

var resolve = wasm.$resolve;

wasm = {
  ready: wasm.ready,

  OK: OK,
  ERROR: ERROR,

  Allocator: Allocator,
  Buffer: Buffer,
  JSStringWriter: JSStringWriter,
  JSWriter: JSWriter,
  Lexer: Lexer,
  LibcAllocator: LibcAllocator,
  Location: Location,
  MemoryWriter: MemoryWriter,
  OutputBuffer: OutputBuffer,
  Script: Script,
  SourceErrorHandler: SourceErrorHandler,
  StackAllocator: StackAllocator,
  Stream: Stream,
  WriteBinaryOptions: WriteBinaryOptions,
  Writer: Writer,

  checkAst: checkAst,
  markUsedBlocks: markUsedBlocks,
  parse: parse,
  writeBinaryScript: writeBinaryScript,
};

resolve();

};

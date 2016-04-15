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

var wasm = (function() {

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

function allocateString(str) {
  var len = str.length + 1;
  var mem = malloc(len);
  Module.writeAsciiToMemory(str, mem);
  return mem;
}

function freeString(mem) {
  free(mem);
}

// Allocator ///////////////////////////////////////////////////////////////////
Allocator = function() {
  throw "Allocator is an abstract base class";
};
decorateStruct(Allocator, 'allocator',
    ['alloc', 'realloc', 'free', 'destroy', 'mark', 'reset_to_mark',
     'print_stats']);
Allocator.prototype.$init = function() {
  this.$alloc = loadu32(this.$addr + this.$offsetof_alloc);
  this.$realloc = loadu32(this.$addr + this.$offsetof_realloc);
  this.$free = loadu32(this.$addr + this.$offsetof_free);
  this.$destroy = loadu32(this.$addr + this.$offsetof_destroy);
  this.$mark = loadu32(this.$addr + this.$offsetof_mark);
  this.$reset_to_mark = loadu32(this.$addr + this.$offsetof_reset_to_mark);
  this.$print_stats = loadu32(this.$addr + this.$offsetof_print_stats);
};
Allocator.prototype.alloc = function(size, align) {
  return Runtime.dynCall('iiiiii', this.$alloc,
                         [this.$addr, size, align, 0, 0]);
};
Allocator.prototype.realloc = function(p, size, align) {
  return Runtime.dynCall('iiiiiii', this.$realloc,
                         [this.$addr, p, size, align, 0, 0]);
};
Allocator.prototype.free = function(p) {
  Runtime.dynCall('viiii', this.$free, [this.$addr, p, 0, 0]);
};
Allocator.prototype.destroy = function() {
  Runtime.dynCall('vi', this.$destroy, [this.$addr]);
};
Allocator.prototype.mark = function() {
  Runtime.dynCall('ii', this.$mark, [this.$addr]);
};
Allocator.prototype.resetToMark = function(mark) {
  Runtime.dynCall('vii', this.$reset_to_mark, [this.$addr, mark]);
};
Allocator.prototype.printStats = function() {
  Runtime.dynCall('vi', this.$print_stats, [this.$addr]);
};

LibcAllocator = Allocator.$newAt(Module._wasm_get_libc_allocator());

// Buffer //////////////////////////////////////////////////////////////////////
Buffer = function(size) {
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
Buffer.prototype.$free = function() { free(this.$addr); };
Buffer.prototype.$destroy = function() { this.$free(); };

// Lexer ///////////////////////////////////////////////////////////////////////
Lexer = function() {
  throw "Lexer must be created with $fromFile or $fromBuffer";
};
Lexer.prototype = Object.create(Object.prototype);
Lexer.fromFile = function(allocator, filename) {
  var $filename = allocateString(filename);
  var addr = Module._wasm_new_file_lexer(allocator.$addr, $filename);
  if (addr == 0)
    throw "Lexer.fromFile failed";
  var this_ = Object.create(Lexer.prototype);
  this_.$addr = addr;
  this_.$filename = $filename;
  return this_;
};
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
Location = function() {
  this.$addr = Location.$allocate();
  this.$init();
};
decorateStruct(Location, 'location',
               ['filename', 'line', 'first_column', 'last_column']);
Location.prototype.$init = function() {
  this.filename =
      Module.AsciiToString(loadu32(this.$addr + this.$offsetof_filename));
  this.line = loadu32(this.$addr + this.$offsetof_line);
  this.firstColumn = loadu32(this.$addr + this.$offsetof_first_column);
  this.lastColumn = loadu32(this.$addr + this.$offsetof_last_column);
};

// Script //////////////////////////////////////////////////////////////////////
Script = function() {
  this.$addr = Script.$allocate();
  this.$init();
};
decorateStruct(Script, 'script', []);
Script.prototype.$init = function() {};

// SourceErrorHandler //////////////////////////////////////////////////////////
SourceErrorHandler = function(callback, sourceLineMaxLength) {
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
  this.index = Runtime.addFunction(f);
  storeu32(this.$addr + this.$offsetof_on_error, this.index);
  storeu32(this.$addr + this.$offsetof_source_line_max_length,
           sourceLineMaxLength);
};
SourceErrorHandler.prototype.$destroy = function() {
  Runtime.removeFunction(this.index);
  this.$free();
};

// StackAllocator //////////////////////////////////////////////////////////////
StackAllocator = function(fallback) {
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

// Free functions //////////////////////////////////////////////////////////////
parse = function(lexer, errorHandler) {
  var script = new Script();
  var result =
      Module._wasm_parse(lexer.$addr, script.$addr, errorHandler.$addr);
  if (result != OK)
    throw "parse failed";
  return script;
};

checkAst = function(lexer, script, errorHandler) {
  var result =
      Module._wasm_check_ast(lexer.$addr, script.$addr, errorHandler.$addr);
  if (result != OK)
    throw "check failed";
};

return {
  OK: OK,
  ERROR: ERROR,
  Allocator: Allocator,
  Buffer: Buffer,
  Lexer: Lexer,
  LibcAllocator: LibcAllocator,
  Location: Location,
  Script: Script,
  SourceErrorHandler: SourceErrorHandler,
  StackAllocator: StackAllocator,
  parse: parse,
  checkAst: checkAst,
};

})();

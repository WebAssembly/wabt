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

// Helpers /////////////////////////////////////////////////////////////////////

function loadi8(addr) { return HEAP8[addr]; }
function loadi16(addr) { return HEAP16[addr>>1]; }
function loadi32(addr) { return HEAP32[addr>>2]; }
function loadu8(addr) { return HEAPU8[addr]; }
function loadu16(addr) { return HEAPU16[addr>>1]; }
function loadu32(addr) { return HEAPU32[addr>>2]; }
function loadf32(addr) { return HEAPF32[addr>>2]; }
function loadf64(addr) { return HEAPF64[addr>>4]; }
function storei8(addr, value) { HEAP8[addr] = value; }
function storei16(addr, value) { HEAP16[addr>>1] = value; }
function storei32(addr, value) { HEAP32[addr>>2] = value; }
function storeu8(addr, value) { HEAPU8[addr] = value; }
function storeu16(addr, value) { HEAPU16[addr>>1] = value; }
function storeu32(addr, value) { HEAPU32[addr>>2] = value; }
function storef32(addr, value) { HEAPF32[addr>>2] = value; }
function storef64(addr, value) { HEAPF64[addr>>4] = value; }

function loadcstr(addr) { return Module.AsciiToString(addr); }
function loadstrslice(addr, len) { return Module.Pointer_stringify(addr, len); }
function loadbuffer(addr, len) {
  return new Uint8Array(HEAPU8.buffer, addr, len);
}

function sizeof(structName) { return Module['_wasm_sizeof_' + structName](); }
function offsetof(structName, fieldName) {
  return Module['_wasm_offsetof_' + structName + '_' + fieldName]();
}

function malloc(size) {
  var addr = Module._malloc(size);
  if (addr == 0) {
    throw new Error('out of memory');
  }
  return addr;
}

function mallocz(size) {
  var addr = malloc(size);
  HEAPU8.fill(0, addr, addr + size);
  return addr;
}

function free(addr) { Module._free(addr); }

// Types ///////////////////////////////////////////////////////////////////////

function Type(name, id) {
  this.name = name;
  if (id) {
    // alias
    this.id = id;
  } else {
    this.id = Type.id++;
    Type.map[this.id] = this;
  }
}
Type.id = 0;
Type.map = {};
Type.check = function(expected, actual) {
  if (actual.id != expected.id) {
    throw new Error('type mismatch; expected ' + expected.toString() +
                    ', got ' + actual.toString());
  }
};
Type.prototype = Object.create(Object.prototype);
Type.prototype.toString = function(name, lastPrec) {
  var result = ''
  if (this.name) {
    result = this.name;
    if (name) {
      result += ' ' + name;
    }
    return result;
  }
  name = name || '';
  var prec = this.prec;

  if (prec && lastPrec && prec > lastPrec) {
    name = '(' + name + ')';
  }

  if (this.kind == 'ptr') {
    return this.pointee.toString('*' + result + name, prec);
  } else if (this.kind == 'array') {
    return this.element.toString(result + '[]', prec);
  } else if (this.kind == 'fn') {
    name += '(';
    if (this.params.length > 0) {
      var paramNames = [];
      for (var i = 0; i < this.params.length; ++i) {
        paramNames.push(this.params[i].toString());
      }
      name += paramNames.join(', ');
    }
    name += ')';
    return this.result.toString(name, prec);
  } else {
    throw new Error('unknown type kind');
  }
};
Type.prototype.load = function() {
  throw new Error(this.toString() + " cannot be loaded");
};
Type.prototype.store = function() {
  throw new Error(this.toString() + " cannot be stored");
};
Type.prototype.call = function() {
  throw new Error(this.toString() + " cannot be called");
};

function PrimitiveType(name, size, sig, min, max) {
  Type.call(this, name);
  this.size = size;
  this.sig = sig;
  this.min = min;
  this.max = max;
}
PrimitiveType.prototype = Object.create(Type.prototype);
PrimitiveType.prototype.kind = 'primitive'
PrimitiveType.prototype.primitive = true;
PrimitiveType.prototype.checkRange = function(value) {
  if (value < this.min || value > this.max) {
    throw new Error('value out of range: ' + value + ' not in [' + this.min + ',' +
        this.max + ']');
  }
};
PrimitiveType.prototype.toJS = function(value) {
  return value;
};
PrimitiveType.prototype.initAccessors = function(value, addr) {
  var type = this;
  value.load = function() { return type.toJS(type.load(addr)); };
  value.store = function(value) { type.store(addr, type.fromJS(value)); };
};

// TODO(binji): figure out what the signature chars actually are
var Void = new PrimitiveType('Void', 0, 'v');
var Bool = new PrimitiveType('Bool', 1, 'i');
var I8 = new PrimitiveType('I8', 1, 'i', -128, 127);
var I16 = new PrimitiveType('I16', 2, 'i', -32768, 32767);
var I32 = new PrimitiveType('I32', 4, 'i', -0x80000000, 0x7fffffff);
var U8 = new PrimitiveType('U8', 1, 'i', 0, 255);
var U16 = new PrimitiveType('U16', 2, 'i', 0, 65535);
var U32 = new PrimitiveType('U32', 4, 'i', 0, 0xffffffff);
var U64 = new PrimitiveType('U64', 8, 'i');
var F32 = new PrimitiveType('F32', 4, 'f');
var F64 = new PrimitiveType('F64', 8, 'd');

Void.fromJS = function(value) {}
Bool.fromJS = function(value) { return !!value; };
I8.fromJS = function(value) { this.checkRange(value); return value|0; };
I16.fromJS = function(value) { this.checkRange(value); return value|0; };
I32.fromJS = function(value) { this.checkRange(value); return value|0; };
U8.fromJS = function(value) { this.checkRange(value); return value>>>0; };
U16.fromJS = function(value) { this.checkRange(value); return value>>>0; };
U32.fromJS = function(value) { this.checkRange(value); return value>>>0; };
U64.fromJS = function(value) { throw new Error('u64 not yet supported'); };
F32.fromJS = function(value) { return Math.fround(value); };
F64.fromJS = function(value) { return +value; };

Bool.toJS = function(value) { return value ? true : false; };

Bool.load = function(addr) { return loadu8(addr); };
I8.load = function(addr) { return loadi8(addr); };
I16.load = function(addr) { return loadi16(addr); };
I32.load = function(addr) { return loadi32(addr); };
U8.load = function(addr) { return loadu8(addr); };
U16.load = function(addr) { return loadu16(addr); };
U32.load = function(addr) { return loadu32(addr); };
U64.load = function(addr) { throw new Error('u64 not yet supported'); };
F32.load = function(addr) { return loadf32(addr); };
F64.load = function(addr) { return loadf64(addr); };

Bool.store = function(addr, value) { return storeu8(addr, value); };
I8.store = function(addr, value) { return storei8(addr, value); };
I16.store = function(addr, value) { return storei16(addr, value); };
I32.store = function(addr, value) { return storei32(addr, value); };
U8.store = function(addr, value) { return storeu8(addr, value); };
U16.store = function(addr, value) { return storeu16(addr, value); };
U32.store = function(addr, value) { return storeu32(addr, value); };
U64.store = function(addr, value) { throw new Error('u64 not yet supported'); };
F32.store = function(addr, value) { return storef32(addr, value); };
F64.store = function(addr, value) { return storef64(addr, value); };

function Alias(name, type) {
  var alias = Object.create(type);
  alias.name = name;
  return alias;
}

function PtrType(type) {
  Type.call(this);
  this.pointee = type;
}
PtrType.map = {};
PtrType.prototype = Object.create(Type.prototype);
PtrType.prototype.kind = 'ptr';
PtrType.prototype.prec = 1;
PtrType.prototype.primitive = true;
PtrType.prototype.sig = 'i'
PtrType.prototype.size = 4;
PtrType.prototype.fromJS = function(value) {
  if (value === null) {
    return 0;
  }
  Value.$check(value);
  Type.check(this, value.$ptrType);
  return value.$addr;
};
PtrType.prototype.toJS = function(value) {
  if (value == 0) {
    return null;
  }
  return new Value(this.pointee, value);
};
PtrType.prototype.initAccessors = function(value, addr) {
  var type = this;
  value.load = function() { return type.toJS(loadu32(addr)); };
  value.store = function(value) { storeu32(addr, type.fromJS(value)); };
};

function Ptr(type) {
  if (type.id in PtrType.map) {
    return PtrType.map[type.id];
  }
  var result = new PtrType(type);
  PtrType.map[type.id] = result;
  return result;
}

var VoidPtr = Ptr(Void);
var ArrayLen = Alias('ArrayLen', U32);
var BufLen = Alias('BufLen', U32);
var BufPtr = Alias('BufPtr', VoidPtr);
var Str = Alias('Str', Ptr(U8));
var StrLen = Alias('StrLen', U32);
var StrPtr = Alias('StrPtr', Ptr(U8));
var UserData = Alias('UserData', VoidPtr);

Str.toJS = function(addr) { return loadcstr(addr); };
Str.initAccessors = function(value, addr) {
  value.load = function() { return loadcstr(loadu32(addr)); }
};

function ArrayType(type) {
  Type.call(this);
  this.element = type;
}
ArrayType.map = {};
ArrayType.prototype = Object.create(Type.prototype);
ArrayType.prototype.kind = 'ptr';
ArrayType.prototype.prec = 2;
ArrayType.prototype.primitive = true;
ArrayType.prototype.sig = 'i'
ArrayType.prototype.size = 4;
ArrayType.prototype.initAccessors = function(value, arrayAddr) {
  var type = this;
  value.index = function(index) {
    return new Value(type.element, arrayAddr + index * type.element.size);
  };
};

function ArrayPtr(type) {
  if (type.id in ArrayType.map) {
    return ArrayType.map[type.id];
  }
  var result = new ArrayType(type);
  ArrayType.map[type.id] = result;
  return result;
}

function FnType(result, params) {
  if (!result.primitive) {
    throw new Error(result.toString() + " is not a primitive type");
  }
  for (var i = 0; i < params.length; ++i) {
    var param = params[i];
    if (!param.primitive) {
      throw new Error(param.toString() + " is not a primitive type");
    }
  }
  Type.call(this);
  this.result = result;
  this.params = params;
  this.funcSig = this.generateSig();
}
FnType.generateKey = function(result, params) {
  var key = result.id + ',';
  for (var i = 0; i < params.length; ++i) {
    key += params[i].id + ',';
  }
  return key;
};
FnType.map = {};
FnType.prototype = Object.create(Type.prototype);
FnType.prototype.kind = 'fn';
FnType.prototype.prec = 3;
FnType.prototype.primitive = true;
FnType.prototype.sig = 'i'
FnType.prototype.size = 4;
FnType.prototype.generateSig = function() {
  var result = this.result.sig;
  for (var i = 0; i < this.params.length; ++i) {
    result += this.params[i].sig;
  }
  return result;
};
FnType.prototype.argsFromJS = function(inArgs) {
  if (inArgs.length != this.params.length) {
    throw new Error("argument count mismatch: expecting " + this.params.length +
        ", got " + inArgs.length);
  }
  var outArgs = [];
  for (var i = 0; i < inArgs.length; ++i) {
    outArgs.push(this.params[i].fromJS(inArgs[i]));
  }
  return outArgs;
};
FnType.prototype.argsToJS = function(inArgs) {
  var outArgs = [];
  for (var i = 0; i < inArgs.length; ++i) {
    outArgs.push(this.params[i].toJS(inArgs[i]));
  }
  return outArgs;
};
FnType.prototype.define = function(name) {
  var type = this;
  return function() {
    var result = Module[name].apply(Module, type.argsFromJS(arguments));
    return type.result.toJS(result);
  };
};
FnType.prototype.initAccessors = function(value, addr) {
  var type = this;
  value.load = function() { return new FnValue(type, loadu32(addr)); };
  value.store = function(value) {
    if (!(value instanceof FnValue)) {
      throw new Error('fn value ' + value + ' not instanceof FnValue');
    }
    Type.check(type, value.$type);
    return storeu32(addr, value.$index);
  };
  value.call = function() {
    var result = Runtime.dynCall(type.funcSig, loadu32(addr),
                                 type.argsFromJS(arguments));
    return type.result.toJS(result);
  };
};

function Fn(result, params) {
  var key = FnType.generateKey(result, params);
  if (key in FnType.map) {
    return FnType.map[key];
  }
  var result = new FnType(result, params);
  FnType.map[key] = result;
  return result;
}

function Field(name, type, offset) {
  this.name = name;
  this.type = type;
  this.offset = offset;
}
Field.prototype = Object.create(Object.prototype);
Field.prototype.initAccessors = function(value, structAddr) {
  value[this.name] = new Value(this.type, structAddr + this.offset);
};

function StructType(name) {
  Type.call(this, name);
  this.fields = {};
}
StructType.prototype = Object.create(Type.prototype);
StructType.prototype.kind = 'struct';
StructType.prototype.primitive = false;
StructType.prototype.define = function(cName, fieldTypes) {
  this.cName = cName;
  this.size = sizeof(cName);
  for (fieldName in fieldTypes) {
    var type = fieldTypes[fieldName];
    var offset = offsetof(cName, fieldName);
    this.fields[fieldName] = new Field(fieldName, type, offset);
  }
};
StructType.prototype.describe = function() {
  var result = 'struct ' + this.name + ' {\n';
  var lines = [];
  for (fieldName in this.fields) {
    var field = this.fields[fieldName];
    lines.push('  ' + field.type.toString(fieldName) + ';');
  }
  result += lines.join('\n');
  result += '\n};';
  return result;
};
StructType.prototype.initAccessors = function(value, addr) {
  var type = this;
  for (fieldName in this.fields) {
    var field = this.fields[fieldName];
    field.initAccessors(value, addr);
  }
};

function Struct(name) {
  return new StructType(name);
}

// Values //////////////////////////////////////////////////////////////////////

function Value(type, addr) {
  this.$type = type;
  this.$ptrType = Ptr(type);
  this.$addr = addr;
  type.initAccessors(this, this.$addr);
}
Value.$at = function(type, addr) { return new Value(type, addr); };
Value.$malloc = function(type) { return new Value(type, mallocz(type.size)); };
Value.$check = function(value) {
  if (!(value instanceof Value)) {
    throw new Error('value ' + value + ' not instanceof Value');
  }
};
Value.prototype = Object.create(Object.prototype);
Value.prototype.toString = function() {
  return '<' + this.$type + '>@' + this.$addr;
};
Value.prototype.$free = function() { free(this.$addr); };

function FnValue(type, f) {
  if (!(type instanceof FnType)) {
    throw new Error('type ' + type + ' not instanceof FnType');
  }
  this.$type = type;
  if (typeof f == 'function') {
    if (f.length != type.params.length) {
      throw new Error("argument count mismatch: expecting " + type.params.length +
          ", got " + f.length);
    }
    var wrapped = function() {
      return type.result.fromJS(f.apply(null, type.argsToJS(arguments)));
    };

    this.$index = Runtime.addFunction(wrapped);
    this.$owned = true;
  } else {
    // assume that if is the table index.
    this.$index = f | 0;
    this.$owned = false;
  }
}
FnValue.prototype = Object.create(Object.prototype);
FnValue.prototype.toString = function() {
  return '<' + this.type + '>@' + this.$index;
};
FnValue.prototype.$destroy = function() {
  if (this.$owned) {
    Runtime.removeFunction(this.$index);
  }
};

function StrValue(addr) {
  Value.call(this, Str.pointee, addr);
}
StrValue.$at = function(addr) { return new StrValue(addr); }
StrValue.$mallocCStr = function(s) {
  var addr = malloc(s.length + 1);
  Module.writeAsciiToMemory(s, addr);
  return new StrValue(addr);
}
StrValue.prototype = Object.create(Value.prototype);
StrValue.prototype.toString = function() {
  return '<Str>@' + this.$addr;
};

function BufferValue(addr, size) {
  Value.call(this, VoidPtr.pointee, addr);
  this.size = size;
}
BufferValue.$malloc = function(buf) {
  var addr;
  var size;
  if (buf instanceof ArrayBuffer) {
    size = buf.byteLength;
    addr = malloc(size);
    HEAPU8.set(new Uint8Array(buf), addr);
  } else if (ArrayBuffer.isView(buf)) {
    size = buf.byteLength;
    addr = malloc(size);
    HEAPU8.set(new Uint8Array(buf.buffer, buf.byteOffset, buf.byteLength),
               addr);
  } else if (typeof buf == 'string') {
    size = buf.length;
    addr = malloc(size);
    Module.writeAsciiToMemory(buf, addr, true);  // don't null-terminate
  } else {
    throw new Error('unknown buffer type: ' + buf);
  }
  return new BufferValue(addr, size);
};
BufferValue.prototype = Object.create(Value.prototype);
BufferValue.prototype.toString = function() {
  return '<Buffer>@' + this.$addr + ', ' + this.size + ' bytes';
};

// Wasm enums //////////////////////////////////////////////////////////////////

// WasmResult
var OK = 0;
var ERROR = 1;

// WasmInterpreterResult
// var OK = 0;
var RETURNED = 1;
var TRAP_MEMORY_ACCESS_OUT_OF_BOUNDS = 2;
var TRAP_INTEGER_OVERFLOW = 3;
var TRAP_INTEGER_DIVIDE_BY_ZERO = 4;
var TRAP_INVALID_CONVERSION_TO_INTEGER = 5;
var TRAP_UNDEFINED_TABLE_INDEX = 6;
var TRAP_UNREACHABLE = 7;
var TRAP_INDIRECT_CALL_SIGNATURE_MISMATCH = 8;
var TRAP_MEMORY_SIZE_OVERFLOW = 9;
var TRAP_OUT_OF_MEMORY = 10;
var TRAP_CALL_STACK_EXHAUSTED = 11;
var TRAP_VALUE_STACK_EXHAUSTED = 12;
var TRAP_IMPORT_RESULT_TYPE_MISMATCH = 13;
var TRAP_IMPORT_TRAPPED = 14;

// Wasm low-level types ////////////////////////////////////////////////////////

var I = (function() {

var Allocator = Struct('Allocator');
var AstLexer = Struct('AstLexer');
var BinaryErrorHandler = Struct('BinaryErrorHandler');
var InterpreterExport = Struct('InterpreterExport');
var InterpreterImport = Struct('InterpreterImport');
var InterpreterMemory = Struct('InterpreterMemory');
var InterpreterModule = Struct('InterpreterModule');
var InterpreterThread = Struct('InterpreterThread');
var InterpreterThreadOptions = Struct('InterpreterThreadOptions');
var InterpreterTypedValue = Struct('InterpreterTypedValue');
var InterpreterValue = Struct('InterpreterValue');
var Location = Struct('Location');
var MemoryWriter = Struct('MemoryWriter');
var OutputBuffer = Struct('OutputBuffer');
var ReadBinaryOptions = Struct('ReadBinaryOptions');
var Script = Struct('Script');
var SourceErrorHandler = Struct('SourceErrorHandler');
var StackAllocator = Struct('StackAllocator');
var Stream = Struct('Stream');
var StrSlice = Struct('StrSlice');
var WriteBinaryOptions = Struct('WriteBinaryOptions');
var Writer = Struct('Writer');

var BinaryErrorHandlerCallback = Fn(Void, [U32, Str, UserData]);
var SourceErrorHandlerCallback =
    Fn(Void, [Ptr(Location), Str, StrPtr, StrLen, U32, UserData]);

Allocator.define('allocator', {
  alloc: Fn(VoidPtr, [Ptr(Allocator), U32, U32, Str, I32]),
  realloc: Fn(VoidPtr, [Ptr(Allocator), VoidPtr, U32, U32, Str, I32]),
  free: Fn(VoidPtr, [Ptr(Allocator), VoidPtr, Str, I32]),
  destroy: Fn(Void, [Ptr(Allocator)]),
  mark: Fn(I32, [Ptr(Allocator)]),
  reset_to_mark: Fn(Void, [Ptr(Allocator), I32]),
  print_stats: Fn(Void, [Ptr(Allocator)]),
});

BinaryErrorHandler.define('binary_error_handler', {
  on_error: BinaryErrorHandlerCallback,
  user_data: UserData,
});

InterpreterExport.define('interpreter_export', {
  name: StrSlice,
  func_offset: I32,
});

InterpreterImport.define('interpreter_import', {
  module_name: StrSlice,
  func_name: StrSlice,
  callback: Ptr(
      Fn(I32, [Ptr(InterpreterModule), Ptr(InterpreterImport),
               ArrayLen, ArrayPtr(InterpreterTypedValue),
               Ptr(InterpreterTypedValue), UserData])),
});

InterpreterMemory.define('interpreter_memory', {
  data: BufPtr,
  page_size: U32,
  byte_size: BufLen,
});

InterpreterModule.define('interpreter_module', {
  memory: InterpreterMemory,
  start_func_offset: U32,
});

InterpreterThread.define('interpreter_thread');

InterpreterThreadOptions.define('interpreter_thread_options', {
  value_stack_size: U32,
  call_stack_size: U32,
  pc: U32,
});

InterpreterTypedValue.define('interpreter_typed_value', {
  type: U32,
  value: InterpreterValue,
});

InterpreterValue.define('interpreter_value', {
  i32: U32,
  i64: U64,
  f32_bits: F32, // better to just read these as floats in JS
  f64_bits: F64,
});

Location.define('location', {
  filename: Str,
  line: U32,
  first_column: U32,
  last_column: U32,
});

MemoryWriter.define('memory_writer', {
  base: Writer,
  buf: OutputBuffer,
});

OutputBuffer.define('output_buffer', {
  allocator: Allocator,
  start: BufPtr,
  size: BufLen,
  capacity: U32,
});

ReadBinaryOptions.define('read_binary_options', {
  read_debug_names: Bool,
});

Script.define('script');

SourceErrorHandler.define('source_error_handler', {
  on_error: SourceErrorHandlerCallback,
  source_line_max_length: U32,
  user_data: UserData,
});

StackAllocator.define('stack_allocator', {
  allocator: Allocator,
});

Stream.define('stream', {
  writer: Ptr(Writer),
  result: U32,
  offset: U32,
  log_stream: Ptr(Stream),
});

StrSlice.define('string_slice', {
  start: StrPtr,
  length: StrLen,
});

WriteBinaryOptions.define('write_binary_options', {
  log_stream: Ptr(Stream),
  canonicalize_lebs: Bool,
  remap_locals: Bool,
  write_debug_names: Bool,
});

Writer.define('writer', {
  write_data: Fn(I32, [U32, BufPtr, BufLen, UserData]),
  move_data: Fn(I32, [U32, U32, U32, UserData]),
});

// Wasm low-level functions ////////////////////////////////////////////////////

var checkAst = Fn(I32, [Ptr(AstLexer), Ptr(Script), Ptr(SourceErrorHandler)]).define('_wasm_check_ast');
var closeMemWriter = Fn(Void, [Ptr(MemoryWriter)]).define('_wasm_close_mem_writer');
var defaultBinaryErrorCallback = BinaryErrorHandlerCallback.define('_wasm_default_binary_error_callback');
var defaultSourceErrorCallback = SourceErrorHandlerCallback.define('_wasm_default_source_error_callback');
var destroyAstLexer = Fn(Void, [Ptr(AstLexer)]).define('_wasm_destroy_ast_lexer');
var destroyInterpreterModule = Fn(Void, [Ptr(Allocator), Ptr(InterpreterModule)]).define('_wasm_destroy_interpreter_module');
var destroyInterpreterThread = Fn(Void, [Ptr(Allocator), Ptr(InterpreterThread)]).define('_wasm_destroy_interpreter_thread');
var destroyOutputBuffer = Fn(Void, [Ptr(OutputBuffer)]).define('_wasm_destroy_output_buffer');
var destroyScript = Fn(Void, [Ptr(Script)]).define('_wasm_destroy_script');
var destroyStackAllocator = Fn(Void, [Ptr(StackAllocator)]).define('_wasm_destroy_stack_allocator');
var disassembleModule = Fn(Void, [Ptr(InterpreterModule), Ptr(Stream), U32, U32]).define('_wasm_disassemble_module');
var getInterpreterExportByName = Fn(Ptr(InterpreterExport), [Ptr(InterpreterModule), Str]).define('_wasm_get_interpreter_export_by_name');
var getInterpreterImportByName = Fn(Ptr(InterpreterImport), [Ptr(InterpreterModule), Str, Str]).define('_wasm_get_interpreter_import_by_name');
var getLibcAllocator = Fn(Ptr(Allocator), []).define('_wasm_get_libc_allocator');
var initInterpreterThread = Fn(I32, [Ptr(Allocator), Ptr(InterpreterModule), Ptr(InterpreterThread), Ptr(InterpreterThreadOptions)]).define('_wasm_init_interpreter_thread');
var initMemWriter = Fn(I32, [Ptr(Allocator), Ptr(MemoryWriter)]).define('_wasm_init_mem_writer');
var initStackAllocator = Fn(Void, [Ptr(StackAllocator), Ptr(Allocator)]).define('_wasm_init_stack_allocator');
var initStream = Fn(Void, [Ptr(Stream), Ptr(Writer), Ptr(Stream)]).define('_wasm_init_stream');
var newAstBufferLexer = Fn(Ptr(AstLexer), [Ptr(Allocator), Str, BufPtr, BufLen]).define('_wasm_new_ast_buffer_lexer');
var parseAst = Fn(I32, [Ptr(AstLexer), Ptr(Script), Ptr(SourceErrorHandler)]).define('_wasm_parse_ast');
var pushThreadValue = Fn(I32, [Ptr(InterpreterThread), Ptr(InterpreterValue)]);
var readBinaryInterpreter = Fn(I32, [Ptr(Allocator), Ptr(Allocator), BufPtr, BufLen, Ptr(ReadBinaryOptions), Ptr(BinaryErrorHandler), Ptr(InterpreterModule)]).define('_wasm_read_binary_interpreter');
var runInterpreter = Fn(I32, [Ptr(InterpreterModule), Ptr(InterpreterThread), U32, U32]).define('_wasm_run_interpreter');
var tracePC = Fn(Void, [Ptr(InterpreterModule), Ptr(InterpreterThread), Ptr(Stream)]).define('_wasm_trace_pc');
var writeBinaryScript = Fn(I32, [Ptr(Allocator), Ptr(Writer), Ptr(Script), Ptr(WriteBinaryOptions)]).define('_wasm_write_binary_script');

return {
  // Types
  Allocator: Allocator,
  AstLexer: AstLexer,
  BinaryErrorHandler: BinaryErrorHandler,
  InterpreterExport: InterpreterExport,
  InterpreterImport: InterpreterImport,
  InterpreterModule: InterpreterModule,
  InterpreterThread: InterpreterThread,
  InterpreterThreadOptions: InterpreterThreadOptions,
  InterpreterTypedValue: InterpreterTypedValue,
  Location: Location,
  MemoryWriter: MemoryWriter,
  OutputBuffer: OutputBuffer,
  ReadBinaryOptions: ReadBinaryOptions,
  Script: Script,
  SourceErrorHandler: SourceErrorHandler,
  StackAllocator: StackAllocator,
  Stream: Stream,
  StrSlice: StrSlice,
  WriteBinaryOptions: WriteBinaryOptions,
  Writer: Writer,

  // Functions
  checkAst: checkAst,
  closeMemWriter: closeMemWriter,
  defaultBinaryErrorCallback: defaultBinaryErrorCallback,
  defaultSourceErrorCallback: defaultSourceErrorCallback,
  destroyAstLexer: destroyAstLexer,
  destroyInterpreterModule: destroyInterpreterModule,
  destroyInterpreterThread: destroyInterpreterThread,
  destroyOutputBuffer: destroyOutputBuffer,
  destroyScript: destroyScript,
  destroyStackAllocator: destroyStackAllocator,
  disassembleModule: disassembleModule,
  getInterpreterExportByName: getInterpreterExportByName,
  getInterpreterImportByName: getInterpreterImportByName,
  getLibcAllocator: getLibcAllocator,
  initInterpreterThread: initInterpreterThread,
  initMemWriter: initMemWriter,
  initStackAllocator: initStackAllocator,
  initStream: initStream,
  newAstBufferLexer: newAstBufferLexer,
  parseAst: parseAst,
  pushThreadValue: pushThreadValue,
  readBinaryInterpreter: readBinaryInterpreter,
  runInterpreter: runInterpreter,
  tracePC: tracePC,
  writeBinaryScript: writeBinaryScript,
};

})();

// Helpers for friendly objects ////////////////////////////////////////////////

function define$From(obj) {
  obj.$from = function($) {
    var o = Object.create(obj.prototype);
    o.$ = $;
    return o;
  };
}

function definePrimitiveGetter(proto, name, cName) {
  cName = cName || name;
  Object.defineProperty(proto, name, {
    get: function() { return this.$[cName].load(); }
  });
}

function definePrimitiveGetterSetter(proto, name, cName) {
  cName = cName || name;
  Object.defineProperty(proto, name, {
    get: function() { return this.$[cName].load(); },
    set: function(value) { this.$[cName].store(value); },
  });
}

function defineStrGetter(proto, name, cName) {
  cName = cName || name;
  Object.defineProperty(proto, name, {
    get: function() {
      return this.$[cName].load();
    }
  });
}

// Get a StrSlice, given a pointer and length
function defineStrSliceGetter(proto, name, ptrName, lenName) {
  Object.defineProperty(proto, name, {
    get: function() {
      var offset = this.$[ptrName].load();
      var length = this.$[lenName].load();
      return loadstrslice(offset.$addr, length);
    }
  });
}

// Get a StrSlice, given a StrSlice object
function defineStrSliceObjGetter(proto, name, cName) {
  cName = cName || name;
  Object.defineProperty(proto, name, {
    get: function() {
      var offset = this.$[cName].start.load();
      var length = this.$[cName].length.load();
      return loadstrslice(offset.$addr, length);
    }
  });
}

function defineBufferGetter(proto, name, ptrName, lenName) {
  Object.defineProperty(proto, name, {
    get: function() {
      var offset = this.$[ptrName].load();
      var length = this.$[lenName].load();
      return loadbuffer(offset.$addr, length);
    }
  });
}

// Friendly objects ////////////////////////////////////////////////////////////

// Allocator
function Allocator() {
  this.$ = null;
}
define$From(Allocator);
Allocator.prototype = Object.create(Object.prototype);

// StackAllocator
function StackAllocator(fallbackAllocator) {
  this.$ = Value.$malloc(I.StackAllocator);
  I.initStackAllocator(this.$, fallbackAllocator.$);
  this.allocator = Allocator.$from(this.$.allocator);
}
StackAllocator.prototype = Object.create(Object.prototype);
StackAllocator.prototype.$destroy = function() {
  I.destroyStackAllocator(this.$);
  this.$.$free();
};

// LibcAllocator
LibcAllocator = Allocator.$from(I.getLibcAllocator());

// Writer
function Writer(writeData, moveData) {
  this.$ = Value.$malloc(I.Writer);
  this.$writeData = new FnValue(I.Writer.fields.write_data.type, writeData);
  this.$moveData = new FnValue(I.Writer.fields.move_data.type, moveData);
  this.$.write_data.store(this.$writeData);
  this.$.move_data.store(this.$moveData);
}
define$From(Writer);
Writer.prototype = Object.create(Object.prototype);
Writer.prototype.$destroy = function() {
  this.$moveData.$destroy();
  this.$writeData.$destroy();
  this.$.$free();
};

// MemoryWriter
function MemoryWriter(allocator) {
  this.$ = Value.$malloc(I.MemoryWriter);
  var result = I.initMemWriter(allocator.$, this.$);
  if (result != OK) {
    throw new Error('unable to initialize MemoryWriter');
  }
  this.writer = Writer.$from(this.$.base);
  this.buf = OutputBuffer.$from(this.$.buf);
}
MemoryWriter.prototype = Object.create(MemoryWriter.prototype);
MemoryWriter.prototype.$destroy = function() {
  I.closeMemWriter(this.$);
  this.$.$free();
};

// OutputBuffer
function OutputBuffer() {
  this.$ = null;
}
define$From(OutputBuffer);
OutputBuffer.prototype = Object.create(Object.prototype);
defineBufferGetter(OutputBuffer.prototype, 'buf', 'start', 'size');
defineStrSliceGetter(OutputBuffer.prototype, 'bufStr', 'start', 'size');
definePrimitiveGetter(OutputBuffer.prototype, 'size');
definePrimitiveGetter(OutputBuffer.prototype, 'capacity');
OutputBuffer.prototype.$destroy = function() {
  I.destroyOutputBuffer(this.$);
  this.$.$free();
};

// Stream
function Stream(writer, logStream) {
  this.$ = Value.$malloc(I.Stream);
  I.initStream(this.$, writer.$, logStream ? logStream.$ : null);
}
Stream.prototype = Object.create(Object.prototype);
definePrimitiveGetter(Stream.prototype, 'offset');
definePrimitiveGetter(Stream.prototype, 'result');
Stream.prototype.$destroy = function() {
  this.$.$free();
};

// StringStream
function StringStream() {
  this.$writer = new MemoryWriter(LibcAllocator);
  Stream.call(this, this.$writer.writer, null);
}
StringStream.prototype = Object.create(Object.prototype);
Object.defineProperty(StringStream.prototype, 'string', {
  get: function() {
    return this.$writer.buf.bufStr;
  }
});
StringStream.prototype.$destroy = function() {
  Stream.prototype.$destroy.call(this);
  this.$writer.$destroy();
};

// parseAst
function parseAst(allocator, filename, buffer) {
  var sourceLineMaxLength = 80;
  var astLexer = new AstLexer(allocator, filename, buffer);
  var errorHandler = new SourceErrorHandler(sourceLineMaxLength);
  var script = new Script();
  var result = I.parseAst(astLexer.$, script.$, errorHandler.$);
  script.$allocator = allocator;
  script.$astLexer = astLexer;
  script.$errorHandler = errorHandler;
  if (result != OK) {
    script.$destroy();
    throw new Error('parseAst failed:\n' + errorHandler.errorMessage);
  }
  return script;
}

// AstLexer
function AstLexer(allocator, filename, buffer) {
  this.$filename = StrValue.$mallocCStr(filename);
  this.$buffer = BufferValue.$malloc(buffer);
  this.$ = I.newAstBufferLexer(allocator.$, this.$filename, this.$buffer,
                               this.$buffer.size);
  if (this.$ === null) {
    this.$buffer.$free();
    this.$filename.$free();
    throw new Error('unable to create AstLexer');
  }
}
AstLexer.prototype = Object.create(Object.prototype);
AstLexer.prototype.$destroy = function() {
  I.destroyAstLexer(this.$);
  this.$.$free();
  this.$buffer.$free();
  this.$filename.$free();
}

// SourceErrorHandler
function SourceErrorHandler(sourceLineMaxLength) {
  this.$ = Value.$malloc(I.SourceErrorHandler);
  var wrapper = function(loc, error, sourceLine, sourceLineLength,
                         sourceLineColumnOffset, userData){
    loc = Location.$from(loc);
    sourceLine = loadstrslice(sourceLine.$addr, sourceLineLength);

    var lines = [
      loc.filename + ':' + loc.line + ':' + loc.firstColumn,
      error
    ];
    if (sourceLine.length > 0) {
      var numSpaces = loc.firstColumn - 1 - sourceLineColumnOffset;
      var numCarets =
          Math.min(loc.lastColumn - loc.firstColumn, sourceLine.length);
      lines.push(sourceLine);
      lines.push(' '.repeat(numSpaces) + '^'.repeat(numCarets));
    }
    this.errorMessage += lines.join('\n') + '\n';
  }.bind(this);
  this.$callback =
      new FnValue(I.SourceErrorHandler.fields.on_error.type, wrapper);
  this.$.on_error.store(this.$callback);
  this.$.source_line_max_length.store(sourceLineMaxLength);
  this.errorMessage = '';
}
SourceErrorHandler.prototype = Object.create(Object.prototype);
SourceErrorHandler.prototype.$destroy = function() {
  this.$callback.$destroy();
  this.$.$free();
};

// Location
function Location() {
  this.$ = null;
}
define$From(Location);
Location.prototype = Object.create(Object.prototype);
defineStrGetter(Location.prototype, 'filename');
definePrimitiveGetter(Location.prototype, 'line');
definePrimitiveGetter(Location.prototype, 'firstColumn', 'first_column');
definePrimitiveGetter(Location.prototype, 'lastColumn', 'last_column');
Location.prototype.$destroy = function() {
  this.$.$free();
};

// Script
function Script() {
  this.$ = Value.$malloc(I.Script);
  this.$allocator = null;
  this.$astLexer = null;
  this.$errorHandler = null;
}
Script.prototype = Object.create(Object.prototype);
Script.prototype.check = function() {
  var result = I.checkAst(this.$astLexer.$, this.$, this.$errorHandler.$);
  if (result != OK) {
    throw new Error('check failed:\n' + this.$errorHandler.errorMessage);
  }
};
Script.prototype.toBinary = function(options) {
  var mw = new MemoryWriter(this.$allocator);
  options = new WriteBinaryOptions(this.$allocator, options || {});
  try {
    var result =
        I.writeBinaryScript(this.$allocator.$, mw.writer.$, this.$, options.$);
    if (result != OK) {
      throw new Error('writeBinaryScript failed');
    }
    return {buffer: mw.buf.buf, log: options.log}
  } finally {
    options.$destroy();
    mw.$destroy();
  }
};
Script.prototype.$destroy = function() {
  I.destroyScript(this.$);
  if (this.$errorHandler) this.$errorHandler.$destroy();
  if (this.$astLexer) this.$astLexer.$destroy();
  this.$.$free();
};

// WriteBinaryOptions
function WriteBinaryOptions(allocator, options) {
  this.$ = Value.$malloc(I.WriteBinaryOptions);
  if (options.log) {
    this.$logStream = new StringStream();
    this.$.log_stream.store(this.$logStream.$);
  } else {
    this.$logStream = null;
    this.$.log_stream.store(null);
  }
  var optBool = function(v, def) { return v === undefined ? def : v; };
  this.$.canonicalize_lebs.store(optBool(options.canonicalizeLebs, true));
  this.$.remap_locals.store(optBool(options.remapLocals, true));
  this.$.write_debug_names.store(optBool(options.writeDebugNames, false));
}
WriteBinaryOptions.prototype = Object.create(Object.prototype);
Object.defineProperty(WriteBinaryOptions.prototype, 'log', {
  get: function() {
    return this.$logStream ? this.$logStream.string : '';
  }
});
WriteBinaryOptions.prototype.$destroy = function() {
  if (this.$logStream) {
    this.$logStream.$destroy();
  }
  this.$.$free();
};

// readInterpreterModule
function readInterpreterModule(allocator, memoryAllocator, buf, options) {
  buf = BufferValue.$malloc(buf);
  var errorHandler = new BinaryErrorHandler();
  options = new ReadBinaryOptions(options || {});
  var module = new InterpreterModule();
  try {
    var result =
        I.readBinaryInterpreter(allocator.$, memoryAllocator.$, buf, buf.size,
                                options.$, errorHandler.$, module.$);
    module.$allocator = allocator;
    if (result != OK) {
      module.$destroy();
      throw new Error('readInterpreterModule failed:\n' +
                      errorHandler.errorMessage);
    }
    return module;
  } finally {
    options.$destroy();
    errorHandler.$destroy();
    buf.$free();
  }
};

// BinaryErrorHandler
function BinaryErrorHandler() {
  this.$ = Value.$malloc(I.BinaryErrorHandler);
  this.$callback = new FnValue(
      I.BinaryErrorHandler.fields.on_error.type,
      function(offset, error, userData) {
        this.errorMessage += '@0x' + offset.toString(16) + ': ' + error + '\n';
      }.bind(this));
  this.$.on_error.store(this.$callback);
  this.$.user_data.store(null);
  this.errorMessage = '';
}
BinaryErrorHandler.prototype = Object.create(Object.prototype);
BinaryErrorHandler.prototype.$destroy = function() {
  this.$callback.$destroy();
  this.$.$free();
};

// ReadBinaryOptions
function ReadBinaryOptions(options) {
  this.$ = Value.$malloc(I.ReadBinaryOptions);
  this.$.read_debug_names.store(options.readDebugNames || false);
}
ReadBinaryOptions.prototype = Object.create(Object.prototype);
ReadBinaryOptions.prototype.$destroy = function() {
  this.$.$free();
};

// InterpreterModule
function InterpreterModule() {
  this.$ = Value.$malloc(I.InterpreterModule);
  this.$allocator = null;
  this.memory = InterpreterMemory.$from(this.$.memory);
};
InterpreterModule.prototype = Object.create(Object.prototype);
definePrimitiveGetter(InterpreterModule.prototype,
                      'startFuncOffset',
                      'start_func_offset');
InterpreterModule.prototype.getImportByName = function(module, func) {
  module = StrValue.$mallocCStr(module);
  func = StrValue.$mallocCStr(func);
  var import_ = I.getInterpreterImportByName(this.$, module, func);
  module.$free();
  func.$free();
  if (import_ == 0) {
    throw new Error('invalid import index');
  }
  return InterpreterImport.$from(import_);
};
InterpreterModule.prototype.getExportByName = function(name) {
  name = StrValue.$mallocCStr(name);
  var export_ = I.getInterpreterExportByName(this.$, name);
  name.$free();
  if (export_ == 0) {
    throw new Error('invalid export index');
  }
  return InterpreterExport.$from(export_);
};
InterpreterModule.prototype.run =
    function(thread, numInstructions, callStackReturnTop) {
  var result =
      I.runInterpreter(this.$, thread.$, numInstructions, callStackReturnTop);
  switch (result) {
    case OK:
      return true;
    case RETURNED:
      return false;
    case TRAP_MEMORY_ACCESS_OUT_OF_BOUNDS:
      throw new Error('memory access out of bounds');
    case TRAP_INTEGER_OVERFLOW:
      throw new Error('integer overflow');
    case TRAP_INTEGER_DIVIDE_BY_ZERO:
      throw new Error('divide by zero');
    case TRAP_INVALID_CONVERSION_TO_INTEGER:
      throw new Error('invalid conversion to integer');
    case TRAP_UNDEFINED_TABLE_INDEX:
      throw new Error('undefined table index');
    case TRAP_UNREACHABLE:
      throw new Error('unreachable');
    case TRAP_INDIRECT_CALL_SIGNATURE_MISMATCH:
      throw new Error('indirect call signature mismatch');
    case TRAP_MEMORY_SIZE_OVERFLOW:
      throw new Error('memory size overflow');
    case TRAP_OUT_OF_MEMORY:
      throw new Error('out of memory');
    case TRAP_CALL_STACK_EXHAUSTED:
      throw new Error('call stack exhausted');
    case TRAP_VALUE_STACK_EXHAUSTED:
      throw new Error('value stack exhausted');
    case TRAP_IMPORT_RESULT_TYPE_MISMATCH:
      throw new Error('result type mismatch');
    case TRAP_IMPORT_TRAPPED:
      throw new Error('import trapped');
    default:
      throw new Error('unknown error');
  }
};
InterpreterModule.prototype.disassemble = function(from, to) {
  var stream = new StringStream();
  I.disassembleModule(this.$, stream.$, from, to);
  var result = stream.string;
  stream.$destroy();
  return result;
};
InterpreterModule.prototype.tracePC = function(thread) {
  var stream = new StringStream();
  I.tracePC(this.$, thread.$, stream.$);
  var result = stream.string;
  stream.$destroy();
  return result;
};
InterpreterModule.prototype.$destroy = function() {
  if (this.$allocator) {
    I.destroyInterpreterModule(this.$allocator.$, this.$);
  }
  this.$.$free();
};

// InterpreterMemory
function InterpreterMemory() {
  this.$ = null;
}
define$From(InterpreterMemory);
InterpreterMemory.prototype = Object.create(Object.prototype);
defineBufferGetter(InterpreterMemory.prototype, 'buf', 'data', 'byte_size');
definePrimitiveGetter(InterpreterMemory.prototype, 'byteSize', 'byte_size');
definePrimitiveGetter(InterpreterMemory.prototype, 'pageSize', 'page_size');

// InterpreterExport
function InterpreterExport() {
  this.$ = null;
}
define$From(InterpreterExport);
InterpreterExport.prototype = Object.create(Object.prototype);
defineStrSliceObjGetter(InterpreterExport.prototype, 'name');
definePrimitiveGetter(InterpreterExport.prototype, 'funcOffset', 'func_offset');

// InterpreterImport
function InterpreterImport() {
  this.$ = null;
}
define$From(InterpreterImport);
InterpreterImport.prototype = Object.create(Object.prototype);
defineStrSliceObjGetter(InterpreterImport.prototype, 'moduleName',
                        'module_name');
defineStrSliceObjGetter(InterpreterImport.prototype, 'funcName', 'func_name');
definePrimitiveGetter(InterpreterImport.prototype, 'funcOffset', 'func_offset');

// InterpreterThread
function InterpreterThread(allocator, module, options) {
  this.$allocator = allocator;
  this.$ = Value.$malloc(I.InterpreterThread);
  options = new InterpreterThreadOptions(options || {});
  I.initInterpreterThread(allocator.$, module.$, this.$, options.$);
  options.$destroy();
}
InterpreterThread.prototype = Object.create(Object.prototype);
InterpreterThread.prototype.$destroy = function() {
  I.destroyInterpreterThread(this.$allocator.$, this.$);
  this.$.$free();
};

// InterpreterThreadOptions
function InterpreterThreadOptions(options) {
  this.$ = Value.$malloc(I.InterpreterThreadOptions);
  this.$.value_stack_size.store(options.valueStackSize || 64 * 1024);
  this.$.call_stack_size.store(options.callStackSize || 64 * 1024);
  this.$.pc.store(options.pc || 0);
}
InterpreterThreadOptions.prototype = Object.create(Object.prototype);
InterpreterThreadOptions.prototype.$destroy = function() {
  this.$.$free();
};

// InterpreterValue
function InterpreterValue() {
  this.$ = Value.$malloc(I.InterpreterValue);
}
define$From(InterpreterValue);
InterpreterValue.prototype = Object.create(Object.prototype);
definePrimitiveGetterSetter(InterpreterValue.prototype, 'i32');
definePrimitiveGetterSetter(InterpreterValue.prototype, 'i64');
definePrimitiveGetterSetter(InterpreterValue.prototype, 'f32', 'f32_bits');
definePrimitiveGetterSetter(InterpreterValue.prototype, 'f64', 'f64_bits');
InterpreterValue.prototype.$destroy = function() {
  this.$.$free();
};

// InterpreterTypedValue
function InterpreterTypedValue() {
  this.$ = Value.$malloc(I.InterpreterTypedValue);
  this.value = InterpreterValue.$from(this.$.value);
}
InterpreterTypedValue.prototype = Object.create(Object.prototype);
definePrimitiveGetterSetter(InterpreterTypedValue.prototype, 'type');
InterpreterTypedValue.prototype.$destroy = function() {
  this.$.$free();
};


////////////////////////////////////////////////////////////////////////////////

var resolve = wasm.$resolve;

wasm = {
  ready: wasm.ready,

  // Types
  InterpreterThread: InterpreterThread,
  LibcAllocator: LibcAllocator,
  StackAllocator: StackAllocator,
  // InterpreterTypedValue: InterpreterTypedValue,

  // Functions
  parseAst: parseAst,
  readInterpreterModule: readInterpreterModule,
};

resolve();

};

/*
wasm.ready.then(function() {
  if (false) {
    try {
      var sa = new wasm.StackAllocator(wasm.LibcAllocator);
      var a = sa.allocator;
      var data = '(module\n  (func (result i32)\n    (i32.const 1)))';
      var s = wasm.parseAst(a, 'foo.wast', data);
      s.check();
      var output = s.toBinary({log: true});
      print('log output:\n' + output.log);
      print('output:\n' + output.buffer);
    } catch(e) {
      print('ERROR' + (e.stack ? e.stack : e));
    }
    if (s) s.$destroy();
    if (sa) sa.$destroy();
  }

  if (false) {
    try {
      var a = new wasm.StackAllocator(wasm.LibcAllocator);
      var ma = wasm.LibcAllocator;
      var buf = new Uint8Array([
          0,  97, 115, 109,  11,   0,   0,   0,   4, 116, 121, 112, 101,   8,   2,  64,
          1,   1,   0,  64,   0,   0,   6, 105, 109, 112, 111, 114, 116,  12,   1,   0,
          3, 115, 116, 100,   5, 112, 114, 105, 110, 116,   8, 102, 117, 110,  99, 116,
        105, 111, 110,   2,   1,   1,   6, 101, 120, 112, 111, 114, 116,   4,   1,   0,
          1, 102,   4,  99, 111, 100, 101,   8,   1,   6,   0,  16,  42,  24,   1,   0,
      ]);
      var im = wasm.readInterpreterModule(a.allocator, ma, buf, {});
      var it = new wasm.InterpreterThread(a.allocator, im);
      im.run(it, 1000, 0);
    } catch(e) {
      print('ERROR:' + (e.stack ? e.stack : e));
    }
    if (it) it.$destroy();
    if (im) im.$destroy();
    if (a) a.$destroy();
  }

  if (true) {
    try {
      var sa = new wasm.StackAllocator(wasm.LibcAllocator);
      var a = sa.allocator;
      var source = `
        (module
          (func $test (result i32)
            (call $fib (i32.const 4)))
          (func $fib (param $p i32) (result i32)
            (local $a i32)
            (local $b i32)
            (local $t i32)
            (set_local $a (i32.const 1))
            (set_local $b (i32.const 1))
            (loop $exit $cont
              (set_local $p (i32.sub (get_local $p) (i32.const 1)))
              (br_if $exit (i32.le_s (get_local $p) (i32.const 0)))
              (set_local $t (get_local $b))
              (set_local $b (i32.add (get_local $a) (get_local $b)))
              (set_local $a (get_local $t))
              (br $cont))
            (return (get_local $b)))
          (export "test" $test))
      `;
      var s = wasm.parseAst(a, 'foo.wast', source);
      s.check();
      var output = s.toBinary();
      var im = wasm.readInterpreterModule(a, wasm.LibcAllocator, output.buffer);
      if (true) {
        print('disassemble:\n' + im.disassemble(0, 1000));
      }
      if (true) {
        var it = new wasm.InterpreterThread(a, im);
        print('running');
        do {
          print(im.tracePC(it).trimRight());
        } while (im.run(it, 1, 0));
        print('done');
      }
    } catch(e) {
      print('ERROR:' + (e.stack ? e.stack : e));
    }
    if (it) it.$destroy();
    if (im) im.$destroy();
    if (s) s.$destroy();
    if (sa) sa.$destroy();
  }

}).catch(function(error) {
  print('ERROR:' + (error.stack ? error.stack : error));
});
*/

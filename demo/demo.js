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

var defaultIndent = '  ';
var input = document.getElementById('input');
var output = document.getElementById('output');
var select = document.getElementById('select');

function debounce(f, wait) {
  var lastTime = 0;
  var timeoutId = -1;
  var wrapped = function() {
    var time = +new Date();
    if (time - lastTime < wait) {
      if (timeoutId == -1)
        timeoutId = setTimeout(wrapped, (lastTime + wait) - time);
      return;
    }
    if (timeoutId != -1)
      clearTimeout(timeoutId);
    timeoutId = -1;
    lastTime = time;
    f.apply(null, arguments);
  };
  return wrapped;
}

function insertTextAtSelection(input, src) {
  var selectionStart = input.selectionStart;
  var selectionEnd = input.selectionEnd;
  var oldValue = input.value;
  input.value = oldValue.slice(0, selectionStart) + src +
                oldValue.slice(selectionEnd);
  input.selectionStart = input.selectionEnd = selectionStart + src.length;
}

function onInputKeyDown(e) {
  if (e.keyCode == 9) {  // tab
    insertTextAtSelection(this, defaultIndent);
    e.preventDefault();
  } else if (e.keyCode == 13) {  // newline
    // count nesting depth
    var parens = 0;
    var lastOpen = -1;
    var indent = '';
    for (var i = this.selectionStart - 1; i >= 0; --i) {
      var c = this.value[i];
      if (c == '(') {
        if (--parens < 0) {
          if (lastOpen != -1)
            i = lastOpen;
          else
            indent = defaultIndent;
          break;
        // find first sibling "(", if any
        } else if (parens == 0 && lastOpen == -1) {
          lastOpen = i;
        }
      } else if (c == ')') {
        parens++;
      }
    }
    // get column of current nesting
    var col = 0;
    for (; i > 0; --i) {
      var c = this.value[i];
      if (c == '\n') {
        col--;  // went back too far
        break;
      } else {
        col++;
      }
    }
    // write newline, plus indentation
    insertTextAtSelection(this, '\n' + ' '.repeat(col) + indent);
    e.preventDefault();
  }
}

function onError(loc, error, sourceLine, sourceLineColumnOffset) {
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
  output.textContent += lines.join('\n') + '\n';
}

function compile(text) {
  wasm.ready.then(function() {
    output.textContent = '';
    try {
      var allocator = wasm.LibcAllocator;
      var eh = new wasm.SourceErrorHandler(onError, 80);
      var buf = wasm.Buffer.fromString(text);
      var lexer = wasm.Lexer.fromBuffer(allocator, 'test.wast', buf);
      var script = wasm.parse(lexer, eh);
      wasm.checkAst(lexer, script, eh);
      var memoryWriter = new wasm.MemoryWriter(allocator);
      var jsWriter = new wasm.JSStringWriter();
      var logStream = new wasm.Stream(jsWriter.writer);
      var options = new wasm.WriteBinaryOptions({logStream: logStream});
      wasm.markUsedBlocks(allocator, script);
      wasm.writeBinaryScript(allocator, memoryWriter.base, script, options);
      output.textContent = jsWriter.string;
    } catch (e) {
      output.textContent += e.toString();
    } finally {
      if (options)
        options.$destroy();
      if (logStream)
        logStream.$destroy();
      if (script)
        script.$destroy();
      if (jsWriter)
        jsWriter.$destroy();
      if (memoryWriter)
        memoryWriter.$destroy();
      if (lexer)
        lexer.$destroy();
      if (buf)
        buf.$destroy();
      if (eh)
        eh.$destroy();
    }
  });
}

var compileInput = debounce(function() { compile(input.value); }, 100);

function onInputInput(e) {
  compileInput();
}

var examples = [
  {
    name: 'empty',
    contents: '(module)'
  },

  {
    name: 'simple',
    contents:
      '(module\n' +
      '  (func $addTwo (param i32 i32) (result i32)\n' +
      '    (i32.add\n' +
      '      (get_local 0)\n' +
      '      (get_local 1)))\n' +
      '  (export "addTwo" $addTwo))\n'
  },

  {
    name: 'factorial',
    contents:
      '(module\n' +
      '  (func $fac (param i64) (result i64)\n' +
      '    (if (i64.lt_s (get_local 0) (i64.const 1))\n' +
      '      (then (i64.const 1))\n' +
      '      (else\n' +
      '        (i64.mul\n' +
      '          (get_local 0)\n' +
      '          (call $fac\n' +
      '            (i64.sub\n' +
      '              (get_local 0)\n' +
      '              (i64.const 1)))))))\n' +
      '  (export "fac" $fac))\n'
  },

  {
    name: 'stuff',
    contents:
      '(module\n' +
      '  (memory 1 (segment 0 "hi"))\n' +
      '  (type (func (param i32) (result i32)))\n' +
      '  (import "foo" "bar" (param f32))\n' +
      '  (start 0)\n' +
      '  (table 0 1)\n' +
      '  (func)\n' +
      '  (func (type 0) (i32.const 42))\n' +
      '  (export "e" 1))\n'
  }
];

function setExample(index) {
  var contents = examples[index].contents;
  input.value = contents;
  compileInput();
}

function onSelectChanged(e) {
  setExample(this.selectedIndex);
}

input.addEventListener('keydown', onInputKeyDown);
input.addEventListener('input', onInputInput);
select.addEventListener('change', onSelectChanged);

for (var i = 0; i < examples.length; ++i) {
  var example = examples[i];
  var option = document.createElement('option');
  option.textContent = example.name;
  select.appendChild(option);
}
select.selectedIndex = 1;
setExample(select.selectedIndex);

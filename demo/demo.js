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
var download = document.getElementById('download');
var downloadLink = document.getElementById('downloadLink');
var binaryBlobUrl = null;

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

function compile(text) {
  wabt.ready.then(function() {
    output.textContent = '';
    try {
      var script = wabt.parseWast('test.wast', text);
      script.resolveNames();
      script.validate();
      var binaryOutput = script.toBinary({log: true});
      output.textContent = binaryOutput.log;
      var blob = new Blob([binaryOutput.buffer]);
      if (binaryBlobUrl) {
        URL.revokeObjectURL(binaryBlobUrl);
      }
      binaryBlobUrl = URL.createObjectURL(blob);
      downloadLink.setAttribute('href', binaryBlobUrl);
      download.classList.remove('disabled');
    } catch (e) {
      output.textContent += e.toString();
      download.classList.add('disabled');
    } finally {
      if (script) script.destroy();
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
      '    get_local 0\n' +
      '    get_local 1\n' +
      '    i32.add)\n' +
      '  (export "addTwo" (func $addTwo)))\n'
  },

  {
    name: 'factorial',
    contents:
      '(module\n' +
      '  (func $fac (param i64) (result i64)\n' +
      '    get_local 0\n' +
      '    i64.const 1\n' +
      '    i64.lt_s\n' +
      '    if i64\n' +
      '      i64.const 1\n' +
      '    else\n' +
      '      get_local 0\n' +
      '      get_local 0\n' +
      '      i64.const 1\n' +
      '      i64.sub\n' +
      '      call $fac\n' +
      '      i64.mul\n' +
      '    end)\n' +
      '  (export "fac" (func $fac)))\n'
  },

  {
    name: 'stuff',
    contents:
      '(module\n' +
      '  (import "foo" "bar" (func (param f32)))\n' +
      '  (memory (data "hi"))\n' +
      '  (type (func (param i32) (result i32)))\n' +
      '  (start 1)\n' +
      '  (table 0 1 anyfunc)\n' +
      '  (func)\n' +
      '  (func (type 0)\n' +
      '    i32.const 42\n' +
      '    drop)\n' +
      '  (export "e" (func 1)))\n'
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

function onDownloadClicked(e) {
  // See https://developer.mozilla.com/en-US/docs/Web/API/MouseEvent
  var event = new MouseEvent('click', {
    view: window,
    bubbles: true,
    cancelable: true,
  });
  downloadLink.dispatchEvent(event);
}

input.addEventListener('input', onInputInput);
select.addEventListener('change', onSelectChanged);
download.addEventListener('click', onDownloadClicked);

for (var i = 0; i < examples.length; ++i) {
  var example = examples[i];
  var option = document.createElement('option');
  option.textContent = example.name;
  select.appendChild(option);
}
select.selectedIndex = 1;
setExample(select.selectedIndex);

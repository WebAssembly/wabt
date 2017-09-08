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

var kCompileMinMS = 100;

var editorEl = document.querySelector('.editor');
var outputEl = document.getElementById('output');
var selectEl = document.getElementById('select');
var downloadEl = document.getElementById('download');
var downloadLink = document.getElementById('downloadLink');
var binaryBuffer = null;
var binaryBlobUrl = null;

var wasmInstance = null;

var options = {mode: 'wast', lineNumbers: true};
var editor = CodeMirror.fromTextArea(editorEl, options);

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

function compile(text) {
  wabt.ready.then(function() {
    outputEl.textContent = '';
    try {
      var script = wabt.parseWast('test.wast', text);
      script.resolveNames();
      script.validate();
      var binaryOutput = script.toBinary({log: true});
      outputEl.textContent = binaryOutput.log;
      binaryBuffer = binaryOutput.buffer;
      var blob = new Blob([binaryOutput.buffer]);
      if (binaryBlobUrl) {
        URL.revokeObjectURL(binaryBlobUrl);
      }
      binaryBlobUrl = URL.createObjectURL(blob);
      downloadLink.setAttribute('href', binaryBlobUrl);
      downloadEl.classList.remove('disabled');
    } catch (e) {
      outputEl.textContent += e.toString();
      downloadEl.classList.add('disabled');
    } finally {
      if (script) script.destroy();
    }
  });
}

var compileInput =
    debounce(function() { compile(editor.getValue()); }, kCompileMinMS);

function onEditorChange(e) {
  compileInput();
}

function setExample(index) {
  var contents = examples[index].contents;
  editor.setValue(contents);
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

editor.on('change', onEditorChange);
selectEl.addEventListener('change', onSelectChanged);
downloadEl.addEventListener('click', onDownloadClicked);

for (var i = 0; i < examples.length; ++i) {
  var example = examples[i];
  var option = document.createElement('option');
  option.textContent = example.name;
  selectEl.appendChild(option);
}
selectEl.selectedIndex = 1;
setExample(selectEl.selectedIndex);

/*
 * Copyright 2017 WebAssembly Community Group participants
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


var features = getLocalStorageFeatures()

function getLocalStorageFeatures() {
  try {
    return JSON.parse(localStorage && localStorage.getItem("features")) || { mutable_globals: true };
  } catch (e) {
    console.log(e);
    return { mutable_globals: true };
  }
}

WabtModule().then(function(wabt) {

// TODO: Share feature selection and other code with wat2wasm/demo.js
var FEATURES = [
  'exceptions',
  'mutable_globals',
  'sat_float_to_int',
  'sign_extension',
  'simd',
  'threads',
  'multi_value',
  'tail_call',
  'bulk_memory',
  'reference_types',
];

var editorEl = document.querySelector('.editor');
var uploadEl = document.getElementById('upload');
var selectEl = document.getElementById('select');
var uploadInputEl = document.getElementById('uploadInput');
var generateNamesEl = document.getElementById('generateNames');
var foldExprsEl = document.getElementById('foldExprs');
var inlineExportEl = document.getElementById('inlineExport');
var readDebugNamesEl = document.getElementById('readDebugNames');
var options = {mode: 'wast', lineNumbers: true};
var editor = CodeMirror.fromTextArea(editorEl, options);

var editorContainer = document.querySelector('.CodeMirror.cm-s-default')

editorContainer.ondrop = function(e) {
  e.preventDefault();
  let file = e.dataTransfer.files[0]
  if (!file) {
    return
  }
  readAndCompileFile(file)
}
var fileBuffer = null;
for (var feature of FEATURES) {
  var featureEl = document.getElementById(feature);
  featureEl.checked = !!features[feature]
  featureEl.addEventListener('change', event => {
    var feature = event.target.id;
    features[feature] = event.target.checked;
    compile(fileBuffer);
    localStorage && localStorage.setItem('features', JSON.stringify(features))
  });
}

function compile(contents) {
  if (!contents) {
    return;
  }

  var readDebugNames = readDebugNamesEl.checked;
  var generateNames = generateNamesEl.checked;
  var foldExprs = foldExprsEl.checked;
  var inlineExport = inlineExportEl.checked;

  try {
    var module =
        wabt.readWasm(contents, {readDebugNames: readDebugNames, ...features});
    if (generateNames) {
      module.generateNames();
      module.applyNames();
    }
    var result =
        module.toText({foldExprs: foldExprs, inlineExport: inlineExport});
    editor.setValue(result);
  } catch (e) {
    editor.setValue(e.toString());
  } finally {
    if (module) module.destroy();
  }
}

function onUploadClicked(e) {
  uploadInput.value = '';
  // See https://developer.mozilla.com/en-US/docs/Web/API/MouseEvent
  var event = new MouseEvent('click', {
    view: window,
    bubbles: true,
    cancelable: true,
  });
  uploadInput.dispatchEvent(event);
}

function onUploadedFile(e) {
  var file = e.target.files[0];
  readAndCompileFile(file)
}
// extract common util function
function readAndCompileFile(file) {
  var reader = new FileReader();
  reader.onload = function(e) {
    fileBuffer = new Uint8Array(e.target.result);
    compile(fileBuffer);
  };
  reader.readAsArrayBuffer(file);
}

function recompileIfChanged(el) {
  el.addEventListener('change', function() { compile(fileBuffer); });
}

function setExample(index) {
  var contents = examples[index].contents;
  fileBuffer = contents;
  compile(contents);
}

function onSelectChanged(e) {
  setExample(this.selectedIndex);
}

uploadEl.addEventListener('click', onUploadClicked);
uploadInputEl.addEventListener('change', onUploadedFile);
recompileIfChanged(generateNamesEl);
recompileIfChanged(foldExprsEl);
recompileIfChanged(inlineExportEl);
recompileIfChanged(readDebugNamesEl);
selectEl.addEventListener('change', onSelectChanged);

for (var i = 0; i < examples.length; ++i) {
  var example = examples[i];
  var option = document.createElement('option');
  option.textContent = example.name;
  selectEl.appendChild(option);
}
selectEl.selectedIndex = 0;
setExample(selectEl.selectedIndex);

});

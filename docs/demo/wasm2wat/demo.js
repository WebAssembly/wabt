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


const features = getLocalStorageFeatures();

function getLocalStorageFeatures() {
  try {
    return JSON.parse(localStorage && localStorage.getItem("features")) || {};
  } catch (e) {
    console.log(e);
    return {};
  }
}

WabtModule({
  locateFile: function(url) {
    if (url.endsWith('.wasm')) {
      return '../' + url;
    }
    return url;
  }
}).then(function(wabt) {

const editorEl = document.querySelector('.editor');
const uploadEl = document.getElementById('upload');
const selectEl = document.getElementById('select');
const uploadInputEl = document.getElementById('uploadInput');
const generateNamesEl = document.getElementById('generateNames');
const foldExprsEl = document.getElementById('foldExprs');
const inlineExportEl = document.getElementById('inlineExport');
const checkEl = document.getElementById('check');
const readDebugNamesEl = document.getElementById('readDebugNames');
const options = {mode: 'wast', lineNumbers: true};
const editor = CodeMirror.fromTextArea(editorEl, options);

const editorContainer = document.querySelector('.CodeMirror.cm-s-default');

editorContainer.ondrop = function(e) {
  e.preventDefault();
  let file = e.dataTransfer.files[0];
  if (!file) {
    return;
  }
  readAndCompileFile(file);
}
let fileBuffer = null;
for (const [f, v] of Object.entries(wabt.FEATURES)) {
  const featureEl = document.getElementById(f);
  featureEl.checked = !!(features[f] !== undefined ? features[f] : v);
  featureEl.addEventListener('change', event => {
    const feature = event.target.id;
    features[feature] = event.target.checked;
    compile(fileBuffer);
    if (localStorage) {
      localStorage.setItem('features', JSON.stringify(features))
    }
  });
}

function compile(contents) {
  if (!contents) {
    return;
  }

  const readDebugNames = readDebugNamesEl.checked;
  const check = checkEl.checked;
  const generateNames = generateNamesEl.checked;
  const foldExprs = foldExprsEl.checked;
  const inlineExport = inlineExportEl.checked;

  let module;
  try {
    module = wabt.readWasm(contents, {readDebugNames: readDebugNames, check: check, ...features});
    if (generateNames) {
      module.generateNames();
      module.applyNames();
    }
    const result = module.toText({foldExprs: foldExprs, inlineExport: inlineExport});
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
  const event = new MouseEvent('click', {
    view: window,
    bubbles: true,
    cancelable: true,
  });
  uploadInput.dispatchEvent(event);
}

function onUploadedFile(e) {
  const file = e.target.files[0];
  readAndCompileFile(file);
}
// extract common util function
function readAndCompileFile(file) {
  const reader = new FileReader();
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
  const contents = examples[index].contents;
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

for (let i = 0; i < examples.length; ++i) {
  const example = examples[i];
  const option = document.createElement('option');
  option.textContent = example.name;
  selectEl.appendChild(option);
}
selectEl.selectedIndex = 0;
setExample(selectEl.selectedIndex);

});

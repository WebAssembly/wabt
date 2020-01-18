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
Split(["#top-left", "#top-right"]);
Split(["#bottom-left", "#bottom-right"]);

Split(["#top-row", "#bottom-row"], {
  direction: 'vertical'
});

var features = {};

WabtModule().then(function(wabt) {

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

var kCompileMinMS = 100;

var outputEl = document.getElementById('output');
var jsLogEl = document.getElementById('js_log');
var selectEl = document.getElementById('select');
var downloadEl = document.getElementById('download');
var downloadLink = document.getElementById('downloadLink');
var binaryBuffer = null;
var binaryBlobUrl = null;

for (var feature of FEATURES) {
  var featureEl = document.getElementById(feature);
  features[feature] = featureEl.checked;
  featureEl.addEventListener('change', event => {
    var feature = event.target.id;
    features[feature] = event.target.checked;
    onWatChange();
  });
}

var wasmInstance = null;

var wrappedConsole = Object.create(console);

wrappedConsole.log = (...args) => {
  let line = args.map(String).join('') + '\n';
  jsLogEl.textContent += line;
  console.log(...args);
}

var watEditor = CodeMirror((elt) => {
  document.getElementById('top-left').appendChild(elt);
}, {
  mode: 'wast',
  lineNumbers: true,
});

var jsEditor = CodeMirror((elt) => {
  document.getElementById('bottom-left').appendChild(elt);
}, {
  mode: 'javascript',
  lineNumbers: true,
});

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

function compile() {
  outputEl.textContent = '';
  var binaryOutput;
  try {
    var module = wabt.parseWat('test.wast', watEditor.getValue(), features);
    module.resolveNames();
    module.validate(features);
    var binaryOutput = module.toBinary({log: true, write_debug_names:true});
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
    if (module) module.destroy();
  }
}

function run() {
  jsLogEl.textContent = '';
  if (binaryBuffer === null) return;
  try {
    let wasm = new WebAssembly.Module(binaryBuffer);
    let js = jsEditor.getValue();
    let fn = new Function('wasmModule', 'console', js + '//# sourceURL=demo.js');
    fn(wasm, wrappedConsole);
  } catch (e) {
    jsLogEl.textContent += String(e);
  }
}

var onWatChange = debounce(compile, kCompileMinMS);
var onJsChange = debounce(run, kCompileMinMS);

function setExample(index) {
  var example = examples[index];
  watEditor.setValue(example.contents);
  onWatChange();
  jsEditor.setValue(example.js);
  onJsChange();
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

watEditor.on('change', onWatChange);
jsEditor.on('change', onJsChange);
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

});

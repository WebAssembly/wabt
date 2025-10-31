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

var kCompileMinMS = 100;
var outputShowBase64 = false;
var outputLog;
var outputBase64;

var outputEl = document.getElementById('output');
var jsLogEl = document.getElementById('js_log');
var selectEl = document.getElementById('select');
var downloadEl = document.getElementById('download');
var runEl = document.getElementById('run');
var downloadLink = document.getElementById('downloadLink');
var buildLogEl = document.getElementById('buildLog');
var base64El = document.getElementById('base64');
var binaryBuffer = null;
var binaryBlobUrl = null;

for (const [f, v] of Object.entries(wabt.FEATURES)) {
  var featureEl = document.getElementById(f);
  featureEl.checked = v;
  featureEl.addEventListener('change', event => {
    var feature = event.target.id;
    features[feature] = event.target.checked;
    onWatChange();
  });
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
  outputLog = '';
  outputBase64 = 'Error occured, base64 output is not available';

  var binaryOutput;
  try {
    var module = wabt.parseWat('test.wast', watEditor.getValue(), features);
    module.resolveNames();
    module.validate(features);
    var binaryOutput = module.toBinary({log: true, write_debug_names:true});
    outputLog = binaryOutput.log;
    binaryBuffer = binaryOutput.buffer;
    // binaryBuffer is a Uint8Array, so we need to convert it to a string to use btoa
    // https://stackoverflow.com/questions/12710001/how-to-convert-uint8-array-to-base64-encoded-string
    outputBase64 = btoa(String.fromCharCode.apply(null, binaryBuffer));

    var blob = new Blob([binaryOutput.buffer]);
    if (binaryBlobUrl) {
      URL.revokeObjectURL(binaryBlobUrl);
    }
    binaryBlobUrl = URL.createObjectURL(blob);
    downloadLink.setAttribute('href', binaryBlobUrl);
    downloadEl.classList.remove('disabled');
  } catch (e) {
    outputLog += e.toString();
    downloadEl.classList.add('disabled');
  } finally {
    if (module) module.destroy();
    outputEl.textContent = outputShowBase64 ? outputBase64 : outputLog;
  }
}

let activeWorker = null;
function run() {
  if (activeWorker != null) stop();
  runEl.textContent = 'Stop';
  jsLogEl.textContent = '';
  if (binaryBuffer === null) return;
  const js = jsEditor.getValue();
  activeWorker = new Worker('./worker.js');
  activeWorker.addEventListener('message', function(event) {
    switch (event.data.type) {
      case 'log':
        jsLogEl.textContent += event.data.data;
        break;
      case 'done':
        stop();
        break;
    }
  });
  activeWorker.postMessage({ binaryBuffer: binaryBuffer.buffer, js }, []);
}

function stop() {
  if (activeWorker != null) {
    activeWorker.terminate();
    activeWorker = null;
  }
  runEl.textContent = 'Run';
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

function onRunClicked(e) {
  if (activeWorker != null) {
    stop();
  } else {
    onJsChange();
  }
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

function onBuildLogClicked(e) {
  outputShowBase64 = false;
  outputEl.textContent = outputLog;
  buildLogEl.style.textDecoration = 'underline';
  base64El.style.textDecoration = 'none';
}

function onBase64Clicked(e) {
  outputShowBase64 = true;
  outputEl.textContent = outputBase64;
  buildLogEl.style.textDecoration = 'none';
  base64El.style.textDecoration = 'underline';
}

watEditor.on('change', onWatChange);
jsEditor.on('change', onJsChange);
selectEl.addEventListener('change', onSelectChanged);
runEl.addEventListener('click', onRunClicked);
downloadEl.addEventListener('click', onDownloadClicked);
buildLogEl.addEventListener('click', onBuildLogClicked );
base64El.addEventListener('click', onBase64Clicked );

for (var i = 0; i < examples.length; ++i) {
  var example = examples[i];
  var option = document.createElement('option');
  option.textContent = example.name;
  selectEl.appendChild(option);
}
selectEl.selectedIndex = 1;
setExample(selectEl.selectedIndex);
runEl.classList.remove('disabled');

});

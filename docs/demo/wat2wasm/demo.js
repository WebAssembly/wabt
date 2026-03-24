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
import WabtModule from '../libwabt.js';
import {renderFeatures} from '../share.js';
import {basicSetup, EditorState, EditorView, javascript, Split, StreamLanguage, wast} from '../third_party.bundle.js';

import {examples} from './examples.js';

Split(["#top-left", "#top-right"]);
Split(["#bottom-left", "#bottom-right"]);

Split(["#top-row", "#bottom-row"], {
  direction: 'vertical'
});

const features = {};

const wabt = await WabtModule({
  locateFile: (url) => {
    if (url.endsWith('.wasm')) {
      return '../' + url;
    }
    return url;
  }
});

const kCompileMinMS = 100;
let outputShowBase64 = false;
let outputLog;
let outputBase64;

const outputEl = document.getElementById('output');
const jsLogEl = document.getElementById('js_log');
const selectEl = document.getElementById('select');
const downloadEl = document.getElementById('download');
const runEl = document.getElementById('run');
const downloadLink = document.getElementById('downloadLink');
const buildLogEl = document.getElementById('buildLog');
const base64El = document.getElementById('base64');
let binaryBuffer = null;
let binaryBlobUrl = null;

renderFeatures(wabt, features, () => onWatChange());

const watEditor = new EditorView({
  state: EditorState.create({
    extensions: [
      basicSetup, StreamLanguage.define(wast),
      EditorView.updateListener.of((update) => {
        if (update.docChanged)
          onWatChange();
      })
    ]
  }),
  parent: document.getElementById('top-left')
});

const jsEditor = new EditorView({
  state: EditorState.create({
    extensions: [
      basicSetup, javascript(), EditorView.updateListener.of((update) => {
        if (update.docChanged)
          onJsChange();
      })
    ]
  }),
  parent: document.getElementById('bottom-left')
});

function debounce(f, wait) {
  let lastTime = 0;
  let timeoutId = -1;
  const wrapped = (...args) => {
    const time = +new Date();
    if (time - lastTime < wait) {
      if (timeoutId == -1)
        timeoutId = setTimeout(wrapped, (lastTime + wait) - time);
      return;
    }
    if (timeoutId != -1)
      clearTimeout(timeoutId);
    timeoutId = -1;
    lastTime = time;
    f(...args);
  };
  return wrapped;
}

function compile() {
  outputLog = '';
  outputBase64 = 'Error occured, base64 output is not available';

  let module;
  try {
    module =
        wabt.parseWat('test.wast', watEditor.state.doc.toString(), features);
    module.validate(features);
    const binaryOutput = module.toBinary({log: true, write_debug_names: true});
    outputLog = binaryOutput.log;
    binaryBuffer = binaryOutput.buffer;
    // binaryBuffer is a Uint8Array, so we need to convert it to a string to use
    // btoa
    // https://stackoverflow.com/questions/12710001/how-to-convert-uint8-array-to-base64-encoded-string
    outputBase64 = btoa(String.fromCharCode.apply(null, binaryBuffer));

    const blob = new Blob([binaryOutput.buffer]);
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
    if (module)
      module.destroy();
    outputEl.textContent = outputShowBase64 ? outputBase64 : outputLog;
  }
}

let activeWorker = null;
function run() {
  if (activeWorker != null)
    stop();
  runEl.textContent = 'Stop';
  jsLogEl.textContent = '';
  if (binaryBuffer === null)
    return;
  const js = jsEditor.state.doc.toString();
  activeWorker = new Worker('./worker.js');
  activeWorker.addEventListener('message', (event) => {
    switch (event.data.type) {
      case 'log':
        jsLogEl.textContent += event.data.data;
        break;
      case 'done':
        stop();
        break;
    }
  });
  activeWorker.postMessage({binaryBuffer: binaryBuffer.buffer, js}, []);
}

function stop() {
  if (activeWorker != null) {
    activeWorker.terminate();
    activeWorker = null;
  }
  runEl.textContent = 'Run';
}

const onWatChange = debounce(compile, kCompileMinMS);
const onJsChange = debounce(run, kCompileMinMS);

function setExample(index) {
  const example = examples[index];
  watEditor.dispatch({
    changes: {from: 0, to: watEditor.state.doc.length, insert: example.contents}
  });
  jsEditor.dispatch(
      {changes: {from: 0, to: jsEditor.state.doc.length, insert: example.js}});
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
  const event = new MouseEvent('click', {
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


selectEl.addEventListener('change', onSelectChanged);
runEl.addEventListener('click', onRunClicked);
downloadEl.addEventListener('click', onDownloadClicked);
buildLogEl.addEventListener('click', onBuildLogClicked);
base64El.addEventListener('click', onBase64Clicked);

for (let i = 0; i < examples.length; ++i) {
  const example = examples[i];
  const option = document.createElement('option');
  option.textContent = example.name;
  selectEl.appendChild(option);
}
selectEl.selectedIndex = 1;
setExample(selectEl.selectedIndex);
runEl.classList.remove('disabled');

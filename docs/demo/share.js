/*
 * Copyright 2026 WebAssembly Community Group participants
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

export function getLocalStorageFeatures() {
  try {
    return JSON.parse(localStorage && localStorage.getItem('features')) || {};
  } catch (e) {
    console.log(e);
    return {};
  }
}

export function saveLocalStorageFeatures(features) {
  if (localStorage) {
    localStorage.setItem('features', JSON.stringify(features));
  }
}

export function renderFeatures(wabt, features, onChange) {
  const featuresEl = document.getElementById('features');
  const featuresList =
      Object.entries(wabt.FEATURES).sort(([a], [b]) => a.localeCompare(b));
  for (const [f, v] of featuresList) {
    const featureEl = document.createElement('input');
    featureEl.type = 'checkbox';
    featureEl.id = f;
    const isChecked = !!(features[f] !== undefined ? features[f] : v);
    featureEl.checked = isChecked;
    features[f] = isChecked;
    featureEl.addEventListener('change', event => {
      features[event.target.id] = event.target.checked;
      if (onChange) {
        onChange();
      }
    });
    const labelEl = document.createElement('label');
    labelEl.htmlFor = f;
    labelEl.textContent = f.replace(/_/g, ' ');
    const itemEl = document.createElement('div');
    itemEl.className = 'feature-item';
    itemEl.appendChild(featureEl);
    itemEl.appendChild(labelEl);
    featuresEl.appendChild(itemEl);
  }
}

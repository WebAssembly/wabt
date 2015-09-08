#!/bin/bash
set -o nounset
set -o errexit

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
ROOT_DIR="$(dirname "${SCRIPT_DIR}")"

cd ${ROOT_DIR}/third_party/v8-native-prototype

# copied from v8-native-prototype/install-dependencies.sh, but this fetches a
# shallow clone.
if [[ ! -d depot_tools ]]; then
  echo "Cloning depot_tools"
  git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi

export PATH=$PWD/depot_tools:$PATH

if [[ ! -d v8 ]]; then
  echo "Fetching v8"
  mkdir v8
  cd v8
  fetch --no-history v8
  ln -fs $PWD/.. v8/third_party/wasm
  cd ..
fi

cd v8
gclient update
cd ..

cd v8/v8
GYP_GENERATORS=ninja build/gyp_v8 -Dv8_wasm=1
time ninja -C out/Release d8

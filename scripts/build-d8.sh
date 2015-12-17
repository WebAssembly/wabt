#!/bin/bash
set -o nounset
set -o errexit

config=Release

while [[ $# > 0 ]]; do
  flag="$1"
  case $flag in
    --debug)
      config=Debug
      ;;
    *)
      echo "unknown arg ${flag}"
      ;;
  esac
  shift
done

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
ROOT_DIR="$(dirname "${SCRIPT_DIR}")"

cd ${ROOT_DIR}/third_party/v8

if [[ ! -d depot_tools ]]; then
  echo "Cloning depot_tools"
  git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi

export PATH=$PWD/depot_tools:$PATH
gclient sync

# Don't use the CC from the environment; v8 doesn't seem to build properly with
# it.
unset CC
cd v8
GYP_GENERATORS=ninja build/gyp_v8 -Dv8_wasm=1
time ninja -C out/${config} d8

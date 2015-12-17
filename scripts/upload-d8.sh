#!/bin/bash
set -o nounset
set -o errexit

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
ROOT_DIR="$(dirname "${SCRIPT_DIR}")"
V8_DIR="${ROOT_DIR}/third_party/v8/v8"
V8_OUT_DIR="${V8_DIR}/out/Release"

V8_SHA=$(cd ${V8_DIR} && git rev-parse HEAD)
BUCKET_URL=gs://webassembly/v8-native-prototype/${V8_SHA}/
FILES="d8 natives_blob.bin snapshot_blob.bin"

for file in ${FILES}; do
  gsutil cp -a public-read ${V8_OUT_DIR}/${file} ${BUCKET_URL}
done

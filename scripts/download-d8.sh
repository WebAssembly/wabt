#!/bin/bash
set -o nounset
set -o errexit

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
ROOT_DIR="$(dirname "${SCRIPT_DIR}")"
OUT_DIR="${ROOT_DIR}/out"

V8_SHA=1ef3cf2b0bf96d9f271d42867103778f23ebfb0a
BUCKET_URL=https://storage.googleapis.com/webassembly/v8-native-prototype/${V8_SHA}

Download() {
  local URL=$1
  local FILENAME=$2
  echo "Downloading ${URL}..."
  CURL_ARGS="--fail --location"
  if [ -t 1 ]; then
    # Add --progress-bar but only if stdout is a TTY device.
    CURL_ARGS+=" --progress-bar"
  else
    # otherwise suppress all status output, since curl always
    # assumes a TTY and writes \r and \b characters.
    CURL_ARGS+=" --silent"
  fi
  if which curl > /dev/null ; then
    curl ${CURL_ARGS} -o "${FILENAME}" "${URL}"
  else
    echo "error: could not find 'curl' in your PATH"
    exit 1
  fi
}

mkdir -p "${OUT_DIR}"

for file in d8 natives_blob.bin snapshot_blob.bin; do
  Download ${BUCKET_URL}/${file} "${OUT_DIR}/${file}"
done

# Make d8 executable
chmod ug+x "${OUT_DIR}/d8"

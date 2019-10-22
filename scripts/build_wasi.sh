#!/bin/bash

# Install latest version of wasienv
curl https://raw.githubusercontent.com/wasienv/wasienv/master/install.sh | sh

mkdir -p out/wasi
cd out/wasi

# Make the regular build, wrapping it with wasimake
wasimake cmake -DBUILD_TESTS=OFF ../..
make -j 4

# Strip and optimize the wasm files
wasm-strip wasm-interp.wasm
wasm-strip wasm-strip.wasm
wasm-strip wasm-validate.wasm
wasm-strip wasm2wat.wasm
wasm-strip wast2json.wasm
wasm-strip wat2wasm.wasm

wasm-opt wasm-interp.wasm -o wasm-interp.wasm
wasm-opt wasm-strip.wasm -o wasm-strip.wasm
wasm-opt wasm-validate.wasm -o wasm-validate.wasm
wasm-opt wasm2wat.wasm -o wasm2wat.wasm
wasm-opt wast2json.wasm -o wast2json.wasm
wasm-opt wat2wasm.wasm -o wat2wasm.wasm

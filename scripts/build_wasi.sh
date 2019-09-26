#!/bin/bash

mkdir -p out/wasi
cd out/wasi

# We download the SDK
if [[ ! -d wasi-sdk-5.0 ]]; then
    OS=$(uname | tr '[:upper:]' '[:lower:]')
    case "$OS" in
        darwin) WASI_URL='https://github.com/CraneStation/wasi-sdk/releases/download/wasi-sdk-5/wasi-sdk-5.0-macos.tar.gz';;
        linux) WASI_URL='https://github.com/CraneStation/wasi-sdk/releases/download/wasi-sdk-5/wasi-sdk-5.0-linux.tar.gz';;
        *) printf "$red> The OS (${OS}) is not supported by this installation script.$reset\n"; exit 1;;
    esac
    curl -L -o wasi-sdk-5.0.tar.gz ${WASI_URL}
    tar zxvf wasi-sdk-5.0.tar.gz
fi

cmake -DCMAKE_TOOLCHAIN_FILE=../../wasi-sdk.cmake -DBUILD_TESTS=OFF ../..
make -j 4

# Strip and optimize the wasm files
wasm-strip wasm-strip.wasm
wasm-strip wasm-validate.wasm
wasm-strip wasm2wat.wasm
wasm-strip wast2json.wasm
wasm-strip wat2wasm.wasm

wasm-opt wasm-strip.wasm -o wasm-strip.wasm
wasm-opt wasm-validate.wasm -o wasm-validate.wasm
wasm-opt wasm2wat.wasm -o wasm2wat.wasm
wasm-opt wast2json.wasm -o wast2json.wasm
wasm-opt wat2wasm.wasm -o wat2wasm.wasm

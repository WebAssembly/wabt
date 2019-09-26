#!/bin/bash

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

mkdir -p build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../wasi-sdk.cmake -DBUILD_TESTS=OFF ..
cd ..
make -C build

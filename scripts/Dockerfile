FROM debian:stable-slim

WORKDIR /wabt

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && \
    echo $TZ > /etc/timezone && \
    apt-get update && \
    apt-get install -y git g++ cmake ninja-build python3 file

COPY . .

RUN cmake -G Ninja -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build
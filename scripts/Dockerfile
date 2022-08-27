FROM debian:latest

WORKDIR /ws

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

COPY . .

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && \
    echo $TZ > /etc/timezone && \
    apt-get update && \
    apt-get install -y git g++ cmake ninja-build python3 file && \
    cmake -G Ninja -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

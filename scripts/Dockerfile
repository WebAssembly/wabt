FROM ubuntu:latest

WORKDIR /ws

ARG DEBIAN_FRONTEND=noninteractive
ARG COMPILER_ARCH
ENV TZ=UTC
ENV DISTCC_HOSTS=127.0.0.1
ENV DISTCC_IO_TIMEOUT=480

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && \
    echo $TZ > /etc/timezone && \
    apt-get update && \
    apt-get install -y git distcc cmake ninja-build python3 file g++

RUN mkdir -p /opt/bin/distcc_symlinks && \
    ln -s /usr/bin/distcc /opt/bin/distcc_symlinks/$COMPILER_ARCH-gcc &&\
    ln -s /usr/bin/distcc /opt/bin/distcc_symlinks/$COMPILER_ARCH-g++

ENV PATH=/opt/bin/distcc_symlinks:$PATH

COPY . .

RUN CC=$COMPILER_ARCH-gcc CXX=$COMPILER_ARCH-g++ cmake -G Ninja -S . -B build -DCMAKE_BUILD_TYPE=Release -DTEST_TIMEOUT=$((120*8)) && \
    cmake --build build

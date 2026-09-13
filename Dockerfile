FROM ubuntu:22.04 AS dev

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    g++ \
    gdb \
    git \
    cmake \
    make \
    flex \
    libfl-dev \
    bison \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

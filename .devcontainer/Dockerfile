FROM ghcr.io/qbos07/cp-dockercross:latest AS toolchain

FROM mcr.microsoft.com/devcontainers/base:debian AS base

# Install required packages KEEP IN SYNC (+clangd bear)
RUN apt-get update -y && apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
    make libncurses6 zstd zlib1g \
    gawk wget bzip2 xz-utils unzip \
    patch libstdc++6 rsync git clangd bear
RUN apt-get install -y --reinstall ca-certificates
COPY --from=toolchain /toolchain /toolchain
ENV PATH=$PATH:/toolchain/bin

FROM ubuntu:18.04
RUN apt-get update \
    && apt-get upgrade -y \
    && apt-get install -y \
      g++ \
      gcc \
      gdb \
      make \
      automake \
      libtool \
      libnuma-dev \
      libc-dev \
      groff \
      git \
      wget \
      binutils-dev \
      python \
      libgsl-dev \
      libjpeg-dev \
      vim-tiny \
      libgoogle-perftools-dev \
      cmake \
    && rm -rf /var/lib/apt/lists/*

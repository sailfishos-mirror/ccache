ARG BASE_IMAGE=ubuntu:24.04
FROM ${BASE_IMAGE} AS build

# Non-interactive: do not set up timezone settings.
ARG GCC_APT="gcc-multilib"
RUN apt-get update \
 && DEBIAN_FRONTEND="noninteractive" apt-get install -y --no-install-recommends \
        asciidoctor \
        bash \
        build-essential \
        ccache \
        clang \
        cmake \
        docbook-xml \
        docbook-xsl \
        elfutils \
        ${GCC_APT} \
        g++-13 \
        g++-14 \
        libhiredis-dev \
        libzstd-dev \
        python3 \
        redis-server \
        redis-tools \
 && rm -rf /var/lib/apt/lists/*

# Redirect all compilers to ccache.
RUN for t in gcc g++ cc c++ clang clang++; do ln -vs /usr/bin/ccache /usr/local/bin/$t; done

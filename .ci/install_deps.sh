#!/bin/sh

install_deps() {
    if [ "${CI_DEPS_SKIP_INSTALL-0}" -eq 1 ]; then
        return
    fi

    if [ "$(id -u)" -ne 0 ]; then
        SUDO=sudo
    else
        SUDO=
    fi

    export DEBIAN_FRONTEND=noninteractive
    $SUDO apt-get update
    $SUDO apt-get install -y \
        git \
        cmake \
        ninja-build \
        clang \
        qt6-base-dev \
        clang-format \
        clang-tidy
}

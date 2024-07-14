#!/bin/sh

install_tools() {
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
        clang-format
}

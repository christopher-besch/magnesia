#!/bin/sh

setup_nix() {
    set -xeu

    if [ "$(id -u)" -ne 0 ]; then
        SUDO="sudo --preserve-env=DEBIAN_FRONTEND"
    else
        SUDO=
    fi

    export DEBIAN_FRONTEND=noninteractive
    $SUDO apt-get update
    $SUDO apt-get install -y nix git
    $SUDO chown -R "$(id -u):$(id -g)" /nix
    echo "experimental-features = nix-command flakes" | $SUDO tee /etc/nix/nix.conf
}

setup_nix

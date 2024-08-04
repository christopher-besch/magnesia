#!/bin/sh

set -xeu

if [ "$(id -u)" -ne 0 ]; then
    SUDO="sudo --preserve-env=DEBIAN_FRONTEND"
else
    SUDO=
fi

export DEBIAN_FRONTEND=noninteractive
$SUDO apt-get update

if [ $# -gt 0 ] && [ "$1" = "nix" ]; then
    $SUDO apt-get install -y git nix
    $SUDO chown -R "$(id -u):$(id -g)" /nix
    echo "experimental-features = nix-command flakes" | $SUDO tee /etc/nix/nix.conf
else
    $SUDO apt-get install -y git cmake build-essential libgl1-mesa-dev qt6-base-dev ninja-build clang doxygen
fi

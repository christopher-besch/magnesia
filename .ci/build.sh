#!/bin/sh

set -xeu

mkdir -p build
BUILD_DIR="$(mktemp -dp build ci.XXXXXXXXX)"
trap 'rm -rf "'"$BUILD_DIR"'"' EXIT

cmake -G Ninja -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

ctest --test-dir "$BUILD_DIR" --build-config "$BUILD_TYPE" --output-on-failed

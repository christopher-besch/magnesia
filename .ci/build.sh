#!/bin/sh

set -xeu

BUILD_DIR=build/ci
trap 'rm -rf "'"$BUILD_DIR"'"' EXIT

cmake -G Ninja -B $BUILD_DIR -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build $BUILD_DIR --config "$BUILD_TYPE"

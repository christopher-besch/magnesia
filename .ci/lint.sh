#!/bin/sh

run_cmake_format() {
    find . \( -name 'CMakeLists.txt' -or -name '*.cmake' \) -and -not -path './build/*' -print0 \
        | xargs -0 cmake-format --check
}

run_clang_format() {
    find src -name '*.[ch]pp' -print0 | xargs -0 clang-format -Werror --dry-run --verbose
}

run_clang_tidy() {
    cmake --fresh -G Ninja -B build/clang-tidy . \
        -D CMAKE_BUILD_TYPE=Debug \
        -D CMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -D CMAKE_C_COMPILER=clang \
        -D CMAKE_CXX_COMPILER=clang++ \
        -D MAGNESIA_WARNINGS_AS_ERRORS=ON \
        -D MAGNESIA_BUILD_DOCS=OFF

    find src -name '*.[ch]pp' -print0 \
        | xargs -0 run-clang-tidy -warnings-as-errors='*' -use-color -p build/clang-tidy
}

main() {
    set -xeu

    run_cmake_format
    run_clang_format
    run_clang_tidy
}

main

#!/bin/sh

run_cmake_format() {
    find . \( -name 'CMakeLists.txt' -or -name '*.cmake' \) -and -not -path './build/*' -print0 \
        | xargs -0 cmake-format --check
}

run_clang_format() {
    find src -name '*.[ch]pp' -print0 | xargs -0 clang-format -Werror --dry-run --verbose
}

run_clang_tidy() {
    mkdir -p build
    BUILD_DIR="$(mktemp -dp build clang-tidy.XXXXXXXXX)"
    trap 'rm -rf "'"$BUILD_DIR"'"' EXIT

    cmake --fresh -G Ninja -B "$BUILD_DIR" . \
        -D CMAKE_BUILD_TYPE=Debug \
        -D CMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -D CMAKE_C_COMPILER=clang \
        -D CMAKE_CXX_COMPILER=clang++ \
        -D MAGNESIA_WARNINGS_AS_ERRORS=ON \
        -D MAGNESIA_BUILD_DOCS=OFF

    find src -name '*.[ch]pp' -print0 \
        | xargs -0 run-clang-tidy -warnings-as-errors='*' -use-color -p "$BUILD_DIR"
}

run_codespell() {
    codespell
}

fast() {
    run_cmake_format
    run_clang_format
    run_codespell
}

slow() {
    run_clang_tidy
}

main() {
    set -xeu

    if [ $# -ge 1 ]; then
        if [ "$1" = "fast" ]; then
            fast
        else
            "run_$(printf '%s' "$1" | tr '-' '_')"
        fi
    else
        fast
        slow
    fi
}

main "$@"

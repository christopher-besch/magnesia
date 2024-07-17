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

    # NOTE: open62541 generates headers during build. This means we have to build open62541 for clang-tidy to find all
    # includes. This is not an issue if the library is provided by the system.
    cmake --build build/clang-tidy --target libopen62541.a || true

    # NOTE: The Qt version in ubuntu (6.4) is too old and doesn't yet contain the "IWYU pragma: export" in the public
    # headers, so don't treat false-positive include-cleaner warnings as errors for now.
    find src -name '*.[ch]pp' -print0 \
        | xargs -0 run-clang-tidy -warnings-as-errors='*,-misc-include-cleaner' -use-color -p build/clang-tidy
}

main() {
    set -xeu

    run_cmake_format
    run_clang_format
    run_clang_tidy
}

main

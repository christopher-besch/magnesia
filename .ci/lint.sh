#!/bin/sh

FAILED=0
STEP_MESSAGES=

fail() {
    FAILED=1
    STEP_MESSAGES="${STEP_MESSAGES}  Step \"$1\" failed with code $2
"
}

run_cmake_format() {
    find . \( -name 'CMakeLists.txt' -or -name '*.cmake' \) -and -not -path './build/*' -print0 \
        | xargs -0 cmake-format --check || fail cmake-format $?
}

run_clang_format() {
    find src -name '*.[ch]pp' -print0 | xargs -0 clang-format -Werror --dry-run --verbose || fail clang-format $?
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

    # NOTE: open62541 generates headers during build. This means we have to build open62541 for clang-tidy to find all
    # includes if using CMake's FetchContent to build it. This is not an issue if the library is provided by the system,
    # so ignore if the target doesn't exist.
    cmake --build "$BUILD_DIR" --target libopen62541.a || true

    find src -name '*.[ch]pp' -print0 \
        | xargs -0 run-clang-tidy -warnings-as-errors='*' -use-color -p "$BUILD_DIR" || fail clang-tidy $?
}

run_codespell() {
    codespell || fail codespell $?
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

    if [ $FAILED -eq 0 ]; then
        printf "\nSUMMARY: All jobs succesfull\n"
    else
        printf "\nSUMMARY:\n%s" "$STEP_MESSAGES"
    fi
    return $FAILED
}

main "$@"

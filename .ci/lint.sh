#!/bin/sh

. "$(dirname "$(readlink -f "$0")")/install_deps.sh"

run_clang_format() {
    find src -name '*.[ch]pp' -print0 | xargs -0 clang-format -Werror --dry-run --verbose
}

main() {
    set -xeu

    install_tools

    run_clang_format
}

main

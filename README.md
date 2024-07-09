# Magnesia

### Install Prerequisites
#### Debian
- `sudo apt-get update`
- `sudo apt-get install -y git python3 cmake build-essential libgl1-mesa-dev qt6-base-dev ninja-build clang doxygen`

#### Arch Linux
- `pacman -S python git cmake base-devel clang qt6-base ninja doxygen`

#### Nix
This repository contains a nix flake. Make sure you have flakes enabled and run `nix develop`. You can also use
[direnv](https://direnv.net/), if you have it.

### Build
- `cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ .`
- `cmake --build build --config Release`

### CMake Options
- `MAGNESIA_BUILD_DOCS` (default: `ON`): Whether to build code documentation with doxygen

## Dependencies
- compiler:
    - gcc 12.2.0 or newer
    - clang 14.0.6 or newer
    <!-- - TODO: msvc -->
- cmake 3.25.0 or newer
- Qt 6.4.2 or newer
- open62541pp 0.13.0

## Contributing to Magnesia
See [CONTRIBUTING.md](CONTRIBUTING.md).

# Magnesia

### Install Prerequisites
#### Debian
- `sudo apt-get update`
- `sudo apt-get install -y git python3 cmake build-essential libgl1-mesa-dev qt6-base-dev ninja-build clang doxygen libssl-dev`

#### Arch Linux
- `pacman -S python git cmake base-devel clang qt6-base ninja doxygen`

#### Nix
This repository contains a nix flake. Make sure you have flakes enabled and run `nix develop`. You can also use
[direnv](https://direnv.net/), if you have it.

### Windows
- [install chocolatey](https://chocolatey.org/install)
- `choco install -y cmake python3 doxygen.install openssl sphinx`
- [install Visual Studio](https://visualstudio.microsoft.com/downloads)
- [install Qt6.7.2](https://doc.qt.io/qt-6/get-and-install-qt.html) for MSVC
- restart

### Build
#### On Linux (gcc)
- `cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ .`
- `cmake --build build --config Release`

#### On Linux (clang)
- `cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .`
- `cmake --build build --config Release`

### On Nix/macOS
- `nix build`

#### On Windows
- You may not have any spaces in the path to the repo. This is because Qt's install script fails otherwise.
- `cmake -B build -DCMAKE_BUILD_TYPE=Release -G  'Visual Studio 17 2022' -A x64 -D CMAKE_PREFIX_PATH=C:\Qt\6.7.2\msvc2019_64 .` Adjust the qt path to your needs.
- `cmake --build build --config Release`
- `cmake --install build --prefix C:\somepath --config Release` Adjust the path to your liking.
- `C:\somepath\bin\magnesia.exe` Adjust the path.

### CMake Options
- `MAGNESIA_BUILD_DOCS` (default: `ON`): Whether to build code documentation with doxygen

## Dependencies
- compiler:
    - gcc 12.2.0 or newer
    - clang 14.0.6 or newer
    - Microsoft Visual Studio 17 2022 with Toolset v143
- cmake 3.25.0 or newer
- Qt 6.4.2 or newer
- open62541pp 0.13.0

## Contributing to Magnesia
See [CONTRIBUTING.md](CONTRIBUTING.md).

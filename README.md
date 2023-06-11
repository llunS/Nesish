# Nesish

A work-in-progress NES emulator.

Any feedback is welcome.

## Table of Contents

- [Status](#status)
- [TODO](#todo)
- [Build From Source](#build-from-source)
  - [Desktop](#desktop)
  - [Web](#web)
- [Acknowledgments](#acknowledgments)

## Status

- Windows, MacOS and [Web](https://lluns.github.io/Nesish/)
- Mapper 0 games (Donkey Kong, Super Mario Bros. and Galaga were tested)
- Custom key mapping
- Sufficiently tested CPU, PPU and APU
- PPU Viewer
- C API for emulator core

## TODO

- Performance and audio quality
- More mappers
- Debug tools
- etc.

## Build From Source

> For stability, use tagged revision.

### Desktop

Prerequisites:

- C++11 compiler and C compiler
- CMake
- vcpkg

Example:

```bash
# configure and generate
cmake -S shell -B build -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake -GNinja -DCMAKE_BUILD_TYPE=Release
# build
cmake --build build
```

### Web

Prerequisites:

- [emsdk](https://github.com/emscripten-core/emsdk)
- CMake
- Ninja

Example:

```bash
# configure and generate
cmake -S shell -B build -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
# build
cmake --build build
```

## Acknowledgments

Sincere thanks to the NesDev community (including and not limited to the great Wiki and the Discord server), wouldn't have made it without your generous help.

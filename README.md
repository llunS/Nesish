# Nesish

A work-in-progress NES emulator.

Any feedback is welcome.

## Table of Contents

- [Status](#status)
- [How To Build](#how-to-build)
- [External Dependencies](#external-dependencies)
- [Acknowledgments](#acknowledgments)

## Status

- Windows/MacOS
- Mapper 0 games (Donkey Kong, Super Mario Bros. and Galaga were tested)
- Custom key mapping
- Sufficiently tested CPU, PPU and APU
- PPU Debugger
- C API for emulator core

## How To Build

Prerequisites:

- C++11 compiler and C compiler
- CMake (>=3.25.1)
- vcpkg

Example:

```bash
# configure and generate
cmake -S shell -B build -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake -GNinja -DCMAKE_BUILD_TYPE=Release
# build
cmake --build build
```

> For stability, use tagged revision.

## External Dependencies

D: Development only

- base
  - gtest (D)
- core
  - gtest (D)
  - fmt
- shell
  - spdlog
  - cli11
  - glfw3
  - imgui
  - rtaudio
  - blip_buf
  - glad
  - [ImGuiFileBrowser](https://github.com/gallickgunner/ImGui-Addons)

## Acknowledgments

Sincere thanks to the NesDev community (including and not limited to the great Wiki and the Discord server), wouldn't have come this far without your generous help.

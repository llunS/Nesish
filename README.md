# Nesish

A work-in-progress NES emulator.

Any feedback is welcome.

## Table of Contents

- [Status](#status)
- [How To Play](#how-to-play)
- [How To Build](#how-to-build)
- [External Dependencies](#external-dependencies)
- [Acknowledgments](#acknowledgments)

## Status

- Support on Windows/MacOS
- Mapper 0 games (Donkey Kong, Super Mario Bros. and Galaga are tested)
- Custom key mapping
- Extensively tested CPU and PPU
- Working APU (more work to be done for sound quality)
- Debugger showing real-time status of PPU
- C API for emulator core

## How To Play

Currently there is only command line entrance, open a terminal and type `[path to Nesish] --help` for details.

Upon first launch, keymap config file is generated at `config/input.lua`, check it for controls and you are free to modify it afterwards.

## How To Build

Prerequisites:

- C++11 compiler and C compiler
- CMake (>=3.25.1)
- vcpkg

Example:

```bash
# configure and generate
cmake -S shell -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_TOOLCHAIN_FILE -GNinja -DCMAKE_BUILD_TYPE=Release
# build
cmake --build build
```

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
  - lua 5.1.5

## Acknowledgments

Sincere thanks to the NesDev community (including and not limited to the great Wiki and the Discord server), wouldn't have come this far without your generous help.

cmake_minimum_required(VERSION 3.15)
project(LightNES)


# --------------------------------------------
# Global compiler/linker options/flags
# --------------------------------------------

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# For convenience
set(BUILD_SHARED_LIBS OFF)

# Warnings as errors
if (MSVC)
    add_compile_options(
        /W4 # warning level 4 is enough
        /WX
        /analyze # enable code analysis warnings
    )
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# Suppress some warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    add_compile_options(-Wno-unused-private-field)
endif()


# --------------------------------------------
# Target platform
# --------------------------------------------

# Default to CMake target platform.
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(LT_TARGET_MACOS ON)
else()
    message(FATAL_ERROR "Add platform support please!")
endif()


# --------------------------------------------
# CMake options
# --------------------------------------------

option(LN_BUILD_TESTS "Build unit tests" ON)


# --------------------------------------------
# Sub modules
# --------------------------------------------

if(LN_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(common)


# --------------------------------------------
# Source files
# --------------------------------------------

add_executable(LightNES)

list(APPEND Sources main.cpp)

list(APPEND Sources console/cpu/cpu.cpp)
list(APPEND Sources console/emulator.cpp)
list(APPEND Sources console/memory.cpp)
list(APPEND Sources console/cpu/opcode_exec.cpp)
list(APPEND Sources console/cpu/address_mode_parse.cpp)
list(APPEND Sources console/cpu/instr_map.cpp)
list(APPEND Sources console/cpu/operand.cpp)

list(APPEND Sources cartridge/cartridge_loader.cpp)
list(APPEND Sources cartridge/ines.cpp)

target_sources(LightNES PRIVATE ${Sources})


# --------------------------------------------
# Include directories
# --------------------------------------------

target_include_directories(LightNES PRIVATE .)


# --------------------------------------------
# Dependencies
# --------------------------------------------

find_package(CLI11 CONFIG REQUIRED)

target_link_libraries(LightNES PRIVATE Common
    CLI11::CLI11
)

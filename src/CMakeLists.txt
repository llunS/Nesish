cmake_minimum_required(VERSION 3.15)
project(LightNES)


# --------------------------------------------
# Global compiler/linker options/flags
# --------------------------------------------

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
# Source files
# --------------------------------------------

add_executable(LightNES main.cpp)

list(APPEND Sources console/cpu.cpp)
list(APPEND Sources console/emulator.cpp)
list(APPEND Sources console/memory.cpp)
list(APPEND Sources console/instruction/instruction.cpp)
list(APPEND Sources console/instruction/opcode_exec.cpp)
list(APPEND Sources console/instruction/operand_get.cpp)

target_sources(LightNES PRIVATE ${Sources})


# --------------------------------------------
# Include directories
# --------------------------------------------

target_include_directories(LightNES PRIVATE .)


# --------------------------------------------
# Dependencies
# --------------------------------------------

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(LightNES PRIVATE spdlog::spdlog_header_only)

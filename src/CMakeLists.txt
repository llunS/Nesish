cmake_minimum_required(VERSION 3.15)
project(LightNES)


# --------------------------------------------
# Global compiler/linker options/flags
# --------------------------------------------

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake/Modules)

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
add_subdirectory(console)


# --------------------------------------------
# Source files
# --------------------------------------------

add_executable(LightNES)

list(APPEND sources main.cpp)

target_sources(LightNES PRIVATE ${sources})


# --------------------------------------------
# Include directories
# --------------------------------------------

target_include_directories(LightNES PRIVATE .)


# --------------------------------------------
# Dependencies
# --------------------------------------------

find_package(CLI11 CONFIG REQUIRED)

target_link_libraries(LightNES PRIVATE
    Common
    Console
    CLI11::CLI11
)


# --------------------------------------------
# Tests
# --------------------------------------------

if(LN_BUILD_TESTS)
    add_subdirectory(tests)
endif()

cmake_minimum_required(VERSION 3.25.1)
project(NesishBase)


# -- Target platform

# Default to CMake target platform.
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(NB_TGT_MACOS ON)
elseif(WIN32)
    set(NB_TGT_WINDOWS ON)
else()
    message(FATAL_ERROR "Platform not supported yet!")
endif()


# -- Global compiler/linker options/flags

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Put binary outputs to single directory
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

# Add cmake scripts
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)


# -- CMake options

option(NB_BUILD_TESTS "Build tests" ON)


# -- Target

# Enable test
if(NB_BUILD_TESTS)
    enable_testing()
endif()

set(tgt_name NesishBase)
add_library(${tgt_name} SHARED)
include(target_utils)
add_cxx(${tgt_name} 11)
add_warnings(${tgt_name})
add_opts(${tgt_name})

include(gen_dllexport_header)
gen_dllexport_header(${tgt_name} ${CMAKE_CURRENT_SOURCE_DIR}/public/nhbase NB)

# --- Include directories

target_include_directories(${tgt_name} PUBLIC public)
target_include_directories(${tgt_name} PRIVATE src)

# --- Source files

set(sources "")

list(APPEND sources src/path.cpp)
list(APPEND sources src/filesystem.cpp)
if(NB_TGT_MACOS)
    list(APPEND sources src/path_macos.mm)
    list(APPEND sources src/filesystem_macos.cpp)
elseif(NB_TGT_WINDOWS)
    list(APPEND sources src/path_windows.cpp)
    list(APPEND sources src/filesystem_windows.cpp)
endif()

target_sources(${tgt_name} PRIVATE ${sources})


# -- Tests

if(NB_BUILD_TESTS)
    add_subdirectory(tests)
endif()
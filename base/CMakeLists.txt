cmake_minimum_required(VERSION 3.25.1)
project(NesishBase)

# -- Target platform

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(NB_TGT_WEB ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
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
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake)

# -- CMake options

option(NB_BUILD_TESTS "Build tests" OFF)

# -- Target

# Enable test
if(NB_BUILD_TESTS AND NOT NB_TGT_WEB)
    enable_testing()
endif()

set(tgt_name NesishBase)
if(NB_TGT_WEB)
    add_library(${tgt_name} STATIC)
else()
    add_library(${tgt_name} SHARED)
endif()
include(target_utils)
configure_cxx(${tgt_name} 11)
configure_warnings(${tgt_name})
if(NB_TGT_WEB)
    configure_em_options(${tgt_name})
else()
    configure_optimizations(${tgt_name})
endif()

include(gen_api_macro)
gen_api_macro(${tgt_name} NB ${CMAKE_CURRENT_SOURCE_DIR}/public/nhbase)

# --- Include directories

target_include_directories(${tgt_name} PUBLIC public)
target_include_directories(${tgt_name} PRIVATE src)

# --- Source files

set(sources "")

list(APPEND sources src/path.cpp)
list(APPEND sources src/filesystem.cpp)
if(NB_TGT_WEB)
    list(APPEND sources src/path_web.cpp)
    list(APPEND sources src/filesystem_web.cpp)
elseif(NB_TGT_MACOS)
    list(APPEND sources src/path_macos.mm)
    list(APPEND sources src/filesystem_macos.cpp)
elseif(NB_TGT_WINDOWS)
    list(APPEND sources src/path_windows.cpp)
    list(APPEND sources src/filesystem_windows.cpp)
endif()

target_sources(${tgt_name} PRIVATE ${sources})

# -- Tests

if(NB_BUILD_TESTS AND NOT NB_TGT_WEB)
    add_subdirectory(tests)
endif()

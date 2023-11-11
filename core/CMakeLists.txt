cmake_minimum_required(VERSION 3.25.1)
project(Nesish C)

# -- Target platform

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(NH_TGT_WEB ON)
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

option(NH_BUILD_TESTS "Build tests" OFF)

# -- Target

# Enable test
if(NH_BUILD_TESTS AND NOT NH_TGT_WEB)
    enable_testing()
endif()

set(tgt_name Nesish)
if(NH_TGT_WEB)
    add_library(${tgt_name} STATIC)
else()
    add_library(${tgt_name} SHARED)
endif()
set_target_properties(${tgt_name} PROPERTIES OUTPUT_NAME NesishCore)
include(target_utils)
configure_c(${tgt_name} 11)
configure_warnings(${tgt_name})
configure_vc_options(${tgt_name} /wd6285)
if(NH_TGT_WEB)
    configure_em_options(${tgt_name})
else()
    configure_optimizations(${tgt_name})
endif()

include(gen_api_macro)
gen_api_macro(${tgt_name} NH ${CMAKE_CURRENT_SOURCE_DIR}/include/nesish)

# --- Include directories

target_include_directories(${tgt_name} PUBLIC include)
target_include_directories(${tgt_name} PRIVATE src)

# --- Source files

set(sources "")

list(APPEND sources src/console.c)

list(APPEND sources src/cartridge/cartloader.c)
list(APPEND sources src/cartridge/ines.c)
list(APPEND sources src/cartridge/inesromaccessor.c)
list(APPEND sources src/cartridge/mapper/mapperbase.c)
list(APPEND sources src/cartridge/mapper/nrom.c)
list(APPEND sources src/cartridge/mapper/mmc1.c)
list(APPEND sources src/cartridge/mapper/cnrom.c)

list(APPEND sources src/memory/membase.c)
list(APPEND sources src/memory/mementry.c)
list(APPEND sources src/memory/mmem.c)
list(APPEND sources src/memory/vmem.c)

list(APPEND sources src/cpu/cpu.c)
list(APPEND sources src/cpu/instrimpl.c)
list(APPEND sources src/cpu/instrtable.c)

list(APPEND sources src/ppu/ppu.c)
list(APPEND sources src/ppu/oamdma.c)
list(APPEND sources src/ppu/placcessor.c)
list(APPEND sources src/ppu/frmbuf.c)
list(APPEND sources src/ppu/palettedef.c)

list(APPEND sources src/ppu/pipeline/pl.c)
list(APPEND sources src/ppu/pipeline/prerendersl.c)
list(APPEND sources src/ppu/pipeline/visiblesl.c)
list(APPEND sources src/ppu/pipeline/bgfetchsl.c)
list(APPEND sources src/ppu/pipeline/spevalfetchsl.c)
list(APPEND sources src/ppu/pipeline/rendersl.c)

list(APPEND sources src/apu/divider.c)
list(APPEND sources src/apu/envelope.c)
list(APPEND sources src/apu/sweep.c)
list(APPEND sources src/apu/seq.c)
list(APPEND sources src/apu/lenctr.c)
list(APPEND sources src/apu/pulse.c)
list(APPEND sources src/apu/linctr.c)
list(APPEND sources src/apu/tri.c)
list(APPEND sources src/apu/noise.c)
list(APPEND sources src/apu/frmctr.c)
list(APPEND sources src/apu/dmc.c)
list(APPEND sources src/apu/apu.c)
list(APPEND sources src/apu/apuclock.c)
list(APPEND sources src/apu/dmcdma.c)

list(APPEND sources src/debug/dbgpal.c)
list(APPEND sources src/debug/dbgoam.c)
list(APPEND sources src/debug/dbgspr.c)
list(APPEND sources src/debug/dbgpattbl.c)

list(APPEND sources src/nesish.c)

target_sources(${tgt_name} PRIVATE ${sources})

# --- Dependencies

add_subdirectory(3rd/fmt)
target_link_libraries(${tgt_name} PRIVATE fmt::fmt-header-only)

# -- Tests

if(NH_BUILD_TESTS AND NOT NH_TGT_WEB)
    add_subdirectory(tests)
endif()

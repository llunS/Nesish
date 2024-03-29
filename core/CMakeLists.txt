cmake_minimum_required(VERSION 3.25.1)
project(Nesish)

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
configure_cxx(${tgt_name} 11)
configure_warnings(${tgt_name})
configure_vc_options(${tgt_name} /wd6285)
if(NH_TGT_WEB)
    configure_em_options(${tgt_name})
else()
    configure_optimizations(${tgt_name})
endif()

include(gen_api_macro)
gen_api_macro(${tgt_name} NH ${CMAKE_CURRENT_SOURCE_DIR}/public/nesish)

# --- Include directories

target_include_directories(${tgt_name} PUBLIC public)
target_include_directories(${tgt_name} PRIVATE src)

# --- Source files

set(sources "")

list(APPEND sources src/console.cpp)

list(APPEND sources src/types.cpp)

list(APPEND sources src/cartridge/cartridge_loader.cpp)
list(APPEND sources src/cartridge/ines.cpp)
list(APPEND sources src/cartridge/mapper/mapper.cpp)
list(APPEND sources src/cartridge/mapper/nrom.cpp)
list(APPEND sources src/cartridge/mapper/mmc1.cpp)
list(APPEND sources src/cartridge/mapper/cnrom.cpp)

list(APPEND sources src/memory/mapping_entry.cpp)
list(APPEND sources src/memory/memory.cpp)
list(APPEND sources src/memory/video_memory.cpp)

list(APPEND sources src/cpu/cpu.cpp)
list(APPEND sources src/cpu/instr_impl.cpp)
list(APPEND sources src/cpu/instr_table.cpp)

list(APPEND sources src/ppu/ppu.cpp)
list(APPEND sources src/ppu/oam_dma.cpp)
list(APPEND sources src/ppu/pipeline_accessor.cpp)
list(APPEND sources src/ppu/frame_buffer.cpp)
list(APPEND sources src/ppu/palette_default.cpp)

list(APPEND sources src/ppu/pipeline/pipeline.cpp)
list(APPEND sources src/ppu/pipeline/pre_render_scanline.cpp)
list(APPEND sources src/ppu/pipeline/visible_scanline.cpp)
list(APPEND sources src/ppu/pipeline/bg_fetch.cpp)
list(APPEND sources src/ppu/pipeline/sp_eval_fetch.cpp)
list(APPEND sources src/ppu/pipeline/render.cpp)

list(APPEND sources src/apu/divider.cpp)
list(APPEND sources src/apu/envelope.cpp)
list(APPEND sources src/apu/sweep.cpp)
list(APPEND sources src/apu/sequencer.cpp)
list(APPEND sources src/apu/length_counter.cpp)
list(APPEND sources src/apu/pulse.cpp)
list(APPEND sources src/apu/linear_counter.cpp)
list(APPEND sources src/apu/triangle.cpp)
list(APPEND sources src/apu/noise.cpp)
list(APPEND sources src/apu/frame_counter.cpp)
list(APPEND sources src/apu/dmc.cpp)
list(APPEND sources src/apu/apu.cpp)
list(APPEND sources src/apu/apu_clock.cpp)
list(APPEND sources src/apu/dmc_dma.cpp)

list(APPEND sources src/debug/palette.cpp)
list(APPEND sources src/debug/oam.cpp)
list(APPEND sources src/debug/sprite.cpp)
list(APPEND sources src/debug/pattern_table.cpp)

list(APPEND sources src/nesish.cpp)

target_sources(${tgt_name} PRIVATE ${sources})

# --- Dependencies

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../base base)
target_link_libraries(${tgt_name} PRIVATE NesishBase)

add_subdirectory(3rd/fmt)
target_link_libraries(${tgt_name} PRIVATE fmt::fmt-header-only)

# -- Tests

if(NH_BUILD_TESTS AND NOT NH_TGT_WEB)
    add_subdirectory(tests)
endif()

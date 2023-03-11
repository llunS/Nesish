cmake_minimum_required(VERSION 3.25.1)
project(Nesish)


# -- Global compiler/linker options/flags

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
    # suppress some warnings
    add_compile_options(-Wno-reorder-ctor)
endif()

# Add cmake scripts
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)


# -- CMake options

option(NH_BUILD_TESTS "Build tests" ON)


# -- Target

# Enable test
if(NH_BUILD_TESTS)
    enable_testing()
endif()

set(tgt_name Nesish)
add_library(${tgt_name} SHARED)

include(gen_dllexport_header)
gen_dllexport_header(${tgt_name} ${CMAKE_CURRENT_SOURCE_DIR}/public/nesish NH)

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

list(APPEND sources src/ppu/pipeline/tickable.cpp)
list(APPEND sources src/ppu/pipeline/functor_tickable.cpp)
list(APPEND sources src/ppu/pipeline/idle_ticker.cpp)

list(APPEND sources src/ppu/pipeline/pipeline.cpp)
list(APPEND sources src/ppu/pipeline/pre_render_scanline.cpp)
list(APPEND sources src/ppu/pipeline/visible_scanline.cpp)
list(APPEND sources src/ppu/pipeline/vblank_scanline.cpp)
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

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${tgt_name} PRIVATE fmt::fmt-header-only)


# -- Tests

if(NH_BUILD_TESTS)
    add_subdirectory(tests)
endif()

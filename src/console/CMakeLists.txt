set(module_name Console)
add_library(${module_name} SHARED)

include(gen_dllexport_header)
gen_dllexport_header(${module_name})


# --------------------------------------------
# Include directories
# --------------------------------------------

target_include_directories(${module_name} PUBLIC ../)


# --------------------------------------------
# Source files
# --------------------------------------------

list(APPEND sources emulator.cpp)

list(APPEND sources clock.cpp)
list(APPEND sources types.cpp)

list(APPEND sources cartridge/cartridge_loader.cpp)
list(APPEND sources cartridge/ines.cpp)
list(APPEND sources cartridge/mapper/mapper.cpp)
list(APPEND sources cartridge/mapper/norm.cpp)

list(APPEND sources memory/mapping_entry.cpp)
list(APPEND sources memory/memory.cpp)
list(APPEND sources memory/video_memory.cpp)

list(APPEND sources cpu/cpu.cpp)
list(APPEND sources cpu/opcode_exec.cpp)
list(APPEND sources cpu/address_mode_parse.cpp)
list(APPEND sources cpu/instr_map.cpp)
list(APPEND sources cpu/address_mode_map.cpp)
list(APPEND sources cpu/operand.cpp)

list(APPEND sources ppu/ppu.cpp)
list(APPEND sources ppu/pipeline_accessor.cpp)
list(APPEND sources ppu/frame_buffer.cpp)
list(APPEND sources ppu/palette_default.cpp)

list(APPEND sources ppu/pipeline/ticker.cpp)
list(APPEND sources ppu/pipeline/lambda_ticker.cpp)
list(APPEND sources ppu/pipeline/idle_ticker.cpp)

list(APPEND sources ppu/pipeline/pipeline.cpp)
list(APPEND sources ppu/pipeline/pre_render_scanline.cpp)
list(APPEND sources ppu/pipeline/visible_scanline.cpp)
list(APPEND sources ppu/pipeline/vblank_scanline.cpp)
list(APPEND sources ppu/pipeline/bg_fetch.cpp)
list(APPEND sources ppu/pipeline/sp_eval_fetch.cpp)
list(APPEND sources ppu/pipeline/render.cpp)

list(APPEND sources debug/color.cpp)
list(APPEND sources debug/palette.cpp)
list(APPEND sources debug/oam.cpp)
list(APPEND sources debug/sprite.cpp)

target_sources(${module_name} PRIVATE ${sources})


# --------------------------------------------
# Dependencies
# --------------------------------------------

target_link_libraries(${module_name} PUBLIC
    Common
)

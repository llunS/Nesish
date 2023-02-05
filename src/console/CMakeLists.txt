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

list(APPEND sources types.cpp)

list(APPEND sources cartridge/cartridge_loader.cpp)
list(APPEND sources cartridge/ines.cpp)
list(APPEND sources cartridge/mapper/mapper.cpp)
list(APPEND sources cartridge/mapper/nrom.cpp)
list(APPEND sources cartridge/mapper/mmc1.cpp)

list(APPEND sources memory/mapping_entry.cpp)
list(APPEND sources memory/memory.cpp)
list(APPEND sources memory/video_memory.cpp)

list(APPEND sources cpu/cpu.cpp)
list(APPEND sources cpu/instr_impl.cpp)
list(APPEND sources cpu/instr_table.cpp)

list(APPEND sources ppu/ppu.cpp)
list(APPEND sources ppu/pipeline_accessor.cpp)
list(APPEND sources ppu/frame_buffer.cpp)
list(APPEND sources ppu/palette_default.cpp)

list(APPEND sources ppu/pipeline/tickable.cpp)
list(APPEND sources ppu/pipeline/functor_tickable.cpp)
list(APPEND sources ppu/pipeline/idle_ticker.cpp)

list(APPEND sources ppu/pipeline/pipeline.cpp)
list(APPEND sources ppu/pipeline/pre_render_scanline.cpp)
list(APPEND sources ppu/pipeline/visible_scanline.cpp)
list(APPEND sources ppu/pipeline/vblank_scanline.cpp)
list(APPEND sources ppu/pipeline/bg_fetch.cpp)
list(APPEND sources ppu/pipeline/sp_eval_fetch.cpp)
list(APPEND sources ppu/pipeline/render.cpp)

list(APPEND sources apu/divider.cpp)
list(APPEND sources apu/envelope.cpp)
list(APPEND sources apu/sweep.cpp)
list(APPEND sources apu/sequencer.cpp)
list(APPEND sources apu/length_counter.cpp)
list(APPEND sources apu/pulse.cpp)
list(APPEND sources apu/linear_counter.cpp)
list(APPEND sources apu/triangle.cpp)
list(APPEND sources apu/noise.cpp)
list(APPEND sources apu/frame_counter.cpp)
list(APPEND sources apu/dmc.cpp)
list(APPEND sources apu/apu.cpp)

list(APPEND sources debug/color.cpp)
list(APPEND sources debug/palette.cpp)
list(APPEND sources debug/oam.cpp)
list(APPEND sources debug/sprite.cpp)
list(APPEND sources debug/pattern_table.cpp)

target_sources(${module_name} PRIVATE ${sources})


# --------------------------------------------
# Dependencies
# --------------------------------------------

target_link_libraries(${module_name} PUBLIC
    Common
)

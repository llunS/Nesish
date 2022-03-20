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

list(APPEND sources cartridge/cartridge_loader.cpp)
list(APPEND sources cartridge/ines.cpp)
list(APPEND sources cartridge/mapper/mapper.cpp)
list(APPEND sources cartridge/mapper/norm.cpp)

list(APPEND sources memory/memory.cpp)
list(APPEND sources memory/mapping_entry.cpp)

list(APPEND sources cpu/cpu.cpp)
list(APPEND sources cpu/opcode_exec.cpp)
list(APPEND sources cpu/address_mode_parse.cpp)
list(APPEND sources cpu/instr_map.cpp)
list(APPEND sources cpu/address_mode_map.cpp)
list(APPEND sources cpu/operand.cpp)

list(APPEND sources ppu/ppu.cpp)
list(APPEND sources ppu/ppu_memory.cpp)

target_sources(${module_name} PRIVATE ${sources})


# --------------------------------------------
# Dependencies
# --------------------------------------------

target_link_libraries(${module_name} PUBLIC
    Common
)

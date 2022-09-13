#pragma once

#include "console/memory/mappable_memory.hpp"
#include "console/spec.hpp"
#include "common/klass.hpp"
#include "console/types.hpp"
#include "common/error.hpp"

namespace ln {

/* @IMPL: Valid enumerators must be unique and can be used as array index */
enum class MemoryMappingPoint : unsigned char {
    INVALID = 0,
    ADHOC, // @TMP

    INTERNAL_RAM,

    PPU_REG,
    OAMDMA,
    APU_CTRL_REG,

    PRG_ROM,
    PRG_RAM,

    SIZE,
};

struct Memory : public MappableMemory<MemoryMappingPoint, LN_ADDRESSABLE_SIZE> {
  public:
    Memory();
    LN_KLZ_DELETE_COPY_MOVE(Memory);

  public:
    static constexpr Address STACK_PAGE_MASK =
        0x0100; // stack page: $0100-$01FF

    static constexpr Address NMI_VECTOR_ADDR = 0xFFFA;
    static constexpr Address RESET_VECTOR_ADDR = 0xFFFC;
    static constexpr Address IRQ_VECTOR_ADDR = 0xFFFE; // IRQ/BRK vector

    Error
    set_bulk(Address i_begin, Address i_end, Byte i_byte);

  private:
    // ---- Memory Map
    // https://wiki.nesdev.org/w/index.php?title=CPU_memory_map

    // On-board RAM
    Byte m_ram[LN_INTERNAL_RAM_SIZE];
    // @TODO: Handle other mappings
    // @TMP: Adhoc area for NES APU and I/O registers
    Byte m_adhoc[0x0018];
};

} // namespace ln

#pragma once

#include "console/memory/mappable_memory.hpp"
#include "console/spec.hpp"
#include "common/klass.hpp"
#include "console/types.hpp"
#include "common/error.hpp"

namespace ln {

enum class MemoryMappingPoint : unsigned char {
    INVALID = 0,
    ADHOC, // @TMP

    INTERNAL_RAM,
    PPU_REGISTER,
    OAMDMA,
    PRG_ROM,
    PRG_RAM,
};

struct Memory : public MappableMemory<MemoryMappingPoint, LN_ADDRESSABLE_SIZE> {
  public:
    Memory();
    LN_KLZ_DELETE_COPY_MOVE(Memory);

  public:
    static constexpr Address STACK_PAGE_MASK =
        0x0100; // stack page: $0100-$01FF
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

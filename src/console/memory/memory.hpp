#pragma once

#include "console/memory/mappable_memory.hpp"
#include "console/spec.hpp"
#include "common/klass.hpp"
#include "console/types.hpp"
#include "common/error.hpp"

namespace nh {

/* @NOTE: Enumerators must be unique and are valid array index */
enum class MemoryMappingPoint : unsigned char {
    INVALID = 0,

    INTERNAL_RAM,

    PPU,
    APU_OAMDMA_CTRL,

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

  public:
    Error
    get_byte(Address i_addr, Byte &o_val) const;

    Byte
    get_latch() const;

  public:
    Error
    set_bulk(Address i_begin, Address i_end, Byte i_byte);

  private:
    // ---- Memory Map
    // https://wiki.nesdev.org/w/index.php?title=CPU_memory_map

    // On-board RAM
    Byte m_ram[LN_INTERNAL_RAM_SIZE];
    mutable Byte m_read_latch;
};

} // namespace nh

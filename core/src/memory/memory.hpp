#pragma once

#include "memory/mappable_memory.hpp"
#include "spec.hpp"
#include "nhbase/klass.hpp"
#include "types.hpp"

namespace nh {

// Enumerators must be unique and are valid array index
enum class MemoryMappingPoint : unsigned char {
    INVALID = 0,

    INTERNAL_RAM,

    PPU,
    APU_OAMDMA_CTRL,

    PRG_ROM,
    PRG_RAM,

    SIZE,
};

struct Memory : public MappableMemory<MemoryMappingPoint, NH_ADDRESSABLE_SIZE> {
  public:
    Memory(NHLogger *i_logger);
    NB_KLZ_DELETE_COPY_MOVE(Memory);

  public:
    static constexpr Address STACK_PAGE_MASK =
        0x0100; // stack page: $0100-$01FF

    static constexpr Address NMI_VECTOR_ADDR = 0xFFFA;
    static constexpr Address RESET_VECTOR_ADDR = 0xFFFC;
    static constexpr Address IRQ_VECTOR_ADDR = 0xFFFE; // IRQ/BRK vector

  public:
    NHErr
    get_byte(Address i_addr, Byte &o_val) const;

    Byte
    get_latch() const;
    void
    override_latch(Byte i_val);

  public:
    NHErr
    set_bulk(Address i_begin, Address i_end, Byte i_byte);

  private:
    // ---- Memory Map
    // https://wiki.nesdev.org/w/index.php?title=CPU_memory_map

    // On-board RAM
    Byte m_ram[NH_INTERNAL_RAM_SIZE];
    mutable Byte m_read_latch;
};

} // namespace nh

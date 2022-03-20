#ifndef LN_CONSOLE_PPU_MEMORY_HPP
#define LN_CONSOLE_PPU_MEMORY_HPP

#include "console/memory/mappable_memory.hpp"
#include "common/klass.hpp"
#include "console/spec.hpp"
#include "console/types.hpp"

namespace ln {

enum class PPUMemoryMappingPoint : unsigned char {
    INVALID = 0,

    // https://www.nesdev.org/wiki/PPU_memory_map
    PATTERN,
    NAMETABLE,
    NAMETABLE_MIRROR,
    PALETTE,
};

struct PPUMemory
    : public MappableMemory<PPUMemoryMappingPoint, LN_PPU_ADDRESSABLE_SIZE> {
  public:
    PPUMemory();
    LN_KLZ_DELETE_COPY_MOVE(PPUMemory);

    void
    configure_mirror(bool i_horizontal);

  private:
    // https://www.nesdev.org/wiki/PPU_memory_map
    Byte m_ram[LN_PPU_INTERNAL_RAM_SIZE];
    Byte m_palette[LN_PALETTE_SIZE];
    Byte m_oam[LN_OAM_SIZE];
};

} // namespace ln

#endif // LN_CONSOLE_PPU_MEMORY_HPP

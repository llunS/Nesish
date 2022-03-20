#ifndef LN_CONSOLE_CARTRIDGE_CARTRIDGE_HPP
#define LN_CONSOLE_CARTRIDGE_CARTRIDGE_HPP

#include "common/error.hpp"
#include "console/memory/memory.hpp"
#include "console/ppu/ppu_memory.hpp"

namespace ln {

struct Cartridge {
  public:
    virtual ~Cartridge() = default;

    virtual Error
    validate() const = 0;

    virtual void
    map_memory(Memory *i_memory, PPUMemory *i_ppu_memory) const = 0;
    virtual void
    unmap_memory(Memory *i_memory, PPUMemory *i_ppu_memory) const = 0;
};

} // namespace ln

#endif // LN_CONSOLE_CARTRIDGE_CARTRIDGE_HPP

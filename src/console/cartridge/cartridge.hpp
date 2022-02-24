#ifndef LN_CONSOLE_CARTRIDGE_CARTRIDGE_HPP
#define LN_CONSOLE_CARTRIDGE_CARTRIDGE_HPP

#include "common/error.hpp"
#include "console/mmu.hpp"

namespace ln {

struct Cartridge {
  public:
    virtual ~Cartridge() = default;

    virtual Error
    validate() const = 0;

    virtual void
    map_memory(MMU *i_mmu) const = 0;
    virtual void
    unmap_memory(MMU *i_mmu) const = 0;
};

} // namespace ln

#endif // LN_CONSOLE_CARTRIDGE_CARTRIDGE_HPP

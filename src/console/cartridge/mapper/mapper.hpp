#ifndef LN_CONSOLE_CARTRIDGE_MAPPER_MAPPER_HPP
#define LN_CONSOLE_CARTRIDGE_MAPPER_MAPPER_HPP

#include "console/cartridge/ines.hpp"

namespace ln {

struct Mapper {
  public:
    Mapper(const INES::RomAccessor *i_accessor);
    virtual ~Mapper() = default;

    virtual Error
    validate() const = 0;

    virtual void
    map_memory(Memory *i_memory) = 0;
    virtual void
    unmap_memory(Memory *i_memory) const = 0;

  protected:
    const INES::RomAccessor *m_rom_accessor;
};

} // namespace ln

#endif // LN_CONSOLE_CARTRIDGE_MAPPER_MAPPER_HPP

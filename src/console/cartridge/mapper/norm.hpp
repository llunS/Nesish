#ifndef LN_CONSOLE_CARTRIDGE_MAPPER_NORM_HPP
#define LN_CONSOLE_CARTRIDGE_MAPPER_NORM_HPP

#include "console/cartridge/mapper/mapper.hpp"

namespace ln {

struct NORM : public Mapper {
  public:
    NORM(const INES::RomAccessor *i_accessor);

    Error
    validate() const override;

    void
    map_memory(Memory *i_memory) override;
    void
    unmap_memory(Memory *i_memory) const override;

  private:
    Byte m_prg_ram[8 * 1024]; // Just support it up to 8KB
    // @TODO: CHR RAM
};

} // namespace ln

#endif // LN_CONSOLE_CARTRIDGE_MAPPER_NORM_HPP

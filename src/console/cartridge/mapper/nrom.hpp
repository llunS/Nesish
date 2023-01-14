#pragma once

#include "console/cartridge/mapper/mapper.hpp"

namespace ln {

struct NROM : public Mapper {
  public:
    NROM(const INES::RomAccessor *i_accessor);

    Error
    validate() const override;

    void
    map_memory(const INES *i_nes, Memory *i_memory,
               VideoMemory *i_video_memory) override;
    void
    unmap_memory(const INES *i_nes, Memory *i_memory,
                 VideoMemory *i_video_memory) const override;

  private:
    Byte m_prg_ram[8 * 1024]; // Just support it up to 8KB
    Byte m_chr_ram[8 * 1024];
};

} // namespace ln

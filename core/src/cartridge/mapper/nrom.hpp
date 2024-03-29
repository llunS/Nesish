#pragma once

#include "cartridge/mapper/mapper.hpp"
#include "types.hpp"

namespace nh {

struct NROM : public Mapper {
  public:
    NROM(const INES::RomAccessor *i_accessor);

    NHErr
    validate() const override;

    void
    power_up() override;
    void
    reset() override;

    void
    map_memory(Memory *o_memory, VideoMemory *o_video_memory) override;
    void
    unmap_memory(Memory *o_memory, VideoMemory *o_video_memory) override;

  private:
    Byte m_prg_ram[8 * 1024]; // 8KB max
    Byte m_chr_ram[8 * 1024];
};

} // namespace nh

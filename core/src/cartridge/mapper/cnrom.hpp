#pragma once

#include "cartridge/mapper/mapper.hpp"
#include "types.hpp"

namespace nh {

/// @brief Mapper 3 implementation, CNROM-like boards
struct CNROM : public Mapper {
  public:
    CNROM(const INES::RomAccessor *i_accessor);

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
    Byte m_chr_bnk;
};

} // namespace nh

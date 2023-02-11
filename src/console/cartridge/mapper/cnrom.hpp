#pragma once

#include "console/cartridge/mapper/mapper.hpp"
#include "console/types.hpp"

namespace ln {

/// @brief Mapper 3 implementation, CNROM-like boards
struct CNROM : public Mapper {
  public:
    CNROM(const INES::RomAccessor *i_accessor);

    Error
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

} // namespace ln

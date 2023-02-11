#pragma once

#include "console/cartridge/mapper/mapper.hpp"
#include "console/types.hpp"

namespace ln {

struct MMC1 : public Mapper {
  public:
    enum Variant {
        V_B,
    };

  public:
    MMC1(const INES::RomAccessor *i_accessor, Variant i_var);

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
    void
    clear_shift();
    void
    reset_prg_bank_mode();
    Byte &
    regsiter_of_addr(Address i_addr);
    bool
    prg_ram_enabled() const;

  private:
    Variant m_variant;

  private:
    Byte m_shift;
    Byte m_ctrl;
    Byte m_chr0_bnk;
    Byte m_chr1_bnk;
    Byte m_prg_bnk;

  private:
    Byte m_prg_ram[32 * 1024]; // 32KB max
    Byte m_chr_ram[8 * 1024];

  private:
    bool m_no_prg_banking_32K;
};

} // namespace ln

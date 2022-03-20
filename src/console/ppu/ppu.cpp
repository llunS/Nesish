#include "ppu.hpp"

namespace ln {

void
PPU::power_up()
{
    get_register(PPUCTRL) = 0x00;
    get_register(PPUMASK) = 0x00;
    get_register(PPUSTATUS) = 0xA0; // +0+x xxxx
    get_register(OAMADDR) = 0x00;
    // @NOTE: latches should be cleared as well
    get_register(PPUSCROLL) = 0x00;
    get_register(PPUADDR) = 0x00;
    get_register(PPUDATA) = 0x00;
}

void
PPU::reset()
{
    get_register(PPUCTRL) = 0x00;
    get_register(PPUMASK) = 0x00;
    // @NOTE: latches should be cleared as well
    get_register(PPUSCROLL) = 0x00;
    get_register(PPUDATA) = 0x00;
}

Byte &
PPU::get_register(Register i_reg)
{
    return m_regs[i_reg];
}

} // namespace ln

#pragma once

#include "console/types.hpp"
#include "common/klass.hpp"

namespace ln {

struct PPU {
  public:
    PPU() = default;
    LN_KLZ_DELETE_COPY_MOVE(PPU);

    void
    power_up();
    void
    reset();

    // @NOTE: The value must can be used as array index.
    // https://wiki.nesdev.org/w/index.php?title=PPU_registers
    enum Register {
        PPUCTRL = 0, // NMI enable (V), PPU master/slave (P), sprite height (H),
                     // background tile select (B), sprite tile select (S),
                     // increment mode (I), nametable select (NN)
        PPUMASK,   // color emphasis (BGR), sprite enable (s), background enable
                   // (b), sprite left column enable (M), background left column
                   // enable (m), greyscale (G)
        PPUSTATUS, // vblank (V), sprite 0 hit (S), sprite overflow (O); read
                   // resets write pair for $2005/$2006
        OAMADDR,   // OAM read/write address
        OAMDATA,   // OAM data read/write
        PPUSCROLL, // fine scroll position (two writes: X scroll, Y scroll)
        PPUADDR,   // PPU read/write address (two writes: most significant byte,
                   // least significant byte)
        PPUDATA,   // PPU data read/write
        OAMDMA,    // OAM DMA high address

        SIZE,
    };

    Byte &
    get_register(Register i_reg);

  private:
    Byte m_regs[Register::SIZE];
};

} // namespace ln

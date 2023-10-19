#pragma once

// https://wiki.nesdev.org/w/index.php?title=PPU_registers
// Values must be valid array index, see "m_regs".
typedef enum ppureg_e {
    /* clang-format off */
    PR_PPUCTRL = 0, // VPHB SINN / NMI enable (V), PPU master/slave (P), sprite height (H), background tile select (B), sprite tile select (S), increment mode (I), nametable select (NN)
    PR_PPUMASK, // BGRs bMmG / color emphasis (BGR), sprite enable (s), background enable (b), sprite left column enable (M), background left column enable (m), greyscale (G)
    PR_PPUSTATUS, // VSO- ---- / vblank (V), sprite 0 hit (S), sprite overflow (O); read resets write pair for $2005/$2006
    PR_OAMADDR, // OAM read/write address
    PR_OAMDATA, // OAM data read/write
    PR_PPUSCROLL, // fine scroll position (two writes: X scroll, Y scroll)
    PR_PPUADDR, // PPU read/write address (two writes: most significant byte, least significant byte)
    PR_PPUDATA, // PPU data read/write

    PR_SIZE,
    /* clang-format on */
} ppureg_e;

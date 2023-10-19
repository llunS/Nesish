#pragma once

typedef enum addrmode_e {
    // http://www.oxyron.de/html/opcodes02.html
    AM_IMP, // Implicit
    AM_ACC, // Implicit accumulator
    AM_IMM, // Immediate
    AM_ZP0, // Zeropage, pad 0 at the end the make it 3
    AM_ZPX, // Zero page indexed X
    AM_ZPY, // Zero page indexed Y
    AM_IZX, // Indexed indirect X
    AM_IZY, // Indirect indexed Y
    AM_ABS, // Absolute
    AM_ABX, // Absolute indexed X
    AM_ABY, // Absolute indexed Y
    AM_IND, // Indirect, i.e. JMP indirect
    AM_REL, // Relative
} addrmode_e;

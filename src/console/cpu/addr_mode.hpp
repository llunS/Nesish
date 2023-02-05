#pragma once

namespace ln {

/// @brief Main category for address mode
enum AddrMode {
    // http://www.oxyron.de/html/opcodes02.html
    IMP, // Implicit
    ACC, // Implicit accumulator
    IMM, // Immediate
    ZP0, // Zeropage, pad 0 at the end the make it 3
    ZPX, // Zero page indexed X
    ZPY, // Zero page indexed Y
    IZX, // Indexed indirect X
    IZY, // Indirect indexed Y
    ABS, // Absolute
    ABX, // Absolute indexed X
    ABY, // Absolute indexed Y
    IND, // Indirect, i.e. JMP indirect
    REL, // Relative
};

} // namespace ln

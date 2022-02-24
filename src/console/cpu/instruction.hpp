#ifndef LN_CONSOLE_CPU_INSTRUCTION_HPP
#define LN_CONSOLE_CPU_INSTRUCTION_HPP

#include "common/klass.hpp"
#include "console/types.hpp"

namespace ln {

enum class OpcodeType {
    // http://www.oxyron.de/html/opcodes02.html
    // ---- Logical and arithmetic commands
    ORA,
    AND,
    EOR,
    ADC,
    SBC,
    CMP,
    CPX,
    CPY,
    DEC,
    DEX,
    DEY,
    INC,
    INX,
    INY,
    ASL,
    ROL,
    LSR,
    ROR,
    // ---- Move commands
    LDA,
    STA,
    LDX,
    STX,
    LDY,
    STY,
    TAX,
    TXA,
    TAY,
    TYA,
    TSX,
    TXS,
    PLA,
    PHA,
    PLP,
    PHP,
    // ---- Jump/Flag commands
    BPL,
    BMI,
    BVC,
    BVS,
    BCC,
    BCS,
    BNE,
    BEQ,
    BRK,
    RTI,
    JSR,
    RTS,
    JMP,
    BIT,
    CLC,
    SEC,
    CLD,
    SED,
    CLI,
    SEI,
    CLV,
    NOP,
    // ---- Exclusive illegal opcodes
    SLO,
    RLA,
    SRE,
    RRA,
    SAX,
    LAX,
    DCP,
    ISC, // aka ISB
    ANC,
    ALR,
    ARR,
    XAA,
    AXS,
    AHX,
    SHY,
    SHX,
    TAS,
    LAS,
    KIL,

    COUNT,
};

static_assert((unsigned long long)OpcodeType::COUNT == 75,
              "Some opcodes missing.");

typedef Byte Opcode;

enum AddressMode {
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

    COUNT,
};

} // namespace ln

#endif // LN_CONSOLE_CPU_INSTRUCTION_HPP

#ifndef LN_CONSOLE_INSTRUCTION_HPP
#define LN_CONSOLE_INSTRUCTION_HPP

#include <cstdint>

#include "common/klass.hpp"

namespace ln {

enum class OpCode
{
    // https://wiki.nesdev.org/w/index.php?title=CPU_unofficial_opcodes

    NOP,

    // ----- Control (RED)
    BRK,
    PHP,
    BPL,
    CLC,
    JSR,
    BIT,
    PLP,
    BMI,
    SEC,
    RTI,
    PHA,
    JMP,
    BVC,
    CLI,
    RTS,
    PLA,
    BVS,
    SEI,
    STY,
    DEY,
    BCC,
    TYA,
    SHY,
    LDY,
    TAY,
    BCS,
    CLV,
    CPY,
    INY,
    BNE,
    CLD,
    CPX,
    INX,
    BEQ,
    SED,

    // ----- ALU (GREEN)
    ORA,
    AND,
    EOR,
    ADC,
    STA,
    LDA,
    CMP,
    SBC,

    // ----- RMW (read-modify-write) (BLUE)
    STP,
    ASL,
    ROL,
    LSR,
    ROR,
    STX,
    TXA,
    TXS,
    SHX,
    LDX,
    TAX,
    TSX,
    DEC,
    DEX,
    INC,

    // ----- Unofficial (GRAY)
    SLO,
    ANC,
    RLA,
    SRE,
    ALR,
    RRA,
    SAX,
    XAA,
    AHX,
    TAS,
    LAX,
    LAS,
    DCP,
    AXS,
    ISC,

    // @TODO: Log invalid opcode encountered in case we miss some.
};

enum class AddressMode
{
    // http://www.oxyron.de/html/opcodes02.html

    IMP,
    IMM,
    ZP0, // Zeropage, pad 0 at the end the make it 3.
    ZPX,
    ZPY,
    IZX,
    IZY,
    ABS,
    ABX,
    ABY,
    IND,
    REL,
};

struct Instruction {
  public:
    Instruction(uint8_t i_raw_instruction);
    LN_KLZ_DEFAULT_COPY(Instruction);

    OpCode op_code;
    AddressMode address_mode;
};

} // namespace ln

#endif // LN_CONSOLE_INSTRUCTION_HPP

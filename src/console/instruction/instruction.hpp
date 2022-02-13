#ifndef LN_CONSOLE_INSTRUCTION_INSTRUCTION_HPP
#define LN_CONSOLE_INSTRUCTION_INSTRUCTION_HPP

#include "common/klass.hpp"
#include "console/types.hpp"

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
    ARR,
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

    IMP, // Implicit
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

struct Instruction {
  public:
    Instruction(OpCode i_op_code, AddressMode i_address_mode);
    LN_KLZ_DEFAULT_COPY(Instruction);

    static Instruction
    decode(Byte i_raw_instruction);

    OpCode op_code;
    AddressMode address_mode;
};

} // namespace ln

#endif // LN_CONSOLE_INSTRUCTION_INSTRUCTION_HPP

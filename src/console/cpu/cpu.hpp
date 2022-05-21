#pragma once

#include <string>
#include <vector>

#include "console/memory/memory.hpp"
#include "common/klass.hpp"
#include "console/types.hpp"
#include "console/cpu/instruction.hpp"
#include "console/cpu/operand.hpp"
#include "common/error.hpp"
#include "console/clock.hpp"
#include "console/dllexport.h"

namespace ln {

struct LN_CONSOLE_API CPU {
  public:
    CPU(Memory *i_memory);
    LN_KLZ_DELETE_COPY_MOVE(CPU);

  public:
    void
    power_up();
    void
    reset();

    void
    set_entry(Address i_entry);
    bool
    tick();
    bool
    step();

    Cycle
    get_cycle() const;

    Byte
    get_a() const;
    Byte
    get_x() const;
    Byte
    get_y() const;
    Address
    get_pc() const;
    Byte
    get_s() const;
    Byte
    get_p() const;

    std::vector<Byte>
    get_instruction_bytes(Address i_addr) const;

  private:
    typedef void (*ExecFunc)(ln::CPU *i_cpu, ln::Operand i_operand,
                             Cycle &o_branch_cycles);
    struct OpcodeExec;
    typedef ln::Operand (*ParseFunc)(const ln::CPU *i_cpu,
                                     Byte &o_operand_bytes,
                                     bool *o_page_crossing);
    struct AddressModeParse;

    struct InstructionDesc {
        OpcodeType op_code;
        AddressMode address_mode;
        ExecFunc exec_func;
        Cycle cycle_base;
        bool cycle_page_dependent; // excluding branch crossing, it's considered
                                   // elsewhere.
    };

    static InstructionDesc s_instr_map[256];

    struct AddressModeParseEntry {
        AddressMode address_mode;
        ParseFunc parse_func;
    };
    static AddressModeParseEntry s_address_mode_map[AddressMode::COUNT];

  private:
    static InstructionDesc
    get_instr_desc(Opcode i_opcode);
    static OpcodeType
    get_op_code(Opcode i_opcode);
    static AddressMode
    get_address_mode(Opcode i_opcode);
    static ExecFunc
    get_opcode_exec(Opcode i_opcode);
    static ParseFunc
    get_address_parse(Opcode i_opcode);

    // ----- memory operations

    Byte
    get_byte(Address i_addr) const;
    void
    set_byte(Address i_addr, Byte i_byte);
    Byte2
    get_byte2(Address i_addr) const;

    // ----- stack operations

    void
    push_byte(Byte i_byte);
    Byte
    pop_byte();
    void
    push_byte2(Byte2 i_byte2);
    Byte2
    pop_byte2();

  private:
    enum StatusFlag : Byte {
        // http://www.oxyron.de/html/opcodes02.html
        C = 1 << 0, // carry flag (1 on unsigned overflow)
        Z = 1 << 1, // zero flag (1 when all bits of a result are 0)
        I = 1 << 2, // IRQ flag (when 1, no interupts will occur (exceptions are
                    // IRQs forced by BRK and NMIs))
        D = 1 << 3, // decimal flag (1 when CPU in BCD mode)
        B = 1 << 4, // break flag (1 when interupt was caused by a BRK)
        U = 1 << 5, // unused (always 1)
        V = 1 << 6, // overflow flag (1 on signed overflow)
        N = 1 << 7, // negative flag (1 when result is negative)
    };

  private:
    bool
    check_flag(StatusFlag i_flag) const;
    void
    set_flag(StatusFlag i_flag);
    void
    unset_flag(StatusFlag i_flag);
    void
    test_flag(StatusFlag i_flag, bool i_cond);

    Byte
    get_operand(Operand i_operand) const;
    Error
    set_operand(Operand i_operand, Byte i_byte);

  private:
    void
    halt();

    void
    report_exec_error(const std::string &i_msg);

  private:
    // ---- Registers
    // https://wiki.nesdev.org/w/index.php?title=CPU_registers
    Byte A;   // Accumulator
    Byte X;   // Index X
    Byte Y;   // Index Y
    Byte2 PC; // Program Counter
    // https://wiki.nesdev.org/w/index.php?title=Stack
    Byte S; // Stack Pointer, 6502 uses a descending, empty stack.
    Byte P; // Status

    // ---- External components references
    Memory *m_memory;

  private:
    Cycle m_cycle;
    Cycle m_next_instr_cycle;
    bool m_halted;
};

} // namespace ln

// #include "cpu.inl"

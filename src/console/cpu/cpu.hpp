#ifndef LN_CONSOLE_CPU_CPU_HPP
#define LN_CONSOLE_CPU_CPU_HPP

#include <string>

#include "console/memory.hpp"
#include "common/klass.hpp"
#include "console/types.hpp"
#include "console/cpu/instruction.hpp"
#include "console/cpu/operand.hpp"
#include "common/error.hpp"

namespace ln {

struct CPU {
  public:
    CPU(Memory *i_memory);
    LN_KLZ_DELETE_COPY_MOVE(CPU);

  public:
    void
    power_up();
    void
    reset();

    void
    step();

  private:
    typedef void (*ExecFunc)(ln::CPU *i_cpu, ln::Operand i_operand);
    struct OpcodeExec;
    typedef ln::Operand (*ParseFunc)(ln::CPU *i_cpu);
    struct AddressModeParse;

    struct InstructionProperty {
        Opcode op_code;
        AddressMode address_mode;
        ExecFunc exec_func;
    };

    static InstructionProperty s_instr_map[256];

  private:
    static Opcode
    get_op_code(Instruction i_instr);
    static AddressMode
    get_address_mode(Instruction i_instr);
    static ExecFunc
    get_opcode_exec(Instruction i_instr);

    // ----- memory operations

    Byte
    get_byte(Address i_address) const;
    void
    set_byte(Address i_address, Byte i_byte);

    // ----- stack operations

    static constexpr Address STACK_BASE = 0x0100; // stack page: $0100-$01FF

    void
    push_byte(Byte i_byte);
    Byte
    pop_byte();
    void
    push_byte2(Byte2 i_byte2);
    Byte2
    pop_byte2();

  private:
    enum StatusFlag : Byte
    {
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
};

} // namespace ln

// #include "cpu.inl"

#endif // LN_CONSOLE_CPU_CPU_HPP

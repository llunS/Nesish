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

struct PPU;
struct APU;

struct CPU {
  public:
    CPU(Memory *i_memory, PPU *i_ppu, const APU *i_apu);
    LN_KLZ_DELETE_COPY_MOVE(CPU);

  public:
    void
    power_up();
    void
    reset();

    /// @return Cycles of the executed instruction. Return 0 if the instruction
    /// is not complete yet.
    Cycle
    tick();

  public:
    /* Query */

    LN_CONSOLE_API Cycle
    get_cycle() const;

    LN_CONSOLE_API Byte
    get_a() const;
    LN_CONSOLE_API Byte
    get_x() const;
    LN_CONSOLE_API Byte
    get_y() const;
    LN_CONSOLE_API Address
    get_pc() const;
    LN_CONSOLE_API Byte
    get_s() const;
    LN_CONSOLE_API Byte
    get_p() const;

    LN_CONSOLE_API std::vector<Byte>
    get_instr_bytes(Address i_addr) const;

    /* For test only */

    void
    set_entry_test(Address i_entry);
    bool
    step_test();

  private:
    /* allow access from other internal components */

    friend struct PPU;
    void
    init_oam_dma(Byte i_val);

    friend struct PPU;
    void
    set_nmi(bool i_flag);

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
    // @NOTE: Same underlying type as "this->P", so the enumerators can be
    // directly used in bitwise operations.
    enum StatusFlag : Byte {
        // http://www.oxyron.de/html/opcodes02.html
        C = 1 << 0, // carry flag (1 on unsigned overflow)
        Z = 1 << 1, // zero flag (1 when all bits of a result are 0)
        // @TODO: If the /IRQ line is low (IRQ pending) when this flag is
        // cleared, an interrupt will immediately be triggered.
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
    poll_interrupt();

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
    // @TODO: Use a bus object to avoid direct reference?
    PPU *m_ppu;
    const APU *m_apu;

  private:
    Cycle m_cycle;
    bool m_halted;

    // ---- interrupt lines
    bool m_nmi;

  private:
    struct OAMDMAContext {
        bool ongoing;
        Byte upper;
        Cycle counter;
        Cycle start_counter;
        bool write;
        Byte tmp;
    } m_oam_dma_ctx;

  private:
    enum class Stage {
        DECODE,
        FETCH_EXEC,
        LAST_CYCLE,
    } m_next_stage;
    Cycle m_next_stage_cycle;

    struct StageContext {
        Opcode opcode;
        InstructionDesc instr_desc;
        Cycle instr_cycles;
    } m_stage_ctx;
};

} // namespace ln

// #include "cpu.inl"

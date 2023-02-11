#pragma once

#include <string>
#include <vector>

#include "console/memory/memory.hpp"
#include "common/klass.hpp"
#include "console/types.hpp"
#include "common/error.hpp"
#include "console/dllexport.h"
#include "console/cpu/addr_mode.hpp"

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

    /// @return Whether an instruction has completed.
    bool
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

  private:
    static Byte
    get_operand_bytes(Byte i_opcode);

  public:
    /* For test only */

    LN_CONSOLE_API void
    set_entry_test(Address i_entry);
    LN_CONSOLE_API void
    set_p_test(Byte i_val);

  private:
    /* allow access from other components */

    friend struct Emulator;
    void
    init_oam_dma(Byte i_val);

    friend struct PPU;
    void
    set_nmi(bool i_flag);

  private:
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

    void
    pre_pop_byte();
    Byte
    post_pop_byte();

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

  private:
    void
    halt();

    void
    poll_interrupt();

  private:
    void
    reset_internal();

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
    // @NOTE: This may wrap around back to 0, which is fine, since current
    // implementation doesn't assume infinite range.
    Cycle m_cycle;
    bool m_halted;

    // ---- interrupt lines
    bool m_nmi;

  private:
    struct InstrImpl;
    typedef void (*InstrCore)(ln::CPU *io_cpu, Byte i_in, Byte &o_out);
    typedef void (*InstrFrame)(int i_idx, ln::CPU *io_cpu, InstrCore i_core,
                               bool &io_done);

    /* @NOTE: Set and used in execution of instruction, not to be confused with
     * the real-time status of the hardware bus (not considered) */
    Address m_addr_bus;
    Byte m_data_bus;
    /* Set and used in instruction */
    Byte m_page_offset;
    /* Set in instruction and used in core */
    Address m_i_eff_addr;

    struct InstrDesc {
        InstrCore core;
        AddrMode addr_mode;
        InstrFrame frm;
    };
    static InstrDesc s_instr_table[256];

    struct InstrContext {
        int cycle_plus1;
        InstrDesc *instr;
    } m_instr_ctx;

  private:
    struct OAMDMAContext {
        bool ongoing;
        Byte upper;
        Cycle counter;
        Cycle start_counter;
        Byte tmp;
    } m_oam_dma_ctx;
};

} // namespace ln

// #include "cpu.inl"

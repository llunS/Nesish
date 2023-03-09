#pragma once

#include <string>
#include <vector>

#include "console/memory/memory.hpp"
#include "common/klass.hpp"
#include "console/types.hpp"
#include "common/error.hpp"
#include "console/dllexport.h"
#include "console/cpu/addr_mode.hpp"

namespace nh {

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

    /// @param i_rdy RDY line input enabled
    /// @param o_2002_read Whether $2002 was read at this tick
    /// @return Whether an instruction has completed.
    bool
    pre_tick(bool i_rdy, bool &o_2002_read);
    void
    post_tick();

    bool
    dma_halt() const;

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

    bool
    in_hardware_irq() const;
    bool
    in_nmi() const;
    bool
    in_reset() const;

    void
    poll_interrupt();

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
    PPU *m_ppu;
    const APU *m_apu;

  private:
    // This may wrap around back to 0, which is fine, since current
    // implementation doesn't assume infinite range.
    Cycle m_cycle;
    bool m_instr_halt; // halt caused by instructions
    bool m_dma_halt;

    // ---- temporaries for one tick
    mutable bool m_2002_read;
    // ---- States for/after one tick
    bool m_write_tick;

    // ---- interrupt lines poll cache
    bool m_nmi_asserted;
    // ---- interrupt signals
    bool m_nmi_sig; // pending NMI
    bool m_irq_sig; // pending IRQ
    bool m_reset_sig;
    // ---- interrupts implementation used in execution of instruction
    bool m_irq_pc_no_inc;
    bool m_irq_no_mem_write;
    bool m_is_nmi;
    bool m_irq_pc_no_inc_tmp;
    bool m_irq_no_mem_write_tmp;
    bool m_is_nmi_tmp;

  private:
    struct InstrImpl;
    typedef void (*InstrCore)(nh::CPU *io_cpu, Byte i_in, Byte &o_out);
    typedef void (*InstrFrame)(int i_idx, nh::CPU *io_cpu, InstrCore i_core,
                               bool &io_done);

    /* Set and used in execution of instruction, not to be confused with
     * the real-time status of the hardware bus (not considered) */
    Address m_addr_bus;
    Byte m_data_bus;
    /* Set and used in one instruction */
    Byte m_page_offset;
    /* Set in one instruction tick and used in core */
    Address m_i_eff_addr;

    struct InstrDesc {
        InstrCore core;
        AddrMode addr_mode;
        InstrFrame frm;
    };
    static InstrDesc s_instr_table[256];

    struct InstrContext {
        Byte opcode;
        int cycle_plus1;
        InstrDesc *instr;
    } m_instr_ctx;
};

} // namespace nh

// #include "cpu.inl"

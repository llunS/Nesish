#include "cpu.hpp"

#include "byte_utils.hpp"
#include "assert.hpp"
#include "spec.hpp"
#include "ppu/ppu.hpp"
#include "apu/apu.hpp"

#define NH_BRK_OPCODE 0

namespace nh {

CPU::CPU(Memory *i_memory, NHLogger *i_logger)
    : m_memory(i_memory)
    , m_logger(i_logger)
{
}

void
CPU::power_up()
{
    // https://wiki.nesdev.org/w/index.php?title=CPU_power_up_state
    P = 0x34; // (IRQ disabled)
    A = X = Y = 0;
    S = 0xFD;

    {
        m_cycle = 0;
        m_instr_halt = false;
        m_dma_halt = false;

        m_mask_read_tmp = false;
        m_prev_ppudata_read = 0;
        m_prev_joy1_read = 0;
        m_prev_joy2_read = 0;

        m_nmi_asserted = false;
        m_nmi_sig = false;
        m_irq_sig = false;
        m_reset_sig = false;
        m_irq_pc_no_inc = false;
        m_irq_no_mem_write = false;
        m_is_nmi = false;
        m_irq_pc_no_inc_tmp = false;
        m_irq_no_mem_write_tmp = false;
        m_is_nmi_tmp = false;

        m_instr_ctx = {0x00, 0, nullptr};
    }

    // program entry point
    this->PC = this->get_byte2(Memory::RESET_VECTOR_ADDR);
}

void
CPU::reset()
{
    m_reset_sig = true;

    m_instr_halt = false;
}

void
CPU::test_set_entry(Address i_entry)
{
    PC = i_entry;
}

void
CPU::test_set_p(Byte i_val)
{
    P = i_val;
}

bool
CPU::pre_tick(bool i_rdy, bool i_dma_op_cycle, bool &o_2002_read)
{
    m_ppustatus_read_tmp = false;
    auto defer_ret = [&o_2002_read, this]() {
        // clear some flags
        m_mask_read_tmp = false;
        // pass out outputs
        o_2002_read = m_ppustatus_read_tmp;
    };

    if (m_instr_halt)
    {
        // Don't inc "m_cycle"?
        defer_ret();
        return false;
    }

    // check to unset halt flag
    if (m_dma_halt)
    {
        if (!i_rdy)
        {
            m_dma_halt = false;
        }
    }
    // do nothing when the CPU is halted and the DMA is doing its work
    // -- inc "m_cycle"?
    // -- still poll interrupts? not sure, take it as yes for now
    // then implement it as masking out read operatoin, so we can still poll
    // interrupts
    // -- CPU must be halted in read cycle since that's when it can be halted by
    // DMA
    m_mask_read_tmp = m_dma_halt && i_dma_op_cycle;
    // common function to check to set halt flag
    // won't conflict with the above unset logic due to "i_rdy"
    auto chk_to_flag_halt = [i_rdy, this]() {
        if (!m_dma_halt)
        {
            // Can only halt on read cycle
            if (i_rdy && !m_write_tick_tmp)
            {
                m_dma_halt = true;
            }
        }
    };

    bool instr_done = false;
    // Fetch opcode
    if (!m_instr_ctx.instr)
    {
        // default to read tick so we only need to mark write operations.
        // i.e. each cycle is read cycle unless marked otherwise.
        m_write_tick_tmp = false;
        Byte opcode = get_byte(PC);
        chk_to_flag_halt();

        if (!m_dma_halt)
        {
            // IRQ/RESET/NMI/BRK all use the same behavior.
            if (in_hardware_irq())
            {
                // Force the instruction register to $00 and discard the fetch
                // opcode
                opcode = NH_BRK_OPCODE;
            }
            if (!m_irq_pc_no_inc)
            {
                ++PC;
            }

            m_instr_ctx.opcode = opcode;
            m_instr_ctx.cycle_plus1 = 0;
            m_instr_ctx.instr = &s_instr_table[opcode];
        }
    }
    // Rest cycles of the instruction
    else
    {
        /* Backup states that may be altered after one instruction cycle and may
         * be used as input of the cycle themselves */
        // Registers
        auto prevA = A;
        auto prevX = X;
        auto prevY = Y;
        auto prevPC = PC;
        auto prevS = S;
        auto prevP = P;
        // Intermediate states
        auto prev_addr_bus = m_addr_bus;
        auto prev_data_bus = m_data_bus;

        m_write_tick_tmp = false;
        m_instr_ctx.instr->frm(m_instr_ctx.cycle_plus1, this,
                               m_instr_ctx.instr->core, instr_done);
        chk_to_flag_halt();

        if (!m_dma_halt)
        {
            ++m_instr_ctx.cycle_plus1;
        }
        // Restore changes, so that repeat cycles get the same result, excluding
        // possible multiple side effects.
        else
        {
            A = prevA;
            X = prevX;
            Y = prevY;
            PC = prevPC;
            S = prevS;
            P = prevP;

            m_addr_bus = prev_addr_bus;
            m_data_bus = prev_data_bus;
            // no need for "m_page_offset" and "m_i_eff_addr"
        }
        if (instr_done)
        {
            // ------ Current
            if (!m_dma_halt)
            {
                // Use in_reset() before updating relevant flags
                if (in_reset())
                {
                    // Indicating RESET is handled.
                    m_reset_sig = false;
                }
                // Use in_nmi() before updating relevant flags
                if (in_nmi())
                {
                    // Indicating NMI is handled.
                    // @TEST: Right to do this at last cycle?
                    m_nmi_sig = false;
                }
            }

            // ------ Next
            /* Poll interrupts */
            // Poll interrupts even it's halted by DMAs (m_dma_halt)
            // Most instructions poll interrupts at the last cycle.
            // Special cases are listed and handled on thier own.
            if (m_instr_ctx.opcode == NH_BRK_OPCODE ||
                m_instr_ctx.instr->addr_mode == AddrMode::REL)
            {
                // The interrupt sequences themselves do not perform
                // interrupt polling, meaning at least one instruction from
                // the interrupt handler will execute before another
                // interrupt is serviced
                // https://www.nesdev.org/wiki/CPU_interrupts#Detailed_interrupt_behavior
                // All types of interrupts reuse the same logic as BRK for
                // the most part.

                // Branch instructions are also special, we handle it
                // in its implementation. They are identified by address
                // mode.
            }
            else
            {
                // Poll interrupts at the last cycle
                poll_interrupt();
            }

            if (!m_dma_halt)
            {
                // Swap at the end of instruction, setup states for next
                // instruction
                {
                    m_irq_pc_no_inc = m_irq_pc_no_inc_tmp;
                    m_irq_no_mem_write = m_irq_no_mem_write_tmp;
                    m_is_nmi = m_is_nmi_tmp;

                    m_irq_pc_no_inc_tmp = false;
                    m_irq_no_mem_write_tmp = false;
                    m_is_nmi_tmp = false;
                }

                // So that next instruction can continue afterwards.
                m_instr_ctx.instr = nullptr;
            }
        }
    }
    ++m_cycle;

    defer_ret();
    // When it's halted by DMCs at read cycle, flagging it as not done seems
    // reasonable. Even if the read cycle may be the last cycle of an
    // instruction.
    return m_dma_halt ? false : instr_done;
}

void
CPU::post_tick(bool i_nmi, bool i_apu_interrupt)
{
    if (m_instr_halt)
    {
        return;
    }

    // Assuming the detectors don't consider if it's halted by DMAs (m_dma_halt)

    // Edge/Level detector polls lines during φ2 of each CPU cycle.
    // So this sets up signals for NEXT cycle.
    // --- NMI
    if (!m_nmi_asserted && i_nmi)
    {
        m_nmi_sig = true; // raise an internal signal
    }
    m_nmi_asserted = i_nmi;
    // --- IRQ
    m_irq_sig = false; // inactive unless keep asserting.
    if (i_apu_interrupt)
    {
        // The cause of delayed IRQ response for some instructions.
        if (!check_flag(StatusFlag::I))
        {
            m_irq_sig = true; // raise an internal signal
        }
    }
}

bool
CPU::dma_halt() const
{
    return m_dma_halt;
}

Cycle
CPU::test_get_cycle() const
{
    return m_cycle;
}

Byte
CPU::test_get_a() const
{
    return A;
}

Byte
CPU::test_get_x() const
{
    return X;
}

Byte
CPU::test_get_y() const
{
    return Y;
}

Address
CPU::test_get_pc() const
{
    return PC;
}

Byte
CPU::test_get_s() const
{
    return S;
}

Byte
CPU::test_get_p() const
{
    return P;
}

std::vector<Byte>
CPU::test_get_instr_bytes(Address i_addr) const
{
    std::vector<Byte> bytes;
    bytes.reserve(3);

    Byte opcode = get_byte(i_addr);
    bytes.push_back(opcode);

    Byte operand_bytes = test_get_operand_bytes(opcode);
    for (decltype(operand_bytes) i = 0; i < operand_bytes; ++i)
    {
        bytes.push_back(get_byte((i_addr + 1) + i));
    }

    return bytes;
}

Byte
CPU::test_get_operand_bytes(Byte i_opcode)
{
    AddrMode addr_mode = s_instr_table[i_opcode].addr_mode;
    switch (addr_mode)
    {
        case IMP:
        case ACC:
            return 0;
            break;

        case IMM:
        case ZP0:
        case ZPX:
        case ZPY:
        case IZX:
        case IZY:
        case REL:
            return 1;
            break;

        case ABS:
        case ABX:
        case ABY:
        case IND:
            return 2;
            break;

        default:
            return 0; // absurd value
            break;
    }
}

Byte
CPU::get_byte(Address i_addr) const
{
    // Mask out read
    if (m_mask_read_tmp)
    {
        // Whatever can be returned, since after CPU resumes it will still do it
        // again (of course without "m_mask_read_tmp" set)
        return 0x00;
    }

    Address real_addr = i_addr;
    if (NH_PPU_REG_ADDR_HEAD <= i_addr && i_addr <= NH_PPU_REG_ADDR_TAIL)
    {
        real_addr = (i_addr & NH_PPU_REG_ADDR_MASK) | NH_PPU_REG_ADDR_HEAD;
    }

    Byte byte;
    // Ignore subsequent joypad reads in contiguous set of CPU cycles.
    // Verified by dmc_dma_during_read4/dma_4016_read.nes
    // https://www.nesdev.org/wiki/DMA#Register_conflicts
    /* Joypads are clocked via direct lines from the CPU, called joypad 1 /OE
     * and joypad 2 /OE, rather than going over the address bus. These output
     * enables remain asserted the entire CPU cycle and even across adjacent
     * cycles if they're both reading the same joypad register. Therefore,
     * controllers only see a single read for each contiguous set of reads of a
     * joypad register. */
    if (NH_CTRL1_REG_ADDR == real_addr && m_prev_joy1_read + 1 == m_cycle)
    {
        byte = m_memory->get_latch();
    }
    else if (NH_CTRL2_REG_ADDR == real_addr && m_prev_joy2_read + 1 == m_cycle)
    {
        byte = m_memory->get_latch();
    }
    else
    {
        bool retain_latch = false;
        Byte prev_latch = 0;
        bool return_latch = false;
        /* Ignore subsequent PPUDATA reads in contiguous set of CPU cycles */
        // Use previous open bus value to pass both
        // 1) dmc_dma_during_read4/dma_2007_read.nes
        // 2) dmc_dma_during_read4/double_2007_read.nes
        if (NH_PPUDATA_ADDR == real_addr && m_prev_ppudata_read + 1 == m_cycle)
        {
            prev_latch = m_memory->get_latch();
            retain_latch = true;
            return_latch = true;
        }

        auto err = m_memory->get_byte(i_addr, byte);
        if (NH_FAILED(err))
        {
            NH_ASSERT_ERROR_COND(false, m_logger,
                                 "Invalid memory read: ${:04X}, {}", i_addr,
                                 err);
            byte = 0xFF; // Apparent value
        }

        if (retain_latch)
        {
            m_memory->override_latch(prev_latch);
        }
        if (return_latch)
        {
            byte = m_memory->get_latch();
        }
    }

    switch (real_addr)
    {
        case NH_PPUSTATUS_ADDR:
            m_ppustatus_read_tmp = true;
            break;
        case NH_PPUDATA_ADDR:
            // Assuming get_byte() is called at most once per CPU cycle
            m_prev_ppudata_read = m_cycle;
            break;
        case NH_CTRL1_REG_ADDR:
            // Assuming get_byte() is called at most once per CPU cycle
            m_prev_joy1_read = m_cycle;
            break;
        case NH_CTRL2_REG_ADDR:
            // Assuming get_byte() is called at most once per CPU cycle
            m_prev_joy2_read = m_cycle;
            break;
        default:
            break;
    }

    return byte;
}

void
CPU::set_byte(Address i_addr, Byte i_byte)
{
    // The mark of this flag relys on the instruction implementation
    // calling read or wrtie interface each cycle.
    m_write_tick_tmp = true;

    // At hardware, this is done by setting to line to read instead of write
    if (!m_irq_no_mem_write)
    {
        auto err = m_memory->set_byte(i_addr, i_byte);
        NH_ASSERT_ERROR_COND(!NH_FAILED(err) || err == NH_ERR_READ_ONLY ||
                                 err == NH_ERR_UNAVAILABLE,
                             m_logger, "Invalid memory write: ${:04X}, {}",
                             i_addr, err);
    }
}

Byte2
CPU::get_byte2(Address i_addr) const
{
    auto lower = get_byte(i_addr);
    auto higher = get_byte(i_addr + 1);
    return to_byte2(higher, lower);
}

void
CPU::push_byte(Byte i_byte)
{
    // It may not actually write.
    NH_LOG_TRACE(m_logger, "Push byte: {:02X}", i_byte);

    set_byte(Memory::STACK_PAGE_MASK | S, i_byte);
    // The pointer decrement happens regardless of write or not
    --S;
}

void
CPU::pre_pop_byte()
{
    ++S;
}

Byte
CPU::post_pop_byte()
{
    Byte byte = get_byte(Memory::STACK_PAGE_MASK | S);
    NH_LOG_TRACE(m_logger, "Pop byte: {:02X}", byte);
    return byte;
}

bool
CPU::check_flag(StatusFlag i_flag) const
{
    return (P & i_flag) == i_flag;
}

void
CPU::set_flag(StatusFlag i_flag)
{
    P |= i_flag;
}

void
CPU::unset_flag(StatusFlag i_flag)
{
    P &= ~i_flag;
}

void
CPU::test_flag(StatusFlag i_flag, bool i_cond)
{
    i_cond ? set_flag(i_flag) : unset_flag(i_flag);
}

void
CPU::halt()
{
    NH_LOG_INFO(m_logger, "Halting...");

    m_instr_halt = true;
}

void
CPU::poll_interrupt()
{
    // Assuming CPU still polls when it is halted by DMAs (m_dma_halt)

    /* It may be polled multiple times during an instruction.
     * e.g. Branch instructions may poll 2 times if branch is taken and page is
     * crossed.
     */
    // Already pending interrupt sequence for execution next.
    bool pending_already = m_irq_pc_no_inc_tmp;
    if (pending_already)
    {
        return;
    }

    // @TEST: a) RESET has highest priority? b)RESET is like others, polled only
    // at certain points?
    if (m_reset_sig)
    {
        m_irq_pc_no_inc_tmp = true;
        m_irq_no_mem_write_tmp = true;
    }
    // NMI has higher precedence than IRQ
    // https://www.nesdev.org/wiki/NMI
    else if (m_nmi_sig)
    {
        m_irq_pc_no_inc_tmp = true;
        m_is_nmi_tmp = true;
    }
    else if (m_irq_sig)
    {
        m_irq_pc_no_inc_tmp = true;
    }
}

/// @brief Is running hardware interrupts (IRQ/RESET/NMI)
bool
CPU::in_hardware_irq() const
{
    return m_irq_pc_no_inc;
}

/// @brief Is running NMI interrupt sequence
bool
CPU::in_nmi() const
{
    return m_is_nmi;
}

/// @brief Is running RESET interrupt sequence
bool
CPU::in_reset() const
{
    return /*in_hardware_irq() &&*/ m_irq_no_mem_write;
}

} // namespace nh

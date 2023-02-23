#include "cpu.hpp"

#include "console/byte_utils.hpp"
#include "console/assert.hpp"
#include "console/spec.hpp"
#include "console/ppu/ppu.hpp"
#include "console/apu/apu.hpp"

#define LN_BRK_OPCODE 0

namespace ln {

CPU::CPU(Memory *i_memory, PPU *i_ppu, const APU *i_apu)
    : m_memory(i_memory)
    , m_ppu(i_ppu)
    , m_apu(i_apu)
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
        m_halted_instr = false;

        m_nmi_asserted = false;
        m_nmi_sig = false;
        m_irq_sig = false;
        m_reset_sig = false;
        m_irq_pc_no_inc = false;
        m_irq_no_mem_write = false;
        m_is_nmi = false;

        m_instr_ctx = {0x00, 0, nullptr};

        m_oam_dma_ctx.ongoing = false;
    }

    // program entry point
    this->PC = this->get_byte2(Memory::RESET_VECTOR_ADDR);
}

void
CPU::reset()
{
    m_reset_sig = true;
}

void
CPU::set_entry_test(Address i_entry)
{
    PC = i_entry;
}

void
CPU::set_p_test(Byte i_val)
{
    P = i_val;
}

bool
CPU::pre_tick(bool &o_2002_read)
{
    this->m_2002_read = false;
    auto write_out = [&o_2002_read, this]() {
        o_2002_read = this->m_2002_read;
    };

    if (m_halted_instr)
    {
        write_out();
        return false;
    }

    bool instr_done = false;
    /* OAM DMA transfer */
    if (m_oam_dma_ctx.ongoing)
    {
        if (m_oam_dma_ctx.counter == 0)
        {
            // @IMPL: 1 wait state cycle while waiting for writes to complete,
            // +1 if on an odd CPU cycle
            // @TODO: What wait state cycle?
            m_oam_dma_ctx.start_counter = m_cycle % 2 == 0 ? 1 : 2;
        }
        if (m_oam_dma_ctx.counter >= m_oam_dma_ctx.start_counter)
        {
            auto dma_counter =
                (m_oam_dma_ctx.counter - m_oam_dma_ctx.start_counter);
            if (dma_counter > 256 * 2 - 1)
            {
                LN_ASSERT_FATAL(
                    "Invalid numeric value, programming error: {}, {}",
                    m_oam_dma_ctx.counter, m_oam_dma_ctx.start_counter);
                write_out();
                return false;
            }
            Byte byte_idx = Byte(dma_counter / 2);

            /* read */
            if (dma_counter % 2 == 0)
            {
                Address addr = (m_oam_dma_ctx.upper << 8) | byte_idx;
                m_oam_dma_ctx.tmp = this->get_byte(addr);
            }
            else
            {
                m_ppu->write_register(PPU::OAMDATA, m_oam_dma_ctx.tmp);

                /* end of the transfer process */
                if (byte_idx >= 256 - 1)
                {
                    m_oam_dma_ctx.ongoing = false;
                }
            }
        }
        ++m_oam_dma_ctx.counter;
    }
    /* CPU is not halted so it can execute instruction or handle interrupts */
    else
    {
        // Fetch opcode
        if (!m_instr_ctx.instr)
        {
            Byte opcode = get_byte(PC);
            // IRQ/RESET/NMI/BRK all use the same behavior.
            if (hardware_interrupts())
            {
                // Force the instruction register to $00 and discard the fetch
                // opcode
                opcode = LN_BRK_OPCODE;
            }
            if (!m_irq_pc_no_inc)
            {
                ++PC;
            }

            m_instr_ctx.opcode = opcode;
            m_instr_ctx.instr = &s_instr_table[opcode];
            m_instr_ctx.cycle_plus1 = 0;
        }
        // Rest cycles of the instruction
        else
        {
            m_instr_ctx.instr->frm(m_instr_ctx.cycle_plus1, this,
                                   m_instr_ctx.instr->core, instr_done);
            ++m_instr_ctx.cycle_plus1;

            if (instr_done)
            {
                // @NOTE: Use is_reset() before clearing relevant flags
                if (is_reset())
                {
                    // Indicating RESET is handled.
                    m_reset_sig = false;

                    // Some work specific to RESET
                    // @TEST: Is this necessary?
                    m_cycle = 0;
                }
                // @NOTE: Use is_nmi() before clearing relevant flags
                if (is_nmi())
                {
                    // Indicating NMI is handled.
                    // @TEST: Is this done at last cycle?
                    m_nmi_sig = false;
                }
                /* Clear flags which last for one instruction */
                // @NOTE: Do this before poll_interrupt() as it may set these
                // flags.
                m_irq_pc_no_inc = false;
                m_irq_no_mem_write = false;
                m_is_nmi = false;

                /* Poll interrupts */
                // @TODO: Deal with DMA
                // For most instructions, poll interrupts at the last cycle,
                // Special cases are listed.
                if (m_instr_ctx.opcode == LN_BRK_OPCODE ||
                    m_instr_ctx.instr->addr_mode == AddrMode::REL)
                {
                    // The interrupt sequences themselves do not perform
                    // interrupt polling, meaning at least one instruction from
                    // the interrupt handler will execute before another
                    // interrupt is serviced
                    // https://www.nesdev.org/wiki/CPU_interrupts#Detailed_interrupt_behavior
                    // All types of interrupts mostly reuse the same logic as
                    // BRK.

                    // Branch instructions are also special, we handle it
                    // in its implementation. They are identified by address
                    // mode.
                }
                else
                {
                    // Poll interrupts at the last cycle
                    poll_interrupt();
                }

                // So that next instruction can continue afterwards.
                m_instr_ctx.instr = nullptr;
            }
        }
    }
    ++m_cycle;

    write_out();
    return instr_done;
}

void
CPU::post_tick()
{
    if (m_halted_instr)
    {
        return;
    }

    // @NOTE: Edge/Level detector polls lines during φ2 of each CPU cycle.
    // So this sets up signals for NEXT cycle.
    // @TEST: Whether we still polls lines even when the CPU is
    // being halted by DMA. If we do, the good side is that the poll happens
    // across adjacent cycles.
    // --- NMI
    if (!m_nmi_asserted && m_ppu->nmi())
    {
        m_nmi_sig = true; // raise an internal signal
    }
    m_nmi_asserted = m_ppu->nmi();
    // --- IRQ
    m_irq_sig = false; // inactive unless keep asserting.
    if (m_apu->interrupt())
    {
        // @IMPL: The cause of delayed IRQ response for some instructions.
        if (!check_flag(StatusFlag::I))
        {
            m_irq_sig = true; // raise an internal signal
        }
    }
}

Cycle
CPU::get_cycle() const
{
    return m_cycle;
}

Byte
CPU::get_a() const
{
    return A;
}

Byte
CPU::get_x() const
{
    return X;
}

Byte
CPU::get_y() const
{
    return Y;
}

Address
CPU::get_pc() const
{
    return PC;
}

Byte
CPU::get_s() const
{
    return S;
}

Byte
CPU::get_p() const
{
    return P;
}

std::vector<Byte>
CPU::get_instr_bytes(Address i_addr) const
{
    std::vector<Byte> bytes;
    bytes.reserve(3);

    Byte opcode = get_byte(i_addr);
    bytes.push_back(opcode);

    Byte operand_bytes = get_operand_bytes(opcode);
    for (decltype(operand_bytes) i = 0; i < operand_bytes; ++i)
    {
        bytes.push_back(get_byte((i_addr + 1) + i));
    }

    return bytes;
}

Byte
CPU::get_operand_bytes(Byte i_opcode)
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

void
CPU::init_oam_dma(Byte i_val)
{
    if (m_oam_dma_ctx.ongoing)
    {
        LN_ASSERT_FATAL(
            "Initialize a OAMDMA transfer while one is already going on.");
        return;
    }
    m_oam_dma_ctx.ongoing = true;
    m_oam_dma_ctx.upper = i_val;
    m_oam_dma_ctx.counter = 0;
}

Byte
CPU::get_byte(Address i_addr) const
{
    if (LN_PPUSTATUS_ADDR == i_addr)
    {
        m_2002_read = true;
    }

    Byte byte;
    auto err = m_memory->get_byte(i_addr, byte);
    if (LN_FAILED(err))
    {
        LN_ASSERT_ERROR_COND(false, "Invalid memory read: ${:04X}, {}", i_addr,
                             err);
        byte = 0xFF; // Apparent value
    }
    return byte;
}

void
CPU::set_byte(Address i_addr, Byte i_byte)
{
    auto err = m_memory->set_byte(i_addr, i_byte);
    LN_ASSERT_ERROR_COND(!LN_FAILED(err) || err == Error::READ_ONLY ||
                             err == Error::UNAVAILABLE,
                         "Invalid memory write: ${:04X}, {}", i_addr, err);
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
    pre_push_byte(i_byte);
    post_push_byte();
}

void
CPU::pre_push_byte(Byte i_byte)
{
    set_byte(Memory::STACK_PAGE_MASK | S, i_byte);
    LN_LOG_TRACE(ln::get_logger(), "Push byte: {:02X}", i_byte);
}

void
CPU::post_push_byte()
{
    --S;
}

Byte
CPU::pop_byte()
{
    pre_pop_byte();
    return post_pop_byte();
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
    LN_LOG_TRACE(ln::get_logger(), "Pop byte: {:02X}", byte);
    return byte;
}

void
CPU::push_byte2(Byte2 i_byte2)
{
    // little-endian, so we stuff in higher byte first.
    push_byte(Byte(i_byte2 >> 8));
    push_byte(Byte(i_byte2));
}

Byte2
CPU::pop_byte2()
{
    // see push_byte2() for details.
    auto lower = pop_byte();
    auto higher = pop_byte();
    return to_byte2(higher, lower);
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
    LN_LOG_INFO(ln::get_logger(), "Halting...");

    m_halted_instr = true;
}

void
CPU::poll_interrupt()
{
    /* It may be polled multiple times during an instruction.
     * e.g. Branch instructions may poll 2 times if branch is taken and page is
     * crossed.
     */
    if (hardware_interrupts())
    {
        return;
    }

    // @TEST: RESET has highest priority?
    // @TEST: RESET is like others, polled only at certain points?
    if (m_reset_sig)
    {
        m_irq_pc_no_inc = true;
        m_irq_no_mem_write = true;
    }
    // NMI has higher precedence than IRQ
    // https://www.nesdev.org/wiki/NMI
    else if (m_nmi_sig)
    {
        m_irq_pc_no_inc = true;
        m_is_nmi = true;
    }
    else if (m_irq_sig)
    {
        m_irq_pc_no_inc = true;
    }
}

/// @brief Is running (or to run) hardware interrupts (IRQ/RESET/NMI)
bool
CPU::hardware_interrupts() const
{
    return m_irq_pc_no_inc;
}

/// @brief Is running (or to run) NMI interrupt sequence
bool
CPU::is_nmi() const
{
    return m_is_nmi;
}

bool
CPU::is_reset() const
{
    return /*hardware_interrupts() &&*/ m_irq_no_mem_write;
}

} // namespace ln

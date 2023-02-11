#include "cpu.hpp"

#include "console/byte_utils.hpp"
#include "console/assert.hpp"
#include "console/spec.hpp"
#include "console/ppu/ppu.hpp"
#include "console/apu/apu.hpp"

namespace ln {

CPU::CPU(Memory *i_memory, PPU *i_ppu, const APU *i_apu)
    : m_memory(i_memory)
    , m_ppu(i_ppu)
    , m_apu(i_apu)
    , m_cycle(0)
    , m_halted(false)
    , m_nmi(false)
    , m_addr_bus(0)
    , m_data_bus(0)
    , m_instr_ctx{0, nullptr}
{
    m_oam_dma_ctx.ongoing = false;
}

void
CPU::power_up()
{
    // @TODO: powerup test

    // https://wiki.nesdev.org/w/index.php?title=CPU_power_up_state
    P = 0x34; // (IRQ disabled)
    A = X = Y = 0;
    S = 0xFD;

    // consistent RAM startup state
    m_memory->set_bulk(LN_INTERNAL_RAM_ADDR_HEAD, LN_INTERNAL_RAM_ADDR_TAIL,
                       0xFF);

    /* implementation state */
    // set program entry point
    this->PC = this->get_byte2(Memory::RESET_VECTOR_ADDR);
}

void
CPU::reset()
{
    // @TODO: reset test

    S -= 3;
    set_flag(StatusFlag::I); // The I (IRQ disable) flag was set to true

    /* implementation state */
    // @TODO: Go through these when implementing reset.
    m_cycle = 0;
    m_halted = false;

    m_nmi = false;

    m_oam_dma_ctx.ongoing = false;
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
CPU::tick()
{
    if (m_halted)
    {
        return false;
    }

    bool instr_done = false;
    Cycle curr_cycle = m_cycle;
    /* OAM DMA transfer */
    if (m_oam_dma_ctx.ongoing)
    {
        if (m_oam_dma_ctx.counter == 0)
        {
            // @IMPL: 1 wait state cycle while waiting for writes to complete,
            // +1 if on an odd CPU cycle
            // @TODO: What wait state cycle?
            m_oam_dma_ctx.start_counter = curr_cycle % 2 == 0 ? 1 : 2;
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
    /* Instruction execution */
    else
    {
        // Fetch opcode
        if (!m_instr_ctx.instr)
        {
            Byte opcode = get_byte(PC++);
            m_instr_ctx.instr = &s_instr_table[opcode];
            m_instr_ctx.cycle_plus1 = 0;
        }
        // Rest of the instruction
        else
        {
            m_instr_ctx.instr->frm(m_instr_ctx.cycle_plus1, this,
                                   m_instr_ctx.instr->core, instr_done);
            ++m_instr_ctx.cycle_plus1;
            if (instr_done)
            {
                // So that next instruction can continue afterwards.
                m_instr_ctx.instr = nullptr;

                /* Check interrupts at the last cycle of each instruction */
                // @TEST: Don't count PPU DMA.
                poll_interrupt();
            }
        }
    }
    ++m_cycle;

    return instr_done;
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

void
CPU::set_nmi(bool i_flag)
{
    m_nmi = i_flag;
}

Byte
CPU::get_byte(Address i_addr) const
{
    Byte byte;
    auto err = m_memory->get_byte(i_addr, byte);

    if (LN_FAILED(err))
    {
        if (err == Error::UNAVAILABLE)
        {
            // @TODO: Open bus?
            byte = 0xFF;
        }
        else
        {
            LN_ASSERT_ERROR_COND(false, "Invalid memory read: ${:04X}, {}",
                                 i_addr, err);
            byte = 0xFF; // Apparent value
        }
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
    set_byte(Memory::STACK_PAGE_MASK | S, i_byte);
    LN_LOG_TRACE(ln::get_logger(), "Push byte: {:02X}", i_byte);
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

    m_halted = true;
}

void
CPU::poll_interrupt()
{
    // @TODO: Detailed implementation of interrupts.
    // https://www.nesdev.org/wiki/CPU_interrupts
    // Specifically, the delayed response.
    // https://www.nesdev.org/wiki/CPU_interrupts#Delayed_IRQ_response_after_CLI,_SEI,_and_PLP
    // Test that with interrupts test roms.

    // @NOTE: The interrupt sequences themselves do not perform interrupt
    // polling, meaning at least one instruction from the interrupt handler will
    // execute before another interrupt is serviced.
    // @IMPL: "interrupt sequences" should refer to the pushes and jumps, etc.
    // And this should be implemented already by polling only at the end of
    // the instruction.

    // NMI has higher precedence than IRQ
    // https://www.nesdev.org/wiki/NMI
    if (m_nmi)
    {
        push_byte2(this->PC);

        // @QUIRK: https://www.nesdev.org/wiki/Status_flags#The_B_flag
        push_byte((this->P & ~StatusFlag::B) | StatusFlag::U);
        set_flag(StatusFlag::I);

        // Jump to interrupt handler.
        this->PC = get_byte2(Memory::NMI_VECTOR_ADDR);

        // Clear the flag
        m_nmi = false;
    }
    // https://www.nesdev.org/wiki/IRQ
    // @TEST: Test this
    else if (!check_flag(StatusFlag::I) && m_apu->interrupt())
    {
        push_byte2(this->PC);

        // @QUIRK: https://www.nesdev.org/wiki/Status_flags#The_B_flag
        push_byte((this->P & ~StatusFlag::B) | StatusFlag::U);
        set_flag(StatusFlag::I);

        // Jump to interrupt handler.
        this->PC = get_byte2(Memory::IRQ_VECTOR_ADDR);
    }
}

void
CPU::report_exec_error(const std::string &i_msg)
{
    // @TODO: Current executing instruction?
    LN_ASSERT_ERROR(i_msg);
}

} // namespace ln

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
    , m_next_stage(Stage::DECODE)
    , m_next_stage_cycle(0)
{
    m_oam_dma_ctx.ongoing = false;
}

void
CPU::power_up()
{
    // @TODO: powerup test

    // https://wiki.nesdev.org/w/index.php?title=CPU_power_up_state
    // P = 0x34; // (IRQ disabled)
    P = 0x24; // according to nestest.log (IRQ disabled still)
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

    m_next_stage = Stage::DECODE;
    m_next_stage_cycle = 0;

    m_oam_dma_ctx.ongoing = false;
}

void
CPU::set_entry_test(Address i_entry)
{
    PC = i_entry;
}

Cycle
CPU::tick()
{
    // @TODO: Support real cycle-level emulation?

    if (m_halted)
    {
        return 1;
    }

    Cycle instr_cycles = 0;
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
                return 1;
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

        instr_cycles = 1;
        ++m_next_stage_cycle;
    }
    /* Normal execution flow */
    else
    {
        if (curr_cycle >= m_next_stage_cycle)
        {
            if (curr_cycle > m_next_stage_cycle)
            {
                LN_ASSERT_FATAL("Wrong CPU cycle management: {}, {}",
                                curr_cycle, m_next_stage_cycle);
                return 1;
            }

            switch (m_next_stage)
            {
                case Stage::DECODE:
                {
                    Byte opcode_byte = get_byte(PC++);
                    Opcode opcode = {opcode_byte};
                    InstructionDesc instr_desc = get_instr_desc(opcode);

                    m_stage_ctx.opcode = opcode;
                    m_stage_ctx.instr_desc = instr_desc;
                    if (instr_desc.cycle_base <= 1)
                    {
                        LN_ASSERT_FATAL("Invalid cycle base for {}",
                                        opcode_byte);
                        return 1;
                    }
                    m_stage_ctx.instr_cycles = instr_desc.cycle_base;

                    m_next_stage = Stage::FETCH_EXEC;
                    // postpone the rest of the work at the last cycle.
                    m_next_stage_cycle += instr_desc.cycle_base - 1;
                }
                break;

                case Stage::FETCH_EXEC:
                {
                    const auto &opcode = m_stage_ctx.opcode;
                    const auto &instr_desc = m_stage_ctx.instr_desc;

                    ParseFunc parse = get_address_parse(opcode);
                    bool read_page_crossing = false;
                    Byte operand_bytes;
                    Operand operand = parse(this, operand_bytes,
                                            instr_desc.cycle_page_dependent
                                                ? &read_page_crossing
                                                : nullptr);
                    PC += operand_bytes;

                    Cycle extra_branch_cycles = 0;
                    ExecFunc exec = get_opcode_exec(opcode);
                    exec(this, operand, extra_branch_cycles);

                    Cycle extra_cycles =
                        (Cycle)read_page_crossing + extra_branch_cycles;
                    m_stage_ctx.instr_cycles += extra_cycles;

                    if (!extra_cycles)
                    {
                        instr_cycles = m_stage_ctx.instr_cycles;

                        m_next_stage = Stage::DECODE;
                        m_next_stage_cycle += 1;
                    }
                    else
                    {
                        m_next_stage = Stage::LAST_CYCLE;
                        m_next_stage_cycle += extra_cycles;
                    }
                }
                break;

                case Stage::LAST_CYCLE:
                {
                    instr_cycles = m_stage_ctx.instr_cycles;

                    m_next_stage = Stage::DECODE;
                    m_next_stage_cycle += 1;
                }
                break;

                default:
                {
                    LN_ASSERT_FATAL("Undefined CPU stage: {}", m_next_stage);
                    return 1;
                }
                break;
            }
        }

        /* Check interrupts at the last cycle of each normal instruction */
        // @TEST: Don't count PPU DMA.
        if (instr_cycles)
        {
            poll_interrupt();
        }
    }
    ++m_cycle;

    return instr_cycles;
}

bool
CPU::step_test()
{
    if (m_halted)
    {
        return false;
    }

    Byte opcode_byte = get_byte(PC++);
    Opcode opcode = {opcode_byte};
    InstructionDesc instr_desc = get_instr_desc(opcode);

    ParseFunc parse = get_address_parse(opcode);
    bool read_page_crossing = false;
    Byte operand_bytes;
    Operand operand =
        parse(this, operand_bytes,
              instr_desc.cycle_page_dependent ? &read_page_crossing : nullptr);
    PC += operand_bytes;

    Cycle extra_branch_cycles = 0;
    ExecFunc exec = get_opcode_exec(opcode);
    exec(this, operand, extra_branch_cycles);

    Cycle cycles_spent =
        instr_desc.cycle_base + (Cycle)read_page_crossing + extra_branch_cycles;
    m_cycle += cycles_spent;

    return true;
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

    Byte opcode_byte = get_byte(i_addr);
    Opcode opcode = {opcode_byte};
    bytes.push_back(opcode_byte);

    ParseFunc parse = get_address_parse(opcode);
    Byte operand_bytes;
    (void)parse(this, operand_bytes, nullptr);
    for (Byte i = 0; i < operand_bytes; ++i)
    {
        bytes.push_back(get_byte((i_addr + 1) + i));
    }

    return bytes;
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

auto
CPU::get_opcode_exec(Opcode i_opcode) -> ExecFunc
{
    auto ret = s_instr_map[i_opcode].exec_func;
    LN_ASSERT_FATAL_COND(
        ret, "Config error, empty ExecFunc for instruction: {}", i_opcode);
    return ret;
}

auto
CPU::get_address_parse(Opcode i_opcode) -> ParseFunc
{
    auto ret = s_address_mode_map[get_address_mode(i_opcode)].parse_func;
    LN_ASSERT_FATAL_COND(
        ret, "Config error, empty ParseFunc for instruction: {}", i_opcode);
    return ret;
}

auto
CPU::get_instr_desc(Opcode i_opcode) -> InstructionDesc
{
    return s_instr_map[i_opcode];
}

OpcodeType
CPU::get_op_code(Opcode i_opcode)
{
    return s_instr_map[i_opcode].op_code;
}

AddressMode
CPU::get_address_mode(Opcode i_opcode)
{
    return s_instr_map[i_opcode].address_mode;
}

Byte
CPU::get_byte(Address i_addr) const
{
    Byte byte;
    auto err = m_memory->get_byte(i_addr, byte);
    LN_ASSERT_ERROR_COND(!LN_FAILED(err), "Invalid memory read: ${:04X}, {}",
                         i_addr, err);
    return byte;
}

void
CPU::set_byte(Address i_addr, Byte i_byte)
{
    auto err = m_memory->set_byte(i_addr, i_byte);
    LN_ASSERT_ERROR_COND(!LN_FAILED(err), "Invalid memory write: ${:04X}, {}",
                         i_addr, err);
}

Byte2
CPU::get_byte2(Address i_addr) const
{
    auto lower = get_byte(i_addr);
    auto higher = get_byte(i_addr + 1);
    return byte2_from_bytes(higher, lower);
}

void
CPU::push_byte(Byte i_byte)
{
    if (S == 0)
    {
        LN_ASSERT_ERROR("Stack overflow! PC: ${:04X}", PC);
    }

    set_byte(Memory::STACK_PAGE_MASK | S, i_byte);
    LN_LOG_TRACE(ln::get_logger(), "Push byte: {:02X}", i_byte);
    --S;
}

Byte
CPU::pop_byte()
{
    if (S == (Byte)-1)
    {
        LN_ASSERT_ERROR("Stack underflow! PC: ${:04X}", PC);
    }

    ++S;
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
    return byte2_from_bytes(higher, lower);
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

Byte
CPU::get_operand(Operand i_operand) const
{
    switch (i_operand.type)
    {
        case OperandType::VALUE:
            return i_operand.value;
            break;
        case OperandType::ADDRESS:
            return get_byte(i_operand.address);
            break;
        case OperandType::ACC:
            return this->A;
            break;
        default:
            LN_ASSERT_ERROR("Unsupported operand type: {}", i_operand.type);
            return (Byte)-1;
            break;
    }
}

Error
CPU::set_operand(Operand i_operand, Byte i_byte)
{
    switch (i_operand.type)
    {
        case OperandType::ADDRESS:
            set_byte(i_operand.address, i_byte);
            break;
        case OperandType::ACC:
            A = i_byte;
            break;
        default:
            return Error::INVALID_ARGUMENT;
            break;
    }
    return Error::OK;
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

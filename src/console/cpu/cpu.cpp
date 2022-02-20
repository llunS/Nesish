#include "cpu.hpp"

#include "console/byte_utils.hpp"
#include "console/assert.hpp"

namespace ln {

CPU::CPU(Memory *i_memory)
    : m_memory(i_memory)
{
}

void
CPU::power_up()
{
    // https://wiki.nesdev.org/w/index.php?title=CPU_power_up_state
    P = 0x34; // (IRQ disabled)
    A = X = Y = 0;
    S = 0xFD;

    m_memory->set_byte(0x4017, 0x00); // (frame irq enabled)
    m_memory->set_byte(0x4015, 0x00); // (all channels disabled)
    m_memory->set_range(0x4000, 0x400F, 0x00);
    m_memory->set_range(0x4010, 0x4013, 0x00);

    // All 15 bits of noise channel LFSR = $0000[5]. The first time the LFSR is
    // clocked from the all-0s state, it will shift in a 1.
    // @TODO: noise channel LFSR

    m_memory->set_apu_frame_counter(0x0); // 2A03G

    // consistent RAM startup state
    m_memory->set_range(0x0000, 0x07FF, 0xFF);
}

void
CPU::reset()
{
    S -= 3;
    set_flag(StatusFlag::I); // The I (IRQ disable) flag was set to true
    m_memory->set_byte(0x4015, 0x00); // APU was silenced
    // @TODO: APU triangle phase is reset to 0 (i.e. outputs a value of 15, the
    // first step of its waveform)
    // @TODO: APU DPCM output ANDed with 1 (upper 6 bits cleared)
    m_memory->set_apu_frame_counter(0x0);
}

void
CPU::step()
{
    // @TODO: instruction execution loop.
}

auto
CPU::get_opcode_exec(Instruction i_instr) -> ExecFunc
{
    auto ret = s_instr_map[i_instr].exec_func;
    ASSERT_ERROR(!ret, "Config error, empty ExecFunc for instruction: {}",
                 i_instr);
    return ret;
}

auto
CPU::get_address_parse(Instruction i_instr) -> ParseFunc
{
    auto ret = s_address_mode_map[get_address_mode(i_instr)].parse_func;
    ASSERT_ERROR(!ret, "Config error, empty ParseFunc for instruction: {}",
                 i_instr);
    return ret;
}

Opcode
CPU::get_op_code(Instruction i_instr)
{
    return s_instr_map[i_instr].op_code;
}

AddressMode
CPU::get_address_mode(Instruction i_instr)
{
    return s_instr_map[i_instr].address_mode;
}

Byte
CPU::get_byte(Address i_address) const
{
    return m_memory->get_byte(i_address);
}

void
CPU::set_byte(Address i_address, Byte i_byte)
{
    return m_memory->set_byte(i_address, i_byte);
}

void
CPU::push_byte(Byte i_byte)
{
    set_byte(STACK_BASE + S, i_byte);
    --S;
}

Byte
CPU::pop_byte()
{
    ++S;
    return get_byte(STACK_BASE + S);
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
            return A;
            break;
        default:
            ASSERT_ERROR(false, "Unsupported operand type: {}", i_operand.type);
            return -1;
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
    // @TODO
}

void
CPU::report_exec_error(const std::string &i_msg)
{
    // @TODO: Current executing instruction.
    ASSERT_ERROR(false, i_msg);
}

auto
CPU::get_circle(Cycle i_base, bool i_read_page_crossing, Cycle i_branch_cycles)
    -> Cycle
{
    // if "i_branch_cycles" is not 0 (i.e. it's referring to branch
    // instruction), then "i_read_page_crossing" should be 0.
    return i_base + i_read_page_crossing + i_branch_cycles;
}

} // namespace ln

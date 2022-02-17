#include "cpu.hpp"

#include "console/byte_utils.hpp"
#include "console/assert.hpp"

namespace ln {

CPU::CPU(Memory *i_memory)
    : m_memory(i_memory)
{
}

auto
CPU::get_opcode_exec(Instruction i_instr) -> ExecFunc
{
    return s_instr_map[i_instr].exec_func;
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

} // namespace ln

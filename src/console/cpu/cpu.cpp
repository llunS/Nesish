#include "cpu.hpp"

#include "console/byte_utils.hpp"
#include "console/assert.hpp"
#include "console/spec.hpp"

namespace ln {

CPU::CPU(Memory *i_memory)
    : m_memory(i_memory)
    , m_cycles(0)
    , m_halted(false)
{
}

void
CPU::power_up()
{
    // https://wiki.nesdev.org/w/index.php?title=CPU_power_up_state
    // P = 0x34; // (IRQ disabled)
    P = 0x24; // according to nestest.log, (IRQ disabled) still
    A = X = Y = 0;
    S = 0xFD;

    // @TODO NES APU and I/O registers
    for (Address i = 0x4000; i <= 0x400F; ++i)
    {
        (void)m_memory->set_byte(i, 0x00);
    }
    for (Address i = 0x4010; i <= 0x4013; ++i)
    {
        (void)m_memory->set_byte(i, 0x00);
    }
    (void)m_memory->set_byte(0x4015, 0x00); // (all channels disabled)
    (void)m_memory->set_byte(0x4017, 0x00); // (frame irq enabled)

    // All 15 bits of noise channel LFSR = $0000[5]. The first time the LFSR is
    // clocked from the all-0s state, it will shift in a 1.
    // @TODO: noise channel LFSR

    (void)m_memory->set_byte(LN_APU_FC_ADDRESS, 0x00); // 2A03G

    // consistent RAM startup state
    m_memory->set_bulk(LN_INTERNAL_RAM_ADDRESS_HEAD,
                       LN_INTERNAL_RAM_ADDRESS_TAIL, 0xFF);
}

void
CPU::reset()
{
    S -= 3;
    set_flag(StatusFlag::I); // The I (IRQ disable) flag was set to true
    // @TODO NES APU and I/O registers
    (void)m_memory->set_byte(0x4015, 0x00); // APU was silenced
    // @TODO: APU triangle phase is reset to 0 (i.e. outputs a value of 15, the
    // first step of its waveform)
    // @TODO: APU DPCM output ANDed with 1 (upper 6 bits cleared)

    (void)m_memory->set_byte(LN_APU_FC_ADDRESS, 0x00);

    m_halted = false;
    m_cycles = 0;
}

void
CPU::set_entry(Address i_entry)
{
    PC = i_entry;
}

bool
CPU::step()
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
        instr_desc.cycle_base + read_page_crossing + extra_branch_cycles;
    m_cycles += cycles_spent;

    return true;
}

Cycle
CPU::get_cycle() const
{
    return m_cycles;
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
CPU::get_instruction_bytes(Address i_addr) const
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

auto
CPU::get_opcode_exec(Opcode i_opcode) -> ExecFunc
{
    auto ret = s_instr_map[i_opcode].exec_func;
    ASSERT_ERROR(ret, "Config error, empty ExecFunc for instruction: {}",
                 i_opcode);
    return ret;
}

auto
CPU::get_address_parse(Opcode i_opcode) -> ParseFunc
{
    auto ret = s_address_mode_map[get_address_mode(i_opcode)].parse_func;
    ASSERT_ERROR(ret, "Config error, empty ParseFunc for instruction: {}",
                 i_opcode);
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
    // @TODO: report invalid operation
    ASSERT_ERROR(!LN_FAILED(err), "Invalid memory read: ${:04X}", i_addr);
    return byte;
}

void
CPU::set_byte(Address i_addr, Byte i_byte)
{
    auto err = m_memory->set_byte(i_addr, i_byte);
    // @TODO: report invalid operation
    ASSERT_ERROR(!LN_FAILED(err), "Invalid memory write: ${:04X}", i_addr);
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
        get_logger()->error("Stack overflow! PC: ${:04X}", PC);
    }

    set_byte(Memory::STACK_PAGE_MASK | S, i_byte);
    get_logger()->trace("Push byte: {:02X}", i_byte);
    --S;
}

Byte
CPU::pop_byte()
{
    if (S == (Byte)-1)
    {
        get_logger()->error("Stack underflow! PC: ${:04X}", PC);
    }

    ++S;
    Byte byte = get_byte(Memory::STACK_PAGE_MASK | S);
    get_logger()->trace("Pop byte: {:02X}", byte);
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
    get_logger()->info("Halting...");

    m_halted = true;
}

void
CPU::report_exec_error(const std::string &i_msg)
{
    // @TODO: Current executing instruction?
    ASSERT_ERROR(false, i_msg);
}

} // namespace ln

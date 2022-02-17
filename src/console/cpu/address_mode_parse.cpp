#include "address_mode_parse.hpp"

#include "console/byte_utils.hpp"

namespace ln {

ln::Operand
CPU::AddressModeParse::parse_imp(ln::CPU *i_cpu)
{
    (void)(i_cpu);
    // Return a dummy value, since it's implicit anyway.
    return Operand::from_value(0);
}

ln::Operand
CPU::AddressModeParse::parse_acc(ln::CPU *i_cpu)
{
    (void)(i_cpu);
    return Operand::from_acc();
}

ln::Operand
CPU::AddressModeParse::parse_imm(ln::CPU *i_cpu)
{
    return Operand::from_value(i_cpu->get_byte(i_cpu->PC++));
}

ln::Operand
CPU::AddressModeParse::parse_zp0(ln::CPU *i_cpu)
{
    Byte arg = i_cpu->get_byte(i_cpu->PC++);
    return Operand::from_address(arg);
}

ln::Operand
CPU::AddressModeParse::parse_zpx(ln::CPU *i_cpu)
{
    Byte arg = i_cpu->get_byte(i_cpu->PC++);
    return Operand::from_address(((Byte2)arg + i_cpu->X) & 0xff);
}

ln::Operand
CPU::AddressModeParse::parse_zpy(ln::CPU *i_cpu)
{
    Byte arg = i_cpu->get_byte(i_cpu->PC++);
    return Operand::from_address(((Byte2)arg + i_cpu->Y) & 0xff);
}

ln::Operand
CPU::AddressModeParse::parse_izx(ln::CPU *i_cpu)
{
    Byte arg = i_cpu->get_byte(i_cpu->PC++);

    Byte lower = i_cpu->get_byte(((Byte2)arg + i_cpu->X) & 0xff);
    Byte higher = i_cpu->get_byte(((Byte2)arg + i_cpu->X + 1) & 0xff);
    Address address = byte2_from_bytes(higher, lower);

    return Operand::from_address(address);
}

ln::Operand
CPU::AddressModeParse::parse_izy(ln::CPU *i_cpu)
{
    Byte arg = i_cpu->get_byte(i_cpu->PC++);

    Byte lower = i_cpu->get_byte(arg);
    Byte higher = i_cpu->get_byte(((Byte2)arg + 1) & 0xff);
    Address address = byte2_from_bytes(higher, lower) + i_cpu->Y;

    return Operand::from_address(address);
}

ln::Operand
CPU::AddressModeParse::parse_abs(ln::CPU *i_cpu)
{
    Byte2 arg = get_byte2(i_cpu, i_cpu->PC);
    i_cpu->PC += 2;
    return Operand::from_address(arg);
}

ln::Operand
CPU::AddressModeParse::parse_abx(ln::CPU *i_cpu)
{
    Byte2 arg = get_byte2(i_cpu, i_cpu->PC);
    i_cpu->PC += 2;
    return Operand::from_address(arg + i_cpu->X);
}

ln::Operand
CPU::AddressModeParse::parse_aby(ln::CPU *i_cpu)
{
    Byte2 arg = get_byte2(i_cpu, i_cpu->PC);
    i_cpu->PC += 2;
    return Operand::from_address(arg + i_cpu->Y);
}

ln::Operand
CPU::AddressModeParse::parse_ind(ln::CPU *i_cpu)
{
    Byte2 arg = get_byte2(i_cpu, i_cpu->PC);
    i_cpu->PC += 2;

    // https://wiki.nesdev.org/w/index.php/Errata
    if ((arg & 0xff) == 0xff)
    {
        // the indirect address value does not advance pages.
        Byte lower = i_cpu->get_byte(arg);
        Byte higher = i_cpu->get_byte(arg & 0xff00);
        Address address = byte2_from_bytes(higher, lower);
        return Operand::from_address(address);
    }
    else
    {
        Address address = get_byte2(i_cpu, arg);
        return Operand::from_address(address);
    }
}

ln::Operand
CPU::AddressModeParse::parse_rel(ln::CPU *i_cpu)
{
    // caller should cast it to int8_t, because it's an 8-bit signed offset
    // relative to the current PC.
    return Operand::from_address(i_cpu->PC++);
}

Byte2
CPU::AddressModeParse::get_byte2(ln::CPU *i_cpu, Address i_address)
{
    // little-endian
    return byte2_from_bytes(i_cpu->get_byte(i_address + 1),
                            i_cpu->get_byte(i_address));
}

} // namespace ln

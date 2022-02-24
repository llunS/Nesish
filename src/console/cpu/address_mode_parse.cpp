#include "address_mode_parse.hpp"

#include "console/byte_utils.hpp"

namespace ln {

ln::Operand
CPU::AddressModeParse::parse_imp(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    (void)(i_cpu);
    (void)(o_page_crossing);

    o_operand_bytes = 0;
    // Return a dummy value, since it's implicit anyway.
    return Operand::from_value(0);
}

ln::Operand
CPU::AddressModeParse::parse_acc(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    (void)(i_cpu);
    (void)(o_page_crossing);

    o_operand_bytes = 0;
    return Operand::from_acc();
}

ln::Operand
CPU::AddressModeParse::parse_imm(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    (void)(o_page_crossing);

    o_operand_bytes = 1;
    return Operand::from_value(i_cpu->get_byte(i_cpu->PC));
}

ln::Operand
CPU::AddressModeParse::parse_zp0(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    (void)(o_page_crossing);

    Byte arg = i_cpu->get_byte(i_cpu->PC);
    o_operand_bytes = 1;
    return Operand::from_address(arg);
}

ln::Operand
CPU::AddressModeParse::parse_zpx(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    (void)(o_page_crossing);

    Byte arg = i_cpu->get_byte(i_cpu->PC);
    o_operand_bytes = 1;
    return Operand::from_address(((Byte2)arg + i_cpu->X) & 0xff);
}

ln::Operand
CPU::AddressModeParse::parse_zpy(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    (void)(o_page_crossing);

    Byte arg = i_cpu->get_byte(i_cpu->PC);
    o_operand_bytes = 1;
    return Operand::from_address(((Byte2)arg + i_cpu->Y) & 0xff);
}

ln::Operand
CPU::AddressModeParse::parse_izx(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    (void)(o_page_crossing);

    Byte arg = i_cpu->get_byte(i_cpu->PC);
    o_operand_bytes = 1;

    Byte lower = i_cpu->get_byte(((Byte2)arg + i_cpu->X) & 0xff);
    Byte higher = i_cpu->get_byte(((Byte2)arg + i_cpu->X + 1) & 0xff);
    Address address = byte2_from_bytes(higher, lower);

    return Operand::from_address(address);
}

ln::Operand
CPU::AddressModeParse::parse_izy(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    (void)(o_page_crossing);

    Byte arg = i_cpu->get_byte(i_cpu->PC);
    o_operand_bytes = 1;

    Byte lower = i_cpu->get_byte(arg);
    Byte higher = i_cpu->get_byte(((Byte2)arg + 1) & 0xff);
    Address pre_add_address = byte2_from_bytes(higher, lower);
    Address address = pre_add_address + i_cpu->Y;
    if (o_page_crossing)
    {
        *o_page_crossing = is_page_crossing(pre_add_address, address);
    }
    return Operand::from_address(address);
}

ln::Operand
CPU::AddressModeParse::parse_abs(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    (void)(o_page_crossing);

    Byte2 arg = get_byte2(i_cpu, i_cpu->PC);
    o_operand_bytes = 2;
    return Operand::from_address(arg);
}

ln::Operand
CPU::AddressModeParse::parse_abx(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    Byte2 arg = get_byte2(i_cpu, i_cpu->PC);
    o_operand_bytes = 2;

    Address address = arg + i_cpu->X;
    if (o_page_crossing)
    {
        *o_page_crossing = is_page_crossing(arg, address);
    }
    return Operand::from_address(address);
}

ln::Operand
CPU::AddressModeParse::parse_aby(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    Byte2 arg = get_byte2(i_cpu, i_cpu->PC);
    o_operand_bytes = 2;

    Address address = arg + i_cpu->Y;
    if (o_page_crossing)
    {
        *o_page_crossing = is_page_crossing(arg, address);
    }
    return Operand::from_address(address);
}

ln::Operand
CPU::AddressModeParse::parse_ind(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    (void)(o_page_crossing);

    Byte2 arg = get_byte2(i_cpu, i_cpu->PC);
    o_operand_bytes = 2;

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
CPU::AddressModeParse::parse_rel(const ln::CPU *i_cpu, Byte &o_operand_bytes,
                                 bool *o_page_crossing)
{
    (void)(o_page_crossing);

    // caller should cast it to int8_t, because it's an 8-bit signed offset
    // relative to the current PC.
    o_operand_bytes = 1;
    return Operand::from_address(i_cpu->PC);
}

Byte2
CPU::AddressModeParse::get_byte2(const ln::CPU *i_cpu, Address i_addr)
{
    // little-endian
    return byte2_from_bytes(i_cpu->get_byte(i_addr + 1),
                            i_cpu->get_byte(i_addr));
}

bool
CPU::AddressModeParse::is_page_crossing(Address i_prev, Address i_current)
{
    // After addition, the page is crossed.
    return (i_prev & 0xFF00) != (i_current & 0xFF00);
}

} // namespace ln

#include "operand_get.hpp"

namespace ln {

static Byte2
pvt_assemble_byte2(Byte i_upper, Byte i_lower);
static Byte2
pvt_get_byte2(ln::Memory *i_memory, Address i_address);

Byte
CPU::OperandGet::get_imp(ln::CPU *i_cpu, ln::Memory *i_memory)
{
    (void)(i_cpu);
    (void)(i_memory);
    // Return a dummy value, since it's implicit anyway.
    return 0;
}

Byte
CPU::OperandGet::get_imm(ln::CPU *i_cpu, ln::Memory *i_memory)
{
    return i_memory->get_byte(i_cpu->PC++);
}

Byte
CPU::OperandGet::get_zp0(ln::CPU *i_cpu, ln::Memory *i_memory)
{
    Byte arg = i_memory->get_byte(i_cpu->PC++);
    return i_memory->get_byte(arg);
}

Byte
CPU::OperandGet::get_zpx(ln::CPU *i_cpu, ln::Memory *i_memory)
{
    Byte arg = i_memory->get_byte(i_cpu->PC++);
    return i_memory->get_byte(((Byte2)arg + i_cpu->X) & 0xff);
}

Byte
CPU::OperandGet::get_zpy(ln::CPU *i_cpu, ln::Memory *i_memory)
{
    Byte arg = i_memory->get_byte(i_cpu->PC++);
    return i_memory->get_byte(((Byte2)arg + i_cpu->Y) & 0xff);
}

Byte
CPU::OperandGet::get_izx(ln::CPU *i_cpu, ln::Memory *i_memory)
{
    Byte arg = i_memory->get_byte(i_cpu->PC++);

    Byte lower = i_memory->get_byte(((Byte2)arg + i_cpu->X) & 0xff);
    Byte upper = i_memory->get_byte(((Byte2)arg + i_cpu->X + 1) & 0xff);
    Address address = pvt_assemble_byte2(upper, lower);

    return i_memory->get_byte(address);
}

Byte
CPU::OperandGet::get_izy(ln::CPU *i_cpu, ln::Memory *i_memory)
{
    Byte arg = i_memory->get_byte(i_cpu->PC++);

    Byte lower = i_memory->get_byte(arg);
    Byte upper = i_memory->get_byte(((Byte2)arg + 1) & 0xff);
    Address address = pvt_assemble_byte2(upper, lower) + i_cpu->Y;

    return i_memory->get_byte(address);
}

Byte
CPU::OperandGet::get_abs(ln::CPU *i_cpu, ln::Memory *i_memory)
{
    Byte2 arg = pvt_get_byte2(i_memory, i_cpu->PC);
    i_cpu->PC += 2;
    return i_memory->get_byte(arg);
}

Byte
CPU::OperandGet::get_abx(ln::CPU *i_cpu, ln::Memory *i_memory)
{
    Byte2 arg = pvt_get_byte2(i_memory, i_cpu->PC);
    i_cpu->PC += 2;
    return i_memory->get_byte(arg + i_cpu->X);
}

Byte
CPU::OperandGet::get_aby(ln::CPU *i_cpu, ln::Memory *i_memory)
{
    Byte2 arg = pvt_get_byte2(i_memory, i_cpu->PC);
    i_cpu->PC += 2;
    return i_memory->get_byte(arg + i_cpu->Y);
}

Byte
CPU::OperandGet::get_ind(ln::CPU *i_cpu, ln::Memory *i_memory)
{
    Byte2 arg = pvt_get_byte2(i_memory, i_cpu->PC);
    i_cpu->PC += 2;

    // https://wiki.nesdev.org/w/index.php/Errata
    if ((arg & 0xff) == 0xff)
    {
        // the indirect address value does not advance pages.
        Byte lower = i_memory->get_byte(arg);
        Byte upper = i_memory->get_byte(arg & 0xff00);
        Address address = pvt_assemble_byte2(upper, lower);
        return i_memory->get_byte(address);
    }
    else
    {
        Address address = pvt_get_byte2(i_memory, arg);
        return i_memory->get_byte(address);
    }
}

Byte
CPU::OperandGet::get_rel(ln::CPU *i_cpu, ln::Memory *i_memory)
{
    // caller should cast it to int8_t, because it's an 8-bit signed offset
    // relative to the current PC.
    return i_memory->get_byte(i_cpu->PC++);
}

Byte2
pvt_assemble_byte2(Byte i_upper, Byte i_lower)
{
    return ((Byte2)i_upper << 8) + i_lower;
}

Byte2
pvt_get_byte2(ln::Memory *i_memory, Address i_address)
{
    // little-endian
    return pvt_assemble_byte2(i_memory->get_byte(i_address + 1),
                              i_memory->get_byte(i_address));
}

} // namespace ln

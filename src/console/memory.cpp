#include "memory.hpp"

#include <cstring>

namespace ln {

Memory::Memory()
    : m_ram{}
{
}

Byte
Memory::get_byte(Address i_address) const
{
    return m_ram[i_address];
}

void
Memory::set_byte(Address i_address, Byte i_byte)
{
    m_ram[i_address] = i_byte;
}

void
Memory::set_range(Address i_begin, Address i_end, Byte i_byte)
{
    if (i_begin >= i_end)
    {
        return;
    }

    std::memset(m_ram + i_begin, i_byte, i_end - i_begin);
}

void
Memory::set_apu_frame_counter(Byte i_byte)
{
    set_byte(APU_FC_ADDRESS, i_byte);
}

} // namespace ln

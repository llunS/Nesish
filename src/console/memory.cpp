#include "memory.hpp"

namespace ln {

Memory::Memory()
    : m_ram{}
{
}

Byte
Memory::get_byte(Address i_address) const
{
    // @TODO
    (void)(i_address);
    return 0;
}

void
Memory::set_byte(Address i_address, Byte i_byte)
{
    // @TODO
    (void)(i_address);
    (void)(i_byte);
}

} // namespace ln

#include "nametable_entry.hpp"

namespace ln {

NametableEntry::NametableEntry(Byte i_byte)
    : m_byte(i_byte)
{
}

Byte
NametableEntry::get_tile_pos() const
{
    return m_byte;
}

} // namespace ln

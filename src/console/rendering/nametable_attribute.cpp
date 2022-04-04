#include "nametable_attribute.hpp"

namespace ln {

NametableAttribute::NametableAttribute(Byte i_byte)
    : m_byte(i_byte)
{
}

Byte
NametableAttribute::get_palette_idx(Pos2x2Tiles i_pos) const
{
    return m_byte >> i_pos & 0x03;
}

} // namespace ln

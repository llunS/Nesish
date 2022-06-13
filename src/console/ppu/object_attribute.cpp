#include "object_attribute.hpp"

#include <cstring>

namespace ln {

constexpr int ObjectAttribute::BYTES;

ObjectAttribute::ObjectAttribute(const Byte (&i_bytes)[BYTES])
{
    std::memcpy(m_bytes, i_bytes, sizeof(m_bytes));
}

Byte
ObjectAttribute::get_x() const
{
    static_assert(BYTES >= 4, "Wrong byte count.");
    return m_bytes[3];
}

Byte
ObjectAttribute::get_y() const
{
    static_assert(BYTES >= 1, "Wrong byte count.");
    return m_bytes[0];
}

void
ObjectAttribute::get_tile_idx(Byte &o_tile_idx, bool *o_table_idx) const
{
    static_assert(BYTES >= 2, "Wrong byte count.");
    Byte byte = m_bytes[1];

    bool is8x16 = o_table_idx != nullptr;
    if (!is8x16)
    {
        o_tile_idx = byte;
    }
    else
    {
        o_tile_idx = byte >> 1;
        *o_table_idx = byte & 0x01;
    }
}

Byte
ObjectAttribute::get_palette_idx() const
{
    static_assert(BYTES >= 3, "Wrong byte count.");
    return m_bytes[2] & 0x03;
}

bool
ObjectAttribute::get_priority() const
{
    static_assert(BYTES >= 3, "Wrong byte count.");
    return m_bytes[2] & 0x20;
}

bool
ObjectAttribute::get_flip_x() const
{
    static_assert(BYTES >= 3, "Wrong byte count.");
    return m_bytes[2] & 0x40;
}

bool
ObjectAttribute::get_flip_y() const
{
    static_assert(BYTES >= 3, "Wrong byte count.");
    return m_bytes[2] & 0x80;
}

} // namespace ln

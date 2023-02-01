#include "sprite.hpp"

#include <type_traits>

namespace lnd {

Sprite::Sprite()
    : m_8x16(false)
    , m_pixels{}
    , y(255)
    , tile(255)
    , attr(255)
    , x(255)
{
}

int
Sprite::get_width() const
{
    return 8;
}

int
Sprite::get_height() const
{
    return m_8x16 ? 16 : 8;
}

const ln::Byte *
Sprite::get_data() const
{
    static_assert(std::is_pod<ln::Color>::value, "Incorrect pointer position");
    return (ln::Byte *)(&m_pixels[0]);
}

LN_CONSOLE_API int
Sprite::palette_set() const
{
    return attr & 0x03;
}

LN_CONSOLE_API bool
Sprite::background() const
{
    return attr & 0x20;
}

LN_CONSOLE_API bool
Sprite::flip_x() const
{
    return attr & 0x40;
}

LN_CONSOLE_API bool
Sprite::flip_y() const
{
    return attr & 0x80;
}

void
Sprite::set_mode(bool i_8x16)
{
    m_8x16 = i_8x16;
}

void
Sprite::set_pixel(int i_row, int i_col, const ln::Color &i_color)
{
    // Save the bound check, leave it to the user.
    m_pixels[i_row * 8 + i_col] = i_color;
}

void
Sprite::set_raw(ln::Byte i_y, ln::Byte i_tile, ln::Byte i_attr, ln::Byte i_x)
{
    y = i_y;
    tile = i_tile;
    attr = i_attr;
    x = i_x;
}

} // namespace lnd

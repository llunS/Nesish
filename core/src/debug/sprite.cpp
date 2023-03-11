#include "sprite.hpp"

#include <type_traits>

namespace nhd {

constexpr int Sprite::WIDTH;

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
    return WIDTH;
}

int
Sprite::get_height() const
{
    return m_8x16 ? 16 : 8;
}

const nh::Byte *
Sprite::get_data() const
{
    static_assert(std::is_pod<nh::Color>::value, "Incorrect pointer position");
    return (nh::Byte *)(&m_pixels[0]);
}

int
Sprite::palette_set() const
{
    return attr & 0x03;
}

bool
Sprite::background() const
{
    return attr & 0x20;
}

bool
Sprite::flip_x() const
{
    return attr & 0x40;
}

bool
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
Sprite::set_pixel(int i_row, int i_col, const nh::Color &i_color)
{
    // Save the bound check, leave it to the user.
    m_pixels[i_row * WIDTH + i_col] = i_color;
}

void
Sprite::set_raw(nh::Byte i_y, nh::Byte i_tile, nh::Byte i_attr, nh::Byte i_x)
{
    y = i_y;
    tile = i_tile;
    attr = i_attr;
    x = i_x;
}

} // namespace nhd

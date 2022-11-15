#include "sprite.hpp"

#include <type_traits>

namespace lnd {

Sprite::Sprite()
    : m_pixels{}
{
}

int
Sprite::get_width() const
{
    // @TODO: 8x16
    return 8;
}

int
Sprite::get_height() const
{
    // @TODO: 8x16
    return 8;
}

const ln::Byte *
Sprite::get_data() const
{
    static_assert(std::is_pod<ln::Color>::value, "Rework code below.");
    return (ln::Byte *)(&m_pixels[0]);
}

void
Sprite::set_pixel(int i_row, int i_col, const ln::Color &i_color)
{
    // @TODO: 8x16
    m_pixels[i_row * 8 + i_col] = i_color;
}

} // namespace lnd

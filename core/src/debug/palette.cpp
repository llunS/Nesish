#include "palette.hpp"

namespace nhd {

Palette::Palette()
    : m_colors{}
{
}

const NHDColor &
Palette::get_color(int i_idx) const
{
    return m_colors[i_idx];
}

void
Palette::set_color(int i_idx, unsigned char i_index, unsigned char i_r,
                   unsigned char i_g, unsigned char i_b)
{
    m_colors[i_idx].index = i_index;
    m_colors[i_idx].r = i_r;
    m_colors[i_idx].g = i_g;
    m_colors[i_idx].b = i_b;
}

} // namespace nhd

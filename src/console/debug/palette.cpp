#include "palette.hpp"

namespace lnd {

Palette::Palette()
    : m_colors{}
{
}

const ln::Color &
Palette::get_color(int i_idx) const
{
    return m_colors[i_idx];
}

void
Palette::set_color(int i_idx, const ln::Color &i_color)
{
    m_colors[i_idx] = i_color;
}

} // namespace lnd

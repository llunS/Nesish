#include "pattern_table.hpp"

#include <type_traits>

namespace lnd {

constexpr int PatternTable::TILES_W;
constexpr int PatternTable::TILES_H;
constexpr int PatternTable::TILE_W;
constexpr int PatternTable::TILE_H;

PatternTable::PatternTable()
    : m_pixels{}
{
}

const ln::Byte *
PatternTable::get_data() const
{
    static_assert(std::is_pod<ln::Color>::value, "Incorrect pointer position");
    return (ln::Byte *)(&m_pixels[0]);
}

void
PatternTable::set_pixel(int i_tile_idx, int i_fine_y, int i_fine_x,
                        const ln::Color &i_color)
{
    // Save the bound check, leave it to the user.
    int tile_y = i_tile_idx / TILES_W;
    int tile_x = i_tile_idx % TILES_W;
    int row = tile_y * get_tile_height() + i_fine_y;
    int col = tile_x * get_tile_width() + i_fine_x;
    m_pixels[row * get_width() + col] = i_color;
}

} // namespace lnd

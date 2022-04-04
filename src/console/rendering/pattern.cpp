#include "pattern.hpp"

#include <cstring>

namespace ln {

constexpr int Pattern::BYTES;
constexpr int Pattern::PIXEL_ROWS;
constexpr int Pattern::PIXEL_COLS;

Pattern::Pattern(const Byte (&i_bytes)[BYTES])
{
    std::memcpy(m_bytes, i_bytes, sizeof(m_bytes));
}

Byte
Pattern::get_color_index(Byte i_pixel_idx) const
{
    int plane_row_bit_idx = i_pixel_idx / PIXEL_COLS;
    // flip horizontal coordinate
    // pixel index counts left col as 0, while in memory the rightmost is bit 0.
    int plane_col_bit_idx = (PIXEL_COLS - 1) - i_pixel_idx % PIXEL_COLS;

    auto get_bit = [this](int plane_row_bit_idx, int plane_col_bit_idx,
                          bool plane1) -> int {
        static_assert(PIXEL_ROWS == 8, "Wrong byte count.");
        int ary_idx =
            plane1 ? plane_row_bit_idx + PIXEL_ROWS : plane_row_bit_idx;
        static_assert(sizeof(this->m_bytes) == 16, "Wrong byte count.");
        int byte = m_bytes[ary_idx];
        return byte >> plane_col_bit_idx & 0x01;
    };

    int plane0_val = get_bit(plane_row_bit_idx, plane_col_bit_idx, false);
    int plane1_val = get_bit(plane_row_bit_idx, plane_col_bit_idx, true);
    return plane1_val << 1 | plane0_val;
}

} // namespace ln

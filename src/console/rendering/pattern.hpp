#pragma once

#include "console/dllexport.h"
#include "common/klass.hpp"
#include "console/types.hpp"
#include "console/rendering/palette_color.hpp"
#include "console/types.hpp"

namespace ln {

/**
 * @brief A 8x8 pixel tile
 */
struct LN_CONSOLE_API Pattern {
  public:
    static constexpr int BYTES = 16;
    static constexpr int PIXEL_ROWS = 8;
    static constexpr int PIXEL_COLS = 8;

  public:
    Pattern(const Byte (&i_bytes)[BYTES]);
    LN_KLZ_DEFAULT_COPY(Pattern);

    /**
     * @brief Get the 2-bit index of index color in single palette
     * @details Each palette contains 4 index colors
     *
     * @param i_pixel_idx the 6-bit index of the pixel to query
     */
    Byte
    get_color_index(Byte i_pixel_idx) const;

  private:
    Byte m_bytes[BYTES];
};

} // namespace ln

#pragma once

#include "types.hpp"
#include "nhbase/klass.hpp"

namespace nh {

/**
 * @brief Palette index color. 8-bit in memory, 6-bit in use.
 */
struct PaletteColor {
  public:
    PaletteColor(Byte i_index)
        : value(i_index){};
    NB_KLZ_DEFAULT_COPY(PaletteColor);

    /**
     * @brief The number of representable colors.
     */
    static constexpr int
    size()
    {
        return 64;
    }

  public:
    Byte value;
};

} // namespace nh

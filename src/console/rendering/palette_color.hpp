#pragma once

#include "console/types.hpp"
#include "common/klass.hpp"

namespace ln {

/**
 * @brief Palette index color. 8-bit in memory, 6-bit in use.
 */
struct PaletteColor {
  public:
    PaletteColor(Byte i_index)
        : value(i_index){};
    LN_KLZ_DEFAULT_COPY(PaletteColor);

    static constexpr int
    size()
    {
        return 64;
    }

  public:
    Byte value;
};

} // namespace ln

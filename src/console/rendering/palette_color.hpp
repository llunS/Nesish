#ifndef LN_CONSOLE_RENDERING_PALETTECOLOR_HPP
#define LN_CONSOLE_RENDERING_PALETTECOLOR_HPP

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

#endif // LN_CONSOLE_RENDERING_PALETTECOLOR_HPP

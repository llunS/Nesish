#ifndef LN_CONSOLE_RENDERING_PALETTE_HPP
#define LN_CONSOLE_RENDERING_PALETTE_HPP

#include "console/rendering/palette_color.hpp"
#include "console/rendering/color.hpp"
#include "console/dllexport.h"

namespace ln {

struct LN_CONSOLE_API Palette {
  public:
    virtual Color
    get_color(PaletteColor i_color) const = 0;
};

} // namespace ln

#endif // LN_CONSOLE_RENDERING_PALETTE_HPP

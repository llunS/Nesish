#pragma once

#include "console/rendering/palette_color.hpp"
#include "console/rendering/color.hpp"

namespace ln {

struct Palette {
  public:
    virtual Color
    get_color(PaletteColor i_color) const = 0;
};

} // namespace ln

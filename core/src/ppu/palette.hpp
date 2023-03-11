#pragma once

#include "ppu/palette_color.hpp"
#include "ppu/color.hpp"

namespace nh {

struct Palette {
  public:
    virtual Color
    to_rgb(PaletteColor i_color) const = 0;
};

} // namespace nh

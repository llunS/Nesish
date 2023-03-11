#pragma once

#include "ppu/palette.hpp"
#include "nhbase/klass.hpp"

namespace nh {

struct PaletteDefault : public Palette {
  public:
    PaletteDefault() = default;
    NB_KLZ_DEFAULT_COPY(PaletteDefault);

    Color
    to_rgb(PaletteColor i_color) const override;
};

} // namespace nh

#pragma once

#include "console/ppu/palette.hpp"
#include "common/klass.hpp"

namespace ln {

struct PaletteDefault : public Palette {
  public:
    PaletteDefault() = default;
    LN_KLZ_DEFAULT_COPY(PaletteDefault);

    Color
    to_rgb(PaletteColor i_color) const override;
};

} // namespace ln

#ifndef LN_CONSOLE_RENDERING_PALETTEDEFAULT_HPP
#define LN_CONSOLE_RENDERING_PALETTEDEFAULT_HPP

#include "console/rendering/palette.hpp"
#include "common/klass.hpp"

namespace ln {

struct PaletteDefault : public Palette {
  public:
    PaletteDefault() = default;
    LN_KLZ_DEFAULT_COPY(PaletteDefault);

    Color
    get_color(PaletteColor i_color) const override;
};

} // namespace ln

#endif // LN_CONSOLE_RENDERING_PALETTEDEFAULT_HPP

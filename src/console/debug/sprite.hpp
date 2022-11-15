#pragma once

#include "common/klass.hpp"

#include "console/dllexport.h"
#include "console/ppu/color.hpp"
#include "console/types.hpp"

namespace lnd {

/// @brief Information needed to display a sprite
struct Sprite {
  public:
    Sprite();
    ~Sprite() = default;
    LN_KLZ_DELETE_COPY_MOVE(Sprite);

  public:
    LN_CONSOLE_API int
    get_width() const;
    LN_CONSOLE_API int
    get_height() const;

    LN_CONSOLE_API const ln::Byte *
    get_data() const;

  public:
    void
    set_pixel(int i_row, int i_col, const ln::Color &i_color);

  private:
    ln::Color m_pixels[64];
};

} // namespace lnd

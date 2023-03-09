#pragma once

#include "common/klass.hpp"

#include "console/dllexport.h"
#include "console/ppu/color.hpp"
#include "console/types.hpp"

namespace nhd {

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
    LN_CONSOLE_API const nh::Byte *
    get_data() const;

  public:
    LN_CONSOLE_API int
    palette_set() const;
    LN_CONSOLE_API bool
    background() const;
    LN_CONSOLE_API bool
    flip_x() const;
    LN_CONSOLE_API bool
    flip_y() const;

  public:
    void
    set_mode(bool i_8x16);
    void
    set_pixel(int i_row, int i_col, const nh::Color &i_color);
    void
    set_raw(nh::Byte i_y, nh::Byte i_tile, nh::Byte i_attr, nh::Byte i_x);

  private:
    static constexpr int WIDTH = 8;

  private:
    bool m_8x16;
    nh::Color m_pixels[WIDTH * 16]; // up to 8x16

  public:
    nh::Byte y;
    nh::Byte tile;
    nh::Byte attr;
    nh::Byte x;
};

} // namespace nhd

#pragma once

#include "nhbase/klass.hpp"

#include "ppu/color.hpp"
#include "types.hpp"

namespace nhd {

/// @brief Information needed to display a sprite
struct Sprite {
  public:
    Sprite();
    ~Sprite() = default;
    NB_KLZ_DELETE_COPY_MOVE(Sprite);

  public:
    int
    get_width() const;
    int
    get_height() const;
    const nh::Byte *
    get_data() const;

  public:
    int
    palette_set() const;
    bool
    background() const;
    bool
    flip_x() const;
    bool
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

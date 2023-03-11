#pragma once

#include "nesish/nesish.h"
#include "nhbase/klass.hpp"

namespace nhd {

struct Palette {
  public:
    Palette();
    ~Palette() = default;
    NB_KLZ_DELETE_COPY_MOVE(Palette);

  public:
    constexpr static int
    color_count()
    {
        return NHD_PALETTE_COLORS;
    }
    const NHDColor &
    get_color(int i_idx) const;

  public:
    void
    set_color(int i_idx, unsigned char i_index, unsigned char i_r,
              unsigned char i_g, unsigned char i_b);

  private:
    NHDColor m_colors[NHD_PALETTE_COLORS];
};

} // namespace nhd

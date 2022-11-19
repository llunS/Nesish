#pragma once

#include "common/klass.hpp"

#include "console/dllexport.h"
#include "console/debug/color.hpp"

namespace lnd {

struct Palette {
  public:
    Palette();
    ~Palette() = default;
    LN_KLZ_DELETE_COPY_MOVE(Palette);

  public:
    LN_CONSOLE_API constexpr static int
    color_count()
    {
        return 32;
    }
    LN_CONSOLE_API const lnd::Color &
    get_color(int i_idx) const;

  public:
    void
    set_color(int i_idx, unsigned char i_index, unsigned char i_r,
              unsigned char i_g, unsigned char i_b);

  private:
    lnd::Color m_colors[32];
};

} // namespace lnd

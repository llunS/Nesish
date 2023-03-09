#pragma once

#include "common/klass.hpp"

#include "console/dllexport.h"
#include "console/ppu/color.hpp"
#include "console/types.hpp"

namespace nhd {

struct PatternTable {
  public:
    PatternTable();
    ~PatternTable() = default;
    LN_KLZ_DELETE_COPY_MOVE(PatternTable);

  public:
    static constexpr int
    get_width()
    {
        return TILES_W * TILE_W;
    }
    static constexpr int
    get_height()
    {
        return TILES_H * TILE_H;
    }
    LN_CONSOLE_API const nh::Byte *
    get_data() const;

  public:
    static constexpr int
    get_tiles()
    {
        return TILES_W * TILES_H;
    }
    static constexpr int
    get_tiles_width()
    {
        return TILES_W;
    }
    static constexpr int
    get_tiles_height()
    {
        return TILES_H;
    }
    static constexpr int
    get_tile_width()
    {
        return TILE_W;
    }
    static constexpr int
    get_tile_height()
    {
        return TILE_H;
    }

  public:
    void
    set_pixel(int i_tile_idx, int i_fine_y, int i_fine_x,
              const nh::Color &i_color);

  private:
    static constexpr int TILES_W = 16;
    static constexpr int TILES_H = 16;
    static constexpr int TILE_W = 8;
    static constexpr int TILE_H = 8;

  private:
    // 16x16 tiles each with 8x8 pixels
    nh::Color m_pixels[(TILES_W * TILES_H) * (TILE_W * TILE_H)];
};

} // namespace nhd

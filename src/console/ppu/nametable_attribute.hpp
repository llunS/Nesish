#pragma once

#include "console/types.hpp"
#include "common/klass.hpp"

namespace ln {

struct NametableAttribute {
  public:
    NametableAttribute(Byte i_byte);
    LN_KLZ_DEFAULT_COPY(NametableAttribute);

  public:
    /**
     * @brief Position of each 2x2 tile block
     * @details One instance specifies attributes of 4 such tile blocks
     */
    enum Pos2x2Tiles : Byte {
        // NOTE: the value is the bit of the corresponding starting bit.
        TOP_LEFT = 0,
        TOP_RIGHT = 2,
        BOTTOM_LEFT = 4,
        BOTTOM_RIGHT = 6,
    };

  public:
    /**
     * @brief Get the 2-bit index of palette in Background Palettes
     */
    Byte
    get_palette_idx(Pos2x2Tiles i_pos) const;

  private:
    Byte m_byte;
};

} // namespace ln

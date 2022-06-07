#pragma once

#include "console/types.hpp"
#include "common/klass.hpp"

namespace ln {

struct NametableEntry {
  public:
    NametableEntry(Byte i_byte);
    LN_KLZ_DEFAULT_COPY(NametableEntry);

  public:
    /**
     * @brief Get the tile (8x8 pixel block) index in Pattern Table
     *
     * @return RRRRCCCC R: Tile row C: Tile column
     */
    Byte
    get_tile_pos() const;

  private:
    Byte m_byte;
};

} // namespace ln

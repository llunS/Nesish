#pragma once

#include "console/types.hpp"
#include "common/klass.hpp"

namespace ln {

/**
 * @brief Attribute of each sprite
 */
struct ObjectAttribute {
  public:
    static constexpr int BYTES = 4;

  public:
    ObjectAttribute(const Byte (&i_bytes)[BYTES]);
    LN_KLZ_DEFAULT_COPY(ObjectAttribute);

  public:
    Byte
    get_x() const;
    Byte
    get_y() const;

    /**
     * @brief Get the tile index number
     *
     * @param o_tile_idx the tile index
     * @param o_table_idx not-null for 8x16 sprite, get the 1-bit index of
     * pattern table
     */
    void
    get_tile_idx(Byte &o_tile_idx, bool *o_table_idx) const;

    /**
     * @brief Get the 2-bit index of palette in Sprite Palettes
     */
    Byte
    get_palette_idx() const;
    /**
     * @brief Get the sprite priority
     *
     * @return Priority (0: in front of background; 1: behind background)
     */
    bool
    get_priority() const;
    bool
    get_flip_x() const;
    bool
    get_flip_y() const;

  private:
    Byte m_bytes[BYTES];
};

} // namespace ln

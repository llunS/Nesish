#pragma once

#include "nhbase/klass.hpp"
#include "nesish/nesish.h"

#include "debug/sprite.hpp"

namespace nhd {

struct OAM {
  public:
    OAM();
    ~OAM() = default;
    NB_KLZ_DELETE_COPY_MOVE(OAM);

  public:
    constexpr static int
    get_sprite_count()
    {
        return NHD_OAM_SPRITES;
    }
    const Sprite &
    get_sprite(int i_idx) const;

  public:
    Sprite &
    sprite_of(int i_idx);

  private:
    Sprite m_sprites[NHD_OAM_SPRITES];
};

} // namespace nhd

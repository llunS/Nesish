#pragma once

#include "common/klass.hpp"

#include "console/dllexport.h"
#include "console/debug/sprite.hpp"

namespace nhd {

struct OAM {
  public:
    OAM();
    ~OAM() = default;
    LN_KLZ_DELETE_COPY_MOVE(OAM);

  public:
    LN_CONSOLE_API constexpr static int
    get_sprite_count()
    {
        return 64;
    }
    LN_CONSOLE_API const Sprite &
    get_sprite(int i_idx) const;

  public:
    Sprite &
    sprite_of(int i_idx);

  private:
    Sprite m_sprites[64];
};

} // namespace nhd

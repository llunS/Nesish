#include "oam.hpp"

namespace nhd {

OAM::OAM()
    : m_sprites{}
{
}

const Sprite &
OAM::get_sprite(int i_idx) const
{
    return m_sprites[i_idx];
}

Sprite &
OAM::sprite_of(int i_idx)
{
    return m_sprites[i_idx];
}

} // namespace nhd

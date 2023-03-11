#pragma once

#include "nhbase/klass.hpp"

#include "types.hpp"
#include "apu/divider.hpp"

namespace nh {

struct Envelope {
  public:
    Envelope();
    ~Envelope() = default;
    NB_KLZ_DELETE_COPY_MOVE(Envelope);

  public:
    /// @return Volume in range of [0, 15]
    Byte
    volume() const;

    void
    tick();

  public:
    void
    set_divider_reload(Byte2 i_reload);
    void
    set_loop(bool i_set);
    void
    set_const(bool i_set);
    void
    set_const_vol(Byte i_vol);
    void
    restart();

  private:
    bool m_start; // start flag
    Divider m_divider;
    Byte m_decay_level; // [0, 15]

    bool m_loop;
    bool m_const;
    Byte m_const_vol;
};

} // namespace nh

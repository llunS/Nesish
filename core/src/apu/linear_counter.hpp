#pragma once

#include "nhbase/klass.hpp"

#include "types.hpp"

namespace nh {

struct LinearCounter {
  public:
    LinearCounter();
    ~LinearCounter() = default;
    NB_KLZ_DELETE_COPY_MOVE(LinearCounter);

  public:
    Byte
    value() const;

    void
    tick();

  public:
    void
    set_control(bool i_set);
    void
    set_reload();
    void
    set_reload_val(Byte i_reload);

  private:
    Byte m_counter;
    bool m_control;
    bool m_reload;
    Byte m_reload_val;
};

} // namespace nh

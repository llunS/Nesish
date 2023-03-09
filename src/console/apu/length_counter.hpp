#pragma once

#include "common/klass.hpp"

#include "console/types.hpp"

namespace nh {

struct LengthCounter {
  public:
    LengthCounter();
    ~LengthCounter() = default;
    LN_KLZ_DELETE_COPY_MOVE(LengthCounter);

  public:
    Byte
    value() const;

    void
    tick();

  public:
    void
    power_up();

  public:
    void
    post_set_halt(bool i_set);
    void
    flush_halt_set();
    void
    set_enabled(bool i_set);
    void
    post_set_load(Byte i_index);
    void
    flush_load_set();

  private:
    void
    check_load(Byte i_index);

  private:
    Byte m_counter;
    bool m_halt;
    bool m_enabled;

    bool m_to_set_halt;
    bool m_halt_val;
    bool m_to_load;
    Byte m_load_val;
};

} // namespace nh

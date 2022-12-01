#pragma once

#include "common/klass.hpp"

#include "console/types.hpp"

namespace ln {

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
    set_halt(bool i_set);
    void
    set_enabled(bool i_set);
    void
    check_load(Byte i_index);

  private:
    Byte m_counter;
    bool m_halt;
    bool m_enabled;
};

} // namespace ln

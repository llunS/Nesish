#pragma once

#include "common/klass.hpp"

#include "console/apu/divider.hpp"
#include "console/types.hpp"

namespace ln {

struct Sweep {
  public:
    Sweep(Divider &io_ch_timer, bool i_mode_1);
    ~Sweep() = default;
    LN_KLZ_DELETE_COPY_MOVE(Sweep);

  public:
    bool
    muting(Byte2 *i_target = nullptr) const;

    void
    tick();

  public:
    void
    set_enabled(bool i_set);
    void
    set_divider_reload(Byte2 i_reload);
    void
    set_negate(bool i_set);
    void
    set_shift_count(Byte i_count);
    void
    reload();

  private:
    Byte2
    target_reload() const;

  private:
    Divider m_divider;
    bool m_reload;

    bool m_enabled;
    bool m_negate;
    Byte m_shift;

    Divider &m_ch_timer; // the channel timer.
    /* clang-format off */
    // Pulse 1 adds the ones' complement (-c - 1). Making 20 negative produces a change amount of -21.
    // Pulse 2 adds the two's complement (-c). Making 20 negative produces a change amount of -20.
    /* clang-format on */
    bool m_mode_1;
};

} // namespace ln

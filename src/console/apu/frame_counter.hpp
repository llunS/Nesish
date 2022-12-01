#pragma once

#include "common/klass.hpp"

namespace ln {

struct Pulse;

struct FrameCounter {
  public:
    FrameCounter(Pulse &o_pulse1, Pulse &o_pulse2);
    ~FrameCounter() = default;
    LN_KLZ_DELETE_COPY_MOVE(FrameCounter);

  public:
    /// @brief Tick this every CPU cycle
    void
    tick();

    bool
    interrupt() const;

    void
    set_mode(bool i_step5);
    void
    set_irq_inhibit(bool i_set);
    void
    reset();
    void
    clear_interrupt();

  private:
    void
    tick_length_counter_and_sweep();
    void
    tick_envelope_and_linear_counter();

  private:
    Pulse &m_pulse1;
    Pulse &m_pulse2;

  private:
    int m_timer;
    bool m_irq;

    bool m_step5;
    bool m_irq_inhibit;
};

} // namespace ln

#pragma once

#include "nhbase/klass.hpp"

namespace nh {

struct Pulse;
struct Triangle;
struct Noise;

struct FrameCounter {
  public:
    FrameCounter(Pulse &o_pulse1, Pulse &o_pulse2, Triangle &o_triangle,
                 Noise &o_noise);
    ~FrameCounter() = default;
    NB_KLZ_DELETE_COPY_MOVE(FrameCounter);

  public:
    void
    power_up();

    /// @brief Tick this every CPU cycle
    void
    tick();

    bool
    interrupt() const;

    void
    reset_timer();

    void
    set_irq_inhibit(bool i_set);
    void
    delay_set_mode(bool i_mode, bool i_delay);
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
    Triangle &m_triangle;
    Noise &m_noise;

  private:
    unsigned int m_timer;
    bool m_irq;

    bool m_mode;
    bool m_irq_inhibit;

    bool m_first_loop;
    unsigned char m_reset_counter;
    bool m_mode_tmp;
};

} // namespace nh

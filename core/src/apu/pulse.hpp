#pragma once

#include "nhbase/klass.hpp"

#include "apu/envelope.hpp"
#include "apu/sweep.hpp"
#include "apu/divider.hpp"
#include "apu/sequencer.hpp"
#include "apu/length_counter.hpp"

#include "types.hpp"

struct NHLogger;

namespace nh {

struct Pulse {
  public:
    Pulse(bool i_mode_1, NHLogger *i_logger);
    ~Pulse() = default;
    NB_KLZ_DELETE_COPY_MOVE(Pulse);

  public:
    /// @return Amplitude in range of [0, 15]
    Byte
    amplitude() const;

    void
    tick_timer();
    void
    tick_envelope();
    void
    tick_sweep();
    void
    tick_length_counter();

  public:
    Envelope &
    envelope();
    Sweep &
    sweep();
    Divider &
    timer();
    Sequencer &
    sequencer();
    LengthCounter &
    length_counter();

  private:
    Envelope m_envel;
    Sweep m_sweep;
    Divider m_timer;
    Sequencer m_seq;
    LengthCounter m_length;
};

} // namespace nh

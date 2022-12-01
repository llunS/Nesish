#pragma once

#include "common/klass.hpp"

#include "console/apu/envelope.hpp"
#include "console/apu/sweep.hpp"
#include "console/apu/divider.hpp"
#include "console/apu/sequencer.hpp"
#include "console/apu/length_counter.hpp"

#include "console/types.hpp"

namespace ln {

struct Pulse {
  public:
    Pulse(bool i_mode_1);
    ~Pulse() = default;
    LN_KLZ_DELETE_COPY_MOVE(Pulse);

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
    LengthCounter m_length_ctr;
};

} // namespace ln

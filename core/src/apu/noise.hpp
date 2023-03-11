#pragma once

#include "nhbase/klass.hpp"

#include "apu/envelope.hpp"
#include "apu/divider.hpp"
#include "apu/length_counter.hpp"

#include "types.hpp"

struct NHLogger;

namespace nh {

struct Noise {
  public:
    Noise(NHLogger *i_logger);
    ~Noise() = default;
    NB_KLZ_DELETE_COPY_MOVE(Noise);

  public:
    /// @return Amplitude in range of [0, 15]
    Byte
    amplitude() const;

    void
    tick_timer();
    void
    tick_envelope();
    void
    tick_length_counter();

  public:
    void
    reset_lfsr();
    void
    set_mode(bool i_set);
    void
    set_timer_reload(Byte i_index);

  public:
    Envelope &
    envelope();
    LengthCounter &
    length_counter();

  private:
    Envelope m_envel;
    Divider m_timer;
    Byte2 m_shift;
    LengthCounter m_length;

    bool m_mode;

  private:
    NHLogger *m_logger;
};

} // namespace nh

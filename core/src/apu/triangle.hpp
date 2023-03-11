#pragma once

#include "nhbase/klass.hpp"

#include "apu/divider.hpp"
#include "apu/linear_counter.hpp"
#include "apu/length_counter.hpp"

#include "types.hpp"

struct NHLogger;

namespace nh {

struct Triangle {
  public:
    Triangle(NHLogger *i_logger);
    ~Triangle() = default;
    NB_KLZ_DELETE_COPY_MOVE(Triangle);

  public:
    /// @return Amplitude in range of [0, 15]
    Byte
    amplitude() const;

    void
    reset();

    void
    tick_timer();
    void
    tick_linear_counter();
    void
    tick_length_counter();

  public:
    Divider &
    timer();
    LinearCounter &
    linear_counter();
    LengthCounter &
    length_counter();

  private:
    Divider m_timer;
    LinearCounter m_linear;
    LengthCounter m_length;

    // Don't bother to make a sequencer class
    unsigned int m_seq_idx;
};

} // namespace nh

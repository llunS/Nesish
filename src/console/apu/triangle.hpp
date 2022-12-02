#pragma once

#include "common/klass.hpp"

#include "console/apu/divider.hpp"
#include "console/apu/linear_counter.hpp"
#include "console/apu/length_counter.hpp"

#include "console/types.hpp"

namespace ln {

struct Triangle {
  public:
    Triangle();
    ~Triangle() = default;
    LN_KLZ_DELETE_COPY_MOVE(Triangle);

  public:
    /// @return Amplitude in range of [0, 15]
    Byte
    amplitude() const;

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

} // namespace ln

#pragma once

#include "nhbase/klass.hpp"

#include "blip_buf.h"

namespace sh {

struct Resampler {
  public:
    Resampler(int i_buffer_size, short i_amp = 0);
    ~Resampler();
    NB_KLZ_DELETE_COPY_MOVE(Resampler);

  public:
    bool
    set_rates(double i_clock_rate, double i_sample_rate);

    void
    clock(short i_amp);
    bool
    samples_avail(short o_samples[], int i_count);

    void
    close();

  private:
    short m_amp;

    blip_buffer_t *m_blip;
    int m_buffer_size; // in samples
    int m_clock_in_frame;
    int m_frame_size; // in clocks
};

} // namespace sh

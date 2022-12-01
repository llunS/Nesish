#pragma once

#include "common/klass.hpp"

#include "glfw_app/blip_buf/blip_buf.h"

namespace ln_app {

struct BlipBufAdapter {
  public:
    BlipBufAdapter(double i_clock_rate, double i_sample_rate, int i_buffer_size,
                   short i_amp = 0);
    ~BlipBufAdapter();
    LN_KLZ_DELETE_COPY_MOVE(BlipBufAdapter);

  public:
    void
    clock(short i_amp);
    bool
    samples_avail(short o_samples[], int i_count);
    int
    flush_samples(short o_samples[], int i_bound);

    void
    close();

  private:
    short m_amp;

    blip_buffer_t *m_blip;
    int m_buffer_size; // in samples
    int m_clock_in_frame;
    int m_frame_size; // in clocks
};

} // namespace ln_app

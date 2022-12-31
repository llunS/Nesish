#include "resampler.hpp"

#include <cassert>

namespace ln_app {

Resampler::Resampler(int i_buffer_size, short i_amp)
    : m_amp(i_amp)
    , m_blip(nullptr)
    , m_buffer_size(i_buffer_size)
    , m_clock_in_frame(0)
    , m_frame_size(1)
{
    int blip_buffer_size = i_buffer_size;
    if (blip_buffer_size <= 0)
    {
        throw "invalid rate";
    }

    m_blip = blip_new(blip_buffer_size);
    if (m_blip == nullptr)
    {
        throw "blip out of memory";
    }
}

Resampler::~Resampler()
{
    close();
}

void
Resampler::close()
{
    if (m_blip)
    {
        blip_delete(m_blip);
    }
    m_blip = nullptr;
}

bool
Resampler::set_rates(double i_clock_rate, double i_sample_rate)
{
    if (i_clock_rate <= 0 || i_sample_rate <= 0 ||
        i_sample_rate / i_clock_rate > m_buffer_size)
    {
        return false;
    }

    blip_set_rates(m_blip, i_clock_rate, i_sample_rate);
    return true;
}

void
Resampler::clock(short i_amp)
{
    int delta = i_amp - m_amp;
    m_amp = i_amp;
    if (delta)
    {
        blip_add_delta(m_blip, m_clock_in_frame, delta);
    }

    if (!m_clock_in_frame)
    {
        m_frame_size = blip_clocks_needed(m_blip, 1);
    }

    if (m_clock_in_frame + 1 >= m_frame_size)
    {
        blip_end_frame(m_blip, m_frame_size);
        m_clock_in_frame = 0;
    }
    else
    {
        ++m_clock_in_frame;
    }
}

bool
Resampler::samples_avail(short o_samples[], int i_count)
{
    if (blip_samples_avail(m_blip) >= i_count)
    {
        int count = blip_read_samples(m_blip, o_samples, i_count, 0);
        assert(count == i_count);
        return bool(count);
    }
    return false;
}

} // namespace ln_app

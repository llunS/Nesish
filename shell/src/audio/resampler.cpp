#include "resampler.hpp"

#include <cassert>

namespace sh {

Resampler::Resampler()
    : m_amp(0)
    , m_blip(nullptr)
    , m_buffer_size(0)
    , m_clock_in_frame(0)
    , m_frame_size(1)
{
}

bool
Resampler::init(int i_buffer_size, short i_amp)
{
    if (i_buffer_size <= 0)
    {
        return false;
    }

    m_amp = i_amp;
    m_buffer_size = i_buffer_size;
    m_clock_in_frame = 0;
    m_frame_size = 1;

    m_blip = blip_new(i_buffer_size);
    if (m_blip == nullptr)
    {
        return false;
    }
    return true;
}

void
Resampler::close()
{
    if (m_blip)
    {
        blip_delete(m_blip);
        m_blip = nullptr;
    }
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

} // namespace sh

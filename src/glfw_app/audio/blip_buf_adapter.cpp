#include "blip_buf_adapter.hpp"

#include <cassert>

namespace ln_app {

BlipBufAdapter::BlipBufAdapter(double i_clock_rate, double i_sample_rate,
                               int i_buffer_size, short i_amp)
    : m_amp(i_amp)
    , m_blip(nullptr)
    , m_buffer_size(i_buffer_size)
    , m_clock_in_frame(0)
    , m_frame_size(1)
{
    int blip_buffer_size = i_sample_rate / 10;
    if (!blip_buffer_size || i_sample_rate / i_clock_rate > blip_buffer_size)
    {
        throw "invalid rate";
    }

    if (blip_buffer_size < i_buffer_size)
    {
        throw "invalid buffer size";
    }

    m_blip = blip_new(blip_buffer_size);
    if (m_blip == nullptr)
    {
        throw "blip out of memory";
    }

    blip_set_rates(m_blip, i_clock_rate, i_sample_rate);
}

BlipBufAdapter::~BlipBufAdapter()
{
    close();
}

void
BlipBufAdapter::close()
{
    if (m_blip)
    {
        blip_delete(m_blip);
    }
    m_blip = nullptr;
}

void
BlipBufAdapter::clock(short i_amp)
{
    int delta = i_amp - m_amp;
    m_amp = i_amp;
    if (delta)
    {
        blip_add_delta(m_blip, m_clock_in_frame, delta);
    }

    if (!m_clock_in_frame)
    {
        m_frame_size = blip_clocks_needed(m_blip, m_buffer_size);
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
BlipBufAdapter::samples_avail(short o_samples[], int i_count)
{
    if (blip_samples_avail(m_blip) >= i_count)
    {
        int count = blip_read_samples(m_blip, o_samples, i_count, 0);
        assert(count == i_count);
        return bool(count);
    }
    return false;
}

int
BlipBufAdapter::flush_samples(short o_samples[], int i_bound)
{
    if (blip_samples_avail(m_blip) > 0)
    {
        return blip_read_samples(m_blip, o_samples, i_bound, 0);
    }
    return 0;
}

} // namespace ln_app

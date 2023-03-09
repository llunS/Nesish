#include "dmc.hpp"

#include "console/assert.hpp"

#include "console/apu/dmc_dma.hpp"

// https://www.nesdev.org/wiki/APU_DMC

namespace nh {

DMC::DMC(DMCDMA &o_dmc_dma)
    : m_dmc_dma(o_dmc_dma)

    , m_irq_enabled(false)
    , m_loop(false)
    , m_sample_addr(0)
    , m_sample_length(0)

    , m_shift(0)
    , m_bits_remaining(8)
    , m_level(0)
    , m_silence(true)

    , m_sample_buffer(0)
    , m_sample_buffer_empty(true)

    , m_sample_curr(0)
    , m_sample_bytes_left(0)

    , m_irq(false)
{
}

Byte
DMC::amplitude() const
{
    return m_level;
}

bool
DMC::interrupt() const
{
    return m_irq;
}

void
DMC::tick_timer()
{
    /* Sample fetch */
    // The sample fetch is done by Memory Reader (DMA), which runs independently
    // of the Output Unit.

    // @TEST: Not certain about this
    // When sample fetch done by DMA and silence check below occur at the same
    // cycle, Ticking DMA first plays the fetched sample sooner by one output
    // cycle than it would do otherwise.

    /* Output Unit */
    // i.e. playback of samples
    if (m_timer.tick())
    {
        if (!m_silence)
        {
            if (m_shift & 0x01)
            {
                if (m_level + 2 <= 127)
                {
                    m_level += 2;
                }
            }
            else
            {
                if (m_level >= 2)
                {
                    m_level -= 2;
                }
            }
        }

        m_shift >>= 1;

        if (m_bits_remaining <= 0)
        {
            LN_ASSERT_FATAL("Invalid bits remaining value {}",
                            m_bits_remaining);
        }
        // let it overflow to make the bug apparent.
        --m_bits_remaining;
        /* new output cycle */
        if (!m_bits_remaining)
        {
            m_bits_remaining = 8;

            if (m_sample_buffer_empty)
            {
                m_silence = true;
            }
            else
            {
                m_silence = false;

                m_shift = m_sample_buffer;
                m_sample_buffer_empty = true;

                // Check to do reload DMA
                // No need to check "m_sample_buffer_empty" in this block
                if (/*m_sample_buffer_empty &&*/ m_sample_bytes_left > 0)
                {
                    m_dmc_dma.initiate(m_sample_curr, true);
                }
            }
        }
    }
}

void
DMC::set_interrupt_enabled(bool i_set)
{
    m_irq_enabled = i_set;
    if (!i_set)
    {
        m_irq = false;
    }
}

void
DMC::set_loop(bool i_set)
{
    m_loop = i_set;
}

void
DMC::set_timer_reload(Byte i_index)
{
    static constexpr Byte2 periods[0x10] = {
        428 / 2, 380 / 2, 340 / 2, 320 / 2, 286 / 2, 254 / 2, 226 / 2, 214 / 2,
        190 / 2, 160 / 2, 142 / 2, 128 / 2, 106 / 2, 84 / 2,  72 / 2,  54 / 2};
    static_assert(periods[0x0F], "Missing elements");

    if (i_index >= 0x10)
    {
        LN_ASSERT_FATAL("Invalid dmc timer reload index: {}", i_index);
        return;
    }
    Byte2 reload = periods[i_index] - 1;
    m_timer.set_reload(reload);
}

void
DMC::load(Byte i_val)
{
    m_level = i_val;
}

void
DMC::set_sample_addr(Byte i_sample_addr)
{
    m_sample_addr = 0xC000 + (i_sample_addr << 6);
}

void
DMC::set_sample_length(Byte i_sample_length)
{
    m_sample_length = (i_sample_length << 4) + 1;
}

void
DMC::set_enabled(bool i_set)
{
    if (!i_set)
    {
        m_sample_bytes_left = 0;
    }
    else
    {
        // Restart only if there's no bytes left to load
        if (!m_sample_bytes_left)
        {
            restart_playback();

            // Load DMA only if we restarts, it looks right since
            // remaining bytes could be left to reload DMA.
            if (m_sample_buffer_empty && m_sample_bytes_left > 0)
            {
                m_dmc_dma.initiate(m_sample_curr, false);
            }
        }
    }
}

void
DMC::clear_interrupt()
{
    m_irq = false;
}

bool
DMC::bytes_remained() const
{
    return m_sample_bytes_left;
}

void
DMC::put_sample(Address i_sample_addr, Byte i_sample)
{
    // Due to DMA costing time, the DMC may be disabled (i.e.
    // m_sample_bytes_left == 0) after DMA has been initiated.
    // Check address as well in case it's stale request (IF there exists this
    // kind).
    if (i_sample_addr == m_sample_curr && m_sample_bytes_left > 0)
    {
        m_sample_buffer = i_sample;
        m_sample_buffer_empty = false;

        if (m_sample_curr >= 0xFFFF)
        {
            m_sample_curr = 0x8000;
        }
        else
        {
            ++m_sample_curr;
        }

        --m_sample_bytes_left;
        if (!m_sample_bytes_left)
        {
            if (m_loop)
            {
                restart_playback();
            }
            else
            {
                if (m_irq_enabled)
                {
                    m_irq = true;
                }
            }
        }
    }
    else
    {
        // Failed due to address mismatch (if any)
        // if (m_sample_bytes_left <= 0)
        {
            // Discard previously initiated DMA result
        }

        // else if (i_sample_addr != m_sample_curr)
        {
            // In current implementation, no way i_sample_addr > m_sample_curr,
            // so it must be that m_sample_curr < m_sample_curr. Then it's a
            // stale DMA we are happy to ignore (if this exists). The bytes left
            // counter must have been updated already, i.e. we are free of
            // possible stuck due to counter not being updated.
        }
    }
}

void
DMC::restart_playback()
{
    m_sample_curr = m_sample_addr;
    m_sample_bytes_left = m_sample_length;
}

} // namespace nh

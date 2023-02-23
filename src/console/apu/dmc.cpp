#include "dmc.hpp"

#include "console/assert.hpp"

#include "console/memory/memory.hpp"

// https://www.nesdev.org/wiki/APU_DMC

namespace ln {

DMC::DMC()
    : m_irq_enabled(false)
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
    , m_fetch_timer(0)

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
DMC::tick_timer(const Memory &i_memory)
{
    /* Sample fetch */
    // On even CPU cycle (i.e. every APU cycle)
    // @NOTE: The sample fetch is done by Memory Reader, which behaves like it's
    // running independently of the Output Unit.
    // @TEST: Note that putting this ahead so that we don't waste another output
    // cycle of the playback if the silence check and the sample fetch occur at
    // the same tick.

    // @TODO: A DMC DMA may also alter the last value read if it interrupts an
    // instruction

    /* 1. Check if there is timed fetch */
    if (m_fetch_timer > 0)
    {
        --m_fetch_timer;
        if (!m_fetch_timer)
        {
            fetch_sample(i_memory);
        }
    }
    /* 2. Check if we should schedule a new fetch */
    if (m_sample_buffer_empty && m_sample_bytes_left)
    {
        /* Schedule the actual fetch 2 APU cycles after this tick, ONLY IF we
         * are not in a fetch already */
        if (m_fetch_timer <= 0)
        {
            // @IMPL: The CPU is stalled for up to 4 CPU cycles to allow the
            // longest possible write (the return address and write after an
            // IRQ) to finish. If OAM DMA is in progress, it is paused for
            // two cycles. The sample fetch always occurs on an even CPU
            // cycle due to its alignment with the APU. Stall the CPU for
            // https://forums.nesdev.org/viewtopic.php?p=62690#p62690
            // https://forums.nesdev.org/viewtopic.php?p=95703#95703

            // @IMPL: The CPU does not always block for 4 cycles, we make it
            // 4 for simplicity, though this loses accuracy.
            // https://www.nesdev.org/wiki/APU_DMC#Likely_internal_implementation_of_the_read
            m_fetch_timer = 4 / 2;
        }
    }

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
    static constexpr Byte2 lookup[0x10] = {
        428 / 2, 380 / 2, 340 / 2, 320 / 2, 286 / 2, 254 / 2, 226 / 2, 214 / 2,
        190 / 2, 160 / 2, 142 / 2, 128 / 2, 106 / 2, 84 / 2,  72 / 2,  54 / 2};
    static_assert(lookup[0x0F], "Missing elements");

    if (i_index >= 0x10)
    {
        LN_ASSERT_FATAL("Invalid dmc timer reload index: {}", i_index);
        return;
    }
    m_timer.set_reload(lookup[i_index]);
}

void
DMC::load(Byte i_val)
{
    m_level = i_val;

    // @QUIRK: The DMC output level is set to D, an unsigned value. If the timer
    // is outputting a clock at the same time, the output level is occasionally
    // not changed properly.
    // http://forums.nesdev.org/viewtopic.php?p=104491#p104491
    // @NOTE: We don't emulate this, since we are basically running CPU and APU
    // serially.
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

        // @IMPL: Stop any pending fetch, otherwise fetch_sample() will be
        // called with "m_sample_bytes_left" being 0.
        m_fetch_timer = 0;
    }
    else
    {
        // @IMPL: There must be no pending fetch (i.e. "m_fetch_timer" <= 0
        // now), if "m_sample_bytes_left" == 0.
        if (!m_sample_bytes_left)
        {
            restart_playback();
        }
    }
}

void
DMC::clear_interrupt()
{
    m_irq = false;
}

bool
DMC::bytes_remaining() const
{
    return m_sample_bytes_left;
}

bool
DMC::fetching() const
{
    return m_fetch_timer > 0;
}

void
DMC::fetch_sample(const Memory &i_memory)
{
    Byte sample = 0xFF;
    // @TODO: Open bus
    ln::Error err = i_memory.get_byte(m_sample_curr, sample);
    if (LN_FAILED(err))
    {
        LN_ASSERT_FATAL("Failed to get sample byte at ${:04X}", m_sample_curr);
    }
    m_sample_buffer = sample;
    m_sample_buffer_empty = false;

    if (m_sample_curr >= 0xFFFF)
    {
        m_sample_curr = 0x8000;
    }
    else
    {
        ++m_sample_curr;
    }

    // If "m_sample_bytes_left" is zero, fetch_sample() shouldn't be called.
    if (m_sample_bytes_left <= 0)
    {
        LN_ASSERT_FATAL("Called fetch_sample() when bytes remaining is 0");
    }
    // let it overflow to make the bug apparent.
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

void
DMC::restart_playback()
{
    m_sample_curr = m_sample_addr;
    m_sample_bytes_left = m_sample_length;
}

} // namespace ln

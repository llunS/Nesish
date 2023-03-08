#include "frame_counter.hpp"

#include "console/apu/pulse.hpp"
#include "console/apu/triangle.hpp"
#include "console/apu/noise.hpp"

// https://www.nesdev.org/wiki/APU_Frame_Counter

namespace ln {

FrameCounter::FrameCounter(Pulse &o_pulse1, Pulse &o_pulse2,
                           Triangle &o_triangle, Noise &o_noise)
    : m_pulse1(o_pulse1)
    , m_pulse2(o_pulse2)
    , m_triangle(o_triangle)
    , m_noise(o_noise)
{
}

/// @brief  Reset some states only required in implementation rather than in
/// hardware
void
FrameCounter::power_up()
{
    m_timer = 0; // set again later due to register write powerup
    m_irq = false;

    m_mode = false;        // set again later due to register write powerup
    m_irq_inhibit = false; // set again later due to register write powerup

    m_first_loop = true;
    m_reset_counter = 0;
    // m_mode_tmp: doesn't matter
}

void
FrameCounter::tick()
{
    /* delay reset logic */
    if (m_reset_counter)
    {
        --m_reset_counter;
        if (!m_reset_counter)
        {
            m_mode = m_mode_tmp;
            m_timer = 0;
            m_first_loop = true;
        }
    }

    auto check_set_irq = [this]() {
        if (!this->m_mode && !this->m_irq_inhibit)
        {
            this->m_irq = true;
        }
    };

    // Check details of the following timing in blargg_apu_2005.07.30/readme.txt
    switch (m_timer)
    {
        case 7458:
        {
            tick_envelope_and_linear_counter();
        }
        break;

        case 14914:
        {
            tick_envelope_and_linear_counter();
            tick_length_counter_and_sweep();
        }
        break;

        case 22372:
        {
            tick_envelope_and_linear_counter();
        }
        break;

        case 29829:
        {
            check_set_irq();
        }
        break;
        case 0:
        {
            if (!m_mode)
            {
                if (!m_first_loop)
                {
                    tick_envelope_and_linear_counter();
                    tick_length_counter_and_sweep();

                    check_set_irq();
                }
            }
            else
            {
                tick_envelope_and_linear_counter();
                tick_length_counter_and_sweep();
            }
        }
        break;
        case 1:
        {
            if (!m_first_loop)
            {
                check_set_irq();
            }
        }
        break;

        default:
            break;
    }

    // Update timer to next value
    ++m_timer;
    if (!m_mode)
    {
        if (m_timer >= 29830)
        {
            m_timer = 0;
            m_first_loop = false;
        }
    }
    else
    {
        if (m_timer >= 37282)
        {
            m_timer = 0;
            m_first_loop = false;
        }
    }
}

bool
FrameCounter::interrupt() const
{
    return m_irq;
}

void
FrameCounter::reset_timer()
{
    m_timer = 0;
}

void
FrameCounter::set_irq_inhibit(bool i_set)
{
    m_irq_inhibit = i_set;
    if (i_set)
    {
        m_irq = false;
    }
}

void
FrameCounter::delay_set_mode(bool i_mode, bool i_delay)
{
    // call this while one is pending would just replace the pending one

    constexpr decltype(m_reset_counter) tick_order = 1; // ticked after CPU
    constexpr decltype(m_reset_counter) pre_dec = 1;    // check tick()
    decltype(m_reset_counter) delay = i_delay;
    m_mode_tmp = i_mode;
    m_reset_counter = tick_order + pre_dec + delay;
}

void
FrameCounter::clear_interrupt()
{
    m_irq = false;
}

void
FrameCounter::tick_length_counter_and_sweep()
{
    m_pulse1.tick_length_counter();
    m_pulse1.tick_sweep();

    m_pulse2.tick_length_counter();
    m_pulse2.tick_sweep();

    m_triangle.tick_length_counter();

    m_noise.tick_length_counter();
}

void
FrameCounter::tick_envelope_and_linear_counter()
{
    m_pulse1.tick_envelope();

    m_pulse2.tick_envelope();

    m_triangle.tick_linear_counter();

    m_noise.tick_envelope();
}

} // namespace ln

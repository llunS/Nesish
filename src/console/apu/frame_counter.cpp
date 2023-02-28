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
    , m_timer(0)
    , m_irq(false)
    , m_step5(false)
    , m_irq_inhibit(false)
{
}

/// @brief  Reset some states only required in implementation rather than in
/// hardware
void
FrameCounter::power_up()
{
    m_irq = false;

    m_first_loop = true;
    m_reset_counter = 0;
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
            m_timer = 0;
            m_first_loop = true;
        }
    }

    auto check_set_irq = [this]() {
        if (!this->m_step5 && !this->m_irq_inhibit)
        {
            this->m_irq = true;
        }
    };

    // @NOTE: Check details of the following timing on
    // blargg_apu_2005.07.30/readme.txt
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
            if (!m_first_loop)
            {
                tick_envelope_and_linear_counter();
                tick_length_counter_and_sweep();

                check_set_irq();
            }
            else
            {
                // Tick both even it's not first loop for 5-step mode
                if (m_step5)
                {
                    tick_envelope_and_linear_counter();
                    tick_length_counter_and_sweep();
                }
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
    if (!m_step5)
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
FrameCounter::set_mode(bool i_step5)
{
    m_step5 = i_step5;
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
FrameCounter::delay_reset(bool i_delay)
{
    // @IMPL: The value set is based on
    // the assumption that APU is ticked after CPU
    if (i_delay)
    {
        // delay by one clock
        m_reset_counter = 3;
    }
    else
    {
        m_reset_counter = 2;
    }
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

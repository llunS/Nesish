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

void
FrameCounter::tick()
{
    if ((!m_step5 && m_timer >= 29830 - 1) || (m_step5 && m_timer >= 37282 - 1))
    {
        m_timer = 0;
    }
    else
    {
        ++m_timer;
    }

    auto check_set_irq = [this]() {
        if (!this->m_step5 && !this->m_irq_inhibit)
        {
            this->m_irq = true;
        }
    };

    switch (m_timer)
    {
        case 7457:
        {
            tick_envelope_and_linear_counter();
        }
        break;

        case 14913:
        {
            tick_envelope_and_linear_counter();
            tick_length_counter_and_sweep();
        }
        break;

        case 22371:
        {
            tick_envelope_and_linear_counter();
        }
        break;

        case 29828:
        {
            check_set_irq();
        }
        break;

        case 29829:
        {
            if (!m_step5)
            {
                tick_envelope_and_linear_counter();
                tick_length_counter_and_sweep();
            }

            check_set_irq();
        }
        break;

        case 37281:
        {
            if (m_step5)
            {
                tick_envelope_and_linear_counter();
                tick_length_counter_and_sweep();
            }
        }
        break;

        case 0:
        {
            check_set_irq();
        }
        break;

        default:
            break;
    }
}

bool
FrameCounter::interrupt() const
{
    return m_irq;
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
FrameCounter::reset(unsigned int i_val)
{
    m_timer = i_val;
    if (m_step5)
    {
        tick_envelope_and_linear_counter();
        tick_length_counter_and_sweep();
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

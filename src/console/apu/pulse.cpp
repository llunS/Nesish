#include "pulse.hpp"

// https://www.nesdev.org/wiki/APU_Pulse

namespace ln {

Pulse::Pulse(bool i_mode_1)
    : m_sweep(m_timer, i_mode_1)
{
}

Byte
Pulse::amplitude() const
{
    if (!m_seq.value())
    {
        return 0;
    }
    if (m_sweep.muting())
    {
        return 0;
    }
    if (!m_length.value())
    {
        return 0;
    }
    return m_envel.volume();
}

void
Pulse::tick_timer()
{
    if (m_timer.tick())
    {
        m_seq.tick();
    }
}

void
Pulse::tick_envelope()
{
    m_envel.tick();
}

void
Pulse::tick_sweep()
{
    m_sweep.tick();
}

void
Pulse::tick_length_counter()
{
    m_length.tick();
}

Envelope &
Pulse::envelope()
{
    return m_envel;
}

Divider &
Pulse::timer()
{
    return m_timer;
}

Sweep &
Pulse::sweep()
{
    return m_sweep;
}

Sequencer &
Pulse::sequencer()
{
    return m_seq;
}

LengthCounter &
Pulse::length_counter()
{
    return m_length;
}

} // namespace ln

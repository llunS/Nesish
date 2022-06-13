#include "clock.hpp"

namespace ln {

Clock::Clock(Hz_t i_frequency)
    : m_cycle(i_frequency > 0 ? 1000.0 / i_frequency : 1000.0)
    , m_time(0.0)
{
}

Cycle
Clock::advance(Time_t i_ms)
{
    auto next = m_time + i_ms;
    Cycle cycles = Cycle(next / m_cycle) - Cycle(m_time / m_cycle);
    m_time = next;
    return cycles;
}

} // namespace ln

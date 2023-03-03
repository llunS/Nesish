#include "apu_clock.hpp"

namespace ln {

APUClock::APUClock()
    : m_cycle(0)
{
}

void
APUClock::tick()
{
    ++m_cycle;
}

void
APUClock::power_up()
{
    m_cycle = 0;
}

void
APUClock::reset()
{
    // do nothing
}

bool
APUClock::get() const
{
    return even();
}

bool
APUClock::put() const
{
    return odd();
}

bool
APUClock::even() const
{
    return m_cycle % 2 == 0;
}

bool
APUClock::odd() const
{
    return !even();
}

} // namespace ln

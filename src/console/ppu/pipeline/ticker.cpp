#include "ticker.hpp"

namespace ln {

Ticker::Ticker(Cycle i_total)
    : m_curr(0)
    , m_total(i_total)
{
}

void
Ticker::reset()
{
    m_curr = 0;
}

bool
Ticker::done()
{
    return m_curr >= m_total;
}

void
Ticker::set_done()
{
    m_curr = m_total;
}

void
Ticker::tick()
{
    if (done())
    {
        return;
    }

    auto cycles = on_tick(m_curr, m_total);
    m_curr += cycles;
}

} // namespace ln

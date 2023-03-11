#include "tickable.hpp"

namespace nh {

Tickable::Tickable(Cycle i_total)
    : m_curr(0)
    , m_total(i_total)
{
}

void
Tickable::reset()
{
    m_curr = 0;
}

bool
Tickable::done()
{
    return m_curr >= m_total;
}

void
Tickable::set_done()
{
    m_curr = m_total;
}

void
Tickable::tick()
{
    if (done())
    {
        return;
    }

    auto cycles = on_tick(m_curr, m_total);
    m_curr += cycles;
}

} // namespace nh

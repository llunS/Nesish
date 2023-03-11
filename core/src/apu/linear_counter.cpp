#include "linear_counter.hpp"

/* Basically like length counter, but with a higher resolution (quarter frame
 * instead of half frame). */
// https://www.nesdev.org/wiki/APU_Triangle

namespace nh {

LinearCounter::LinearCounter()
    : m_counter(0)
    , m_control(false)
    , m_reload(false)
    , m_reload_val(0)
{
}

Byte
LinearCounter::value() const
{
    return m_counter;
}

void
LinearCounter::tick()
{
    if (m_reload)
    {
        m_counter = m_reload_val;
    }
    else
    {
        if (m_counter > 0)
        {
            --m_counter;
        }
    }

    if (!m_control)
    {
        m_reload = false;
    }
}

void
LinearCounter::set_control(bool i_set)
{
    m_control = i_set;
}

void
LinearCounter::set_reload()
{
    m_reload = true;
}

void
LinearCounter::set_reload_val(Byte i_reload)
{
    m_reload_val = i_reload;
}

} // namespace nh

#include "envelope.hpp"

// https://www.nesdev.org/wiki/APU_Envelope

namespace nh {

Envelope::Envelope()
    : m_start(false)
    , m_decay_level(0)
    , m_loop(false)
    , m_const(false)
    , m_const_vol(0)
{
}

Byte
Envelope::volume() const
{
    if (m_const)
    {
        return m_const_vol;
    }
    else
    {
        return m_decay_level;
    }
}

void
Envelope::tick()
{
    if (!m_start)
    {
        if (m_divider.tick())
        {
            if (m_decay_level)
            {
                --m_decay_level;
            }
            else
            {
                if (m_loop)
                {
                    m_decay_level = 15;
                }
                else
                {
                    // stay at 0.
                }
            }
        }
    }
    else
    {
        m_start = 0;
        m_decay_level = 15;
        m_divider.reload();
    }
}

void
Envelope::set_divider_reload(Byte2 i_reload)
{
    m_divider.set_reload(i_reload);
}

void
Envelope::set_loop(bool i_set)
{
    m_loop = i_set;
}

void
Envelope::set_const(bool i_set)
{
    m_const = i_set;
}

void
Envelope::set_const_vol(Byte i_vol)
{
    m_const_vol = i_vol;
}

void
Envelope::restart()
{
    m_start = true;
}

} // namespace nh

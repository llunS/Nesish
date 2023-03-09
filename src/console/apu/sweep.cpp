#include "sweep.hpp"

// https://www.nesdev.org/wiki/APU_Sweep

namespace nh {

Sweep::Sweep(Divider &io_ch_timer, bool i_mode_1)
    : m_reload(false)
    , m_enabled(false)
    , m_negate(false)
    , m_shift(0)
    , m_ch_timer(io_ch_timer)
    , m_mode_1(i_mode_1)
{
}

bool
Sweep::muting(Byte2 *i_target) const
{
    if (m_ch_timer.get_reload() < 8)
    {
        return true;
    }
    Byte2 target = i_target ? *i_target : target_reload();
    if (target > 0x07FF)
    {
        return true;
    }
    return false;
}

void
Sweep::tick()
{
    // Tick as normal first, then check reload.
    // This means if the divider was 0 before the reload, the period adjustment
    // check applies as normal.
    // https://archive.nes.science/nesdev-forums/f3/t11083.xhtml
    // https://archive.nes.science/nesdev-forums/f2/t19285.xhtml

    if (m_divider.tick())
    {
        if (m_enabled && m_shift)
        {
            auto target = target_reload();
            if (!muting(&target))
            {
                // update the channel's period.
                m_ch_timer.set_reload(target);
            }
        }
    }

    if (m_reload)
    {
        // Do this even if it may have been reloaded already in the above
        // tick(), for it doesn't matter.
        m_divider.reload();
    }
    m_reload = false; // clear the flag
}

void
Sweep::set_enabled(bool i_set)
{
    m_enabled = i_set;
}

void
Sweep::set_divider_reload(Byte2 i_reload)
{
    m_divider.set_reload(i_reload);
}

void
Sweep::set_negate(bool i_set)
{
    m_negate = i_set;
}

void
Sweep::set_shift_count(Byte i_count)
{
    m_shift = i_count;
}

void
Sweep::reload()
{
    m_reload = true;
}

Byte2
Sweep::target_reload() const
{
    Byte2 current = m_ch_timer.get_reload();
    Byte2 delta = current >> m_shift;
    if (m_mode_1)
    {
        delta = -delta - 1;
    }
    else
    {
        delta = -delta;
    }
    Byte2 target = current + delta;
    return target;
}

} // namespace nh

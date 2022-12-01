#include "divider.hpp"

namespace ln {

Divider::Divider()
    : Divider(0)
{
}

Divider::Divider(Byte2 i_reload)
    : m_reload(i_reload)
    , m_ctr(i_reload)
{
}

Byte2
Divider::get_reload() const
{
    return m_reload;
}

void
Divider::set_reload(Byte2 i_reload)
{
    m_reload = i_reload;
}

void
Divider::reload()
{
    m_ctr = m_reload;
}

Byte2
Divider::value() const
{
    return m_ctr;
}

bool
Divider::tick()
{
    if (!m_ctr)
    {
        reload();
        return true;
    }
    else
    {
        --m_ctr;
        return false;
    }
}

} // namespace ln

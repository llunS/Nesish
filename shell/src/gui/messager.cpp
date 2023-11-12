#include "messager.hpp"

#include "gui/application.hpp"

namespace sh
{

bool
Messager::running_game() const
{
    return m_app->running_game();
}

Logger *
Messager::get_logger() const
{
    return m_app->m_logger;
}

const KeyMapping &
Messager::get_key_mapping(NHCtrlPort i_port) const
{
    return i_port == NH_CTRL_P1 ? m_app->m_p1_keys : m_app->m_p2_keys;
}

void
Messager::reset_key_mapping(const KeyMapping &i_p1,
                            const KeyMapping &i_p2) const
{
    m_app->m_p1_keys = i_p1;
    m_app->m_p2_keys = i_p2;
}

} // namespace sh

#include "messager.hpp"

#include "gui/application.hpp"

namespace sh {

bool
Messager::running_game() const
{
    return m_app->running_game();
}

} // namespace sh

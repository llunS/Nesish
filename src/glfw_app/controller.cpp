#include "controller.hpp"

namespace ln_app {

Controller::Controller(GLFWwindow *window)
    : m_window(window)
    , m_strobing(false)
    , m_strobe_idx(ln::KEY_END)
{
}

void
Controller::strobe(bool i_on)
{
    m_strobing = i_on;

    if (!i_on)
    {
        reload_states();
        m_strobe_idx = ln::KEY_BEGIN;
    }
}

bool
Controller::report()
{
    if (m_strobing)
    {
        reload_states();
        return m_key_state[ln::KEY_A];
    }
    else
    {
        if (m_strobe_idx < ln::KEY_END)
        {
            return m_key_state[m_strobe_idx++];
        }
        else
        {
            // https://www.nesdev.org/wiki/Standard_controller
            // "All subsequent reads will return 1 on official Nintendo brand
            // controllers but may return 0 on third party controllers such as
            // the U-Force."
            return true;
        }
    }
}

void
Controller::reload_states()
{
    // reload all bits with latest state.
    for (ln::KeyIt it = ln::KEY_BEGIN; it < ln::KEY_END; ++it)
    {
        auto key = ln::Key(it);

        auto app_key = map_key(key);
        m_key_state[key] = glfwGetKey(m_window, app_key) == GLFW_PRESS;
    }
}

GLFWKey
ControllerP1::map_key(ln::Key i_key)
{
    // @TODO: Support custom mapping.
    constexpr static GLFWKey s_mapping[ln::KEY_SIZE] = {
        GLFW_KEY_K, GLFW_KEY_J, GLFW_KEY_V, GLFW_KEY_B,
        GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D,
    };
    return s_mapping[i_key];
}

GLFWKey
ControllerP2::map_key(ln::Key i_key)
{
    // @TODO: Support custom mapping.
    constexpr static GLFWKey s_mapping[ln::KEY_SIZE] = {
        GLFW_KEY_PERIOD,        GLFW_KEY_COMMA, GLFW_KEY_LEFT_BRACKET,
        GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_UP,    GLFW_KEY_DOWN,
        GLFW_KEY_LEFT,          GLFW_KEY_RIGHT,
    };
    return s_mapping[i_key];
}

} // namespace ln_app

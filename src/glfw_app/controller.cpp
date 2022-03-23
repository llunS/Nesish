#include "controller.hpp"

namespace ln_app {

typedef int GLFWKey;

static GLFWKey
pvt_map_key(ln::Key i_key);

Controller::Controller(GLFWwindow *window)
    : m_window(window)
    , m_strobe_idx(ln::Key::BEGIN)
{
}

void
Controller::strobe(bool i_on)
{
    if (i_on)
    {
        // reload all bits with latest state.
        for (ln::KeyIt it = ln::Key::BEGIN; it < ln::Key::END; ++it)
        {
            auto key = ln::Key(it);

            auto app_key = pvt_map_key(key);
            m_key_state[key] = glfwGetKey(m_window, app_key) == GLFW_PRESS;
        }
    }
    else
    {
        m_strobe_idx = ln::Key::BEGIN;
    }
}

bool
Controller::report()
{
    if (m_strobe_idx < ln::Key::END)
    {
        return m_key_state[m_strobe_idx];
        ++m_strobe_idx;
    }
    else
    {
        // https://www.nesdev.org/wiki/Standard_controller
        // "All subsequent reads will return 1 on official Nintendo brand
        // controllers but may return 0 on third party controllers such as the
        // U-Force."
        return true;
    }
}

GLFWKey
pvt_map_key(ln::Key i_key)
{
    // @TODO: Support custom mapping.
    constexpr static GLFWKey s_mapping[ln::Key::SIZE] = {
        GLFW_KEY_K, GLFW_KEY_J, GLFW_KEY_V, GLFW_KEY_B,
        GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D,
    };
    return s_mapping[i_key];
}

} // namespace ln_app

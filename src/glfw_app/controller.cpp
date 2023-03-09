#include "controller.hpp"

namespace sh {

Controller::Controller(GLFWwindow *window)
    : m_window(window)
{
    reset();
}

void
Controller::strobe(bool i_on)
{
    m_strobing = i_on;

    if (!i_on)
    {
        reload_states();
        m_strobe_idx = nh::KEY_BEGIN;
        m_8_bits_read = false;
    }
}

bool
Controller::report()
{
    if (m_strobing)
    {
        reload_states();
        return m_key_state[nh::KEY_A];
    }
    else
    {
        if (m_strobe_idx < nh::KEY_END)
        {
            if (m_strobe_idx + 1 >= nh::KEY_END)
            {
                m_8_bits_read = true;
            }
            return m_key_state[m_strobe_idx++];
        }
        else
        {
            // https://www.nesdev.org/wiki/Standard_controller
            // "All subsequent reads will return 1 on official Nintendo brand
            // controllers but may return 0 on third party controllers such as
            // the U-Force."

            // @TEST: Is this necessary? Or the test itself
            // (cpu_exec_space/test_cpu_exec_space_apu.nes) is defected?
            return m_8_bits_read ? true : false;
        }
    }
}

void
Controller::reset()
{
    m_strobing = false;
    m_strobe_idx = nh::KEY_END;
    for (nh::KeyIt it = nh::KEY_BEGIN; it < nh::KEY_END; ++it)
    {
        m_key_state[it] = false;
    }

    m_8_bits_read = false;
}

void
Controller::reload_states()
{
    // reload all bits with latest state.
    for (nh::KeyIt it = nh::KEY_BEGIN; it < nh::KEY_END; ++it)
    {
        auto key = nh::Key(it);

        auto app_key = map_key(key);
        m_key_state[key] = glfwGetKey(m_window, app_key) == GLFW_PRESS;
    }
}

GLFWKey
ControllerP1::map_key(nh::Key i_key)
{
    // @TODO: Custom mapping
    constexpr static GLFWKey s_mapping[nh::KEY_SIZE] = {
        GLFW_KEY_K, GLFW_KEY_J, GLFW_KEY_V, GLFW_KEY_B,
        GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D,
    };
    return s_mapping[i_key];
}

GLFWKey
ControllerP2::map_key(nh::Key i_key)
{
    // @TODO: Custom mapping
    constexpr static GLFWKey s_mapping[nh::KEY_SIZE] = {
        GLFW_KEY_PERIOD,        GLFW_KEY_COMMA, GLFW_KEY_LEFT_BRACKET,
        GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_UP,    GLFW_KEY_DOWN,
        GLFW_KEY_LEFT,          GLFW_KEY_RIGHT,
    };
    return s_mapping[i_key];
}

} // namespace sh

#include "controller.hpp"

namespace sh
{

Controller::Controller(GLFWwindow *i_window,
                       const std::array<VirtualKey, NH_KEYS> &i_mapping)
    : m_window(i_window)
    , m_mapping(i_mapping)
{
    reset();
}

void
Controller::strobe(bool i_on)
{
    m_strobing = i_on;

    if (!i_on) {
        reload_states();
        m_strobe_idx = NH_KEY_BEGIN;
        m_8_bits_read = false;
    }
}

bool
Controller::report()
{
    if (m_strobing) {
        reload_states();
        return m_key_state[NH_KEY_A];
    } else {
        if (m_strobe_idx < NH_KEY_END) {
            if (m_strobe_idx + 1 >= NH_KEY_END) {
                m_8_bits_read = true;
            }
            return m_key_state[m_strobe_idx++];
        } else {
            // https://www.nesdev.org/wiki/Standard_controller
            // "All subsequent reads will return 1 on official Nintendo brand
            // controllers but may return 0 on third party controllers such as
            // the U-Force."

            // After 8 bits are polled, bit 1 is returned. Verified by:
            // 1) cpu_exec_space/test_cpu_exec_space_apu.nes
            // 2) dmc_dma_during_read4/dma_4016_read.nes
            return m_8_bits_read ? true : false;
        }
    }
}

void
Controller::reset()
{
    m_strobing = false;
    m_strobe_idx = NH_KEY_END;
    for (NHKey nhkey = NH_KEY_BEGIN; nhkey < NH_KEY_END; ++nhkey) {
        m_key_state[nhkey] = false;
    }

    m_8_bits_read = false;
}

void
Controller::reload_states()
{
    // reload all bits with latest state.
    for (NHKey nhkey = NH_KEY_BEGIN; nhkey < NH_KEY_END; ++nhkey) {
        auto vkey = map_key(nhkey);
        m_key_state[nhkey] = glfwGetKey(m_window, vkey) == GLFW_PRESS;
    }
}

VirtualKey
Controller::map_key(NHKey i_key)
{
    return m_mapping[i_key];
}

} // namespace sh

#pragma once

#include "nesish/nesish.h"

#include "GLFW/glfw3.h"

#include <array>

namespace sh {

typedef int VirtualKey;
typedef std::array<VirtualKey, NH_KEYS> KeyMapping;

struct Controller {
  public:
    Controller(GLFWwindow *i_window, const KeyMapping &i_mapping);
    ~Controller() = default;

    void
    strobe(bool i_on);
    bool
    report();

    void
    reset();

  private:
    void
    reload_states();

    VirtualKey
    map_key(NHKey i_key);

  private:
    GLFWwindow *m_window;

    bool m_strobing;
    unsigned int m_strobe_idx; // valid only if "m_strobing" == false.
    bool m_key_state[NH_KEYS];

    bool m_8_bits_read;

    const KeyMapping &m_mapping;
};

} // namespace sh

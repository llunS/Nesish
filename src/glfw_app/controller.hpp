
#pragma once

#include "console/peripheral/controller.hpp"

#include "glfw/glfw3.h"

namespace ln_app {

struct Controller : public ln::Controller {
  public:
    Controller(GLFWwindow *window);

    void
    strobe(bool i_on) override;
    bool
    report() override;

  private:
    GLFWwindow *m_window;

    unsigned int m_strobe_idx;
    bool m_key_state[ln::Key::SIZE];
};

} // namespace ln_app

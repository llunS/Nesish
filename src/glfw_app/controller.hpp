
#pragma once

#include "console/peripheral/controller.hpp"

#include "glfw/glfw3.h"

namespace ln_app {

typedef int GLFWKey;

struct Controller : public ln::Controller {
  public:
    Controller(GLFWwindow *window);

    void
    strobe(bool i_on) override;
    bool
    report() override;

    void
    reset() override;

  private:
    void
    reload_states();

  protected:
    virtual GLFWKey
    map_key(ln::Key i_key) = 0;

  private:
    GLFWwindow *m_window;

    bool m_strobing;
    unsigned int m_strobe_idx; // valid only if "m_strobing" == false.
    bool m_key_state[ln::KEY_SIZE];

    bool m_8_bits_read;
};

struct ControllerP1 : public Controller {
  public:
    ControllerP1(GLFWwindow *window)
        : Controller(window)
    {
    }

  protected:
    GLFWKey
    map_key(ln::Key i_key) override;
};

struct ControllerP2 : public Controller {
  public:
    ControllerP2(GLFWwindow *window)
        : Controller(window)
    {
    }

  protected:
    GLFWKey
    map_key(ln::Key i_key) override;
};

} // namespace ln_app

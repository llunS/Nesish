
#pragma once

#include "nesish/nesish.h"

#include "glfw/glfw3.h"

namespace sh {

typedef int GLFWKey;

struct Controller {
  public:
    Controller(GLFWwindow *window);
    virtual ~Controller()
    {
    }

    void
    strobe(bool i_on);
    bool
    report();

    void
    reset();

  private:
    void
    reload_states();

  protected:
    virtual GLFWKey
    map_key(NHKeyIndex i_key) = 0;

  private:
    GLFWwindow *m_window;

    bool m_strobing;
    unsigned int m_strobe_idx; // valid only if "m_strobing" == false.
    bool m_key_state[NH_KEYS];

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
    map_key(NHKeyIndex i_key) override;
};

struct ControllerP2 : public Controller {
  public:
    ControllerP2(GLFWwindow *window)
        : Controller(window)
    {
    }

  protected:
    GLFWKey
    map_key(NHKeyIndex i_key) override;
};

} // namespace sh

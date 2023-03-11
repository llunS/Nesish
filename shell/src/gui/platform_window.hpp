#pragma once

#include "glfw/glfw3.h"

namespace sh {

/// @brief Helper glfw window class
struct PlatformWindow {
  public:
    PlatformWindow();
    virtual ~PlatformWindow(){};

    bool
    init(int i_width, int i_height, bool i_load_gl, bool i_resizable = false,
         const char *i_name = nullptr);
    virtual void
    release();

  public:
    bool
    shouldClose();

    void
    set_pos(int i_x, int i_y);

  protected:
    virtual bool
    post_init()
    {
        return true;
    };

    virtual void
    makeCurrent();
    void
    updateViewportSize();

  protected:
    GLFWwindow *m_win;
    bool m_resizable;
};

} // namespace sh

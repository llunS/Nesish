#include "platform_window.hpp"

#include <cassert>

#include "glfw_app/glad/glad.h"

namespace ln_app {

PlatformWindow::PlatformWindow()
    : m_win(nullptr)
    , m_resizable(false)
{
}

bool
PlatformWindow::init(int i_width, int i_height, bool i_load_gl,
                     bool i_resizable, const char *i_name)
{
    // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);         // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true); // Required on Mac
#endif

    glfwWindowHint(GLFW_RESIZABLE, i_resizable);

    m_win = glfwCreateWindow(i_width, i_height, i_name, NULL, NULL);
    if (!m_win)
    {
        return false;
    }

    glfwMakeContextCurrent(m_win);
    if (i_load_gl)
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            return false;
        }
    }

    // we want to do the timing ourselves.
    glfwSwapInterval(0);

    m_resizable = i_resizable;
    if (!m_resizable)
    {
        updateViewportSize();
    }

    if (!post_init())
    {
        return false;
    }

    return true;
}

void
PlatformWindow::release()
{
    if (m_win)
    {
        glfwDestroyWindow(m_win);
    }
}

bool
PlatformWindow::shouldClose()
{
    assert(m_win);

    return glfwWindowShouldClose(m_win);
}

void
PlatformWindow::makeCurrent()
{
    if (m_win)
    {
        glfwMakeContextCurrent(m_win);
    }
}

void
PlatformWindow::updateViewportSize()
{
    int width, height;
    glfwGetFramebufferSize(m_win, &width, &height);
    glViewport(0, 0, width, height);
}

} // namespace ln_app

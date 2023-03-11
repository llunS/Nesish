#include "emulator_window.hpp"

#include <cassert>

#include "controller.hpp"
#include "glad/glad.h"

namespace sh {

static void
pv_strobe(int enabled, void *user)
{
    sh::Controller *ctrl = static_cast<sh::Controller *>(user);
    ctrl->strobe(enabled);
}
static int
pv_report(void *user)
{
    sh::Controller *ctrl = static_cast<sh::Controller *>(user);
    return ctrl->report();
}
static void
pv_reset(void *user)
{
    sh::Controller *ctrl = static_cast<sh::Controller *>(user);
    ctrl->reset();
}
#define ASM_CTRL(ctrl)                                                         \
    {                                                                          \
        ctrl.strobe = pv_strobe;                                               \
        ctrl.report = pv_report;                                               \
        ctrl.reset = pv_reset;                                                 \
    }

EmulatorWindow::EmulatorWindow()
    : m_console(NH_NULL)
    , m_renderer(nullptr)
{
    m_p1.user = nullptr;
    m_p2.user = nullptr;
}

void
EmulatorWindow::release()
{
    if (m_p1.user)
    {
        if (NH_VALID(m_console))
        {
            nh_unplug_ctrl(m_console, NH_CTRL_P1);
        }
        delete (sh::Controller *)(m_p1.user);
    }
    if (m_p2.user)
    {
        if (NH_VALID(m_console))
        {
            nh_unplug_ctrl(m_console, NH_CTRL_P2);
        }
        delete (sh::Controller *)(m_p2.user);
    }

    if (m_renderer)
    {
        glfwMakeContextCurrent(m_win); // need to call gl functions.
        delete m_renderer;
    }

    PlatformWindow::release();
}

bool
EmulatorWindow::init(NHConsole i_console, int i_width, int i_height,
                     bool i_load_gl, bool i_resizable, const char *i_name)
{
    if (!NH_VALID(i_console))
    {
        return false;
    }
    m_console = i_console;

    return init(i_width, i_height, i_load_gl, i_resizable, i_name);
}

bool
EmulatorWindow::post_init()
{
    assert(m_win);
    assert(NH_VALID(m_console));

    /* Setup emulator */
    m_p1.user = new sh::ControllerP1(m_win);
    ASM_CTRL(m_p1);
    m_p2.user = new sh::ControllerP2(m_win);
    ASM_CTRL(m_p2);

    nh_plug_ctrl(m_console, NH_CTRL_P1, &m_p1);
    nh_plug_ctrl(m_console, NH_CTRL_P2, &m_p2);

    nh_power_up(m_console);

    /* Setup renderer */
    glfwMakeContextCurrent(m_win); // need to call gl functions.

    m_renderer = new Renderer();
    if (m_renderer->setup())
    {
        return false;
    }

    return true;
}

void
EmulatorWindow::render()
{
    assert(m_win);

    makeCurrent();

    if (m_resizable)
    {
        updateViewportSize();
    }

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Render emulator output */
    NHFrame framebuf = nh_get_frm(m_console);
    m_renderer->render(framebuf);

    glfwSwapBuffers(m_win);
}

} // namespace sh

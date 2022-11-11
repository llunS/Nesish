#include "emulator_window.hpp"

#include <cassert>

#include "glfw_app/controller.hpp"
#include "glfw_app/glad/glad.h"

#include "common/logger.hpp"

namespace ln_app {

EmulatorWindow::EmulatorWindow()
    : m_p1(nullptr)
    , m_p2(nullptr)
    , m_renderer(nullptr)
{
}

void
EmulatorWindow::release()
{
    if (m_p1)
    {
        m_emu.unplug_controller(ln::CTRL_P1);
        delete m_p1;
    }
    if (m_p2)
    {
        m_emu.unplug_controller(ln::CTRL_P2);
        delete m_p2;
    }

    if (m_renderer)
    {
        glfwMakeContextCurrent(m_win); // need to call gl functions.
        delete m_renderer;
    }

    PlatformWindow::release();
}

bool
EmulatorWindow::post_init()
{
    assert(m_win);

    /* Setup emulator */
    m_p1 = new ln_app::ControllerP1(m_win);
    m_p2 = new ln_app::ControllerP2(m_win);

    m_emu.plug_controller(ln::CTRL_P1, m_p1);
    m_emu.plug_controller(ln::CTRL_P2, m_p2);

    m_emu.power_up();

    /* Setup renderer */
    glfwMakeContextCurrent(m_win); // need to call gl functions.

    m_renderer = new Renderer();
    if (LN_FAILED(m_renderer->setup()))
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
    auto frame_buf = m_emu.frame_dirty();
    if (frame_buf)
    {
        /* swap */
        frame_buf->swap(m_front_buffer);

        // Draw emulator ouput.
        m_renderer->render_direct(m_front_buffer);
    }

    glfwSwapBuffers(m_win);
}

bool
EmulatorWindow::insert_cart(const std::string i_rom_path)
{
    auto ln_err = m_emu.insert_cartridge(i_rom_path);
    if (LN_FAILED(ln_err))
    {
        LN_LOG_INFO(ln::get_logger(), "Failed to load cartridge: {}", ln_err);
        return false;
    }
    return true;
}

void
EmulatorWindow::advance(double i_delta)
{
    m_emu.advance(i_delta);
}

} // namespace ln_app

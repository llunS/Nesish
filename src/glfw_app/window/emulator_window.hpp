#pragma once

#include <string>

#include "common/klass.hpp"

#include "glfw_app/window/platform_window.hpp"

#include "glfw_app/rendering/renderer.hpp"
#include "console/ppu/frame_buffer.hpp"

namespace ln {
struct Emulator;
struct Controller;
} // namespace ln

namespace ln_app {

struct EmulatorWindow : public PlatformWindow {
  public:
    EmulatorWindow();
    void
    release() override;

    LN_KLZ_DELETE_COPY_MOVE(EmulatorWindow);

  private:
    using PlatformWindow::init;

  public:
    bool
    init(ln::Emulator *i_emu, int i_width, int i_height, bool i_load_gl,
         bool i_resizable = false, const char *i_name = nullptr);

  public:
    void
    render();

  protected:
    bool
    post_init() override;

  public:
    bool
    insert_cart(const std::string i_rom_path);

  private:
    ln::Emulator *m_emu;
    ln::Controller *m_p1;
    ln::Controller *m_p2;

    Renderer *m_renderer;
    ln::FrameBuffer m_front_buffer;
};

} // namespace ln_app

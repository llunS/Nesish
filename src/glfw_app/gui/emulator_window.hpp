#pragma once

#include <string>

#include "common/klass.hpp"

#include "glfw_app/gui/platform_window.hpp"

#include "glfw_app/rendering/renderer.hpp"

namespace nh {
struct Emulator;
struct Controller;
} // namespace nh

namespace sh {

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
    init(nh::Emulator *i_emu, int i_width, int i_height, bool i_load_gl,
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
    nh::Emulator *m_emu;
    nh::Controller *m_p1;
    nh::Controller *m_p2;

    Renderer *m_renderer;
};

} // namespace sh

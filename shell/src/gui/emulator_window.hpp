#pragma once

#include <string>

#include "nesish/nesish.h"
#include "nhbase/klass.hpp"

#include "gui/platform_window.hpp"

#include "rendering/renderer.hpp"

namespace sh {

struct EmulatorWindow : public PlatformWindow {
  public:
    EmulatorWindow();
    void
    release() override;

    NB_KLZ_DELETE_COPY_MOVE(EmulatorWindow);

  private:
    using PlatformWindow::init;

  public:
    bool
    init(NHConsole i_console, int i_width, int i_height, bool i_load_gl,
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
    NHConsole m_console;
    NHController m_p1;
    NHController m_p2;

    Renderer *m_renderer;
};

} // namespace sh

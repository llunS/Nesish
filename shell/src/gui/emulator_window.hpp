#pragma once

#include <string>

#include "nesish/nesish.h"
#include "nhbase/klass.hpp"

#include "gui/platform_window.hpp"
#include "rendering/renderer.hpp"
#include "input/controller.hpp"

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
    void
    init_console(const KeyMapping &p1_config, const KeyMapping &p2_config);

  private:
    NHConsole m_console;
    NHController m_p1;
    NHController m_p2;

    Renderer *m_renderer;
};

} // namespace sh

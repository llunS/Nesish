#pragma once

#include "glfw_app/window/platform_window.hpp"

#include "imgui.h"

namespace ln_app {

struct DebuggerWindow : public PlatformWindow {
  public:
    DebuggerWindow();
    void
    release() override;

  public:
    void
    render() override;

  protected:
    bool
    post_init() override;

    void
    makeCurrent() override;

  private:
    ImGuiContext *m_imgui_ctx;
};

} // namespace ln_app

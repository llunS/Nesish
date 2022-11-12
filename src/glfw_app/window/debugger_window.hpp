#pragma once

#include "glfw_app/window/platform_window.hpp"

#include "common/klass.hpp"

#include "imgui.h"

#include "glfw_app/rendering/texture.hpp"

namespace ln {
struct Emulator;
} // namespace ln

namespace ln_app {

struct DebuggerWindow : public PlatformWindow {
  public:
    DebuggerWindow();
    void
    release() override;

    LN_KLZ_DELETE_COPY_MOVE(DebuggerWindow);

  public:
    void
    render(const ln::Emulator &i_emu);

  protected:
    bool
    post_init() override;

    void
    makeCurrent() override;

  private:
    ImGuiContext *m_imgui_ctx;

  private:
    Texture m_emu_frame;
};

} // namespace ln_app

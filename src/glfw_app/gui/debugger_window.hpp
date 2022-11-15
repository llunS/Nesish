#pragma once

#include "glfw_app/gui/platform_window.hpp"

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
    pre_render(ln::Emulator &io_emu);
    void
    render(ln::Emulator &io_emu);

    bool
    isPaused() const;

  protected:
    bool
    post_init() override;

    void
    makeCurrent() override;

  private:
    ImGuiContext *m_imgui_ctx;

  private:
    bool m_paused;
    Texture m_frame_tex;
    Texture m_sp_tex[64];
};

} // namespace ln_app

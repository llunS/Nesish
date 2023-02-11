#pragma once

#include "glfw_app/gui/platform_window.hpp"

#include "common/klass.hpp"

#include "imgui.h"

#include "glfw_app/rendering/texture.hpp"

namespace ln {
struct Emulator;
} // namespace ln

namespace ln_app {
struct Rect;
} // namespace ln_app

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
    void
    post_render(ln::Emulator &io_emu);

    bool
    isPaused() const;
    bool
    shouldQuit() const;

  protected:
    bool
    post_init() override;

    void
    makeCurrent() override;

  private:
    ImGuiContext *m_imgui_ctx;

  private:
    void
    draw_control(const Rect &i_layout, ln::Emulator &io_emu);

    void
    draw_frame_debugger(const Rect &i_layout, ln::Emulator &io_emu);
    void
    draw_fd_pattern(const ln::Emulator &i_emu);
    void
    draw_fd_frame(const ln::Emulator &i_emu);
    void
    draw_fd_palette(const ln::Emulator &i_emu);
    void
    draw_fd_oam(const ln::Emulator &i_emu);

  private:
    bool m_paused;
    bool m_should_quit;

    Texture m_frame_tex;
    Texture m_sp_tex[64];
    Texture m_ptn_tbl_texs[2];

    ln::Emulator::PaletteSet m_ptn_tbl_palette;
};

} // namespace ln_app

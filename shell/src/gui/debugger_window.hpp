#pragma once

#include "gui/platform_window.hpp"

#include "nesish/nesish.h"
#include "nhbase/klass.hpp"

#include "imgui.h"

#include "rendering/texture.hpp"

namespace sh {
struct Rect;
} // namespace sh

namespace sh {

struct DebuggerWindow : public PlatformWindow {
  public:
    DebuggerWindow();
    void
    release() override;

    NB_KLZ_DELETE_COPY_MOVE(DebuggerWindow);

  public:
    void
    pre_render(NHConsole io_emu);
    void
    render(NHConsole io_emu);
    void
    post_render(NHConsole io_emu);

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
    draw_control(const Rect &i_layout, NHConsole io_emu);

    void
    draw_frame_debugger(const Rect &i_layout, NHConsole io_emu);
    void
    draw_fd_pattern(NHConsole i_emu);
    void
    draw_fd_frame(NHConsole i_emu);
    void
    draw_fd_palette(NHConsole i_emu);
    void
    draw_fd_oam(NHConsole i_emu);

  private:
    bool m_paused;
    bool m_should_quit;

    Texture m_frame_tex;
    Texture m_sp_tex[64];
    Texture m_ptn_tbl_texs[2];

    NHDPaletteSet m_ptn_tbl_palette;
};

} // namespace sh

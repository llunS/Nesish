#pragma once

#include "gui/window.hpp"

#include "rendering/texture.hpp"

namespace sh {

struct PPUDebugger : public Window {
  public:
    PPUDebugger(const std::string &i_name, NHConsole io_emu,
                Messager *i_messager);
    ~PPUDebugger();

  public:
    void
    render() override;

  private:
    void
    draw_pattern();
    void
    draw_palette();
    void
    draw_oam();

  private:
    Texture m_frame_tex;
    Texture m_sp_tex[64];
    Texture m_ptn_tbl_texs[2];

    NHDPaletteSet m_ptn_tbl_palette;
};

} // namespace sh

#pragma once

#include <string>

// @FIXME: spdlog will include windows header files, we need to include them
// before "glfw3.h" so that glfw won't redefine symbols.
#include "console/emulator.hpp"
#include "glfw_app/window/platform_window.hpp"

#include "glfw_app/rendering/renderer.hpp"
#include "console/ppu/frame_buffer.hpp"

namespace ln_app {

struct EmulatorWindow : public PlatformWindow {
  public:
    EmulatorWindow();
    void
    release() override;

  public:
    void
    render() override;

  protected:
    bool
    post_init() override;

  public:
    bool
    insert_cart(const std::string i_rom_path);
    void
    advance(double i_delta);

  private:
    ln::Emulator m_emu;

    ln::Controller *m_p1;
    ln::Controller *m_p2;

    Renderer *m_renderer;
    ln::FrameBuffer m_front_buffer;
};

} // namespace ln_app

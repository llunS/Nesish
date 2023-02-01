// @FIXME: spdlog will include windows header files, we need to include them
// before "glfw3.h" so that glfw won't redefine symbols.
#include "console/emulator.hpp"
#include "debugger_window.hpp"

#include <cassert>

#include "glfw_app/glad/glad.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glfw_app/gui/rect_cut.hpp"

#include <string>

namespace ln_app {

static void
HelpMarker(const char *desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

} // namespace ln_app

namespace ln_app {

DebuggerWindow::DebuggerWindow()
    : m_imgui_ctx(nullptr)
    , m_paused(false)
    , m_should_quit(false)
    , m_sp_tex{}
{
}

void
DebuggerWindow::release()
{
    if (m_imgui_ctx)
    {
        ImGui::SetCurrentContext(m_imgui_ctx);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        ImGui::DestroyContext(m_imgui_ctx);
    }

    PlatformWindow::release();
}

bool
DebuggerWindow::post_init()
{
    assert(m_win);

    /* imgui setup */
    glfwMakeContextCurrent(m_win); // need to call gl functions (maybe).

    IMGUI_CHECKVERSION();
    m_imgui_ctx = ImGui::CreateContext();
    if (!m_imgui_ctx)
    {
        return false;
    }

    ImGui::SetCurrentContext(m_imgui_ctx);
    ImGui_ImplGlfw_InitForOpenGL(m_win, true);
    const char *glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);

    return true;
}

void
DebuggerWindow::makeCurrent()
{
    PlatformWindow::makeCurrent();

    if (m_imgui_ctx)
    {
        ImGui::SetCurrentContext(m_imgui_ctx);
    }
}

void
DebuggerWindow::pre_render(ln::Emulator &io_emu)
{
    io_emu.set_debug_on(lnd::DBG_PALETTE);
    io_emu.set_debug_on(lnd::DBG_OAM);
}

void
DebuggerWindow::post_render(ln::Emulator &io_emu)
{
    io_emu.set_debug_off(lnd::DBG_PALETTE);
    io_emu.set_debug_off(lnd::DBG_OAM);
}

void
DebuggerWindow::render(const ln::Emulator &i_emu)
{
    assert(m_win);

    makeCurrent();

    if (m_resizable)
    {
        updateViewportSize();
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    Rect layout_win = {viewport->WorkPos, viewport->WorkSize};

    /* Control */
    Rect layout_ctrl = cut_top(layout_win, 55);
    draw_control(layout_ctrl);
    /* Frame Debugger */
    Rect layout_fd = layout_win;
    draw_frame_debugger(layout_fd, i_emu);

    // End the Dear ImGui frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_win);
}

void
DebuggerWindow::draw_control(const Rect &i_layout)
{
    ImGui::SetNextWindowPos(i_layout.pos(), ImGuiCond_Once);
    ImGui::SetNextWindowSize(i_layout.size(), ImGuiCond_Once);
    if (ImGui::Begin("Control", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        if (m_paused)
        {
            if (ImGui::Button("Play"))
            {
                m_paused = !m_paused;
            }
        }
        else
        {
            if (ImGui::Button("Pause"))
            {
                m_paused = !m_paused;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Quit"))
        {
            m_should_quit = true;
        }
    }
    ImGui::End();
}

void
DebuggerWindow::draw_frame_debugger(const Rect &i_layout,
                                    const ln::Emulator &i_emu)
{
    ImGui::SetNextWindowPos(i_layout.pos(), ImGuiCond_Once);
    ImGui::SetNextWindowSize(i_layout.size(), ImGuiCond_Once);
    if (ImGui::Begin("Frame Debugger", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        draw_fd_frame(i_emu);
        draw_fd_oam(i_emu);
        draw_fd_palette(i_emu);
    }
    ImGui::End();
}

void
DebuggerWindow::draw_fd_frame(const ln::Emulator &i_emu)
{
    ImGui::PushID("<Frame>");

    ImGui::Text("<Frame>");
    const auto &framebuf = i_emu.get_frame();
    if (m_frame_tex.from_frame(framebuf))
    {
        ImGui::Image(
            (ImTextureID)(std::intptr_t)m_frame_tex.texture(),
            {float(m_frame_tex.get_width()), float(m_frame_tex.get_height())},
            {0, 0}, {1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1});
    }
    else
    {
        ImGui::Text("Failed to get frame");
    }

    ImGui::PopID();
}

void
DebuggerWindow::draw_fd_palette(const ln::Emulator &i_emu)
{
    ImGui::PushID("<Palette>");

    ImGui::Spacing();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("<Palette>");
    ImGui::SameLine();
    HelpMarker("The snapshot was took at the end of the rendering.");

    const auto &palette = i_emu.get_palette_dbg();
    static_assert(lnd::Palette::color_count() == 32, "Check fixed loop below");
    char const *const pa_rows[2] = {"Background", "Sprite"};
    float lock_x = 0.0;
    for (int r = 0; r < 2; ++r)
    {
        const char *row = pa_rows[r];

        ImGui::PushID(row);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(row);
        ImGui::SameLine(0.0f, 20.f);
        if (r <= 0)
        {
            lock_x = ImGui::GetCursorPosX();
        }
        else
        {
            ImGui::SetCursorPosX(lock_x);
        }
        for (int c = 0; c < 16; ++c)
        {
            int palette_idx = r * 16 + c;

            ImGui::PushID(c);

            auto rgb_to_imvec4 = [](lnd::Color i_clr) -> ImVec4 {
                return ImVec4(i_clr.r / 255.f, i_clr.g / 255.f, i_clr.b / 255.f,
                              1.0f);
            };
            lnd::Color color = palette.get_color(palette_idx);
            ImVec4 im_color = rgb_to_imvec4(color);

            if (c % 4 == 0)
            {
                std::string s = std::to_string(c / 4);
                ImGui::TextUnformatted(s.c_str());
                ImGui::SameLine();
            }

            ImGui::ColorButton("##color", im_color,
                               ImGuiColorEditFlags_NoBorder |
                                   ImGuiColorEditFlags_NoAlpha |
                                   ImGuiColorEditFlags_NoTooltip);
            if ((c + 1) % 4 != 0)
            {
                ImGui::SameLine(0.0f, 0.0f);
            }
            else
            {
                ImGui::SameLine(0.0f, 25.f);
            }

            // details tooltip
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::ColorButton("##color", im_color,
                                   ImGuiColorEditFlags_NoBorder |
                                       ImGuiColorEditFlags_NoAlpha |
                                       ImGuiColorEditFlags_NoTooltip,
                                   ImVec2(40, 40));
                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::Text("%u (0x%02X)", color.index, color.index);
                ImGui::Text("(%d, %d, %d)", color.r, color.g, color.b);
                ImGui::EndGroup();
                ImGui::EndTooltip();
            }

            ImGui::PopID();
        }
        ImGui::NewLine();
        ImGui::PopID();
    }

    ImGui::PopID();
}

void
DebuggerWindow::draw_fd_oam(const ln::Emulator &i_emu)
{
    ImGui::PushID("<OAM>");

    ImGui::Spacing();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("<OAM>");
    ImGui::SameLine();
    HelpMarker("The snapshot was took at the end of the rendering, and "
               "assumes OAMADDR starts with 0.");

    const auto &oam = i_emu.get_oam_dbg();
    static_assert(lnd::OAM::get_sprite_count() == 64, "Check fixed loop below");
    static_assert(sizeof(m_sp_tex) / sizeof(Texture) == 64,
                  "Check fixed loop below");
    ImGui::Spacing();
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            int k = i * 16 + j;

            const auto &sp = oam.get_sprite(k);
            if (m_sp_tex[k].from_sprite(sp))
            {
                constexpr float scale = 3.f;
                ImGui::Image((ImTextureID)(std::intptr_t)m_sp_tex[k].texture(),
                             {float(m_sp_tex[k].get_width() * scale),
                              float(m_sp_tex[k].get_height() * scale)},
                             {0, 0}, {1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1});

                // details tooltip
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();

                    constexpr float scale_details = 12.f;
                    ImGui::Image(
                        (ImTextureID)(std::intptr_t)m_sp_tex[k].texture(),
                        {float(m_sp_tex[k].get_width() * scale_details),
                         float(m_sp_tex[k].get_height() * scale_details)},
                        {0, 0}, {1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1});

                    ImGui::SameLine();
                    ImGui::BeginGroup();

                    ImGui::BeginGroup();
                    ImGui::Text("X: %u (0x%02X)", sp.x, sp.x);
                    ImGui::Text("Y: %u (0x%02X)", sp.y, sp.y);
                    ImGui::EndGroup();

                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::Text("Tile: %u (0x%02X)", sp.tile, sp.tile);
                    ImGui::Text("Attr: %u (0x%02X)", sp.attr, sp.attr);
                    ImGui::EndGroup();

                    ImGui::Text("SP Palette: %u", sp.palette_set());
                    ImGui::Text("Background: %u", sp.background());
                    ImGui::Text("Flip X: %u", sp.flip_x());
                    ImGui::Text("Flip Y: %u", sp.flip_y());

                    ImGui::EndGroup();

                    ImGui::EndTooltip();
                }
            }
            else
            {
                ImGui::Text("[X]");
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();
    }

    ImGui::PopID();
}

bool
DebuggerWindow::isPaused() const
{
    return m_paused;
}

bool
DebuggerWindow::shouldQuit() const
{
    return m_should_quit;
}

} // namespace ln_app

#include "debugger_window.hpp"

#include <cassert>

#include "glfw_app/glad/glad.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "console/emulator.hpp"

#include "glfw_app/gui/rect_cut.hpp"

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
DebuggerWindow::render(ln::Emulator &io_emu)
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
    ImGui::SetNextWindowPos(layout_ctrl.pos(), ImGuiCond_Once);
    ImGui::SetNextWindowSize(layout_ctrl.size(), ImGuiCond_Once);
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
    }
    ImGui::End();

    /* Frame Debugger */
    Rect layout_fd = layout_win;
    ImGui::SetNextWindowPos(layout_fd.pos(), ImGuiCond_Once);
    ImGui::SetNextWindowSize(layout_fd.size(), ImGuiCond_Once);
    if (ImGui::Begin("Frame Debugger", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        /* Render current frame */
        ImGui::PushID("<Frame>");
        ImGui::Text("<Frame>");
        const auto &framebuf = io_emu.get_frame();
        if (m_frame_tex.from_frame(framebuf))
        {
            ImGui::Image((ImTextureID)(std::intptr_t)m_frame_tex.texture(),
                         {float(m_frame_tex.get_width()),
                          float(m_frame_tex.get_height())},
                         {0, 0}, {1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1});
        }
        else
        {
            ImGui::Text("Failed to get frame");
        }
        ImGui::PopID();

        /* Palette */
        ImGui::PushID("<Palette>");
        ImGui::Spacing();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("<Palette>");
        ImGui::SameLine();
        HelpMarker("The snapshot was took at the end of the rendering.");

        auto rgb_to_imvec4 = [](ln::Color i_clr) -> ImVec4 {
            return ImVec4(i_clr.r / 255.f, i_clr.g / 255.f, i_clr.b / 255.f,
                          1.0f);
        };

        static_assert(lnd::Palette::color_count() == 32, "Rework code below.");
        const auto &palette = io_emu.get_palette_dbg();

        // Background
        ImGui::PushID("Background");
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Background");
        ImGui::SameLine(0.0f, 20.f);
        float lock_x = ImGui::GetCursorPosX();
        for (int i = 0; i < 16; ++i)
        {
            ImGui::PushID(i);

            ImGui::ColorButton("##color", rgb_to_imvec4(palette.get_color(i)),
                               ImGuiColorEditFlags_NoBorder |
                                   ImGuiColorEditFlags_NoAlpha);
            if ((i + 1) % 4 != 0)
            {
                ImGui::SameLine(0.0f, 0.0f);
            }
            else
            {
                ImGui::SameLine(0.0f, 25.f);
            }

            ImGui::PopID();
        }
        ImGui::NewLine();
        ImGui::PopID();

        // Sprite
        ImGui::PushID("Sprite");
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Sprite");
        ImGui::SameLine();
        ImGui::SetCursorPosX(lock_x);
        for (int i = 16; i < 32; ++i)
        {
            ImGui::PushID(i);

            ImGui::ColorButton("##color", rgb_to_imvec4(palette.get_color(i)),
                               ImGuiColorEditFlags_NoBorder |
                                   ImGuiColorEditFlags_NoAlpha);
            if ((i + 1) % 4 != 0)
            {
                ImGui::SameLine(0.0f, 0.0f);
            }
            else
            {
                ImGui::SameLine(0.0f, 25.f);
            }

            ImGui::PopID();
        }
        ImGui::NewLine();
        ImGui::PopID();

        ImGui::PopID();

        /* OAM */
        ImGui::PushID("<OAM>");
        ImGui::Spacing();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("<OAM>");
        ImGui::SameLine();
        HelpMarker("The snapshot was took at the end of the rendering, and "
                   "assumes OAMADDR starts with 0.");

        const auto &oam = io_emu.get_oam_dbg();
        static_assert(lnd::OAM::get_sprite_count() == 64, "Rework code below.");
        static_assert(sizeof(m_sp_tex) / sizeof(Texture) == 64,
                      "Rework code below.");
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
                    ImGui::Image(
                        (ImTextureID)(std::intptr_t)m_sp_tex[k].texture(),
                        {float(m_sp_tex[k].get_width() * scale),
                         float(m_sp_tex[k].get_height() * scale)});
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
    ImGui::End();

    // End the Dear ImGui frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_win);
}

bool
DebuggerWindow::isPaused() const
{
    return m_paused;
}

} // namespace ln_app

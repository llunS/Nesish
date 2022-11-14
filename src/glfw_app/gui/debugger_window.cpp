#include "debugger_window.hpp"

#include <cassert>

#include "glfw_app/glad/glad.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "console/emulator.hpp"

#include "glfw_app/gui/rect_cut.hpp"

namespace ln_app {

DebuggerWindow::DebuggerWindow()
    : m_imgui_ctx(nullptr)
    , m_paused(false)
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
        ImGui::Text("<Frame>");
        const auto &framebuf = i_emu.get_frame();
        if (m_emu_frame.from_frame(framebuf))
        {
            ImGui::Image((ImTextureID)(std::intptr_t)m_emu_frame.texture(),
                         {float(m_emu_frame.get_width()),
                          float(m_emu_frame.get_height())},
                         {0, 0}, {1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1});
        }
        else
        {
            ImGui::Text("Failed to get frame");
        }

        static_assert(ln::Emulator::palette_color_count() == 32,
                      "Rework code below.");
        /* Palette */
        ImGui::Spacing();
        ImGui::Text("<Palette>");

        auto rgb_to_imvec4 = [](ln::Color i_clr) -> ImVec4 {
            return ImVec4(i_clr.r / 255.f, i_clr.g / 255.f, i_clr.b / 255.f,
                          1.0f);
        };

        // Background
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Background");
        ImGui::SameLine(0.0f, 20.f);
        float lock_x = ImGui::GetCursorPosX();
        for (int i = 0; i < 16; ++i)
        {
            ImGui::ColorButton("", rgb_to_imvec4(i_emu.get_palette_color(i)),
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
        }
        ImGui::NewLine();

        // Sprite
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Sprite");
        ImGui::SameLine();
        ImGui::SetCursorPosX(lock_x);
        for (int i = 16; i < 32; ++i)
        {
            ImGui::ColorButton("", rgb_to_imvec4(i_emu.get_palette_color(i)),
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
        }
        ImGui::NewLine();
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

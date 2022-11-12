#include "debugger_window.hpp"

#include <cassert>

#include "glfw_app/glad/glad.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "console/emulator.hpp"

namespace ln_app {

DebuggerWindow::DebuggerWindow()
    : m_imgui_ctx(nullptr)
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
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(300, 300));
    if (ImGui::Begin("Frame debugger", nullptr, ImGuiWindowFlags_NoResize))
    {
        /* Render current frame */
        ImGui::Text("Frame");
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
            ImGui::Text("[Empty]");
        }
    }
    ImGui::End();

    // End the Dear ImGui frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_win);
}

} // namespace ln_app

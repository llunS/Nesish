#include "app.hpp"

#include <cstdio>
#include <cstdint>
#include <memory>

// @FIXME: spdlog will include windows header files, we need to include them
// before "glfw3.h"
#include "console/emulator.hpp"

#include "glfw_app/controller.hpp"
#include "glfw_app/rendering/renderer.hpp"

#include "glfw/glfw3.h"
#include "glfw_app/glad/glad.h"

#include "imconfig.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "common/time.hpp"

#include "console/spec.hpp"
#include "console/ppu/frame_buffer.hpp"

namespace ln_app {

static constexpr double FRAME_TIME = 1.0 / 60.0;

static void
error_callback(int error, const char *description);

int
App::run(const std::string &i_rom_path)
{
    int err = 0;

    /* Insert cartridge */
    auto emulator = std::unique_ptr<ln::Emulator>(new ln::Emulator());
    auto ln_err = emulator->insert_cartridge(i_rom_path);
    if (LN_FAILED(ln_err))
    {
        LN_LOG_INFO(ln::get_logger(), "Failed to load cartridge: {}", ln_err);
        return 1;
    }

    /* App and OpenGL setup */
    GLFWwindow *window = NULL;
    const char *glsl_version = "#version 330";
    {
        glfwSetErrorCallback(error_callback);

        if (!glfwInit())
        {
            return 1;
        }

        // OpenGL 3.3
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_PROFILE,
                       GLFW_OPENGL_CORE_PROFILE);            // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#endif
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        if (!monitor)
        {
            return 1;
        }

        // fullscreen
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        window =
            glfwCreateWindow(mode->width, mode->height, "LightNES", NULL, NULL);
        if (!window)
        {
            err = 1;
            goto l_cleanup_glfw;
        }
        glfwMakeContextCurrent(window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

        // we want to do the timing ourselves.
        glfwSwapInterval(0);
    }

    /* Main loop */
    {
        /* Setup app state */
        ln::FrameBuffer front_buffer;

        /* Setup emulator */
        emulator->plug_controller(ln::Emulator::P1,
                                  new ln_app::Controller(window));
        emulator->power_up();

        /* Setup renderer */
        Renderer renderer;
        if (LN_FAILED(renderer.setup()))
        {
            err = 1;
            goto l_cleanup_glfw;
        }

        /* imgui setup */
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
        }
        {
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init(glsl_version);
        }

        /* Event Loop */
        constexpr double S_TO_MS = 1000.0;
        constexpr double S_TO_US = S_TO_MS * S_TO_MS;
        constexpr double US_TO_MS = 1.0 / 1000.0;
        constexpr double FRAME_TIME_US = FRAME_TIME * S_TO_US;

        double currTimeUS = ln::get_now_micro();
        double prevSimTimeUS = currTimeUS;
        double nextRenderTimeUS = currTimeUS + FRAME_TIME_US;
        while (!glfwWindowShouldClose(window))
        {
            // Dispatch events
            glfwPollEvents();

            /* Simulate */
            currTimeUS = ln::get_now_micro();
            auto deltaTimeMS = (currTimeUS - prevSimTimeUS) * US_TO_MS;
            emulator->advance(deltaTimeMS);
            prevSimTimeUS = currTimeUS;

            /* Render, at most at fixed rate */
            if (currTimeUS >= nextRenderTimeUS)
            {
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);
                glViewport(0, 0, width, height);

                glClearColor(0.0f, float(0x64) / 0xff, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                // Start the Dear ImGui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                {
                    if (ImGui::Begin("Console", NULL,
                                     ImGuiWindowFlags_NoResize |
                                         ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        ImGui::Text("Welcome to LightNES!");
                    }
                    ImGui::End();

                    /* Render emulator output */
                    if (ImGui::Begin("Emulator", NULL,
                                     ImGuiWindowFlags_NoResize |
                                         ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        auto frame_buf = emulator->frame_dirty();
                        if (frame_buf)
                        {
                            /* swap */
                            frame_buf->swap(front_buffer);

                            // Draw emulator ouput.
                            renderer.render(front_buffer);

                            ImGui::Image(
                                (ImTextureID)(std::intptr_t)renderer.texture(),
                                {float(renderer.get_width()),
                                 float(renderer.get_height())},
                                {0, 1}, {1, 0}, {1, 1, 1, 1}, {1, 1, 1, 1});
                        }
                    }
                    ImGui::End();
                }

                // End the Dear ImGui frame
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                glfwSwapBuffers(window);

                nextRenderTimeUS = currTimeUS + FRAME_TIME_US;
            }
        }
    }

    /* l_cleanup_imgui: */
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
l_cleanup_glfw:
    /* l_cleanup_glfw: */
    {
        if (window)
            glfwDestroyWindow(window);
        window = NULL;

        glfwTerminate();
    }

    return err;
}

void
error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %d, %s\n", error, description);
}

} // namespace ln_app

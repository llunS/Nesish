#include "app.hpp"

#include "console/emulator.hpp"
#include "console/spec.hpp"

#include "glfw_app/controller.hpp"
#include "glfw_app/rendering/renderer.hpp"

#include "glfw/glfw3.h"
#include "glfw_app/glad/glad.h"

#include "imconfig.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cstdio>
#include <cstdint>

namespace ln_app {

static void
error_callback(int error, const char *description);

int
App::run()
{
    /* App and OpenGL setup */
    //{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        return 1;
    }

    // OpenGL 3.3
    const char *glsl_version = "#version 330";
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

    int err = 0;

    // fullscreen
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    GLFWwindow *window =
        glfwCreateWindow(mode->width, mode->height, "LightNES", NULL, NULL);
    if (!window)
    {
        err = 1;
        goto l_cleanup;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // we want to do the timing ourselves.
    glfwSwapInterval(0);
    //}

    /* App logic */
    {
        /* Setup renderer */
        Renderer renderer;
        if (LN_FAILED(renderer.setup()))
        {
            err = 1;
            goto l_cleanup;
        }

        /* Setup emulator */
        ln::Emulator emulator;
        emulator.plug_controller(ln::Emulator::P1,
                                 new ln_app::Controller(window));
        emulator.power_up();

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
        while (!glfwWindowShouldClose(window))
        {
            // Dispatch events
            glfwPollEvents();

            // Render
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

                    if (ImGui::Begin("Emulator", NULL,
                                     ImGuiWindowFlags_NoResize |
                                         ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        // Draw our content.
                        renderer.draw();

                        ImGui::Image(
                            (ImTextureID)(std::intptr_t)renderer.texture(),
                            {float(renderer.get_width()),
                             float(renderer.get_height())},
                            {0, 1}, {1, 0}, {1, 1, 1, 1}, {1, 1, 1, 1});
                    }
                    ImGui::End();
                }

                // End the Dear ImGui frame
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                glfwSwapBuffers(window);
            }
        }
    }

l_cleanup:
    if (true) // for format only
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
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

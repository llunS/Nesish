#include "app.hpp"

#include <cstdio>
#include <memory>

#include "glfw/glfw3.h"

#include "glfw_app/window/emulator_window.hpp"
#include "glfw_app/window/debugger_window.hpp"

#include "console/spec.hpp"

namespace ln_app {

static constexpr double FRAME_TIME = 1.0 / 60.0;

static void
error_callback(int error, const char *description);

int
App::run(const std::string &i_rom_path)
{
    std::unique_ptr<EmulatorWindow> emulatorWin{new EmulatorWindow()};
    /* Insert cartridge */
    if (!emulatorWin->insert_cart(i_rom_path))
    {
        return 1;
    }

    /* init glfw */
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        return 1;
    }

    int err = 0;
    std::unique_ptr<DebuggerWindow> debuggerWin{new DebuggerWindow()};

    /* init windows */
    if (!debuggerWin->init(640, 480, true, false, "Debugger"))
    {
        err = 1;
        goto l_end;
    }
    if (!emulatorWin->init(LN_NES_WIDTH * 2, LN_NES_HEIGHT * 2, false, false,
                           "Emulator"))
    {
        err = 1;
        goto l_end;
    }

    /* Main loop */
    {
        constexpr double S_TO_MS = 1000.0;
        constexpr double S_TO_US = S_TO_MS * S_TO_MS;
        constexpr double US_TO_MS = 1.0 / 1000.0;
        constexpr double FRAME_TIME_US = FRAME_TIME * S_TO_US;

        double currTimeUS = ln::get_now_micro();
        double prevSimTimeUS = currTimeUS;
        double nextRenderTimeUS = currTimeUS + FRAME_TIME_US;

        while (emulatorWin)
        {
            // Dispatch events
            glfwPollEvents();

            if (emulatorWin && emulatorWin->shouldClose())
            {
                emulatorWin->release();
                emulatorWin.reset();
            }
            if (debuggerWin && debuggerWin->shouldClose())
            {
                debuggerWin->release();
                debuggerWin.reset();
            }

            /* Simulate */
            currTimeUS = ln::get_now_micro();
            auto deltaTimeMS = (currTimeUS - prevSimTimeUS) * US_TO_MS;
            prevSimTimeUS = currTimeUS;
            if (emulatorWin)
            {
                emulatorWin->advance(deltaTimeMS);
            }

            /* Render, at most at fixed rate */
            if (currTimeUS >= nextRenderTimeUS)
            {
                if (emulatorWin)
                {
                    emulatorWin->render();
                }
                if (debuggerWin)
                {
                    debuggerWin->render();
                }

                nextRenderTimeUS = currTimeUS + FRAME_TIME_US;
            }
        }
    }

l_end:
    // before glfw termination
    if (emulatorWin)
    {
        emulatorWin->release();
        emulatorWin.reset();
    }
    if (debuggerWin)
    {
        debuggerWin->release();
        debuggerWin.reset();
    }
    glfwTerminate();

    return err;
}

void
error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %d, %s\n", error, description);
}

} // namespace ln_app

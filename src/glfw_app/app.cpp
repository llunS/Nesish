#include "app.hpp"

#include <cstdio>
#include <memory>

#include "glfw/glfw3.h"

#include "glfw_app/window/emulator_window.hpp"
#include "glfw_app/window/debugger_window.hpp"

#include "console/spec.hpp"
#include "console/emulator.hpp"

namespace ln_app {

static constexpr double FRAME_TIME = 1.0 / 60.0;

static void
error_callback(int error, const char *description);

int
App::run(const std::string &i_rom_path)
{
    ln::Emulator emulator{};
    auto ln_err = ln::Error::OK;

    /* Insert cartridge */
    ln_err = emulator.insert_cartridge(i_rom_path);
    if (LN_FAILED(ln_err))
    {
        LN_LOG_INFO(ln::get_logger(), "Failed to load cartridge: {}", ln_err);
        return 1;
    }

    /* init glfw */
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        return 1;
    }

    int err = 0;
    std::unique_ptr<EmulatorWindow> emulatorWin{new EmulatorWindow()};
    std::unique_ptr<DebuggerWindow> debuggerWin{new DebuggerWindow()};

    /* init windows */
    if (!debuggerWin->init(640, 480, true, false, "Debugger"))
    {
        err = 1;
        goto l_end;
    }
    debuggerWin->set_pos(50 + LN_NES_WIDTH * 2 + 50, 150);
    if (!emulatorWin->init(&emulator, LN_NES_WIDTH * 2, LN_NES_HEIGHT * 2,
                           false, false, "Emulator"))
    {
        err = 1;
        goto l_end;
    }
    emulatorWin->set_pos(50, 150);

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

            /* Close window if necessary */
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

            /* Calculate elapsed time */
            currTimeUS = ln::get_now_micro();
            auto deltaTimeMS = (currTimeUS - prevSimTimeUS) * US_TO_MS;
            prevSimTimeUS = currTimeUS;
            /* Simulate */
            emulator.advance(deltaTimeMS);

            /* Render, at most at fixed rate */
            if (currTimeUS >= nextRenderTimeUS)
            {
                if (emulatorWin)
                {
                    emulatorWin->render();
                }
                if (debuggerWin)
                {
                    debuggerWin->render(emulator);
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

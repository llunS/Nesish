#include "app.hpp"

#include <cstdio>
#include <memory>

// @FIXME: spdlog will include windows header files, we need to include them
// before "glfw3.h" so that glfw won't redefine symbols.
#include "console/emulator.hpp"
#include "glfw/glfw3.h"
#include "glfw_app/gui/emulator_window.hpp"
#include "glfw_app/gui/debugger_window.hpp"

#include "console/spec.hpp"

#include "glfw_app/audio/blip_buf_adapter.hpp"
#include "common/path.hpp"
#include "glfw_app/audio/pcm_writer.hpp"

#define LN_APP_AUDIO_BUF_SIZE 64
#define LN_APP_AUDIO_SAMPLE_RATE 48000

namespace ln_app {

static constexpr double FRAME_TIME = 1.0 / 60.0;

static void
error_callback(int error, const char *description);

int
run_app(const std::string &i_rom_path, AppOpt i_opts)
{
    std::unique_ptr<ln::Emulator> emulator{new ln::Emulator()};
    auto ln_err = ln::Error::OK;

    /* Insert cartridge */
    ln_err = emulator->insert_cartridge(i_rom_path);
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
    BlipBufAdapter blip{emulator->get_sample_rate(), LN_APP_AUDIO_SAMPLE_RATE,
                        LN_APP_AUDIO_BUF_SIZE};
    PCMWriter pcm_writer{};
    if (i_opts & ln_app::OPT_PCM)
    {
        if (pcm_writer.open(ln::join_exec_rel_path("audio.pcm")))
        {
            err = 1;
            goto l_end;
        }
    }

    /* init windows */
    if (!debuggerWin->init(640, 480, true, false, "Debugger"))
    {
        err = 1;
        goto l_end;
    }
    debuggerWin->set_pos(50 + LN_NES_WIDTH * 2 + 50, 150);
    if (!emulatorWin->init(emulator.get(), LN_NES_WIDTH * 2, LN_NES_HEIGHT * 2,
                           false, false, "Emulator"))
    {
        err = 1;
        goto l_end;
    }
    emulatorWin->set_pos(50, 150);

    debuggerWin->pre_render(*emulator);

    /* Main loop */
    {
        constexpr double S_TO_US = 1000.0 * 1000.0;
        constexpr double US_TO_MS = 1.0 / 1000.0;
        constexpr double FRAME_TIME_US = FRAME_TIME * S_TO_US;

        double currTimeUS = ln::get_now_micro();
        double prevLoopTimeUS = currTimeUS;
        double nextRenderTimeUS = currTimeUS + FRAME_TIME_US;

        while (emulatorWin)
        {
            // Dispatch events
            glfwPollEvents();

            /* Close window if necessary */
            if (emulatorWin && (emulatorWin->shouldClose() ||
                                (debuggerWin && debuggerWin->shouldQuit())))
            {
                emulatorWin->release();
                emulatorWin.reset();
            }
            if (debuggerWin && debuggerWin->shouldClose())
            {
                debuggerWin->post_render(*emulator);
                debuggerWin->release();
                debuggerWin.reset();

                /* The release of the debugger window will add overhead to this
                 * loop, so we skip this one to make each emulation delta taking
                 * roughly the same time (or small enough) to smooth the
                 * emulation process */
                prevLoopTimeUS = ln::get_now_micro();
                continue;
            }

            /* Emulate */
            currTimeUS = ln::get_now_micro();
            auto deltaTimeMS = (currTimeUS - prevLoopTimeUS) * US_TO_MS;
            if (!debuggerWin || !debuggerWin->isPaused())
            {
                auto cycles = emulator->ticks(deltaTimeMS);
                for (decltype(cycles) i = 0; i < cycles; ++i)
                {
                    if (emulator->tick())
                    {
                        if (pcm_writer.is_open())
                        {
                            float sample = emulator->get_sample();
                            // @NOTE: Once clocked, samples must be drained to
                            // avoid buffer overflow.
                            // @FIXME: Which is correct? [0, 1] or [-1, 1]
                            blip.clock((sample * 2. - 1.) * 32767);
                            // blip.clock(sample * 32767);

                            // Drain the sample buffer
                            short buf[LN_APP_AUDIO_BUF_SIZE] = {};
                            while (
                                blip.samples_avail(buf, LN_APP_AUDIO_BUF_SIZE))
                            {
                                for (int j = 0; j < LN_APP_AUDIO_BUF_SIZE; ++j)
                                {
                                    pcm_writer.write_s16le(buf[j]);
                                }
                            }
                        }
                    }
                }
            }
            prevLoopTimeUS = currTimeUS;

            /* Render, at most at fixed rate */
            if (currTimeUS >= nextRenderTimeUS)
            {
                // Don't bother to render emulator if paused.
                // @IMPL: Handle emulator window first anyhow to reflect latest
                // visuals.
                if (!debuggerWin || !debuggerWin->isPaused())
                {
                    if (emulatorWin)
                    {
                        emulatorWin->render();
                    }
                }
                if (debuggerWin)
                {
                    debuggerWin->render(*emulator);
                }

                nextRenderTimeUS = currTimeUS + FRAME_TIME_US;
            }
        }
    }

l_end:
    if (pcm_writer.is_open())
    {
        short buf[LN_APP_AUDIO_BUF_SIZE] = {};
        int count = blip.flush_samples(buf, LN_APP_AUDIO_BUF_SIZE);
        for (int i = 0; i < count; ++i)
        {
            pcm_writer.write_s16le(buf[i]);
        }
    }
    pcm_writer.close();
    blip.close();

    // before glfw termination
    if (emulatorWin)
    {
        emulatorWin->release();
        emulatorWin.reset();
    }
    if (debuggerWin)
    {
        debuggerWin->post_render(*emulator);
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

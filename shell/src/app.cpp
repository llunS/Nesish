#include "app.hpp"

#include <cstdio>
#include <memory>
#include <chrono>
#ifdef SH_TGT_MACOS
#include <thread>
#endif

// @FIXME: spdlog will include windows header files, we need to include them
// before "glfw3.h" so that glfw won't redefine symbols.
#include "misc/logger.hpp"
#include "glfw/glfw3.h"
#include "gui/emulator_window.hpp"
#include "gui/debugger_window.hpp"

#include "nesish/nesish.h"
#include "nhbase/path.hpp"

#include "audio/resampler.hpp"
#include "audio/pcm_writer.hpp"
#include "audio/channel.hpp"
#include "audio/device.hpp"

#include "misc/config.hpp"
#include "input/controller.hpp"

#define DEBUG_AUDIO 0

#define FRAME_TIME (1.0 / 60.0)
#define AUDIO_SAMPLE_RATE 48000 // Most common rate for audio hardware
#define AUDIO_BUF_SIZE 512      // Close to 1 frame worth of buffer
// 800 = 1 / 60 * 48000, x2 for peak storage
#define AUDIO_CH_SIZE (800 * 2)

namespace sh {

#define SAMPLE_FORMAT RTAUDIO_SINT16
typedef short sample_t;
typedef Channel<sample_t, AUDIO_CH_SIZE> AudioBuffer;

struct AudioData {
    sample_t prev;
    AudioBuffer *buf;
    Logger *logger;
};

static void
error_callback(int error, const char *description);

static void
pv_log(NHLogLevel level, const char *msg, void *user);

static int
audio_playback(void *outputBuffer, void *inputBuffer,
               unsigned int nBufferFrames, double streamTime,
               RtAudioStreamStatus status, void *userData);

int
run(const std::string &i_rom_path, AppOpt i_opts, Logger *i_logger)
{
    KeyMapping p1_config;
    KeyMapping p2_config;
    if (!load_key_config(p1_config, p2_config, i_logger))
    {
        return 1;
    }

    NHLogger logger{pv_log, i_logger, i_logger->level};
    NHConsole console = nh_new_console(&logger);
    if (!NH_VALID(console))
    {
        return 1;
    }
    /* Insert cartridge */
    NHErr nh_err = nh_insert_cart(console, i_rom_path.c_str());
    if (NH_FAILED(nh_err))
    {
        SH_LOG_ERROR(i_logger, "Failed to load cartridge: {}", nh_err);
        nh_release_console(console);
        return 1;
    }

    /* init glfw */
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        nh_release_console(console);
        return 1;
    }

    int err = 0;

    std::unique_ptr<EmulatorWindow> emu_win{new EmulatorWindow()};
    std::unique_ptr<DebuggerWindow> dbg_win;
    if (i_opts & sh::OPT_DEBUG)
    {
        dbg_win.reset(new DebuggerWindow());
    }

    AudioBuffer audio_buf;
    AudioData audio_data{0, &audio_buf, i_logger};
    Resampler resampler{AUDIO_BUF_SIZE * 2}; // More than enough
    PCMWriter pcm_writer;

    /* init audio */
    // device
    if (!audio_init())
    {
        err = 1;
        goto l_end;
    }
    if (!audio_open(AUDIO_SAMPLE_RATE, AUDIO_BUF_SIZE, SAMPLE_FORMAT,
                    audio_playback, &audio_data))
    {
        err = 1;
        goto l_end;
    }
    // resampler
    if (!resampler.set_rates(double(nh_get_sample_rate(console)),
                             AUDIO_SAMPLE_RATE))
    {
        err = 1;
        goto l_end;
    }
    // pcm recorder
    if (i_opts & sh::OPT_PCM)
    {
        if (pcm_writer.open(nb::path_join_exe("audio.pcm")))
        {
            err = 1;
            goto l_end;
        }
    }

    /* init windows */
    if (dbg_win)
    {
        if (!dbg_win->init(580, 710, true, false, "Debugger"))
        {
            err = 1;
            goto l_end;
        }
        dbg_win->set_pos(50 + NH_NES_WIDTH * 2 + 50, 115);
    }
    if (!emu_win->init(console, NH_NES_WIDTH * 2, NH_NES_HEIGHT * 2, !dbg_win,
                       false, "Nesish"))
    {
        err = 1;
        goto l_end;
    }
    emu_win->set_pos(50, 170);
    emu_win->init_console(p1_config, p2_config);

    if (dbg_win)
    {
        dbg_win->pre_render(console);
    }

    /* start audio playback right before the loop */
    if (!audio_start())
    {
        err = 1;
        goto l_end;
    }
    /* Main loop */
    {
        auto currTime = std::chrono::steady_clock::now();
        auto nextLoopTime = currTime + std::chrono::duration<double>(0.0);
        while (emu_win)
        {
            currTime = std::chrono::steady_clock::now();
            if (currTime >= nextLoopTime)
            {
                /* Handle inputs, process events */
                glfwPollEvents();

                /* Close window if necessary */
                if (emu_win->shouldClose() ||
                    (dbg_win && dbg_win->shouldQuit()))
                {
                    emu_win->release();
                    emu_win.reset();
                    break;
                }
                if (dbg_win && dbg_win->shouldClose())
                {
                    dbg_win->post_render(console);
                    dbg_win->release();
                    dbg_win.reset();
                }

                /* Emulate */
                if (!(dbg_win && dbg_win->isPaused()))
                {
                    NHCycle ticks = nh_advance(console, FRAME_TIME);
                    for (decltype(ticks) i = 0; i < ticks; ++i)
                    {
                        if (nh_tick(console, nullptr))
                        {
                            double sample = nh_get_sample(console);
                            resampler.clock(short(sample * 32767));
                            // Once clocked, samples must be drained to avoid
                            // buffer overflow.
                            short buf[AUDIO_BUF_SIZE];
                            while (resampler.samples_avail(buf, AUDIO_BUF_SIZE))
                            {
                                for (decltype(AUDIO_BUF_SIZE) j = 0;
                                     j < AUDIO_BUF_SIZE; ++j)
                                {
                                    // If failed, sample gets dropped, but we
                                    // are free of inconsistent emulation due to
                                    // blocking delay
                                    if (!audio_buf.try_send(buf[j]))
                                    {
#if DEBUG_AUDIO
                                        SH_LOG_WARN(i_logger,
                                                    "Sample gets dropped");
#endif
                                    }

                                    if (pcm_writer.is_open())
                                    {
                                        pcm_writer.write_s16le(buf[j]);
                                    }
                                }
                            }
                        }
                    }
                }

                /* Render */
                // Don't bother to render emulator if paused.
                if (!(dbg_win && dbg_win->isPaused()))
                {
                    if (emu_win)
                    {
                        emu_win->render();
                    }
                }
                if (dbg_win)
                {
                    dbg_win->render(console);
                }

                nextLoopTime =
                    currTime + std::chrono::duration<double>(FRAME_TIME);

                // After testing, sleep implementation on MacOS (combined with
                // libc++) has higher resolution and reliability.
#ifdef SH_TGT_MACOS
                if (!(i_opts & sh::OPT_NOSLEEP))
                {
                    std::this_thread::sleep_until(nextLoopTime);
                }
#endif
            }
        }
    }

l_end:
    (void)pcm_writer.close();
    resampler.close();
    (void)audio_stop();
    audio_close();
    audio_uninit();

    // before glfw termination
    if (emu_win)
    {
        emu_win->release();
        emu_win.reset();
    }
    if (dbg_win)
    {
        dbg_win->post_render(console);
        dbg_win->release();
        dbg_win.reset();
    }

    glfwTerminate();

    nh_release_console(console);

    return err;
}

void
error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %d, %s\n", error, description);
}

void
pv_log(NHLogLevel level, const char *msg, void *user)
{
    sh::Logger *logger = static_cast<sh::Logger *>(user);
    switch (level)
    {
        case NH_LOG_FATAL:
            SH_LOG_FATAL(logger, msg);
            break;
        case NH_LOG_ERROR:
            SH_LOG_ERROR(logger, msg);
            break;
        case NH_LOG_WARN:
            SH_LOG_WARN(logger, msg);
            break;
        case NH_LOG_INFO:
            SH_LOG_INFO(logger, msg);
            break;
        case NH_LOG_DEBUG:
            SH_LOG_DEBUG(logger, msg);
            break;
        case NH_LOG_TRACE:
            SH_LOG_TRACE(logger, msg);
            break;

        default:
            break;
    }
}

int
audio_playback(void *outputBuffer, void *inputBuffer,
               unsigned int nBufferFrames, double streamTime,
               RtAudioStreamStatus status, void *userData)
{
    (void)(inputBuffer);
    (void)(streamTime);
    (void)(status);

    AudioData *audio_data = (AudioData *)userData;

#if DEBUG_AUDIO
    if (status)
    {
        SH_LOG_WARN(audio_data->logger,
                    "Stream underflow for buffer of {} detected!",
                    nBufferFrames);
    }
#endif

    // Write audio data
    sample_t *buffer = (sample_t *)outputBuffer;
    for (unsigned int i = 0; i < nBufferFrames; ++i)
    {
        sample_t sample;
        if (audio_data->buf->try_receive(sample))
        {
            audio_data->prev = sample;
        }
        else
        {
            sample = audio_data->prev;
        }

        // interleaved, 2 channels, mono ouput
        for (unsigned int j = 0; j < 2; ++j)
        {
            *buffer++ = sample;
        }
    }

    return 0;
}

} // namespace sh

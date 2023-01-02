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

#include "glfw_app/audio/resampler.hpp"
#include "common/path.hpp"
#include "glfw_app/audio/pcm_writer.hpp"
#include "glfw_app/audio/channel.hpp"
#include "rtaudio/RtAudio.h"

#define LA_AUDIO_SAMPLE_RATE 48000
#define LA_AUDIO_BUF_SIZE 1024
#define LA_AUDIO_RESAMPLE_BUF_SIZE LA_AUDIO_SAMPLE_RATE / 10
#define LA_AUDIO_DYN_D 0.005

namespace ln_app {

static constexpr double FRAME_TIME = 1.0 / 60.0;

typedef Channel<LA_AUDIO_BUF_SIZE> AudioChannel;

static void
error_callback(int error, const char *description);

static bool
audio_init(RtAudio &io_dac);
static bool
audio_open(RtAudio &io_dac, unsigned int i_sample_rate, unsigned i_buffer_size,
           RtAudioCallback i_callback, void *i_user_data);
static bool
audio_start(RtAudio &io_dac);
static bool
audio_stop(RtAudio &io_dac);
static void
audio_close(RtAudio &io_dac);
static int
audio_playback(void *outputBuffer, void *inputBuffer,
               unsigned int nBufferFrames, double streamTime,
               RtAudioStreamStatus status, void *userData);

struct ChannelData {
    AudioChannel *ch;
    bool done;
};

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
    std::unique_ptr<EmulatorWindow> emu_win{new EmulatorWindow()};
    std::unique_ptr<DebuggerWindow> dbg_win;
    if (i_opts & ln_app::OPT_DEBUG_WIN)
    {
        dbg_win.reset(new DebuggerWindow());
    }
    Resampler resampler{LA_AUDIO_RESAMPLE_BUF_SIZE};
    AudioChannel audio_ch;
    ChannelData ch_data{&audio_ch, false};
    RtAudio dac;
    PCMWriter pcm_writer;
    bool enable_audio = i_opts & ln_app::OPT_AUDIO;

    /* init audio */
    if (enable_audio)
    {
        if (!audio_init(dac))
        {
            err = 1;
            goto l_end;
        }
        if (!audio_open(dac, LA_AUDIO_SAMPLE_RATE, LA_AUDIO_BUF_SIZE,
                        audio_playback, &ch_data))
        {
            err = 1;
            goto l_end;
        }
        if (i_opts & ln_app::OPT_PCM)
        {
            if (pcm_writer.open(ln::join_exec_rel_path("audio.pcm")))
            {
                err = 1;
                goto l_end;
            }
        }
    }

    /* init windows */
    if (dbg_win)
    {
        if (!dbg_win->init(640, 480, true, false, "Debugger"))
        {
            err = 1;
            goto l_end;
        }
        dbg_win->set_pos(50 + LN_NES_WIDTH * 2 + 50, 150);
    }
    if (!emu_win->init(emulator.get(), LN_NES_WIDTH * 2, LN_NES_HEIGHT * 2,
                       !dbg_win, false, "Emulator"))
    {
        err = 1;
        goto l_end;
    }
    emu_win->set_pos(50, 150);

    if (dbg_win)
    {
        dbg_win->pre_render(*emulator);
    }

    if (enable_audio)
    {
        /* start audio playback */
        if (!audio_start(dac))
        {
            err = 1;
            goto l_end;
        }
    }
    /* Main loop */
    {
        constexpr double S_TO_US = 1000.0 * 1000.0;
        constexpr double FRAME_TIME_US = FRAME_TIME * S_TO_US;

        double currTimeUS = ln::get_now_micro();
        double nextEmulateTimeUS = currTimeUS;
        double nextRenderTimeUS = currTimeUS + FRAME_TIME_US;

        while (emu_win)
        {
            // Dispatch events
            glfwPollEvents();

            /* Close window if necessary */
            if (emu_win->shouldClose() || (dbg_win && dbg_win->shouldQuit()))
            {
                emu_win->release();
                emu_win.reset();
                break;
            }
            if (dbg_win && dbg_win->shouldClose())
            {
                dbg_win->post_render(*emulator);
                dbg_win->release();
                dbg_win.reset();
            }

            /* Emulate */
            currTimeUS = ln::get_now_micro();
            if (!(dbg_win && dbg_win->isPaused()) &&
                currTimeUS >= nextEmulateTimeUS)
            {
                if (enable_audio)
                {
                    /* Calculate target resample ratio */
                    double resample_ratio =
                        1. + (LA_AUDIO_BUF_SIZE - 2. * audio_ch.p_size()) /
                                 LA_AUDIO_BUF_SIZE * LA_AUDIO_DYN_D;
                    // target_bufsize > 0 holds
                    int target_bufsize = resample_ratio * LA_AUDIO_BUF_SIZE;
                    int target_sample_rate = LA_AUDIO_SAMPLE_RATE *
                                             double(target_bufsize) /
                                             LA_AUDIO_BUF_SIZE;

                    if (!resampler.set_rates(emulator->get_sample_rate(),
                                             target_sample_rate))
                    {
                        err = 1;
                        goto l_end;
                    }

                    /* Emulate until audio buffer of target size is met */
                    ln::Cycle cpu_ticks = 0;
                    for (int buf_idx = 0; buf_idx < target_bufsize; ++buf_idx)
                    {
                        // Feed input samples until target count is met
                        short buf = 0;
                        while (!resampler.samples_avail(&buf, 1))
                        {
                            if (emulator->tick())
                            {
                                float sample = emulator->get_sample();
                                // @NOTE: Once clocked, samples must be drained
                                // to avoid buffer overflow.
                                // @FIXME: Which is correct? [0, 1] or [-1, 1]
                                resampler.clock((sample * 2. - 1.) * 32767);
                                // resampler.clock(sample * 32767);
                            }
                            ++cpu_ticks;
                        }
                        // Drain the sample buffer
                        {
                            audio_ch.p_send(buf / 32767.);

                            if (pcm_writer.is_open())
                            {
                                pcm_writer.write_s16le(buf);
                            }
                        }
                    }

                    double emualteTimeUS =
                        emulator->elapsed(cpu_ticks) * S_TO_US;
                    nextEmulateTimeUS = currTimeUS + emualteTimeUS;
                }
                else
                {
                    ln::Cycle ticks = emulator->ticks(0.002);
                    for (decltype(ticks) i = 0; i < ticks; ++i)
                    {
                        emulator->tick();
                    }

                    double emualteTimeUS = emulator->elapsed(ticks) * S_TO_US;
                    nextEmulateTimeUS = currTimeUS + emualteTimeUS;
                }
            }

            /* Render, AT MOST at fixed rate */
            if (currTimeUS >= nextRenderTimeUS)
            {
                // Don't bother to render emulator if paused.
                // @IMPL: Handle emulator window first anyhow to reflect latest
                // visuals.
                if (!(dbg_win && dbg_win->isPaused()))
                {
                    if (emu_win)
                    {
                        emu_win->render();
                    }
                }
                if (dbg_win)
                {
                    dbg_win->render(*emulator);
                }

                nextRenderTimeUS = currTimeUS + FRAME_TIME_US;
            }
        }
    }

l_end:
    (void)pcm_writer.close();
    ch_data.done = true;
    (void)audio_stop(dac);
    audio_close(dac);
    resampler.close();

    // before glfw termination
    if (emu_win)
    {
        emu_win->release();
        emu_win.reset();
    }
    if (dbg_win)
    {
        dbg_win->post_render(*emulator);
        dbg_win->release();
        dbg_win.reset();
    }
    glfwTerminate();

    return err;
}

void
error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %d, %s\n", error, description);
}

bool
audio_init(RtAudio &io_dac)
{
    if (io_dac.getDeviceCount() < 1)
    {
        return false;
    }
    return true;
}

bool
audio_open(RtAudio &io_dac, unsigned int i_sample_rate, unsigned i_buffer_size,
           RtAudioCallback i_callback, void *i_user_data)
{
    if (!i_callback)
    {
        return false;
    }

    RtAudio::StreamParameters parameters;
    parameters.deviceId = io_dac.getDefaultOutputDevice();
    parameters.nChannels = 2;
    parameters.firstChannel = 0;
    try
    {
        io_dac.openStream(&parameters, NULL, RTAUDIO_FLOAT32, i_sample_rate,
                          &i_buffer_size, i_callback, i_user_data);
    }
    catch (RtAudioError &e)
    {
        return false;
    }
    return true;
}

bool
audio_start(RtAudio &io_dac)
{
    try
    {
        io_dac.startStream();
    }
    catch (RtAudioError &e)
    {
        return false;
    }
    return true;
}

bool
audio_stop(RtAudio &io_dac)
{
    try
    {
        if (io_dac.isStreamRunning())
        {
            io_dac.stopStream();
        }
    }
    catch (RtAudioError &e)
    {
        return false;
    }
    return true;
}

void
audio_close(RtAudio &io_dac)
{
    if (io_dac.isStreamOpen())
    {
        io_dac.closeStream();
    }
}

int
audio_playback(void *outputBuffer, void *inputBuffer,
               unsigned int nBufferFrames, double streamTime,
               RtAudioStreamStatus status, void *userData)
{
    (void)(inputBuffer);
    (void)(streamTime);

    if (status)
    {
        LN_LOG_INFO(ln::get_logger(), "Stream underflow of {} detected!",
                    nBufferFrames);
    }

    float *buffer = (float *)outputBuffer;
    ChannelData *ch_data = (ChannelData *)userData;
    // Write interleaved audio data.
    for (unsigned int i = 0; i < nBufferFrames; ++i)
    {
        float sample = 0.;
        while (!ch_data->done && !ch_data->ch->c_try_receive(sample))
        {
        }

        for (unsigned int j = 0; j < 2; ++j)
        {
            *buffer++ = sample;
        }
    }

    return 0;
}

} // namespace ln_app

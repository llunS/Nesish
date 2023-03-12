#include "app.hpp"

#include <cstdio>
#include <memory>
#include <chrono>

// @FIXME: spdlog will include windows header files, we need to include them
// before "glfw3.h" so that glfw won't redefine symbols.
#include "logger.hpp"
#include "glfw/glfw3.h"
#include "gui/emulator_window.hpp"
#include "gui/debugger_window.hpp"

#include "nesish/nesish.h"

#include "audio/resampler.hpp"
#include "nhbase/path.hpp"
#include "audio/pcm_writer.hpp"
#include "audio/channel.hpp"
#include "rtaudio/RtAudio.h"

#define SH_FRM_TIME (1.0 / 60.0)
#define SH_AUDIO_SAMPLE_RATE 48000
#define SH_AUDIO_BUF_SIZE                                                      \
    512 // previous power of 2 of (SH_FRM_TIME * SH_AUDIO_SAMPLE_RATE)
#define SH_AUDIO_RESAMPLE_BUF_SIZE (SH_AUDIO_SAMPLE_RATE / 10)
#define SH_AUDIO_DYN_D 0.005
#define SH_AUDIO_DYN 0

namespace sh {

typedef Channel<SH_AUDIO_BUF_SIZE> AudioChannel;

static void
error_callback(int error, const char *description);

static void
pv_log(NHLogLevel level, const char *msg, void *user);

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
    NHLogger logger{pv_log, nullptr, sh::get_log_level()};
    NHConsole console = nh_new_console(&logger);
    if (!NH_VALID(console))
    {
        return 1;
    }
    /* Insert cartridge */
    NHErr nh_err = nh_insert_cart(console, i_rom_path.c_str());
    if (NH_FAILED(nh_err))
    {
        SH_LOG_ERROR(sh::get_logger(), "Failed to load cartridge: {}", nh_err);
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
    if (i_opts & sh::OPT_DEBUG_WIN)
    {
        dbg_win.reset(new DebuggerWindow());
    }
    Resampler resampler{SH_AUDIO_RESAMPLE_BUF_SIZE};
    AudioChannel audio_ch;
    ChannelData ch_data{&audio_ch, false};
    RtAudio dac;
    PCMWriter pcm_writer;
    bool enable_audio = i_opts & sh::OPT_AUDIO;

    /* init audio */
    if (enable_audio)
    {
        if (!audio_init(dac))
        {
            err = 1;
            goto l_end;
        }
        if (!audio_open(dac, SH_AUDIO_SAMPLE_RATE, SH_AUDIO_BUF_SIZE,
                        audio_playback, &ch_data))
        {
            err = 1;
            goto l_end;
        }
        if (i_opts & sh::OPT_PCM)
        {
            if (pcm_writer.open(nb::path_join_exe("audio.pcm")))
            {
                err = 1;
                goto l_end;
            }
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

    if (dbg_win)
    {
        dbg_win->pre_render(console);
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
        auto currTime = std::chrono::steady_clock::now();
        auto nextEmulateTime = currTime + std::chrono::duration<double>(0.0);
        auto nextRenderTime =
            currTime + std::chrono::duration<double>(SH_FRM_TIME);

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
                dbg_win->post_render(console);
                dbg_win->release();
                dbg_win.reset();
            }

            /* Emulate */
            currTime = std::chrono::steady_clock::now();
            if (!(dbg_win && dbg_win->isPaused()) &&
                currTime >= nextEmulateTime)
            {
                if (enable_audio)
                {
#if SH_AUDIO_DYN
                    /* Calculate target resample ratio */
                    double resample_ratio =
                        1. + (SH_AUDIO_BUF_SIZE - 2. * audio_ch.p_size()) /
                                 SH_AUDIO_BUF_SIZE * SH_AUDIO_DYN_D;
                    // target_bufsize > 0 holds
                    int target_bufsize =
                        int(resample_ratio * SH_AUDIO_BUF_SIZE);
                    int target_sample_rate =
                        int(SH_AUDIO_SAMPLE_RATE * double(target_bufsize) /
                            SH_AUDIO_BUF_SIZE);
#else
#define target_bufsize SH_AUDIO_BUF_SIZE
#define target_sample_rate SH_AUDIO_SAMPLE_RATE
#endif
                    if (!resampler.set_rates(nh_get_sample_rate(console),
                                             target_sample_rate))
                    {
                        err = 1;
                        goto l_end;
                    }

                    /* Emulate until audio buffer of target size is met */
                    NHCycle cpu_ticks = 0;
                    for (int buf_idx = 0; buf_idx < target_bufsize; ++buf_idx)
                    {
                        // Feed input samples until target count is met
                        short buf = 0;
                        while (!resampler.samples_avail(&buf, 1))
                        {
                            if (nh_tick(console, nullptr))
                            {
                                float sample = nh_get_sample(console);
                                // @NOTE: Once clocked, samples must be
                                // drained to avoid buffer overflow.
                                // @FIXME: Which is correct, [0, 1] or [-1,
                                // 1]?
                                resampler.clock(
                                    short((sample * 2. - 1.) * 32767));
                                // resampler.clock(sample * 32767);
                            }
                            ++cpu_ticks;
                        }
                        // Drain the sample buffer
                        {
                            audio_ch.p_send(
                                AudioChannel::value_t(buf / 32767.));

                            if (pcm_writer.is_open())
                            {
                                pcm_writer.write_s16le(buf);
                            }
                        }
                    }

                    double emualteTime = nh_elapsed(console, cpu_ticks);
                    nextEmulateTime =
                        currTime + std::chrono::duration<double>(emualteTime);
                }
                else
                {
                    NHCycle ticks = nh_advance(console, SH_FRM_TIME);
                    for (decltype(ticks) i = 0; i < ticks; ++i)
                    {
                        nh_tick(console, nullptr);
                    }

                    nextEmulateTime =
                        currTime + std::chrono::duration<double>(SH_FRM_TIME);
                }
            }

            /* Render, AT MOST at fixed rate */
            if (currTime >= nextRenderTime)
            {
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

                nextRenderTime =
                    currTime + std::chrono::duration<double>(SH_FRM_TIME);
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
    catch (RtAudioError &)
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
    catch (RtAudioError &)
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
    catch (RtAudioError &)
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
        SH_LOG_INFO(sh::get_logger(),
                    "Stream underflow for buffer of {} detected!",
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

void
pv_log(NHLogLevel level, const char *msg, void *user)
{
    (void)(user);
    switch (level)
    {
        case NH_LOG_FATAL:
            sh::get_logger()->critical(msg);
            break;
        case NH_LOG_ERROR:
            sh::get_logger()->error(msg);
            break;
        case NH_LOG_WARN:
            sh::get_logger()->warn(msg);
            break;
        case NH_LOG_INFO:
            sh::get_logger()->info(msg);
            break;
        case NH_LOG_DEBUG:
            sh::get_logger()->debug(msg);
            break;
        case NH_LOG_TRACE:
            sh::get_logger()->trace(msg);
            break;

        default:
            break;
    }
}

} // namespace sh

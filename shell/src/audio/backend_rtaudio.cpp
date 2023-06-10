#include "backend_rtaudio.hpp"

#include "misc/exception.hpp"

namespace sh {

static RtAudio *g_rta;

bool
audio_start(unsigned int i_sample_rate, unsigned int i_buffer_size,
            stream_cb_t i_callback, void *i_user_data)
{
    /* Init */
    {
        SH_TRY
        {
            g_rta = new RtAudio();
        }
        SH_CATCH(const std::exception &)
        {
            return false;
        }

        if (g_rta->getDeviceCount() < 1)
        {
            delete g_rta;
            g_rta = nullptr;
            return false;
        }
    }

    /* Open */
    {
        if (!i_callback)
        {
            return false;
        }

        RtAudio::StreamParameters parameters;
        parameters.deviceId = g_rta->getDefaultOutputDevice();
        parameters.nChannels = 2;
        parameters.firstChannel = 0;
        SH_TRY
        {
            // Align format with sokol
            // 32-bit float numbers in the range -1.0 to +1.0
            g_rta->openStream(&parameters, NULL, RTAUDIO_FLOAT32, i_sample_rate,
                              &i_buffer_size, i_callback, i_user_data);
        }
        SH_CATCH(RtAudioError &)
        {
            return false;
        }
    }

    /* Start */
    {
        SH_TRY
        {
            g_rta->startStream();
        }
        SH_CATCH(RtAudioError &)
        {
            return false;
        }
    }

    return true;
}

void
audio_stop()
{
    if (g_rta)
    {
        /* Stop */
        SH_TRY
        {
            if (g_rta->isStreamRunning())
            {
                g_rta->stopStream();
            }
        }
        SH_CATCH(RtAudioError &) {}

        /* Close */
        if (g_rta->isStreamOpen())
        {
            g_rta->closeStream();
        }

        /* Uninit */
        delete g_rta;
        g_rta = nullptr;
    }
}

} // namespace sh

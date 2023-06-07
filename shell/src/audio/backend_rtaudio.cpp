#include "backend_rtaudio.hpp"

namespace sh {

static RtAudio *g_rta;

bool
audio_start(unsigned int i_sample_rate, unsigned int i_buffer_size,
            stream_cb_t i_callback, void *i_user_data)
{
    /* Init */
    {
        try
        {
            g_rta = new RtAudio();
        }
        catch (const std::exception &)
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
        try
        {
            // Align format with sokol
            // 32-bit float numbers in the range -1.0 to +1.0
            g_rta->openStream(&parameters, NULL, RTAUDIO_FLOAT32, i_sample_rate,
                              &i_buffer_size, i_callback, i_user_data);
        }
        catch (RtAudioError &)
        {
            return false;
        }
    }

    /* Start */
    {
        try
        {
            g_rta->startStream();
        }
        catch (RtAudioError &)
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
        try
        {
            if (g_rta->isStreamRunning())
            {
                g_rta->stopStream();
            }
        }
        catch (RtAudioError &)
        {
        }

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

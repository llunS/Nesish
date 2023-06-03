#include "device.hpp"

namespace sh {

static RtAudio *g_rta;

bool
audio_init()
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
    return true;
}

bool
audio_open(unsigned int i_sample_rate, unsigned i_buffer_size,
           RtAudioFormat i_format, RtAudioCallback i_callback,
           void *i_user_data)
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
        g_rta->openStream(&parameters, NULL, i_format, i_sample_rate,
                          &i_buffer_size, i_callback, i_user_data);
    }
    catch (RtAudioError &)
    {
        return false;
    }
    return true;
}

bool
audio_start()
{
    try
    {
        g_rta->startStream();
    }
    catch (RtAudioError &)
    {
        return false;
    }
    return true;
}

bool
audio_stop()
{
    if (g_rta)
    {
        try
        {
            if (g_rta->isStreamRunning())
            {
                g_rta->stopStream();
            }
        }
        catch (RtAudioError &)
        {
            return false;
        }
        return true;
    }
    return true;
}

void
audio_close()
{
    if (g_rta)
    {
        if (g_rta->isStreamOpen())
        {
            g_rta->closeStream();
        }
    }
}

void
audio_uninit()
{
    if (g_rta)
    {
        delete g_rta;
        g_rta = nullptr;
    }
}

} // namespace sh

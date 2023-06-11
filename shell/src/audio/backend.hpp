#pragma once

#ifndef SH_USE_SOKOL_AUDIO
#include "rtaudio/RtAudio.h"
#else
#include "sokol_audio.h"
#endif

namespace sh {

#ifndef SH_USE_SOKOL_AUDIO

typedef RtAudioCallback stream_cb_t;

bool
audio_setup(unsigned int i_sample_rate, unsigned int i_buffer_size,
            stream_cb_t i_callback, void *i_user_data);

bool
audio_start();

void
audio_stop();

#else

typedef void (*stream_cb_t)(float *buffer, int num_frames, int num_channels,
                            void *user_data);

bool
audio_setup(unsigned int i_sample_rate, unsigned int i_buffer_size,
            stream_cb_t i_callback, void *i_user_data);

#endif

void
audio_shutdown();

} // namespace sh

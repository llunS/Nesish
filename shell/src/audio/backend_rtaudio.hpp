#pragma once

#include "rtaudio/RtAudio.h"

namespace sh {

typedef RtAudioCallback stream_cb_t;

bool
audio_start(unsigned int i_sample_rate, unsigned int i_buffer_size,
            stream_cb_t i_callback, void *i_user_data);

void
audio_stop();

} // namespace sh

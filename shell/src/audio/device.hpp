#pragma once

#include "rtaudio/RtAudio.h"

namespace sh {

bool
audio_init();
bool
audio_open(unsigned int i_sample_rate, unsigned i_buffer_size,
           RtAudioFormat i_format, RtAudioCallback i_callback,
           void *i_user_data);
bool
audio_start();
bool
audio_stop();
void
audio_close();
void
audio_uninit();

} // namespace sh

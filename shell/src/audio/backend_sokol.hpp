#pragma once

#include "sokol_audio.h"

namespace sh {

typedef void (*stream_cb_t)(float *buffer, int num_frames, int num_channels,
                            void *user_data);

bool
audio_start(unsigned int i_sample_rate, unsigned int i_buffer_size,
            stream_cb_t i_callback, void *i_user_data);

void
audio_stop();

} // namespace sh

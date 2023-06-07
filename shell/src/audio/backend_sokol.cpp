#define SOKOL_AUDIO_IMPL
#include "backend_sokol.hpp"

#include "sokol_log.h"

namespace sh {

bool
audio_start(unsigned int i_sample_rate, unsigned int i_buffer_size,
            stream_cb_t i_callback, void *i_user_data)
{
    saudio_desc desc{};
    desc.logger.func = slog_func;
    desc.stream_userdata_cb = i_callback;
    desc.user_data = i_user_data;
    desc.sample_rate = (int)i_sample_rate;
    desc.num_channels = 2;
    desc.buffer_frames = (int)i_buffer_size;
    // init sokol-audio with default params
    saudio_setup(&desc);

    return saudio_isvalid() && saudio_buffer_frames() == (int)i_buffer_size &&
           saudio_sample_rate() == (int)i_sample_rate && saudio_channels() == 2;
}

void
audio_stop()
{
    saudio_shutdown();
}

} // namespace sh

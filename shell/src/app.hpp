#pragma once

#include <string>
#include <type_traits>

namespace sh {

enum AppOpt {
    OPT_NONE = 0,
    OPT_DEBUG_WIN = 1 << 0, // With debug window
    OPT_PCM = 1 << 1,       // Generate audio pcm file
    OPT_AUDIO = 1 << 2,     // WIP audio
};

inline AppOpt &
operator|=(AppOpt &a, AppOpt b)
{
    a = static_cast<AppOpt>(std::underlying_type<AppOpt>::type(a) |
                            std::underlying_type<AppOpt>::type(b));
    return a;
}

int
run_app(const std::string &i_rom_path, AppOpt i_opts);

} // namespace sh

#pragma once

#include <string>
#include <type_traits>

namespace sh {
struct Logger;
} // namespace sh

namespace sh {

enum AppOpt {
    OPT_NONE = 0,
    OPT_DEBUG = 1 << 0,   // With debug window
    OPT_PCM = 1 << 1,     // Record audio pcm file
    OPT_NOSLEEP = 1 << 2, // No battery saver
};

inline AppOpt &
operator|=(AppOpt &a, AppOpt b)
{
    a = static_cast<AppOpt>(std::underlying_type<AppOpt>::type(a) |
                            std::underlying_type<AppOpt>::type(b));
    return a;
}

int
run(const std::string &i_rom_path, AppOpt i_opts, Logger *i_logger);

} // namespace sh

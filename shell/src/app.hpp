#pragma once

#include <type_traits>

namespace sh {
struct Logger;
} // namespace sh

namespace sh {

enum AppOpt {
    OPT_NONE = 0,
    OPT_PCM = 1 << 0,     // Record audio pcm file
    OPT_NOSLEEP = 1 << 1, // No battery saver
};

inline AppOpt &
operator|=(AppOpt &a, AppOpt b)
{
    a = static_cast<AppOpt>(std::underlying_type<AppOpt>::type(a) |
                            std::underlying_type<AppOpt>::type(b));
    return a;
}

int
run(AppOpt i_opts, Logger *i_logger);

} // namespace sh

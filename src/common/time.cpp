#include "time.hpp"

#include <chrono>

namespace nh {

Time_t
get_now_micro()
{
    // we intentionally use implicit conversion here, so if there is any loss of
    // precision, it'll fail to compile.
    // duration_cast won't give us the benefit.
    std::chrono::duration<Time_t, std::micro> timeSinceEpoch =
        std::chrono::steady_clock::now().time_since_epoch();
    return timeSinceEpoch.count();
}

} // namespace nh

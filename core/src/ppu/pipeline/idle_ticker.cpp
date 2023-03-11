#include "idle_ticker.hpp"

namespace nh {

IdleTicker::IdleTicker(Cycle i_total)
    : Tickable(i_total)
{
}

Cycle
IdleTicker::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_curr);
    (void)(i_total);

    return 1;
}

} // namespace nh

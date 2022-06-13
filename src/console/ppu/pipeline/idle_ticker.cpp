#include "idle_ticker.hpp"

namespace ln {

IdleTicker::IdleTicker(Cycle i_total)
    : Ticker(i_total)
{
}

Cycle
IdleTicker::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_curr);
    (void)(i_total);

    return 1;
}

} // namespace ln

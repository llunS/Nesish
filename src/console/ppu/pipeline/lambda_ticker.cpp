#include "lambda_ticker.hpp"

namespace ln {

LambdaTicker::LambdaTicker(Cycle i_total, const CycleFunc &i_cycle_func)
    : Ticker(i_total)
    , m_func(i_cycle_func)
{
}

Cycle
LambdaTicker::on_tick(Cycle i_curr, Cycle i_total)
{
    return m_func(i_curr, i_total);
}

} // namespace ln

#include "functor_tickable.hpp"

namespace nh {

FunctorTickable::FunctorTickable(Cycle i_total, const CycleFunc &i_cycle_func)
    : Tickable(i_total)
    , m_func(i_cycle_func)
{
}

Cycle
FunctorTickable::on_tick(Cycle i_curr, Cycle i_total)
{
    return m_func(i_curr, i_total);
}

} // namespace nh

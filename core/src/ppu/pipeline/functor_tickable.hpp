#pragma once

#include "ppu/pipeline/tickable.hpp"
#include "nhbase/klass.hpp"

#include <functional>

namespace nh {

struct FunctorTickable : public Tickable {
  public:
    typedef std::function<Cycle(Cycle, Cycle)> CycleFunc;

    FunctorTickable(Cycle i_total, const CycleFunc &i_cycle_func);
    NB_KLZ_DELETE_COPY_MOVE(FunctorTickable);

    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    CycleFunc m_func;
};

} // namespace nh

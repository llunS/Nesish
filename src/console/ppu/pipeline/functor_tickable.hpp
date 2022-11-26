#pragma once

#include "console/ppu/pipeline/tickable.hpp"
#include "common/klass.hpp"

#include <functional>

namespace ln {

struct FunctorTickable : public Tickable {
  public:
    typedef std::function<Cycle(Cycle, Cycle)> CycleFunc;

    FunctorTickable(Cycle i_total, const CycleFunc &i_cycle_func);
    LN_KLZ_DELETE_COPY_MOVE(FunctorTickable);

    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    CycleFunc m_func;
};

} // namespace ln

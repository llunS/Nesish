#pragma once

#include "console/ppu/pipeline/ticker.hpp"
#include "common/klass.hpp"

#include <functional>

namespace ln {

struct LambdaTicker : public Ticker {
  public:
    typedef std::function<Cycle(Cycle, Cycle)> CycleFunc;

    LambdaTicker(Cycle i_total, const CycleFunc &i_cycle_func);
    LN_KLZ_DELETE_COPY_MOVE(LambdaTicker);

    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    CycleFunc m_func;
};

} // namespace ln

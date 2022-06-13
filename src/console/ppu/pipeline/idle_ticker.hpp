#pragma once

#include "console/ppu/pipeline/ticker.hpp"
#include "common/klass.hpp"

namespace ln {

struct IdleTicker : public Ticker {
  public:
    IdleTicker(Cycle i_total);
    LN_KLZ_DELETE_COPY_MOVE(IdleTicker);

    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;
};

} // namespace ln

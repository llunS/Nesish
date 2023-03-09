#pragma once

#include "console/ppu/pipeline/tickable.hpp"
#include "common/klass.hpp"

namespace nh {

struct IdleTicker : public Tickable {
  public:
    IdleTicker(Cycle i_total);
    LN_KLZ_DELETE_COPY_MOVE(IdleTicker);

    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;
};

} // namespace nh

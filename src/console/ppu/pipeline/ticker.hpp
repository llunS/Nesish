#pragma once

#include "console/clock.hpp"

namespace ln {

struct Ticker {
  public:
    Ticker(Cycle i_total);
    virtual ~Ticker() = default;

    virtual void
    reset();

    bool
    done();
    void
    set_done();

    void
    tick();

    virtual Cycle
    on_tick(Cycle i_curr, Cycle i_total) = 0;

  private:
    Cycle m_curr;
    Cycle m_total;
};

} // namespace ln

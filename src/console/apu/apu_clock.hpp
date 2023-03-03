#pragma once

#include "common/klass.hpp"

#include "console/types.hpp"

namespace ln {

/// @brief A struct to sync get/put cycle among APU and DMA(OAM/DMC)
struct APUClock {
  public:
    APUClock();
    ~APUClock() = default;
    LN_KLZ_DELETE_COPY_MOVE(APUClock);

  public:
    void
    tick();

    void
    power_up();
    void
    reset();

    bool
    get() const;
    bool
    put() const;

    bool
    even() const;
    bool
    odd() const;

  private:
    // @NOTE: This may wrap around back to 0, which is fine, since current
    // implementation doesn't assume infinite range.
    Cycle m_cycle;
};

} // namespace ln

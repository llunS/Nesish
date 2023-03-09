#pragma once

#include "common/klass.hpp"

#include "console/types.hpp"

namespace nh {

struct APUClock;
struct Memory;
struct PPU;

struct OAMDMA {
  public:
    OAMDMA(const APUClock &i_clock, const Memory &i_memory, PPU &o_ppu);
    ~OAMDMA() = default;
    LN_KLZ_DELETE_COPY_MOVE(OAMDMA);

  public:
    void
    power_up();
    void
    reset();

    void
    tick(bool i_cpu_dma_halt, bool dmc_dma_get);

    void
    initiate(Byte i_addr_high);

    bool
    rdy() const;

  private:
    const APUClock &m_clock;
    const Memory &m_memory;
    PPU &m_ppu;

    bool m_rdy; // RDY enable source
    bool m_working;
    Address m_addr_cur;
    bool m_got;
    Byte m_bus;

    bool m_swap;
    Address m_addr_tmp;
};

} // namespace nh

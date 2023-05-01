#pragma once

#include "nhbase/klass.hpp"

#include "types.hpp"

namespace nh {

struct APUClock;
struct Memory;
struct PPU;

struct OAMDMA {
  public:
    OAMDMA(const APUClock &i_clock, const Memory &i_memory, PPU &o_ppu);
    ~OAMDMA() = default;
    NB_KLZ_DELETE_COPY_MOVE(OAMDMA);

  public:
    void
    power_up();
    void
    reset();

    /// @param i_cpu_dma_halt Whether CPU is halted
    /// @param i_dmc_dma_get Whether DMC DMA get was performed
    /// @return Whether the DMA performed work (i.e. get or put)
    bool
    tick(bool i_cpu_dma_halt, bool i_dmc_dma_get);

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

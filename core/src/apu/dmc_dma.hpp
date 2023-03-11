#pragma once

#include "nhbase/klass.hpp"

#include "types.hpp"

namespace nh {

struct APUClock;
struct Memory;
struct APU;

struct DMCDMA {
  public:
    DMCDMA(const APUClock &i_clock, const Memory &i_memory, APU &o_apu);
    ~DMCDMA() = default;
    NB_KLZ_DELETE_COPY_MOVE(DMCDMA);

  public:
    void
    power_up();
    void
    reset();

    /// @param i_cpu_dma_halt Whether CPU is halted
    /// @return Whether the DMA get was performed
    bool
    tick(bool i_cpu_dma_halt);

    void
    initiate(Address i_sample_addr, bool i_reload);

    bool
    rdy() const;

  private:
    const APUClock &m_clock;
    const Memory &m_memory;
    APU &m_apu;

    bool m_reload;
    unsigned int m_load_counter;
    bool m_rdy; // RDY enable source
    bool m_working;
    Address m_sample_addr;
    bool m_dummy;

    bool m_swap;
    bool m_reload_tmp;
    Address m_sample_addr_tmp;
};

} // namespace nh

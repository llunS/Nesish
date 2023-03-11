#include "dmc_dma.hpp"

// https://www.nesdev.org/wiki/DMA

/* There are 2 bugs regarding to DMC DMA, we don't emulate them
 * https://www.nesdev.org/wiki/DMA#Bugs
 * 1. When sample playback is stopped during the APU cycle before a reload DMA
 * would schedule (that is, on the 2nd or 3rd CPU cycle before the halt
 * attempt), the DMA starts, but is aborted after a single cycle. If the halt is
 * delayed due to a write cycle, the aborted DMA doesn't occur at all.
 * 2. On RP2A03H and late RP2A03G CPUs, when playback is stopped implicitly on
 * the same APU cycle that a reload DMA would schedule (that is, the 1st CPU
 * cycle before the halt attempt), an unexpected reload DMA occurs.
 */

#include "apu/apu_clock.hpp"
#include "memory/memory.hpp"
#include "apu/apu.hpp"

namespace nh {

DMCDMA::DMCDMA(const APUClock &i_clock, const Memory &i_memory, APU &o_apu)
    : m_clock(i_clock)
    , m_memory(i_memory)
    , m_apu(o_apu)
{
}

void
DMCDMA::power_up()
{
    m_reload = false;
    m_load_counter = 0;
    m_rdy = false;
    m_working = false;
    m_sample_addr = 0;
    m_dummy = false;

    m_swap = false;
    m_reload_tmp = false;
    m_sample_addr_tmp = 0;
}

void
DMCDMA::reset()
{
    // do nothing, which implies a DMA may delay the reset sequence
}

bool
DMCDMA::tick(bool i_cpu_dma_halt)
{
    bool get_cycle = false;

    if (m_swap)
    {
        m_reload = m_reload_tmp;
        if (!m_reload)
        {
            // If this clock is a put cycle, wait for another 2 put clocks (this
            // clock included)
            m_load_counter = m_clock.put() ? 2 : 1;
        }
        m_rdy = false;
        m_working = true;
        m_sample_addr = m_sample_addr_tmp;
        m_dummy = false;

        m_swap = false;
    }

    if (m_working)
    {
        // Wait until the cycle we start to halt
        if (!m_rdy)
        {
            // Reload DMA starts to halt on put cycle
            if (m_reload)
            {
                if (m_clock.put())
                {
                    m_rdy = true;
                }
            }
            // Load DMA starts to halt on the get cycle during the 2nd following
            // APU cycle
            else
            {
                if (m_clock.get())
                {
                    if (!m_load_counter)
                    {
                        m_rdy = true;
                    }
                }
                else
                {
                    --m_load_counter;
                }
            }
        }
        else
        {
            // Wait until CPU is halted on read cycle
            if (!i_cpu_dma_halt)
            {
            }
            else
            {
                if (!m_dummy)
                {
                    m_dummy = true;
                }
                else
                {
                    if (m_clock.get())
                    {
                        Byte sample = 0xFF;
                        (void)m_memory.get_byte(m_sample_addr, sample);
                        m_apu.put_dmc_sample(m_sample_addr, sample);

                        get_cycle = true;
                        m_working = false;
                    }
                    // Alignment cycle
                    else
                    {
                    }
                }
            }
        }
    }
    else
    {
        // Delay the RDY disable by 1 cycle since DMA is ticked before
        // CPU and we want the CPU to keep halting on our last cycle.
        // No risk of another DMA initiation inbetween the two adjacent cycles.
        m_rdy = false;
    }
    return get_cycle;
}

void
DMCDMA::initiate(Address i_sample_addr, bool i_reload)
{
    // For current implmentation, if one has been started, override the
    // existing one, restart from the beginning.
    m_reload_tmp = i_reload;
    m_sample_addr_tmp = i_sample_addr;
    m_swap = true;
}

bool
DMCDMA::rdy() const
{
    return m_rdy;
}

} // namespace nh

#include "oam_dma.hpp"

// https://www.nesdev.org/wiki/DMA

#include "console/apu/apu_clock.hpp"
#include "console/memory/memory.hpp"
#include "console/ppu/ppu.hpp"

namespace ln {

OAMDMA::OAMDMA(const APUClock &i_clock, const Memory &i_memory, PPU &o_ppu)
    : m_clock(i_clock)
    , m_memory(i_memory)
    , m_ppu(o_ppu)
{
}

void
OAMDMA::power_up()
{
    m_rdy = false;
    m_working = false;
    m_addr_cur = 0;
    m_got = false;
    m_bus = 0;

    m_swap = false;
    m_addr_tmp = 0;
}

void
OAMDMA::reset()
{
    // do nothing, which implies a DMA may delay the reset sequence
}

void
OAMDMA::tick(bool i_cpu_dma_halt, bool dmc_dma_get)
{
    if (m_swap)
    {
        m_rdy = false;
        m_working = true;
        m_addr_cur = m_addr_tmp;
        m_got = false;

        m_swap = false;
    }

    if (m_working)
    {
        m_rdy = true;
        // Wait until CPU is halted on read cycle
        if (!i_cpu_dma_halt)
        {
        }
        else
        {
            // Get cycle
            if (m_clock.get())
            {
                // Back away for DMC DMA, which results in one extra alignment
                // cycle.
                if (dmc_dma_get)
                {
                    // "m_got" remaining false means extra alignment cycles.
                }
                else
                {
                    m_bus = 0xFF;
                    (void)m_memory.get_byte(m_addr_cur, m_bus);
                    m_got = true;
                    ++m_addr_cur;
                }
            }
            // Put cycle
            else
            {
                // Alignment cycle
                if (!m_got)
                {
                }
                // Write cycle
                else
                {
                    m_ppu.write_register(PPU::OAMDATA, m_bus);
                    m_got = false;
                    // Check if it's done after a write.
                    if (!(m_addr_cur & 0x00FF))
                    {
                        m_working = false;
                    }
                }
            }
        }
    }
    else
    {
        // @NOTE: Delay the RDY disable by 1 cycle since DMA is ticked before
        // CPU and we want the CPU to keep halting on our last cycle.
        // No risk of another DMA initiation inbetween the two adjacent cycles.
        m_rdy = false;
    }
}

void
OAMDMA::initiate(Byte i_addr_high)
{
    // Consecutive calls across adjacent cycles enables OAM going with the last
    // address provided.
    m_addr_tmp = Address(i_addr_high) << 8;
    m_swap = true;
}

bool
OAMDMA::rdy() const
{
    return m_rdy;
}

} // namespace ln

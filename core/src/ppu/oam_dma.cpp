#include "oam_dma.hpp"

// https://www.nesdev.org/wiki/DMA

#include "apu/apu_clock.hpp"
#include "memory/memory.hpp"
#include "ppu/ppu.hpp"

namespace nh {

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

bool
OAMDMA::tick(bool i_cpu_dma_halt, bool i_dmc_dma_get)
{
    bool op_cycle = false;

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
                // Back away for DMC DMA. This results in one extra alignment
                // cycle.
                if (i_dmc_dma_get)
                {
                    // "m_got" remaining false means extra alignment cycles.
                }
                else
                {
                    m_bus = 0xFF;
                    (void)m_memory.get_byte(m_addr_cur, m_bus);
                    m_got = true;
                    ++m_addr_cur;

                    op_cycle = true;
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

                    op_cycle = true;
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
        // Delay the RDY disable by 1 cycle since DMA is ticked before
        // CPU and we want the CPU to keep halting on the DMA's last cycle.
        // No risk of another DMA initiation inbetween the two adjacent cycles.
        m_rdy = false;
    }

    return op_cycle;
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

} // namespace nh

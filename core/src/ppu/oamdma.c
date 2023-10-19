#include "oamdma.h"

// https://www.nesdev.org/wiki/DMA

#include "apu/apuclock.h"
#include "memory/mmem.h"
#include "ppu/ppu.h"

void
oamdma_Init(oamdma_s *self, const apuclock_s *clock, const mmem_s *mmem,
            ppu_s *ppu)
{
    self->clock_ = clock;
    self->mmem_ = mmem;
    self->ppu_ = ppu;
}

void
oamdma_Powerup(oamdma_s *self)
{
    self->rdy_ = false;
    self->working_ = false;
    self->addrcurr_ = 0;
    self->got_ = false;
    self->bus_ = 0;

    self->swap_ = false;
    self->addrtmp_ = 0;
}

void
oamdma_Reset(oamdma_s *self)
{
    (void)(self);
    // do nothing, which implies a DMA may delay the reset sequence
}

bool
oamdma_Tick(oamdma_s *self, bool cpuDmaHalt, bool dmcDmaGet)
{
    bool opCycle = false;

    if (self->swap_)
    {
        self->rdy_ = false;
        self->working_ = true;
        self->addrcurr_ = self->addrtmp_;
        self->got_ = false;

        self->swap_ = false;
    }

    if (self->working_)
    {
        self->rdy_ = true;
        // Wait until CPU is halted on read cycle
        if (!cpuDmaHalt)
        {
        }
        else
        {
            // Get cycle
            if (apuclock_Get(self->clock_))
            {
                // Back away for DMC DMA. This results in one extra alignment
                // cycle.
                if (dmcDmaGet)
                {
                    // "self->got_" remaining false means extra alignment
                    // cycles.
                }
                else
                {
                    self->bus_ = 0xFF;
                    (void)mmem_GetB(self->mmem_, self->addrcurr_, &self->bus_);
                    self->got_ = true;
                    ++self->addrcurr_;

                    opCycle = true;
                }
            }
            // Put cycle
            else
            {
                // Alignment cycle
                if (!self->got_)
                {
                }
                // Write cycle
                else
                {
                    ppu_WriteReg(self->ppu_, PR_OAMDATA, self->bus_);
                    self->got_ = false;

                    opCycle = true;
                    // Check if it's done after a write.
                    if (!(self->addrcurr_ & 0x00FF))
                    {
                        self->working_ = false;
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
        self->rdy_ = false;
    }

    return opCycle;
}

void
oamdma_Initiate(oamdma_s *self, u8 addrhigh)
{
    // Consecutive calls across adjacent cycles enables OAM going with the last
    // address provided.
    self->addrtmp_ = (addr_t)addrhigh << 8;
    self->swap_ = true;
}

bool
oamdma_Rdy(const oamdma_s *self)
{
    return self->rdy_;
}

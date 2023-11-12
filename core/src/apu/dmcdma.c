#include "dmcdma.h"

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

#include "apu/apuclock.h"
#include "memory/mmem.h"
#include "apu/apu.h"

void
dmcdma_Init(dmcdma_s *self, const apuclock_s *clock, const mmem_s *mmem,
            apu_s *apu)
{
    self->clock_ = clock;
    self->mmem_ = mmem;
    self->apu_ = apu;
}

void
dmcdma_Powerup(dmcdma_s *self)
{
    self->reload_ = false;
    self->loadctr_ = 0;
    self->rdy_ = false;
    self->working_ = false;
    self->sampleaddr_ = 0;
    self->dummy_ = false;

    self->swap_ = false;
    self->reloadtmp_ = false;
    self->sampleAddrTmp_ = 0;
}

void
dmcdma_Reset(dmcdma_s *self)
{
    (void)(self);
    // do nothing, which implies a DMA may delay the reset sequence
}

bool
dmcdma_Tick(dmcdma_s *self, bool cpuDmaHalt)
{
    bool getcycle = false;

    if (self->swap_) {
        self->reload_ = self->reloadtmp_;
        if (!self->reload_) {
            // If this clock is a put cycle, wait for another 2 put clocks (this
            // clock included)
            self->loadctr_ = apuclock_Put(self->clock_) ? 2 : 1;
        }
        self->rdy_ = false;
        self->working_ = true;
        self->sampleaddr_ = self->sampleAddrTmp_;
        self->dummy_ = false;

        self->swap_ = false;
    }

    if (self->working_) {
        // Wait until the cycle we start to halt
        if (!self->rdy_) {
            // Reload DMA starts to halt on put cycle
            if (self->reload_) {
                if (apuclock_Put(self->clock_)) {
                    self->rdy_ = true;
                }
            }
            // Load DMA starts to halt on the get cycle during the 2nd following
            // APU cycle
            else {
                if (apuclock_Get(self->clock_)) {
                    if (!self->loadctr_) {
                        self->rdy_ = true;
                    }
                } else {
                    --self->loadctr_;
                }
            }
        } else {
            // Wait until CPU is halted on read cycle
            if (!cpuDmaHalt) {
            } else {
                if (!self->dummy_) {
                    self->dummy_ = true;
                } else {
                    if (apuclock_Get(self->clock_)) {
                        u8 sample = 0xFF;
                        (void)mmem_GetB(self->mmem_, self->sampleaddr_,
                                        &sample);
                        apu_PutDmcSample(self->apu_, self->sampleaddr_, sample);

                        getcycle = true;
                        self->working_ = false;
                    }
                    // Alignment cycle
                    else {
                    }
                }
            }
        }
    } else {
        // Delay the RDY disable by 1 cycle since DMA is ticked before
        // CPU and we want the CPU to keep halting on the DMA's last cycle.
        // No risk of another DMA initiation inbetween the two adjacent cycles.
        self->rdy_ = false;
    }

    return getcycle;
}

void
dmcdma_Initiate(dmcdma_s *self, addr_t sampleAddr, bool reload)
{
    // For current implmentation, if one has been started, override the
    // existing one, restart from the beginning.
    self->reloadtmp_ = reload;
    self->sampleAddrTmp_ = sampleAddr;
    self->swap_ = true;
}

bool
dmcdma_Rdy(const dmcdma_s *self)
{
    return self->rdy_;
}

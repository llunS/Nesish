#pragma once

#include "types.h"

typedef struct apuclock_s apuclock_s;
typedef struct mmem_s mmem_s;
typedef struct apu_s apu_s;

typedef struct dmcdma_s {
    const apuclock_s *clock_;
    const mmem_s *mmem_;
    apu_s *apu_;

    bool reload_;
    unsigned int loadctr_;
    bool rdy_; // RDY enable source
    bool working_;
    addr_t sampleaddr_;
    bool dummy_;

    bool swap_;
    bool reloadtmp_;
    addr_t sampleAddrTmp_;
} dmcdma_s;

void
dmcdma_Init(dmcdma_s *self, const apuclock_s *clock, const mmem_s *mmem,
            apu_s *apu);

void
dmcdma_Powerup(dmcdma_s *self);
void
dmcdma_Reset(dmcdma_s *self);

/// @param cpuDmaHalt Whether CPU is halted
/// @return Whether the DMA get was performed
bool
dmcdma_Tick(dmcdma_s *self, bool cpuDmaHalt);

void
dmcdma_Initiate(dmcdma_s *self, addr_t sampleAddr, bool reload);

bool
dmcdma_Rdy(const dmcdma_s *self);

#pragma once

#include "types.h"

typedef struct apuclock_s apuclock_s;
typedef struct mmem_s mmem_s;
typedef struct ppu_s ppu_s;

typedef struct oamdma_s {
    const apuclock_s *clock_;
    const mmem_s *mmem_;
    ppu_s *ppu_;

    bool rdy_; // RDY enable source
    bool working_;
    addr_t addrcurr_;
    bool got_;
    u8 bus_;

    bool swap_;
    addr_t addrtmp_;
} oamdma_s;

void
oamdma_Init(oamdma_s *self, const apuclock_s *clock, const mmem_s *mmem,
            ppu_s *ppu);

void
oamdma_Powerup(oamdma_s *self);
void
oamdma_Reset(oamdma_s *self);

/// @param cpuDmaHalt Whether CPU is halted
/// @param dmcDmaGet Whether DMC DMA get was performed
/// @return Whether the DMA performed work (i.e. get or put)
bool
oamdma_Tick(oamdma_s *self, bool cpuDmaHalt, bool dmcDmaGet);

void
oamdma_Initiate(oamdma_s *self, u8 addrhigh);

bool
oamdma_Rdy(const oamdma_s *self);

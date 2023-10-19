#pragma once

#include "memory/membase.h"
#include "spec.h"
#include "types.h"

typedef enum mmp_e {
    MMP_NAH, // invalid

    MMP_RAM, // on-board ram

    MMP_PPU,
    MMP_AOC, // APU-OAMDMA-CTRL

    MMP_PRGROM,
    MMP_PRGRAM,

    MMP_SIZE,
} mmp_e;

typedef struct mmem_s {
    membase_s Base;

    // ---- Memory Map
    // https://wiki.nesdev.org/w/index.php?title=CPU_memory_map

    // On-board RAM
    u8 ram_[NH_INTERNAL_RAM_SIZE];
    u8 readlatch_;
} mmem_s;

bool
mmem_Init(mmem_s *self, NHLogger *logger);
void
mmem_Deinit(mmem_s *self);

NHErr
mmem_GetB(const mmem_s *self, addr_t addr, u8 *val);

u8
mmem_GetLatch(const mmem_s *self);
void
mmem_OverrideLatch(mmem_s *self, u8 val);

NHErr
mmem_SetBulk(mmem_s *self, addr_t begin, addr_t end, u8 val);

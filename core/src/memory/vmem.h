#pragma once

#include "memory/membase.h"
#include "spec.h"
#include "types.h"

typedef enum vmp_e {
    VMP_NAH, // invalid

    // https://www.nesdev.org/wiki/PPU_memory_map
    VMP_PAT,    // pattern
    VMP_NT,     // nametable
    VMP_NT_MIR, // nametable mirror
    VMP_PAL,    // palette

    VMP_REST, // invalid rest

    VMP_SIZE,
} vmp_e;

// Mirror mode
typedef enum mirmode_e {
    MM_H,
    MM_V,
    MM_1LOW,
    MM_1HIGH,
} mirmode_e;
typedef mirmode_e (*mirmode_f)(void *opaque);

typedef struct vmem_s {
    membase_s Base;

    // https://www.nesdev.org/wiki/PPU_memory_map
    u8 ram_[NH_PPU_INTERNAL_RAM_SIZE];
    u8 pal_[NH_PALETTE_SIZE];

    mirmode_e mirFixedMode_;
    mirmode_f mirDynDecode_;
    void *mirOpaque_;
} vmem_s;

bool
vmem_Init(vmem_s *self, NHLogger *logger);
void
vmem_Deinit(vmem_s *self);

void
vmem_SetMirrorFixed(vmem_s *self, mirmode_e mode);
void
vmem_SetMirrorDyn(vmem_s *self, mirmode_f modecb, void *opaque);
void
vmem_UnsetMirror(vmem_s *self);

NHErr
vmem_GetB(const vmem_s *self, addr_t addr, u8 *val);

u8
vmem_GetPalByte(vmem_s *self, int idx);

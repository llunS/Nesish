#pragma once

#include "cartridge/mapper/mapper.h"
#include "cartridge/mapper/mapperbase.h"
#include "types.h"

typedef enum mmc1var_e {
    MMC1B,
} mmc1var_e;

typedef struct mmc1_s {
    mapperbase_s base_;

    mmc1var_e var_;

    u8 shift_;
    u8 ctrl_;
    u8 ch0bnk_;
    u8 ch1bnk_;
    u8 prgbnk_;

    u8 prgram_[32 * 1024]; // 32KB at max
    u8 chrram_[8 * 1024];

    bool noprgbanking32K_;

    struct {
        u8 *Base;
        usize Size;
    } prgromctx_;
    struct {
        u8 *Base;
        usize Size;
    } chrctx_;
} mmc1_s;

void
mmc1_Init(mmc1_s *self, const inesromaccessor_s *accessor, mmc1var_e var);

void
mmc1_Deinit(void *self);

NHErr
mmc1_Validate(const void *self);

void
mmc1_Powerup(void *self);
void
mmc1_Reset(void *self);

void
mmc1_MapMemory(void *self, mmem_s *mmem, vmem_s *vmem);
void
mmc1_UnmapMemory(void *self, mmem_s *mmem, vmem_s *vmem);
#pragma once

#include "cartridge/mapper/mapper.h"
#include "cartridge/mapper/mapperbase.h"
#include "types.h"

typedef struct nrom_s {
    mapperbase_s base_;

    u8 prgram_[8 * 1024]; // 8KB max
    u8 chrram_[8 * 1024];

    struct {
        u8 *Base;
        usize Size;
    } prgromctx_;
    struct {
        u8 *Base;
    } chrctx_;
} nrom_s;

void
nrom_Init(nrom_s *self, const inesromaccessor_s *accessor);

inline void
nrom_Deinit(nrom_s *self)
{
    (void)(self);
}

NHErr
nrom_Validate(const nrom_s *self);

void
nrom_Powerup(nrom_s *self);
void
nrom_Reset(nrom_s *self);

void
nrom_MapMemory(nrom_s *self, mmem_s *mmem, vmem_s *vmem);
void
nrom_UnmapMemory(nrom_s *self, mmem_s *mmem, vmem_s *vmem);

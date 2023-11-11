#pragma once

#include "cartridge/mapper/mapper.h"
#include "cartridge/mapper/mapperbase.h"
#include "types.h"

/// @brief Mapper 3 implementation, CNROM-like boards
typedef struct cnrom_s {
    mapperbase_s base_;

    u8 chrbnk_;

    struct {
        u8 *Base;
        usize Size;
    } prgromctx_;
    struct {
        u8 *Base;
        usize Size;
    } chrromctx_;
} cnrom_s;

void
cnrom_Init(cnrom_s *self, const inesromaccessor_s *accessor);

void
cnrom_Deinit(void *self);

NHErr
cnrom_Validate(const void *self);

void
cnrom_Powerup(void *self);
void
cnrom_Reset(void *self);

void
cnrom_MapMemory(void *self, mmem_s *mmem, vmem_s *vmem);
void
cnrom_UnmapMemory(void *self, mmem_s *mmem, vmem_s *vmem);

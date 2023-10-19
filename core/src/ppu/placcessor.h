#pragma once

#include "ppu/ppureg.h"
#include "types.h"
#include "nesish/nesish.h"

typedef struct ppu_s ppu_s;
typedef struct plctx_s plctx_s;
typedef struct vmem_s vmem_s;
typedef struct palette_s palette_s;
typedef struct frmbuf_s frmbuf_s;

typedef struct placcessor_s {
    ppu_s *ppu_;
} placcessor_s;

plctx_s *
placcessor_GetCtx(placcessor_s *self);

NHLogger *
placcessor_GetLogger(const placcessor_s *self);

u8 *
placcessor_RegOf(placcessor_s *self, ppureg_e reg);
u8
placcessor_GetReg(const placcessor_s *self, ppureg_e reg);

u16 *
placcessor_GetV(placcessor_s *self);
u16 *
placcessor_GetT(placcessor_s *self);
u8 *
placcessor_GetX(placcessor_s *self);

u8
placcessor_GetOamByte(placcessor_s *self, u8 addr);
u8 *
placcessor_GetOamPtr(placcessor_s *self, u8 addr);

vmem_s *
placcessor_GetVmem(placcessor_s *self);

u8
placcessor_GetColorByte(placcessor_s *self, int idx);
const palette_s *
placcessor_GetPal(placcessor_s *self);

frmbuf_s *
placcessor_GetFrmbuf(placcessor_s *self);

bool
placcessor_BgEnabled(const placcessor_s *self);
bool
placcessor_SpEnabled(const placcessor_s *self);
bool
placcessor_RenderingEnabled(const placcessor_s *self);
bool
placcessor_Is8x16(const placcessor_s *self);

bool
placcessor_NoNmi(const placcessor_s *self);

void
placcessor_FinishFrame(placcessor_s *self);

addr_t
placcessor_GetSliverAddr(bool tblright, u8 tileidx, bool upper, u8 finey);
NHErr
placcessor_GetPtnSliver(bool tblright, u8 tileidx, bool upper, u8 finey,
                        const vmem_s *vmem, u8 *val);

void
placcessor_ResolveSpPtnTbl(u8 tilebyte, bool is8x16, bool ptnTblBit,
                           bool *highPtnTbl);
void
placcessor_ResolveSpTile(u8 tilebyte, bool is8x16, bool flipy, u8 fineySp,
                         u8 *tileidx);

void
placcessor_init_(placcessor_s *self, ppu_s *ppu);

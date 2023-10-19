#pragma once

#include "types.h"
#include "ppu/frmbuf.h"
#include "ppu/palette.h"
#include "spec.h"
#include "ppu/pipeline/plctx.h"
#include "ppu/placcessor.h"
#include "ppu/pipeline/pl.h"
#include "ppu/ppureg.h"

#include "debug/dbgflags.h"
#include "debug/dbgpal.h"
#include "debug/dbgoam.h"
#include "debug/dbgpattbl.h"

typedef struct NHLogger NHLogger;
typedef struct vmem_s vmem_s;

typedef struct ppu_s {
    u8 regs_[PR_SIZE];
    u8 ppudatabuf_;

    /* OAM */
    u8 oam_[NH_OAM_SIZE];

    // ---- External components references
    vmem_s *vmem_;

    placcessor_s plAccessor_;

    /* internal registers */
    // https://www.nesdev.org/wiki/PPU_scrolling#PPU_internal_registers
    u16 v_;
    u16 t_;
    u8 x_;
    u8 w_;

    plctx_s plctx_;
    pl_s pl_;

    frmbuf_s backbuf_;
    frmbuf_s frontbuf_;

    palette_s pal_;

    // The data bus used to communicate with CPU, to implement open bus
    // behavior
    // https://www.nesdev.org/wiki/PPU_registers#Ports
    // https://www.nesdev.org/wiki/Open_bus_behavior#PPU_open_bus
    u8 iodb_;

    // ---- temporaries for one tick
    bool noNmi_;

    /* debug */

    const NHDFlag *debugflags_;
    dbgpal_s palsnap_;
    dbgoam_s oamsnap_;
    dbgpattbl_s ptntblsnapL_;
    dbgpattbl_s ptntblsnapR_;
    unsigned char ptntblPalIdx_;

    NHLogger *logger_;
} ppu_s;

bool
ppu_Init(ppu_s *self, vmem_s *vmem, const NHDFlag *flags, NHLogger *logger);
void
ppu_Deinit(ppu_s *self);

void
ppu_Powerup(ppu_s *self);
void
ppu_Reset(ppu_s *self);

void
ppu_Tick(ppu_s *self, bool noNmi);

bool
ppu_Nmi(const ppu_s *self);

u8
ppu_ReadReg(ppu_s *self, ppureg_e reg);
void
ppu_WriteReg(ppu_s *self, ppureg_e reg, u8 val);

const frmbuf_s *
ppu_getFrame(const ppu_s *self);

/* debug */

const dbgpal_s *
ppu_dbgGetPal(const ppu_s *self);
const dbgoam_s *
ppu_dbgGetOam(const ppu_s *self);
const dbgpattbl_s *
ppu_dbgGetPtnTbl(const ppu_s *self, bool right);
/// @param idx [0, 7]
void
ppu_dbgSetPtnTblPal(ppu_s *self, unsigned char idx);

u8 *
ppu_regOf(ppu_s *self, ppureg_e reg);
u8
ppu_getReg(const ppu_s *self, ppureg_e reg);

#pragma once

#include "nesish/nesish.h"

#include "types.h"
#include "debug/dbgflags.h"

#include "cpu/cpu.h"
#include "memory/mmem.h"

#include "ppu/ppu.h"
#include "ppu/oamdma.h"
#include "memory/vmem.h"

#include "cartridge/cart.h"

#include "apu/apuclock.h"
#include "apu/apu.h"
#include "apu/dmcdma.h"

typedef struct frmbuf_s frmbuf_s;

// Values must be valid array index, see "ctrlregs_".
typedef enum ctrlreg_e {
    CR_4016,
    CR_4017,
    CR_SIZE,
} ctrlreg_e;

#define CONSOLE_CTRLSIZE 2

typedef struct console_s {
    cpu_s cpu_;
    mmem_s mmem_;

    ppu_s ppu_;
    oamdma_s oamdma_;
    vmem_s vmem_;

    apuclock_s apuclock_;
    apu_s apu_;
    dmcdma_s dmcdma_;

    cart_s cart_;

    u8 ctrlregs_[CR_SIZE];
    NHController *ctrls_[CONSOLE_CTRLSIZE]; // References

    NHLogger *logger_;

    NHDFlag debugflags_;

    double time_;
} console_s;

bool
console_Init(console_s *self, NHLogger *logger);
void
console_Deinit(console_s *self);

void
console_PlugCtrl(console_s *self, NHCtrlPort slot, NHController *ctrl);
void
console_UnplugCtrl(console_s *self, NHCtrlPort slot);

NHErr
console_InsertCart(console_s *self, const char *rompath);
void
console_RemoveCart(console_s *self);

void
console_Powerup(console_s *self);
void
console_Reset(console_s *self);

cycle_t
console_Advance(console_s *self, double delta);
/// @brief Advance 1 tick
/// @param cpuInstr If a CPU instruction has completed
/// @return If a new audio sample is available
bool
console_Tick(console_s *self, bool *cpuInstr);

const frmbuf_s *
console_GetFrm(const console_s *self);

int
console_GetSampleRate(const console_s *self);
/// @return Amplitude in range [0, 1]
double
console_GetSample(const console_s *self);

/* debug */

void
console_SetDebugOn(console_s *self, NHDFlag flag);
void
console_SetDebugOff(console_s *self, NHDFlag flag);

const dbgpal_s *
console_DbgGetPal(const console_s *self);
const dbgoam_s *
console_DbgGetOam(const console_s *self);
const dbgpattbl_s *
console_DbgGetPtnTbl(const console_s *self, bool right);
void
console_DbgSetPtnTblPal(console_s *self, NHDPaletteSet palset);

/* test */

cpu_s *
console_TestGetCpu(console_s *self);

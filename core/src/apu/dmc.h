#pragma once

#include "apu/divider.h"
#include "types.h"

typedef struct NHLogger NHLogger;

typedef struct dmcdma_s dmcdma_s;

typedef struct dmc_s {
    dmcdma_s *dmcdma_;

    bool irqEnabled_;
    bool loop_;
    addr_t sampleaddr_;
    addr_t samplelen_;

    divider_s timer_;
    u8 shift_;
    u8 bitsleft_;
    u8 level_;
    bool silence_;

    u8 samplebuf_;
    bool samplebufEmpty_;

    addr_t samplecur_;
    addr_t sampleBytesLeft_;

    bool irq_;

    NHLogger *logger_;
} dmc_s;

void
dmc_Init(dmc_s *self, dmcdma_s *dmcdma, NHLogger *logger);

/// @return Amplitude in range of [0, 127]
u8
dmc_Amp(const dmc_s *self);

bool
dmc_Irq(const dmc_s *self);

void
dmc_PutSample(dmc_s *self, addr_t sampleaddr, u8 sample);

/// @brief Tick this every APU cycle (2 CPU cycles)
void
dmc_TickTimer(dmc_s *self);

void
dmc_SetIrqEnabled(dmc_s *self, bool set);
void
dmc_SetLoop(dmc_s *self, bool set);
void
dmc_SetTimerReload(dmc_s *self, u8 index);
void
dmc_Load(dmc_s *self, u8 val);
void
dmc_SetSampleAddr(dmc_s *self, u8 sampleaddr);
void
dmc_SetSampleLen(dmc_s *self, u8 samplelen);
void
dmc_SetEnabled(dmc_s *self, bool set);
void
dmc_ClearIrq(dmc_s *self);
bool
dmc_BytesRemained(const dmc_s *self);

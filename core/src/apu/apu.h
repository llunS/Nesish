#pragma once

#include "types.h"
#include "apu/frmctr.h"
#include "apu/pulse.h"
#include "apu/tri.h"
#include "apu/noise.h"
#include "apu/dmc.h"
#include "nesish/nesish.h"

// https://www.nesdev.org/wiki/APU_registers
// Values must be valid array index, see "regs_".
// Values must correspond to address, see apu_Addr2Reg().
typedef enum apureg_e {
    /* clang-format off */
    AR_PULSE1_DUTY = 0, // DDLC NNNN / Duty, loop envelope/disable length counter, constant volume, envelope period/volume
    AR_PULSE1_SWEEP, // EPPP NSSS / Sweep unit: enabled, period, negative, shift count
    AR_PULSE1_TIMER_LOW,
    AR_PULSE1_LEN, // LLLL LHHH / Length counter load, timer high (also resets duty and starts envelope)

    AR_PULSE2_DUTY,
    AR_PULSE2_SWEEP,
    AR_PULSE2_TIMER_LOW,
    AR_PULSE2_LEN,

    AR_TRI_LINEAR, // CRRR RRRR / Length counter disable/linear counter control, linear counter reload value
    AR_TRI_UNUSED, // Placeholder
    AR_TRI_TIMER_LOW,
    AR_TRI_LEN, // LLLL LHHH / Length counter load, timer high (also reloads linear counter)

    AR_NOISE_ENVELOPE, // --LC NNNN / Loop envelope/disable length counter, constant volume, envelope period/volume
    AR_NOISE_UNUSED, // Placeholder
    AR_NOISE_PERIOD, // L--- PPPP / Loop noise, noise period
    AR_NOISE_LEN, // LLLL L--- / Length counter load (also starts envelope)

    AR_DMC_FREQ, // IL-- FFFF / IRQ enable, loop sample, frequency index
    AR_DMC_LOAD, // -DDD DDDD / Direct load
    AR_DMC_SPADDR, // Sample address %11AAAAAA.AA000000
    AR_DMC_SPLEN, // Sample length %0000LLLL.LLLL0001

    // Write: ---D NT21 / Control: DMC enable, length counter enables: noise, triangle, pulse 2, pulse 1
    // Read: IF-D NT21 / Status: DMC interrupt, frame interrupt, length counter status: noise, triangle, pulse 2, pulse 1
    AR_CTRL_STAT,

    AR_FC, // SD-- ---- / Frame counter: 5-frame sequence, disable frame interrupt

    AR_SIZE,
    /* clang-format on */
} apureg_e;

typedef struct apuclock_s apuclock_s;
typedef struct dmcdma_s dmcdma_s;

typedef struct apu_s {
    u8 regs_[AR_SIZE];

    frmctr_s fc_;
    pulse_s pulse1_;
    pulse_s pulse2_;
    tri_s tri_;
    noise_s noise_;
    dmc_s dmc_;

    const apuclock_s *clock_;
} apu_s;

void
apu_Init(apu_s *self, const apuclock_s *clock, dmcdma_s *dmcdma,
         NHLogger *logger);

void
apu_Powerup(apu_s *self);
void
apu_Reset(apu_s *self);

/// @brief Call this every CPU cycle
void
apu_Tick(apu_s *self);
/// @return Amplitude in range [0, 1]
double
apu_Amp(const apu_s *self);
bool
apu_Irq(const apu_s *self);

void
apu_PutDmcSample(apu_s *self, addr_t sampleaddr, u8 sample);

NHErr
apu_ReadReg(apu_s *self, apureg_e reg, u8 *val);
void
apu_WriteReg(apu_s *self, apureg_e reg, u8 val);

apureg_e
apu_Addr2Reg(addr_t addr);

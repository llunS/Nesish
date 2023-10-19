#include "apu.h"

#include <string.h>

#include "spec.h"
#include "apu/apuclock.h"

typedef struct mixerlut_s {
    double Pulse[31];
    double Tnd[203];
} mixerlut_s;
static mixerlut_s gMixerlut;

static void
initMixerLut(void);

static double
mix(u8 pulse1, u8 pulse2, u8 triangle, u8 noise, u8 dmc);

void
apu_Init(apu_s *self, const apuclock_s *clock, dmcdma_s *dmcdma,
         NHLogger *logger)
{
    initMixerLut();

    memset(self->regs_, 0, sizeof(self->regs_));

    frmctr_Init(&self->fc_, &self->pulse1_, &self->pulse2_, &self->tri_,
                &self->noise_);
    pulse_Init(&self->pulse1_, true, logger);
    pulse_Init(&self->pulse2_, false, logger);
    tri_Init(&self->tri_, logger);
    noise_Init(&self->noise_, logger);
    dmc_Init(&self->dmc_, dmcdma, logger);

    self->clock_ = clock;
}

void
apu_Powerup(apu_s *self)
{
    // @NOTE: These should be done before the following register writes since
    // register writes may further alter the states.
    {
        lenctr_Powerup(pulse_LenCtr(&self->pulse1_));
        lenctr_Powerup(pulse_LenCtr(&self->pulse2_));

        tri_Powerup(&self->tri_);

        noise_ResetLfsr(&self->noise_);
        lenctr_Powerup(noise_LenCtr(&self->noise_));

        frmctr_Powerup(&self->fc_);
    }

    // https://www.nesdev.org/wiki/CPU_power_up_state
    // We want the side effects applied, so we use apu_WriteReg().
    apu_WriteReg(self, AR_PULSE1_DUTY, 0x00);
    apu_WriteReg(self, AR_PULSE1_SWEEP, 0x00);
    apu_WriteReg(self, AR_PULSE1_TIMER_LOW, 0x00);
    apu_WriteReg(self, AR_PULSE1_LEN, 0x00);

    apu_WriteReg(self, AR_PULSE2_DUTY, 0x00);
    apu_WriteReg(self, AR_PULSE2_SWEEP, 0x00);
    apu_WriteReg(self, AR_PULSE2_TIMER_LOW, 0x00);
    apu_WriteReg(self, AR_PULSE2_LEN, 0x00);

    apu_WriteReg(self, AR_TRI_LINEAR, 0x00);
    apu_WriteReg(self, AR_TRI_TIMER_LOW, 0x00);
    apu_WriteReg(self, AR_TRI_LEN, 0x00);

    apu_WriteReg(self, AR_NOISE_ENVELOPE, 0x00);
    apu_WriteReg(self, AR_NOISE_PERIOD, 0x00);
    apu_WriteReg(self, AR_NOISE_LEN, 0x00);

    apu_WriteReg(self, AR_DMC_FREQ, 0x00);
    // DMC is loaded with 0 on power-up
    apu_WriteReg(self, AR_DMC_LOAD, 0x00);
    apu_WriteReg(self, AR_DMC_SPADDR, 0x00);
    apu_WriteReg(self, AR_DMC_SPLEN, 0x00);

    // all channels disabled
    apu_WriteReg(self, AR_CTRL_STAT, 0x00);

    // --- Frame counter
    // tests: apu_reset
    // frame irq enabled
    apu_WriteReg(self, AR_FC, 0x00);
    // @QUIRK: After reset or power-up, APU acts as if $4017 were written with
    // $00 from 9 to 12 clocks before first instruction begins.
    // Pick 10 to tick
    frmctr_ResetTimer(&self->fc_);
    for (int i = 0; i < 10; ++i)
    {
        frmctr_Tick(&self->fc_);
    }
}

void
apu_Reset(apu_s *self)
{
    // APU was silenced
    apu_WriteReg(self, AR_CTRL_STAT, 0x00);
    // APU triangle phase is reset to 0
    tri_Reset(&self->tri_);

    // APU DPCM output ANDed with 1 (upper 6 bits cleared)
    // Since we are not running in parallel, ignore this

    // --- Frame counter
    // tests: apu_reset
    apu_WriteReg(self, AR_FC, self->regs_[AR_FC]);
    // after the register write to ensure irq is cleared
    frmctr_ClearIrq(&self->fc_);
}

void
apu_Tick(apu_s *self)
{
    // Clock frame counter to apply parameter changes first.
    frmctr_Tick(&self->fc_);

    // -------- Unit post update, owing to tick order implementation
    // Changes to length counter halt occur after clocking length, i.e. via
    // frmctr_Tick(&self->fc_). So do this after frmctr_Tick(&self->fc_).
    lenctr_FlushHaltSet(pulse_LenCtr(&self->pulse1_));
    lenctr_FlushHaltSet(pulse_LenCtr(&self->pulse2_));
    lenctr_FlushHaltSet(tri_LenCtr(&self->tri_));
    lenctr_FlushHaltSet(noise_LenCtr(&self->noise_));
    // Write to length counter reload should be ignored when made during
    // length counter clocking and the length counter is not zero.
    // Length counter clocking is done in frmctr_Tick(&self->fc_), so do this
    // after it.
    lenctr_FlushLoadSet(pulse_LenCtr(&self->pulse1_));
    lenctr_FlushLoadSet(pulse_LenCtr(&self->pulse2_));
    lenctr_FlushLoadSet(tri_LenCtr(&self->tri_));
    lenctr_FlushLoadSet(noise_LenCtr(&self->noise_));

    // Every other CPU cycle
    if (apuclock_Odd(self->clock_))
    {
        pulse_TickTimer(&self->pulse1_);
        pulse_TickTimer(&self->pulse2_);
        noise_TickTimer(&self->noise_);
        dmc_TickTimer(&self->dmc_);
    }
    tri_TickTimer(&self->tri_);
}

double
apu_Amp(const apu_s *self)
{
    u8 pulse1 = pulse_Amp(&self->pulse1_);
    u8 pulse2 = pulse_Amp(&self->pulse2_);
    u8 triangle = tri_Amp(&self->tri_);
    u8 noise = noise_Amp(&self->noise_);
    u8 dmc = dmc_Amp(&self->dmc_);
    return mix(pulse1, pulse2, triangle, noise, dmc);
}

bool
apu_Irq(const apu_s *self)
{
    return frmctr_Irq(&self->fc_) || dmc_Irq(&self->dmc_);
}

void
apu_PutDmcSample(apu_s *self, addr_t sampleaddr, u8 sample)
{
    dmc_PutSample(&self->dmc_, sampleaddr, sample);
}

double
mix(u8 pulse1, u8 pulse2, u8 triangle, u8 noise, u8 dmc)
{
    // Efficient lookup table
    // https://www.nesdev.org/wiki/APU_Mixer

    // maximum 30 fits in
    u8 pulseidx = pulse1 + pulse2;
    double pulseout = gMixerlut.Pulse[pulseidx];

    // maximum 202 fits in
    u8 tndidx = 3 * triangle + 2 * noise + dmc;
    double tndout = gMixerlut.Tnd[tndidx];

    return pulseout + tndout;
}

NHErr
apu_ReadReg(apu_s *self, apureg_e reg, u8 *val)
{
    switch (reg)
    {
        case AR_CTRL_STAT:
        {
            // @TODO: If an interrupt flag was set at the same moment of the
            // read, it will read back as 1 but it will not be cleared.
            // However, it seems to contradict with "sync_apu" in test source,
            // not sure how we should do this.

            bool p1 = lenctr_Val(pulse_LenCtr(&self->pulse1_)) > 0;
            bool p2 = lenctr_Val(pulse_LenCtr(&self->pulse2_)) > 0;
            bool tri = lenctr_Val(tri_LenCtr(&self->tri_)) > 0;
            bool noise = lenctr_Val(noise_LenCtr(&self->noise_)) > 0;
            bool D = dmc_BytesRemained(&self->dmc_);
            bool F = frmctr_Irq(&self->fc_);
            frmctr_ClearIrq(&self->fc_);
            bool I = dmc_Irq(&self->dmc_);

            *val = (I << 7) | (F << 6) | (D << 4) | (noise << 3) | (tri << 2) |
                   (p2 << 1) | (p1 << 0);
            return NH_ERR_OK;
        }
        break;

        default:
            return NH_ERR_WRITE_ONLY;
            break;
    }
}

void
apu_WriteReg(apu_s *self, apureg_e reg, u8 val)
{
    self->regs_[reg] = val;

    switch (reg)
    {
        case AR_PULSE1_DUTY:
        case AR_PULSE2_DUTY:
        {
            pulse_s *pulse =
                AR_PULSE1_DUTY == reg ? &self->pulse1_ : &self->pulse2_;
            seq_SetDuty(pulse_Seq(pulse), val >> 6);
            envelope_SetLoop(pulse_Envelope(pulse), val & 0x20);
            lenctr_PostSetHalt(pulse_LenCtr(pulse), val & 0x20);
            envelope_SetConst(pulse_Envelope(pulse), val & 0x10);
            envelope_SetDividerReload(pulse_Envelope(pulse), val & 0x0F);
            envelope_SetConstVol(pulse_Envelope(pulse), val & 0x0F);
        }
        break;

        case AR_PULSE1_SWEEP:
        case AR_PULSE2_SWEEP:
        {
            pulse_s *pulse =
                AR_PULSE1_SWEEP == reg ? &self->pulse1_ : &self->pulse2_;
            sweep_SetEnabled(pulse_Sweep(pulse), val & 0x80);
            sweep_SetDividerReload(pulse_Sweep(pulse), (val >> 4) & 0x07);
            sweep_SetNegate(pulse_Sweep(pulse), val & 0x08);
            sweep_SetShiftCount(pulse_Sweep(pulse), val & 0x07);

            sweep_Reload(pulse_Sweep(pulse));
        }
        break;

        case AR_PULSE1_TIMER_LOW:
        case AR_PULSE2_TIMER_LOW:
        {
            pulse_s *pulse =
                AR_PULSE1_TIMER_LOW == reg ? &self->pulse1_ : &self->pulse2_;
            u8 timerhigh = AR_PULSE1_TIMER_LOW == reg
                               ? (self->regs_[AR_PULSE1_LEN] & 0x07)
                               : (self->regs_[AR_PULSE2_LEN] & 0x07);
            u16 timer = (timerhigh << 8) | val;
            divider_SetReload(pulse_Timer(pulse), timer);
        }
        break;

        case AR_PULSE1_LEN:
        case AR_PULSE2_LEN:
        {
            pulse_s *pulse =
                AR_PULSE1_LEN == reg ? &self->pulse1_ : &self->pulse2_;
            u8 timerlow = AR_PULSE1_LEN == reg
                              ? self->regs_[AR_PULSE1_TIMER_LOW]
                              : self->regs_[AR_PULSE2_TIMER_LOW];
            u16 timer = ((val & 0x07) << 8) | timerlow;
            divider_SetReload(pulse_Timer(pulse), timer);
            lenctr_PostSetLoad(pulse_LenCtr(pulse), val >> 3);

            seq_Reset(pulse_Seq(pulse));
            envelope_Restart(pulse_Envelope(pulse));
        }
        break;

        case AR_TRI_LINEAR:
        {
            linctr_SetCtrl(tri_LinCtr(&self->tri_), val & 0x80);
            // This bit is also the length counter halt flag
            lenctr_PostSetHalt(tri_LenCtr(&self->tri_), val & 0x80);
            linctr_SetReloadVal(tri_LinCtr(&self->tri_), val & 0x7F);
        }
        break;

        case AR_TRI_TIMER_LOW:
        {
            u16 timer = ((self->regs_[AR_TRI_LEN] & 0x07) << 8) | val;
            divider_SetReload(tri_Timer(&self->tri_), timer);
        }
        break;

        case AR_TRI_LEN:
        {
            u16 timer = ((val & 0x07) << 8) | self->regs_[AR_TRI_TIMER_LOW];
            divider_SetReload(tri_Timer(&self->tri_), timer);
            lenctr_PostSetLoad(tri_LenCtr(&self->tri_), val >> 3);

            linctr_SetReload(tri_LinCtr(&self->tri_));
        }
        break;

        case AR_NOISE_ENVELOPE:
        {
            envelope_SetLoop(noise_Envelope(&self->noise_), val & 0x20);
            lenctr_PostSetHalt(noise_LenCtr(&self->noise_), val & 0x20);
            envelope_SetConst(noise_Envelope(&self->noise_), val & 0x10);
            envelope_SetDividerReload(noise_Envelope(&self->noise_),
                                      val & 0x0F);
            envelope_SetConstVol(noise_Envelope(&self->noise_), val & 0x0F);
        }
        break;

        case AR_NOISE_PERIOD:
        {
            noise_SetMode(&self->noise_, val & 0x80);
            noise_SetTimerReload(&self->noise_, val & 0x0F);
        }
        break;

        case AR_NOISE_LEN:
        {
            lenctr_PostSetLoad(noise_LenCtr(&self->noise_), val >> 3);

            envelope_Restart(noise_Envelope(&self->noise_));
        }
        break;

        case AR_DMC_FREQ:
        {
            dmc_SetIrqEnabled(&self->dmc_, val & 0x80);
            dmc_SetLoop(&self->dmc_, val & 0x40);
            dmc_SetTimerReload(&self->dmc_, val & 0x0F);
        }
        break;

        case AR_DMC_LOAD:
        {
            dmc_Load(&self->dmc_, val & 0x7F);
        }
        break;

        case AR_DMC_SPADDR:
        {
            dmc_SetSampleAddr(&self->dmc_, val);
        }
        break;

        case AR_DMC_SPLEN:
        {
            dmc_SetSampleLen(&self->dmc_, val);
        }
        break;

        case AR_CTRL_STAT:
        {
            lenctr_SetEnabled(pulse_LenCtr(&self->pulse1_), val & 0x01);
            lenctr_SetEnabled(pulse_LenCtr(&self->pulse2_), val & 0x02);
            lenctr_SetEnabled(tri_LenCtr(&self->tri_), val & 0x04);
            lenctr_SetEnabled(noise_LenCtr(&self->noise_), val & 0x08);
            dmc_SetEnabled(&self->dmc_, val & 0x10);

            dmc_ClearIrq(&self->dmc_);
        }
        break;

        case AR_FC:
        {
            // The theory that delay one when written on even cycle is backed by
            // https://www.nesdev.org/wiki/APU_Frame_Counter and test
            // cpu_interrupts_v2/4-irq_and_dma.nes
            bool delay1 = apuclock_Even(self->clock_); // current tick is even
            frmctr_DelaySetMode(&self->fc_, val & 0x80, delay1);
            frmctr_SetIrqInhibit(&self->fc_, val & 0x40);
        }
        break;

        default:
            break;
    }
}

apureg_e
apu_Addr2Reg(addr_t addr)
{
    if (NH_APU_FC_ADDR == addr)
    {
        return AR_FC;
    }
    else if (NH_APU_STATUS_ADDR == addr)
    {
        return AR_CTRL_STAT;
    }
    else if (NH_APU_PULSE1_DUTY_ADDR <= addr &&
             addr <= NH_APU_DMC_SAMPLE_LENGTH_ADDR)
    {
        return (apureg_e)(addr - NH_APU_PULSE1_DUTY_ADDR + AR_PULSE1_DUTY);
    }
    else
    {
        return AR_SIZE;
    }
}

void
initMixerLut(void)
{
    static bool inited = false;
    if (!inited)
    {
        gMixerlut.Pulse[0] = 0.0;
        for (int i = 1; i < 31; ++i)
        {
            gMixerlut.Pulse[i] = 95.52 / (8128.0 / i + 100.0);
        }
        gMixerlut.Tnd[0] = 0.0;
        for (int i = 1; i < 203; ++i)
        {
            gMixerlut.Tnd[i] = 163.67 / (24329.0 / i + 100.0);
        }

        inited = true;
    }
}

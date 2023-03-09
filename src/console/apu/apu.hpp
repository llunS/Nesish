#pragma once

#include "common/klass.hpp"
#include "common/error.hpp"

#include "console/types.hpp"
#include "console/apu/frame_counter.hpp"
#include "console/apu/pulse.hpp"
#include "console/apu/triangle.hpp"
#include "console/apu/noise.hpp"
#include "console/apu/dmc.hpp"

namespace nh {

struct APUClock;
struct DMCDMA;

struct APU {
  public:
    APU(const APUClock &i_clock, DMCDMA &o_dmc_dma);
    ~APU() = default;
    LN_KLZ_DELETE_COPY_MOVE(APU);

  public:
    void
    power_up();
    void
    reset();

    /// @brief Call this every CPU cycle
    void
    tick();
    /// @return Amplitude in range [0, 1]
    float
    amplitude() const;
    bool
    interrupt() const;

    void
    put_dmc_sample(Address i_sample_addr, Byte i_sample);

  public:
    // https://www.nesdev.org/wiki/APU_registers
    // @NOTE: Values must be valid array index, see "m_regs".
    // @NOTE: Values must correspond to address, see addr_to_regsiter().
    enum Register {
        /* clang-format off */
        PULSE1_DUTY = 0, // DDLC NNNN / Duty, loop envelope/disable length counter, constant volume, envelope period/volume
        PULSE1_SWEEP, // EPPP NSSS / Sweep unit: enabled, period, negative, shift count
        PULSE1_TIMER_LOW,
        PULSE1_LENGTH, // LLLL LHHH / Length counter load, timer high (also resets duty and starts envelope)

        PULSE2_DUTY,
        PULSE2_SWEEP,
        PULSE2_TIMER_LOW,
        PULSE2_LENGTH,

        TRI_LINEAR, // CRRR RRRR / Length counter disable/linear counter control, linear counter reload value
        TRI_UNUSED, // Placeholder
        TRI_TIMER_LOW,
        TRI_LENGTH, // LLLL LHHH / Length counter load, timer high (also reloads linear counter)

        NOISE_ENVELOPE, // --LC NNNN / Loop envelope/disable length counter, constant volume, envelope period/volume
        NOISE_UNUSED, // Placeholder
        NOISE_PERIOD, // L--- PPPP / Loop noise, noise period
        NOISE_LENGTH, // LLLL L--- / Length counter load (also starts envelope)

        DMC_FREQUENCY, // IL-- FFFF / IRQ enable, loop sample, frequency index
        DMC_LOAD, // -DDD DDDD / Direct load
        DMC_SAMPLE_ADDR, // Sample address %11AAAAAA.AA000000
        DMC_SAMPLE_LENGTH, // Sample length %0000LLLL.LLLL0001

        // Write: ---D NT21 / Control: DMC enable, length counter enables: noise, triangle, pulse 2, pulse 1
        // Read: IF-D NT21 / Status: DMC interrupt, frame interrupt, length counter status: noise, triangle, pulse 2, pulse 1
        CTRL_STATUS,

        FC, // SD-- ---- / Frame counter: 5-frame sequence, disable frame interrupt

        SIZE,
        /* clang-format on */
    };

    Error
    read_register(Register i_reg, Byte &o_val);
    void
    write_register(Register i_reg, Byte i_val);

    static Register
    addr_to_regsiter(Address i_addr);

  private:
    static float
    mix(Byte i_pulse1, Byte i_pulse2, Byte i_triangle, Byte i_noise,
        Byte i_dmc);

  private:
    Byte m_regs[Register::SIZE];

    FrameCounter m_fc;
    Pulse m_pulse1;
    Pulse m_pulse2;
    Triangle m_triangle;
    Noise m_noise;
    DMC m_dmc;

    const APUClock &m_clock;
};

} // namespace nh

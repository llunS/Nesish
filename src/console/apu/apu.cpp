#include "apu.hpp"

#include "console/spec.hpp"
#include "console/apu/apu_clock.hpp"

namespace ln {

APU::APU(const APUClock &i_clock, DMCDMA &o_dmc_dma)
    : m_regs{}
    , m_fc(m_pulse1, m_pulse2, m_triangle, m_noise)
    , m_pulse1(true)
    , m_pulse2(false)
    , m_dmc(o_dmc_dma)
    , m_clock(i_clock)
{
}

void
APU::power_up()
{
    // @NOTE: These should be done before the following register writes since
    // register writes may further alter the states.
    {
        m_pulse1.length_counter().power_up();
        m_pulse2.length_counter().power_up();
        m_triangle.length_counter().power_up();
        m_noise.length_counter().power_up();

        m_fc.power_up();
    }

    // https://www.nesdev.org/wiki/CPU_power_up_state
    // @IMPL: We want all the side effects applied, so we use write_register().
    write_register(PULSE1_DUTY, 0x00);
    write_register(PULSE1_SWEEP, 0x00);
    write_register(PULSE1_TIMER_LOW, 0x00);
    write_register(PULSE1_LENGTH, 0x00);

    write_register(PULSE2_DUTY, 0x00);
    write_register(PULSE2_SWEEP, 0x00);
    write_register(PULSE2_TIMER_LOW, 0x00);
    write_register(PULSE2_LENGTH, 0x00);

    write_register(TRI_LINEAR, 0x00);
    write_register(TRI_TIMER_LOW, 0x00);
    write_register(TRI_LENGTH, 0x00);

    write_register(NOISE_ENVELOPE, 0x00);
    write_register(NOISE_PERIOD, 0x00);
    write_register(NOISE_LENGTH, 0x00);

    write_register(DMC_FREQUENCY, 0x00);
    write_register(DMC_LOAD, 0x00);
    write_register(DMC_SAMPLE_ADDR, 0x00);
    write_register(DMC_SAMPLE_LENGTH, 0x00);

    // all channels disabled
    write_register(CTRL_STATUS, 0x00);

    // frame irq enabled
    write_register(FC, 0x00);
    // 2A03G: APU Frame Counter reset. (but 2A03letterless: APU frame
    // counter powers up at a value equivalent to 15)
    // We want 2A03 behavior, but the value being referred to means exactly
    // which storage is ambiguous, so we reset it to 0.
    m_fc.reset_timer();
    // @QUIRK: After reset or power-up, APU acts as if $4017 were written with
    // $00 from 9 to 12 clocks before first instruction begins.
    // Pick 10 to tick
    for (int i = 0; i < 10; ++i)
    {
        m_fc.tick();
    }

    // All 15 bits of noise channel LFSR = $0000.
    // The first time the LFSR is clocked from the all-0s state, it will shift
    // in a 1
    // (how is bit 1 shifted in? all zeros should shift in a 0).
    m_noise.reset_lfsr();

    // https://www.nesdev.org/wiki/APU_DMC#Overview
    m_dmc.load(0);
}

void
APU::reset()
{
    // APU was silenced
    write_register(CTRL_STATUS, 0x00);
    // APU triangle phase is reset to 0
    m_triangle.reset();

    // APU DPCM output ANDed with 1 (upper 6 bits cleared)
    // Since we are not running in parallel, ignore this

    // 2A03G: APU Frame Counter reset. (but 2A03letterless: APU frame
    // counter retains old value)
}

void
APU::tick()
{
    // Clock frame counter to apply parameter changes first.
    m_fc.tick();

    // -------- Data flush/update due to tick order implementation
    // Changes to length counter halt occur after clocking length, i.e. via
    // m_fc.tick(). So do this after m_fc.tick().
    m_pulse1.length_counter().flush_halt_set();
    m_pulse2.length_counter().flush_halt_set();
    m_triangle.length_counter().flush_halt_set();
    m_noise.length_counter().flush_halt_set();
    // Write to length counter reload should be ignored when made during
    // length counter clocking and the length counter is not zero.
    // Length counter clocking is done in m_fc.tick(), so do this after it.
    m_pulse1.length_counter().flush_load_set();
    m_pulse2.length_counter().flush_load_set();
    m_triangle.length_counter().flush_load_set();
    m_noise.length_counter().flush_load_set();

    // Every other CPU cycle
    if (m_clock.odd())
    {
        m_pulse1.tick_timer();
        m_pulse2.tick_timer();
        m_noise.tick_timer();
        m_dmc.tick_timer();
    }
    m_triangle.tick_timer();
}

float
APU::amplitude() const
{
    // @TODO: Other channels
    Byte pulse1 = m_pulse1.amplitude();
    Byte pulse2 = m_pulse2.amplitude();
    Byte triangle = m_triangle.amplitude();
    Byte noise = m_noise.amplitude();
    Byte dmc = m_dmc.amplitude();
    return mix(pulse1, pulse2, triangle, noise, dmc);
}

bool
APU::interrupt() const
{
    return m_fc.interrupt() || m_dmc.interrupt();
}

void
APU::put_dmc_sample(Address i_sample_addr, Byte i_sample)
{
    m_dmc.put_sample(i_sample_addr, i_sample);
}

float
APU::mix(Byte i_pulse1, Byte i_pulse2, Byte i_triangle, Byte i_noise,
         Byte i_dmc)
{
    // @TODO: Static lookup table.
    // https://www.nesdev.org/wiki/APU_Mixer
    float pulse_out = 0.0;
    if (i_pulse1 || i_pulse2)
    {
        pulse_out = float(95.88 / (8128. / (i_pulse1 + i_pulse2) + 100.));
    }
    else
    {
        // 0
    }

    float tnd_out = 0.0;
    if (i_triangle || i_noise || i_dmc)
    {
        tnd_out = float(159.79 / (1. / (i_triangle / 8227. + i_noise / 12241. +
                                        i_dmc / 22638.) +
                                  100.));
    }
    else
    {
        // 0
    }

    float output = pulse_out + tnd_out;
    return output;
}

Error
APU::read_register(Register i_reg, Byte &o_val)
{
    switch (i_reg)
    {
        case CTRL_STATUS:
        {
            // @IMPL: If an interrupt flag was set at the same moment of the
            // read, it will read back as 1 but it will not be cleared. We don't
            // emulate this, since we don't actually run cpu and apu in
            // parallel.

            // @TODO: Other channels.
            bool p1 = m_pulse1.length_counter().value() > 0;
            bool p2 = m_pulse2.length_counter().value() > 0;
            bool tri = m_triangle.length_counter().value() > 0;
            bool noise = m_noise.length_counter().value() > 0;
            bool D = m_dmc.bytes_remained();
            // @TODO: If an interrupt flag was set at the same moment of the
            // read, it will read back as 1 but it will not be cleared.
            // @NOTE: It seems to contradict with "sync_apu" in test source
            bool F = m_fc.interrupt();
            m_fc.clear_interrupt();
            bool I = m_dmc.interrupt();

            o_val = (I << 7) | (F << 6) | (D << 4) | (noise << 3) | (tri << 2) |
                    (p2 << 1) | (p1 << 0);
            return Error::OK;
        }
        break;

        default:
            return Error::WRITE_ONLY;
            break;
    }
}

void
APU::write_register(Register i_reg, Byte i_val)
{
    m_regs[i_reg] = i_val;

    // @TODO: Other channels.

    switch (i_reg)
    {
        case PULSE1_DUTY:
        case PULSE2_DUTY:
        {
            Pulse *pulse = PULSE1_DUTY == i_reg ? &m_pulse1 : &m_pulse2;
            pulse->sequencer().set_duty(i_val >> 6);
            pulse->envelope().set_loop(i_val & 0x20);
            // @NOTE: Post set for tick order implmentation
            pulse->length_counter().post_set_halt(i_val & 0x20);
            pulse->envelope().set_const(i_val & 0x10);
            pulse->envelope().set_divider_reload(i_val & 0x0F);
            pulse->envelope().set_const_vol(i_val & 0x0F);
        }
        break;

        case PULSE1_SWEEP:
        case PULSE2_SWEEP:
        {
            Pulse *pulse = PULSE1_SWEEP == i_reg ? &m_pulse1 : &m_pulse2;
            pulse->sweep().set_enabled(i_val & 0x80);
            pulse->sweep().set_divider_reload((i_val >> 4) & 0x07);
            pulse->sweep().set_negate(i_val & 0x08);
            pulse->sweep().set_shift_count(i_val & 0x07);

            pulse->sweep().reload();
        }
        break;

        case PULSE1_TIMER_LOW:
        case PULSE2_TIMER_LOW:
        {
            Pulse *pulse = PULSE1_TIMER_LOW == i_reg ? &m_pulse1 : &m_pulse2;
            Byte timer_high = PULSE1_TIMER_LOW == i_reg
                                  ? (m_regs[PULSE1_LENGTH] & 0x07)
                                  : (m_regs[PULSE2_LENGTH] & 0x07);
            Byte2 timer = (timer_high << 8) | i_val;
            pulse->timer().set_reload(timer);
        }
        break;

        case PULSE1_LENGTH:
        case PULSE2_LENGTH:
        {
            Pulse *pulse = PULSE1_LENGTH == i_reg ? &m_pulse1 : &m_pulse2;
            Byte timer_low = PULSE1_LENGTH == i_reg ? m_regs[PULSE1_TIMER_LOW]
                                                    : m_regs[PULSE2_TIMER_LOW];
            Byte2 timer = ((i_val & 0x07) << 8) | timer_low;
            pulse->timer().set_reload(timer);
            // @NOTE: Post set for tick order implmentation
            pulse->length_counter().post_set_load(i_val >> 3);

            pulse->sequencer().reset();
            pulse->envelope().restart();
        }
        break;

        case TRI_LINEAR:
        {
            m_triangle.linear_counter().set_control(i_val & 0x80);
            // This bit is also the length counter halt flag
            // @NOTE: Post set for tick order implmentation
            m_triangle.length_counter().post_set_halt(i_val & 0x80);
            m_triangle.linear_counter().set_reload_val(i_val & 0x7F);
        }
        break;

        case TRI_TIMER_LOW:
        {
            Byte2 timer = ((m_regs[TRI_LENGTH] & 0x07) << 8) | i_val;
            m_triangle.timer().set_reload(timer);
        }
        break;

        case TRI_LENGTH:
        {
            Byte2 timer = ((i_val & 0x07) << 8) | m_regs[TRI_TIMER_LOW];
            m_triangle.timer().set_reload(timer);
            // @NOTE: Post set for tick order implmentation
            m_triangle.length_counter().post_set_load(i_val >> 3);

            m_triangle.linear_counter().set_reload();
        }
        break;

        case NOISE_ENVELOPE:
        {
            m_noise.envelope().set_loop(i_val & 0x20);
            // @NOTE: Post set for tick order implmentation
            m_noise.length_counter().post_set_halt(i_val & 0x20);
            m_noise.envelope().set_const(i_val & 0x10);
            m_noise.envelope().set_divider_reload(i_val & 0x0F);
            m_noise.envelope().set_const_vol(i_val & 0x0F);
        }
        break;

        case NOISE_PERIOD:
        {
            m_noise.set_mode(i_val & 0x80);
            m_noise.set_timer_reload(i_val & 0x0F);
        }
        break;

        case NOISE_LENGTH:
        {
            // @NOTE: Post set for tick order implmentation
            m_noise.length_counter().post_set_load(i_val >> 3);

            m_noise.envelope().restart();
        }
        break;

        case DMC_FREQUENCY:
        {
            m_dmc.set_interrupt_enabled(i_val & 0x80);
            m_dmc.set_loop(i_val & 0x40);
            m_dmc.set_timer_reload(i_val & 0x0F);
        }
        break;

        case DMC_LOAD:
        {
            m_dmc.load(i_val & 0x7F);
        }
        break;

        case DMC_SAMPLE_ADDR:
        {
            m_dmc.set_sample_addr(i_val);
        }
        break;

        case DMC_SAMPLE_LENGTH:
        {
            m_dmc.set_sample_length(i_val);
        }
        break;

        case CTRL_STATUS:
        {
            // @TODO: Other channels.
            m_pulse1.length_counter().set_enabled(i_val & 0x01);
            m_pulse2.length_counter().set_enabled(i_val & 0x02);
            m_triangle.length_counter().set_enabled(i_val & 0x04);
            m_noise.length_counter().set_enabled(i_val & 0x08);
            m_dmc.set_enabled(i_val & 0x10);

            m_dmc.clear_interrupt();
        }
        break;

        case FC:
        {
            // The theory that delay one when written on even cycle is backed by
            // https://www.nesdev.org/wiki/APU_Frame_Counter and test
            // cpu_interrupts_v2/4-irq_and_dma.nes
            bool delay1 = m_clock.even(); // current tick is even
            m_fc.delay_set_mode(i_val & 0x80, delay1);
            m_fc.set_irq_inhibit(i_val & 0x40);
        }
        break;

        default:
            break;
    }
}

auto
APU::addr_to_regsiter(Address i_addr) -> Register
{
    if (LN_APU_FC_ADDR == i_addr)
    {
        return FC;
    }
    else if (LN_APU_STATUS_ADDR == i_addr)
    {
        return CTRL_STATUS;
    }
    else if (LN_APU_PULSE1_DUTY_ADDR <= i_addr &&
             i_addr <= LN_APU_DMC_SAMPLE_LENGTH_ADDR)
    {
        return Register(i_addr - LN_APU_PULSE1_DUTY_ADDR + PULSE1_DUTY);
    }
    else
    {
        return Register::SIZE;
    }
}

} // namespace ln

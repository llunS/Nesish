#include "apu.hpp"

#include "console/spec.hpp"

namespace ln {

APU::APU()
    : m_regs{}
    , m_fc(m_pulse1, m_pulse2, m_triangle, m_noise)
    , m_divider_cpu2(1)
    , m_pulse1(true)
    , m_pulse2(false)
{
}

void
APU::power_up()
{
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

    write_register(CTRL_STATUS, 0x00);
    write_register(FC, 0x00);

    // All 15 bits of noise channel LFSR = $0000. The first time the LFSR
    // is clocked from the all-0s state, it will shift in a 1.
    m_noise.reset_lfsr();

    // @QUIRK: 2A03G: APU Frame Counter reset. (but 2A03letterless: APU frame
    // counter powers up at a value equivalent to 15)
    m_fc.reset();
}

void
APU::reset()
{
    write_register(CTRL_STATUS, 0x00);
    // @TODO: APU triangle phase is reset to 0 (i.e. outputs a value of 15, the
    // first step of its waveform)
    // @TODO: APU DPCM output ANDed with 1 (upper 6 bits cleared)
    // @QUIRK: 2A03G: APU Frame Counter reset. (but 2A03letterless: APU frame
    // counter retains old value)
    m_fc.reset();
}

void
APU::tick()
{
    // Clock frame counter to apply parameter changes first.
    m_fc.tick();

    // Every second CPU cycle
    if (m_divider_cpu2.tick())
    {
        m_pulse1.tick_timer();
        m_pulse2.tick_timer();
        m_noise.tick_timer();
    }
    m_triangle.tick_timer();

    // @TODO: Other channels
}

float
APU::amplitude() const
{
    // @TODO: Other channels
    Byte pulse1 = m_pulse1.amplitude();
    Byte pulse2 = m_pulse2.amplitude();
    Byte triangle = m_triangle.amplitude();
    Byte noise = m_noise.amplitude();
    return mix(pulse1, pulse2, triangle, noise, 0.0);
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
        pulse_out = 95.88 / (8128. / (i_pulse1 + i_pulse2) + 100.);
    }
    else
    {
        // 0
    }

    float tnd_out = 0.0;
    if (i_triangle || i_noise || i_dmc)
    {
        tnd_out =
            159.79 /
            (1. / (i_triangle / 8227. + i_noise / 12241. + i_dmc / 22638.) +
             100.);
    }
    else
    {
        // 0
    }

    float output = pulse_out + tnd_out;
    return output;
}

Byte
APU::read_register(Register i_reg)
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
            bool frame_irq = m_fc.interrupt();
            m_fc.clear_interrupt();

            return (frame_irq << 6) | (noise << 3) | (tri << 2) | (p2 << 1) |
                   (p1 << 0);
        }
        break;

        default:
            break;
    }
    return 0xFF; // distinct error
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
            pulse->length_counter().set_halt(i_val & 0x20);
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
            pulse->length_counter().check_load(i_val >> 3);

            pulse->sequencer().reset();
            pulse->envelope().restart();
        }
        break;

        case TRI_LINEAR:
        {
            m_triangle.linear_counter().set_control(i_val & 0x80);
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
            m_triangle.length_counter().check_load(i_val >> 3);

            m_triangle.linear_counter().set_reload();
        }
        break;

        case NOISE_ENVELOPE:
        {
            m_noise.envelope().set_loop(i_val & 0x20);
            m_noise.length_counter().set_halt(i_val & 0x20);
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
            m_noise.length_counter().check_load(i_val >> 3);

            m_noise.envelope().restart();
        }
        break;

        case CTRL_STATUS:
        {
            // @TODO: Other channels.
            m_pulse1.length_counter().set_enabled(i_val & 0x01);
            m_pulse2.length_counter().set_enabled(i_val & 0x02);
            m_triangle.length_counter().set_enabled(i_val & 0x04);
            m_noise.length_counter().set_enabled(i_val & 0x08);
        }
        break;

        case FC:
        {
            m_fc.set_mode(i_val & 0x80);
            m_fc.set_irq_inhibit(i_val & 0x40);
        }
        break;

        default:
            // @TODO: Other channels.
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

#include "sp_eval_fetch.hpp"

#include "console/spec.hpp"
#include "console/ppu/pipeline_accessor.hpp"
#include "console/assert.hpp"
#include "console/byte_utils.hpp"

#include <cstring>

namespace ln {

static constexpr int SEC_OAM_CLEAR_CYCLES = 64;

SpEvalFetch::SpEvalFetch(PipelineAccessor *io_accessor, bool i_sp_fetch_only)
    : Tickable(LN_SCANLINE_CYCLES)
    , m_accessor(io_accessor)
    , m_sp_fetch_only(i_sp_fetch_only)
    , m_sec_oam_clear(SEC_OAM_CLEAR_CYCLES,
                      std::bind(pvt_sec_oam_clear, std::placeholders::_1,
                                std::placeholders::_2, io_accessor))
    , m_sp_eval(192, std::bind(pvt_sp_eval, std::placeholders::_1,
                               std::placeholders::_2, io_accessor, &m_ctx))
    , m_sp_fetch_reload(8,
                        std::bind(pvt_sp_fetch_reload, std::placeholders::_1,
                                  std::placeholders::_2, io_accessor, &m_ctx))
{
    m_sec_oam_clear.set_done();
    m_sp_eval.set_done();
    m_sp_fetch_reload.set_done();
}

void
SpEvalFetch::reset()
{
    Tickable::reset();

    m_sec_oam_clear.set_done();
    m_sp_eval.set_done();
    m_sp_fetch_reload.set_done();
}

Cycle
SpEvalFetch::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_total);

    switch (i_curr)
    {
        case 1:
        {
            if (!m_sp_fetch_only)
            {
                m_sec_oam_clear.reset();
            }
        }
        break;

        case 65:
        {
            if (!m_sp_fetch_only && m_accessor->rendering_enabled())
            {
                m_sp_eval.reset();

                Byte oam_addr = m_accessor->get_register(PPU::OAMADDR);
                m_ctx.sec_oam_write_idx = 0;
                m_ctx.cp_counter = 0;
                m_ctx.init_oam_addr = oam_addr;
                m_ctx.n = m_ctx.m = 0;
                m_ctx.n_overflow = false;
                m_ctx.sp_got = 0;
                m_ctx.sp_overflow = false;

                // @QUIRK: Minor corruption
                // https://www.nesdev.org/wiki/PPU_sprite_evaluation#Notes
                Byte oam_cpy_addr = oam_addr & 0xF8;
                if (oam_cpy_addr)
                {
                    // @IMPL: memcpy is ok, no memmove needed.
                    std::memcpy(m_accessor->get_oam_addr(0),
                                m_accessor->get_oam_addr(oam_cpy_addr), 8);
                }
            }
        }
        break;

        case 257:
        {
            m_ctx.sec_oam_read_idx = 0;
        }
        // fall through
        case 265:
        case 273:
        case 281:
        case 289:
        case 297:
        case 305:
        case 313:
        {
            // 8 cycles each for 8 sprites.
            m_sp_fetch_reload.reset();
        }
        break;

#if 1
// Cycles 321-340+0: Read the first byte in secondary OAM (while the PPU fetches
// the first two background tiles for the next scanline)
// We don't emulate it.
#endif

        default:
            break;
    }
    m_sec_oam_clear.tick();
    m_sp_eval.tick();
    m_sp_fetch_reload.tick();

    /* do some checks */
    switch (i_curr)
    {
        case 320:
        {
            if (m_ctx.sec_oam_read_idx != LN_SEC_OAM_SIZE)
            {
                LN_ASSERT_FATAL("Wrong secondary OAM read index: {}",
                                m_ctx.sec_oam_read_idx);
            }
        }
        break;

        default:
            break;
    }

    return 1;
}

Cycle
SpEvalFetch::pvt_sec_oam_clear(Cycle i_curr, Cycle i_total,
                               PipelineAccessor *io_accessor)
{
    (void)(i_total);

    static_assert(SEC_OAM_CLEAR_CYCLES ==
                      2 * sizeof(io_accessor->get_context().sec_oam),
                  "Wrong timing for secondary OAM clear.");
    if (i_curr % 2 == 0)
    {
        // read from OAMDATA, we don't emulate it.
    }
    else
    {
        io_accessor->get_context().sec_oam[i_curr / 2] = 0xFF;
    }

    return 1;
}

Cycle
SpEvalFetch::pvt_sp_eval(Cycle i_curr, Cycle i_total,
                         PipelineAccessor *io_accessor, Context *io_ctx)
{
    (void)(i_total);

    auto got_8_sp = [](Context *io_ctx) -> bool { return io_ctx->sp_got >= 8; };
    auto write_disabled = [&got_8_sp](Context *io_ctx) -> bool {
        return io_ctx->n_overflow || got_8_sp(io_ctx);
    };

    auto add_read = [](PipelineAccessor *io_accessor, Context *io_ctx,
                       bool i_inc_n, bool i_inc_m, bool i_carry) {
        bool carry_n = false;
        if (i_inc_m)
        {
            ++io_ctx->m;
            if (io_ctx->m >= LN_OAM_SP_SIZE)
            {
                io_ctx->m = 0;

                if (i_carry)
                {
                    carry_n = true;
                }
            }
        }
        io_ctx->n +=
            decltype(io_ctx->n)(i_inc_n) + decltype(io_ctx->n)(carry_n);
        if (io_ctx->n >= LN_MAX_SP_NUM)
        {
            io_ctx->n %= LN_MAX_SP_NUM;

            // all 64 are checked
            io_ctx->n_overflow = true;
        }

        int new_read_addr =
            io_ctx->init_oam_addr + (io_ctx->n * LN_OAM_SP_SIZE + io_ctx->m);
        io_accessor->get_register(PPU::OAMADDR) = Byte(new_read_addr);
    };

    auto write_sec_oam = [&write_disabled](PipelineAccessor *io_accessor,
                                           Context *io_ctx, Byte i_val,
                                           bool i_inc) -> void {
        if (write_disabled(io_ctx))
        {
            return;
        }

        io_accessor->get_context().sec_oam[io_ctx->sec_oam_write_idx] = i_val;
        if (i_inc)
        {
            ++io_ctx->sec_oam_write_idx;

            // got 1 sprite
            if (io_ctx->sec_oam_write_idx % LN_OAM_SP_SIZE == 0)
            {
                ++io_ctx->sp_got;
            }
        }
    };
    auto inc_write = [&write_disabled](Context *io_ctx) -> void {
        if (write_disabled(io_ctx))
        {
            return;
        }

        ++io_ctx->sec_oam_write_idx;
    };

    // @TODO: hide sprites before OAMADDR
    // https://www.nesdev.org/wiki/PPU_registers#Values_during_rendering

    // read cycle
    if (i_curr % 2 == 0)
    {
        Byte oam_addr = io_accessor->get_register(PPU::OAMADDR);
        io_ctx->sp_eval_bus = io_accessor->get_oam(oam_addr);
    }
    // write cycle
    else
    {
        // Y
        if (!io_ctx->cp_counter)
        {
            const Byte &y = io_ctx->sp_eval_bus;

            // copy Y to secondary OAM
            write_sec_oam(io_accessor, io_ctx, y, false);

            if (io_ctx->n_overflow || io_ctx->sp_overflow)
            {
                // try next Y
                add_read(io_accessor, io_ctx, true, false, true);
            }
            else
            {
                // @TODO: 8x16 mode
                bool in_range =
                    (y <= io_accessor->get_context().scanline_no &&
                     io_accessor->get_context().scanline_no < y + 8);
                if (in_range)
                {
                    // copy the rest
                    inc_write(io_ctx);
                    io_ctx->cp_counter = LN_OAM_SP_SIZE - 1;

                    add_read(io_accessor, io_ctx, false, true, true);

                    if (got_8_sp(io_ctx))
                    {
                        // set sprite overflow flag
                        io_ctx->sp_overflow = true;
                        io_accessor->get_register(PPU::PPUSTATUS) |= 0x20;
                    }
                }
                else
                {
                    // try next Y
                    if (!got_8_sp(io_ctx))
                    {
                        add_read(io_accessor, io_ctx, true, false, true);
                    }
                    else
                    {
                        // @QUIRK: Sprite overflow bug
                        // https://www.nesdev.org/wiki/PPU_sprite_evaluation#Sprite_overflow_bug
                        add_read(io_accessor, io_ctx, true, true, false);
                    }
                }
            }
        }
        // rest 3
        else
        {
            auto prev_got_8 = got_8_sp(io_ctx);
            write_sec_oam(io_accessor, io_ctx, io_ctx->sp_eval_bus, true);
            if (prev_got_8 != got_8_sp(io_ctx))
            {
                if (!got_8_sp(io_ctx) || io_ctx->cp_counter != 1)
                {
                    LN_ASSERT_FATAL(
                        "Wrong sprite evaluation timing in copying rest: {}",
                        io_ctx->cp_counter);
                }
            }

            add_read(io_accessor, io_ctx, false, true, true);

            --io_ctx->cp_counter;
        }
    }

    return 1;
}

Cycle
SpEvalFetch::pvt_sp_fetch_reload(Cycle i_curr, Cycle i_total,
                                 PipelineAccessor *io_accessor, Context *io_ctx)
{
    (void)(i_total);

    auto read_sec_oam = [](PipelineAccessor *io_accessor,
                           Context *io_ctx) -> Byte {
        return io_accessor->get_context().sec_oam[io_ctx->sec_oam_read_idx++];
    };
    /* fetch pattern table tile byte */
    auto get_pattern_sliver = [](PipelineAccessor *io_accessor, Context *io_ctx,
                                 Byte i_sp_idx, bool i_upper) -> Byte {
        // https://www.nesdev.org/wiki/PPU_rendering#Cycles_257-320
        // @IMPL: transparent values for left-over sprites
        // We don't emulate dummy fetch to 0xFF
        if (i_sp_idx >= io_ctx->sp_got)
        {
            return 0x00;
        }

        bool tbl_right = io_accessor->get_register(PPU::PPUCTRL) & 0x08;

        Byte tile_idx = io_ctx->sp_nt_byte;

        Byte fine_y =
            Byte(io_accessor->get_context().scanline_no - io_ctx->sp_pos_y);
        bool flip_y = io_ctx->sp_attr_byte & 0x80;
        if (flip_y)
        {
            static_assert(LN_PATTERN_TILE_HEIGHT >= 1,
                          "oops, invalid LN_PATTERN_TILE_HEIGHT.");
            fine_y = (LN_PATTERN_TILE_HEIGHT - 1) - fine_y;
        }

        Address sliver_addr =
            fine_y | (i_upper << 3) | (tile_idx << 4) | (tbl_right << 12);

        Byte byte;
        auto error = io_accessor->get_memory()->get_byte(sliver_addr, byte);
        if (LN_FAILED(error))
        {
            LN_ASSERT_FATAL("Failed to fetch pattern byte for sp: {}, {}",
                            sliver_addr, i_upper);
            byte = 0xFF; // set to a consistent value.
        }

        // @IMPL: We choose to reverse the bits to implement sprite horizontal
        // flipping.
        bool flip_x = io_ctx->sp_attr_byte & 0x40;
        if (flip_x)
        {
            byte_reverse(byte);
        }

        return byte;
    };

    switch (i_curr)
    {
        // @IMPL: The first 2 ticks seem to be unspecified in the nesdev
        // forum, we are just guessing here.
        case 0:
        {
            // Read Y.
            io_ctx->sp_pos_y = read_sec_oam(io_accessor, io_ctx);

            // @IMPL: Mark down the current processing sprite index at first
            // tick.
            io_ctx->sp_idx_reload = io_ctx->sec_oam_read_idx / LN_OAM_SP_SIZE;
        }
        break;

        case 1:
        {
            // Read tile nunmber.
            io_ctx->sp_nt_byte = read_sec_oam(io_accessor, io_ctx);
        }
        break;

        case 2:
        {
            // Read attribute.
            Byte attr = read_sec_oam(io_accessor, io_ctx);
            io_ctx->sp_attr_byte = attr;
            io_accessor->get_context().sp_attr[io_ctx->sp_idx_reload] = attr;
        }
        break;

        case 3:
        {
            // Read X.
            Byte x = read_sec_oam(io_accessor, io_ctx);
            io_accessor->get_context().sp_pos_x[io_ctx->sp_idx_reload] = x;
        }
        break;

#if 1
// https://www.nesdev.org/wiki/PPU_sprite_evaluation#Details
// 4-7: Read the X-coordinate of the selected sprite from secondary OAM 4 times
// We don't emulate this.
#endif

        case 5:
        {
            io_accessor->get_context()
                .sf_sp_pattern_lower[io_ctx->sp_idx_reload] =
                get_pattern_sliver(io_accessor, io_ctx, io_ctx->sp_idx_reload,
                                   false);
        }
        break;

        case 7:
        {
            io_accessor->get_context()
                .sf_sp_pattern_upper[io_ctx->sp_idx_reload] =
                get_pattern_sliver(io_accessor, io_ctx, io_ctx->sp_idx_reload,
                                   true);
        }
        break;

        default:
            break;
    }

    // OAMADDR is set to 0 during each of ticks 257-320 (the sprite tile loading
    // interval)
    // https://www.nesdev.org/wiki/PPU_registers#OAM_address_($2003)_%3E_write
    io_accessor->get_register(PPU::OAMADDR) = 0;

    return 1;
}

} // namespace ln

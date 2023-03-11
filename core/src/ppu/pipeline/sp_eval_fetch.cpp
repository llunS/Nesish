#include "sp_eval_fetch.hpp"

#include "spec.hpp"
#include "ppu/pipeline_accessor.hpp"
#include "assert.hpp"
#include "byte_utils.hpp"

#include <cstring>

namespace nh {

static constexpr int SEC_OAM_CLEAR_CYCLES = 64;

SpEvalFetch::SpEvalFetch(PipelineAccessor *io_accessor, bool i_fetch_only)
    : Tickable(NH_SCANLINE_CYCLES)
    , m_accessor(io_accessor)
    , m_fetch_only(i_fetch_only)
    , m_sec_oam_clear(SEC_OAM_CLEAR_CYCLES,
                      std::bind(pv_sec_oam_clear, std::placeholders::_1,
                                std::placeholders::_2, io_accessor))
    , m_eval(192, std::bind(pv_sp_eval, std::placeholders::_1,
                            std::placeholders::_2, io_accessor, &m_ctx))
    , m_fetch_reload(8, std::bind(pv_sp_fetch_reload, std::placeholders::_1,
                                  std::placeholders::_2, io_accessor, &m_ctx))
{
    m_sec_oam_clear.set_done();
    m_eval.set_done();
    m_fetch_reload.set_done();
}

void
SpEvalFetch::reset()
{
    Tickable::reset();

    m_sec_oam_clear.set_done();
    m_eval.set_done();
    m_fetch_reload.set_done();
}

Cycle
SpEvalFetch::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_total);

    switch (i_curr)
    {
        case 1:
        {
            if (!m_fetch_only)
            {
                m_sec_oam_clear.reset();
            }
        }
        break;

        case 65:
        {
            if (!m_fetch_only)
            {
                m_eval.reset();

                Byte oam_addr = m_accessor->get_register(PPU::OAMADDR);
                m_ctx.sec_oam_write_idx = 0;
                m_ctx.cp_counter = 0;
                // Perhaps we shouldn't cache this either
                m_ctx.init_oam_addr = oam_addr;
                m_ctx.n = m_ctx.m = 0;
                m_ctx.n_overflow = false;
                m_ctx.sp_got = 0;
                m_ctx.sp_overflow = false;
                m_ctx.sec_oam_written = false;
                m_ctx.sp0_in_range = false;
            }
        }
        break;

        case 257:
        {
            m_ctx.sec_oam_read_idx = 0;

            // @NOTE: Pass out the flag at fetch stage (not eval stage), in case
            // the flag is used by current scanline instead of the next one.
            // In other words, "with_sp0" should not be overwritten in rendering
            // stage (while it's in use).
            m_accessor->get_context().with_sp0 = m_ctx.sp0_in_range;
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
            if (m_accessor->rendering_enabled())
            {
                // 8 cycles each for 8 sprites.
                m_fetch_reload.reset();
            }
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
    m_eval.tick();
    m_fetch_reload.tick();

    /* do some checks */
    switch (i_curr)
    {
        case 320:
        {
            if (m_accessor->rendering_enabled() &&
                m_ctx.sec_oam_read_idx != NH_SEC_OAM_SIZE)
            {
                NH_ASSERT_ERROR(m_accessor->get_logger(),
                                "Wrong secondary OAM read index: {}",
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
SpEvalFetch::pv_sec_oam_clear(Cycle i_curr, Cycle i_total,
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
SpEvalFetch::pv_sp_eval(Cycle i_curr, Cycle i_total,
                        PipelineAccessor *io_accessor, Context *io_ctx)
{
    (void)(i_total);

#if 1 // local functions
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
            if (io_ctx->m >= NH_OAM_SP_SIZE)
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
        if (io_ctx->n >= NH_MAX_SP_NUM)
        {
            io_ctx->n %= NH_MAX_SP_NUM;

            // all 64 are checked
            io_ctx->n_overflow = true;
        }

        int new_read_addr =
            io_ctx->init_oam_addr + (io_ctx->n * NH_OAM_SP_SIZE + io_ctx->m);
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
        io_ctx->sec_oam_written = true;
        if (i_inc)
        {
            ++io_ctx->sec_oam_write_idx;

            // got 1 sprite
            if (io_ctx->sec_oam_write_idx % NH_OAM_SP_SIZE == 0)
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
#endif

    // @TODO: hide sprites before OAMADDR
    // No more sprites will be found once the end of OAM is reached, effectively
    // hiding any sprites before the starting OAMADDR
    // https://www.nesdev.org/wiki/PPU_registers#Values_during_rendering

    // The process must be halted immediately, i.e. in real time.
    // "If [PPUMASK]] ($2001) with both BG and sprites disabled, rendering will
    // be halted immediately."
    // https://www.nesdev.org/wiki/PPU_sprite_evaluation#Rendering_disable_or_enable_during_active_scanline
    // test: sprite_overflow_tests/5.Emulator.nes failure #4
    if (io_accessor->rendering_enabled())
    {
        // read cycle
        if (i_curr % 2 == 0)
        {
            Byte oam_addr = io_accessor->get_register(PPU::OAMADDR);
            io_ctx->sp_eval_bus = io_accessor->get_oam_byte(oam_addr);
        }
        // write cycle
        else
        {
            // Y
            if (!io_ctx->cp_counter)
            {
                Byte y = io_ctx->sp_eval_bus;

                // Test this before "write_sec_oam"
                bool sp_0 = !io_ctx->sec_oam_written;
                // copy Y to secondary OAM
                write_sec_oam(io_accessor, io_ctx, y, false);

                if (io_ctx->n_overflow || io_ctx->sp_overflow)
                {
                    // try next Y
                    add_read(io_accessor, io_ctx, true, false, true);
                }
                else
                {
                    static_assert(NH_PATTERN_TILE_HEIGHT == 8,
                                  "Invalid NH_PATTERN_TILE_HEIGHT.");
                    decltype(y) sp_h = io_accessor->is_8x16_sp()
                                           ? NH_PATTERN_TILE_HEIGHT * 2
                                           : NH_PATTERN_TILE_HEIGHT;
                    bool in_range =
                        (y <= io_accessor->get_context().scanline_no &&
                         io_accessor->get_context().scanline_no < y + sp_h);
                    if (in_range)
                    {
                        // copy the rest
                        inc_write(io_ctx);
                        io_ctx->cp_counter = NH_OAM_SP_SIZE - 1;

                        add_read(io_accessor, io_ctx, false, true, true);

                        if (sp_0)
                        {
                            io_ctx->sp0_in_range = true;
                        }
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
                        NH_ASSERT_FATAL(io_accessor->get_logger(),
                                        "Wrong sprite evaluation timing in "
                                        "copying rest: {}",
                                        io_ctx->cp_counter);
                    }
                }

                add_read(io_accessor, io_ctx, false, true, true);

                --io_ctx->cp_counter;
            }
        }
    }

    return 1;
}

Cycle
SpEvalFetch::pv_sp_fetch_reload(Cycle i_curr, Cycle i_total,
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
        // We don't emulate dummy fetch to 0xFF
        if (i_sp_idx >= io_ctx->sp_got)
        {
            // Transparent values for left-over sprites
            return 0x00;
        }

        bool tbl_right;
        io_accessor->resolve_sp_ptn_tbl(
            io_ctx->sp_tile_byte, io_accessor->is_8x16_sp(),
            io_accessor->get_register(PPU::PPUCTRL) & 0x08, tbl_right);

        bool flip_y = io_ctx->sp_attr_byte & 0x80;
        Byte y_in_sp =
            Byte(io_accessor->get_context().scanline_no - io_ctx->sp_pos_y);
        Byte tile_idx;
        io_accessor->resolve_sp_tile(io_ctx->sp_tile_byte,
                                     io_accessor->is_8x16_sp(), flip_y, y_in_sp,
                                     tile_idx);

        static_assert(NH_PATTERN_TILE_HEIGHT == 8,
                      "Invalid NH_PATTERN_TILE_HEIGHT.");
        Byte fine_y = y_in_sp % NH_PATTERN_TILE_HEIGHT;
        if (flip_y)
        {
            fine_y = (NH_PATTERN_TILE_HEIGHT - 1) - fine_y;
        }

        Address sliver_addr =
            io_accessor->get_sliver_addr(tbl_right, tile_idx, i_upper, fine_y);
        Byte ptn_byte;
        {
            auto error =
                io_accessor->get_memory()->get_byte(sliver_addr, ptn_byte);
            if (NH_FAILED(error))
            {
                NH_ASSERT_FATAL(io_accessor->get_logger(),
                                "Failed to fetch pattern byte for sp: {}, {}",
                                sliver_addr, i_upper);
                ptn_byte = 0xFF; // set to apparent value.
            }
        }

        // Reverse the bits to implement horizontal flipping.
        bool flip_x = io_ctx->sp_attr_byte & 0x40;
        if (flip_x)
        {
            reverse_byte(ptn_byte);
        }

        return ptn_byte;
    };

    switch (i_curr)
    {
        // The first 2 ticks seem to be unspecified in the nesdev
        // forum, we are just guessing here.
        case 0:
        {
            // Mark down the current processing sprite index at first
            // tick. Do this before "read_sec_oam" since it modifies
            // "sec_oam_read_idx".
            io_ctx->sp_idx_reload = io_ctx->sec_oam_read_idx / NH_OAM_SP_SIZE;

            // Read Y.
            io_ctx->sp_pos_y = read_sec_oam(io_accessor, io_ctx);
        }
        break;

        case 1:
        {
            // Read tile.
            io_ctx->sp_tile_byte = read_sec_oam(io_accessor, io_ctx);
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

} // namespace nh

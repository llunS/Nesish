#include "pipeline_accessor.hpp"

#include "console/debug/sprite.hpp"
#include "console/spec.hpp"
#include "console/byte_utils.hpp"

#include <limits>

namespace ln {

PipelineAccessor::PipelineAccessor(PPU *i_ppu)
    : m_ppu(i_ppu)
{
}

PPU::PipelineContext &
PipelineAccessor::get_context()
{
    return m_ppu->m_pipeline_ctx;
}

Byte &
PipelineAccessor::get_register(PPU::Register i_reg)
{
    return m_ppu->get_register(i_reg);
}

const Byte &
PipelineAccessor::get_register(PPU::Register i_reg) const
{
    return m_ppu->get_register(i_reg);
}

Byte2 &
PipelineAccessor::get_v()
{
    return m_ppu->v;
}

Byte2 &
PipelineAccessor::get_t()
{
    return m_ppu->t;
}

Byte &
PipelineAccessor::get_x()
{
    return m_ppu->x;
}

Byte
PipelineAccessor::get_oam(Byte i_addr)
{
    return *get_oam_addr(i_addr);
}

Byte *
PipelineAccessor::get_oam_addr(Byte i_addr)
{
    static_assert(sizeof(m_ppu->m_oam) == 256, "Wrong primary OAM size.");
    return &m_ppu->m_oam[i_addr];
}

VideoMemory *
PipelineAccessor::get_memory()
{
    return m_ppu->m_memory;
}

const Palette &
PipelineAccessor::get_palette()
{
    return m_ppu->m_palette;
}

FrameBuffer &
PipelineAccessor::get_frame_buf()
{
    return m_ppu->m_back_buf;
}

bool
PipelineAccessor::bg_enabled() const
{
    return get_register(PPU::PPUMASK) & 0x08;
}

bool
PipelineAccessor::sp_enabled() const
{
    return get_register(PPU::PPUMASK) & 0x10;
}

bool
PipelineAccessor::rendering_enabled() const
{
    return bg_enabled() || sp_enabled();
}

bool
PipelineAccessor::is_8x16_sp() const
{
    return get_register(PPU::PPUCTRL) & 0x20;
}

bool
PipelineAccessor::no_nmi() const
{
    return m_ppu->m_no_nmi;
}

void
PipelineAccessor::finish_frame()
{
    // @IMPL: Do these the same time as we swap the buffer to synchronize with
    // it.
    if (capture_palette_on())
    {
        capture_palette();
    }
    if (capture_oam_on())
    {
        capture_oam();
    }
    if (capture_ptn_tbls_on())
    {
        capture_ptn_tbls();
    }

    m_ppu->m_front_buf.swap(m_ppu->m_back_buf);
}

Address
PipelineAccessor::get_sliver_addr(bool i_tbl_right, Byte i_tile_idx,
                                  bool i_upper, Byte i_fine_y)
{
    Address sliver_addr = (Address(i_tbl_right) << 12) |
                          (Address(i_tile_idx) << 4) | (Address(i_upper) << 3) |
                          Address(i_fine_y);
    return sliver_addr;
}

ln::Error
PipelineAccessor::get_ptn_sliver(bool i_tbl_right, Byte i_tile_idx,
                                 bool i_upper, Byte i_fine_y,
                                 const VideoMemory *i_vram, Byte &o_val)
{
    Address sliver_addr =
        get_sliver_addr(i_tbl_right, i_tile_idx, i_upper, i_fine_y);
    auto error = i_vram->get_byte(sliver_addr, o_val);
    return error;
}

void
PipelineAccessor::resolve_sp_ptn_tbl(Byte i_tile_byte, bool i_8x16,
                                     bool i_ptn_tbl_bit, bool &o_high_ptn_tbl)
{
    o_high_ptn_tbl = i_8x16 ? (i_tile_byte & 0x01) : i_ptn_tbl_bit;
}

void
PipelineAccessor::resolve_sp_tile(Byte i_tile_byte, bool i_8x16, bool i_flip_y,
                                  Byte i_fine_y_sp, Byte &o_tile_idx)
{
    if (i_8x16)
    {
        Byte top_tile = ((i_tile_byte >> 1) & 0x7F) << 1;
        bool top_half = 0 <= i_fine_y_sp && i_fine_y_sp < 8;
        o_tile_idx = (top_half ^ !i_flip_y) ? top_tile + 1 : top_tile;
    }
    else
    {
        o_tile_idx = i_tile_byte;
    }
}

bool
PipelineAccessor::capture_palette_on()
{
    return lnd::is_debug_on(m_ppu->m_debug_flags, lnd::DBG_PALETTE);
}

void
PipelineAccessor::capture_palette()
{
    for (decltype(m_ppu->m_palette_snap.color_count()) i = 0;
         i < m_ppu->m_palette_snap.color_count(); ++i)
    {
        static_assert(lnd::Palette::color_count() == 32,
                      "Might overflow below.");
        Address color_addr = Address(LN_PALETTE_ADDR_HEAD + i);
        Byte color_byte = 0;
        // @NOTE: The address is in VRAM.
        (void)m_ppu->m_memory->get_byte(color_addr, color_byte);

        Color clr = get_palette().to_rgb(color_byte);
        m_ppu->m_palette_snap.set_color(i, color_byte, clr.r, clr.g, clr.b);
    }
}

bool
PipelineAccessor::capture_oam_on()
{
    return lnd::is_debug_on(m_ppu->m_debug_flags, lnd::DBG_OAM);
}

void
PipelineAccessor::capture_oam()
{
    for (decltype(m_ppu->m_oam_snap.get_sprite_count()) i = 0;
         i < m_ppu->m_oam_snap.get_sprite_count(); ++i)
    {
        update_oam_sprite(m_ppu->m_oam_snap.sprite_of(i), i);
    }
}

void
PipelineAccessor::update_oam_sprite(lnd::Sprite &o_sprite, int i_idx)
{
    // @IMPL: Assuming OAMADDR starts at 0.
    Byte byte_idx_start = Byte(i_idx * 4);
    Byte i_y = get_oam(byte_idx_start);
    Byte i_tile = get_oam(byte_idx_start + 1);
    Byte i_attr = get_oam(byte_idx_start + 2);
    Byte i_x = get_oam(byte_idx_start + 3);

    o_sprite.set_raw(i_y, i_tile, i_attr, i_x);

    bool mode_8x16 = this->is_8x16_sp();
    o_sprite.set_mode(mode_8x16);

    bool flip_y = i_attr & 0x80;
    bool flip_x = i_attr & 0x40;

    bool tbl_right;
    this->resolve_sp_ptn_tbl(
        i_tile, mode_8x16, this->get_register(PPU::PPUCTRL) & 0x08, tbl_right);
    for (Byte y_in_sp = 0; y_in_sp < 16; ++y_in_sp)
    {
        if (!mode_8x16 && y_in_sp >= 8)
        {
            break;
        }

        Byte tile_idx;
        this->resolve_sp_tile(i_tile, mode_8x16, flip_y, y_in_sp, tile_idx);

        static_assert(LN_PATTERN_TILE_HEIGHT == 8,
                      "Invalid LN_PATTERN_TILE_HEIGHT.");
        Byte fine_y = y_in_sp % LN_PATTERN_TILE_HEIGHT;
        if (flip_y)
        {
            fine_y = (LN_PATTERN_TILE_HEIGHT - 1) - fine_y;
        }

        Byte ptn_bit0_byte;
        {
            auto error =
                this->get_ptn_sliver(tbl_right, tile_idx, false, fine_y,
                                     m_ppu->m_memory, ptn_bit0_byte);
            if (LN_FAILED(error))
            {
                ptn_bit0_byte = 0xFF; // set to apparent value.
            }
        }
        Byte ptn_bit1_byte;
        {
            auto error = this->get_ptn_sliver(tbl_right, tile_idx, true, fine_y,
                                              m_ppu->m_memory, ptn_bit1_byte);
            if (LN_FAILED(error))
            {
                ptn_bit1_byte = 0xFF; // set to apparent value.
            }
        }

        // @IMPL: Reverse the bits to implement horizontal flipping.
        if (flip_x)
        {
            reverse_byte(ptn_bit0_byte);
            reverse_byte(ptn_bit1_byte);
        }

        /* Now that we have a row of data available */
        for (int fine_x = 0; fine_x < 8; ++fine_x)
        {
            auto get_palette_color = [this](int i_idx) {
                // @NOTE: The address is in VRAM.
                Address color_addr = Address(LN_PALETTE_ADDR_HEAD + i_idx);
                Byte color_byte = 0;
                auto error =
                    this->m_ppu->m_memory->get_byte(color_addr, color_byte);
                if (LN_FAILED(error))
                {
                    color_byte = 0x30; // set it to white to be apparent.
                }
                Color clr = this->get_palette().to_rgb(color_byte);
                return clr;
            };

            int addr_palette_set_offset = i_attr & 0x03; // 4-color palette
            int ptn = (bool(ptn_bit1_byte & (0x80 >> fine_x)) << 1) |
                      (bool(ptn_bit0_byte & (0x80 >> fine_x)) << 0);
            static_assert(LN_PALETTE_SIZE == 32,
                          "Incorrect color byte position");
            Color pixel =
                get_palette_color(16 + addr_palette_set_offset * 4 + ptn);
            o_sprite.set_pixel(y_in_sp, fine_x, pixel);
        }
    }
}

bool
PipelineAccessor::capture_ptn_tbls_on()
{
    return lnd::is_debug_on(m_ppu->m_debug_flags, lnd::DBG_PATTERN);
}

void
PipelineAccessor::capture_ptn_tbls()
{
    auto cap_tbl = [this](bool i_right, lnd::PatternTable &o_tbl) -> void {
        static_assert(lnd::PatternTable::get_tiles() == 16 * 16,
                      "Incorrect loop count");
        static_assert(std::numeric_limits<int>::max() >= 16 * 16,
                      "Type of loop variable too small");
        for (int tile_idx = 0; tile_idx < 16 * 16; ++tile_idx)
        {
            static_assert(lnd::PatternTable::get_tile_height() == 8,
                          "Incorrect loop count");
            for (Byte fine_y = 0; fine_y < 8; ++fine_y)
            {
                static_assert(std::numeric_limits<Byte>::max() >= 16 * 16 - 1,
                              "Type of tile index incompatible for use of "
                              "\"get_ptn_sliver\"");
                Byte ptn_bit0_byte;
                {
                    auto error = this->get_ptn_sliver(
                        i_right, Byte(tile_idx), false, fine_y, m_ppu->m_memory,
                        ptn_bit0_byte);
                    if (LN_FAILED(error))
                    {
                        ptn_bit0_byte = 0xFF; // set to apparent value.
                    }
                }
                Byte ptn_bit1_byte;
                {
                    auto error = this->get_ptn_sliver(
                        i_right, Byte(tile_idx), true, fine_y, m_ppu->m_memory,
                        ptn_bit1_byte);
                    if (LN_FAILED(error))
                    {
                        ptn_bit1_byte = 0xFF; // set to apparent value.
                    }
                }

                /* Now that we have a row of data available */
                static_assert(lnd::PatternTable::get_tile_width() == 8,
                              "Incorrect loop count");
                for (int fine_x = 0; fine_x < 8; ++fine_x)
                {
                    auto get_palette_color = [this](int i_idx) {
                        // @NOTE: The address is in VRAM.
                        Address color_addr =
                            Address(LN_PALETTE_ADDR_HEAD + i_idx);
                        Byte color_byte = 0;
                        auto error = this->m_ppu->m_memory->get_byte(
                            color_addr, color_byte);
                        if (LN_FAILED(error))
                        {
                            color_byte =
                                0x30; // set it to white to be apparent.
                        }
                        Color clr = this->get_palette().to_rgb(color_byte);
                        return clr;
                    };

                    int ptn = (bool(ptn_bit1_byte & (0x80 >> fine_x)) << 1) |
                              (bool(ptn_bit0_byte & (0x80 >> fine_x)) << 0);
                    static_assert(LN_PALETTE_SIZE == 32,
                                  "Incorrect color byte position");
                    Color pixel = get_palette_color(
                        this->m_ppu->m_ptn_tbl_palette_idx * 4 + ptn);
                    o_tbl.set_pixel(tile_idx, fine_y, fine_x, pixel);
                }
            }
        }
    };
    cap_tbl(false, m_ppu->m_ptn_tbl_l_snap);
    cap_tbl(true, m_ppu->m_ptn_tbl_r_snap);
}

} // namespace ln

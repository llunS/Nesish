#include "pipeline_accessor.hpp"

#include "console/debug/sprite.hpp"
#include "console/spec.hpp"
#include "console/byte_utils.hpp"

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
PipelineAccessor::bg_enabled()
{
    return get_register(PPU::PPUMASK) & 0x08;
}

bool
PipelineAccessor::sp_enabled()
{
    return get_register(PPU::PPUMASK) & 0x10;
}

bool
PipelineAccessor::rendering_enabled()
{
    return bg_enabled() || sp_enabled();
}

void
PipelineAccessor::check_gen_nmi()
{
    m_ppu->check_gen_nmi();
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

    m_ppu->m_front_buf.swap(m_ppu->m_back_buf);
}

bool
PipelineAccessor::capture_palette_on()
{
    return lnd::is_debug_on(m_ppu->m_debug_flags, lnd::DBG_PALETTE);
}

void
PipelineAccessor::capture_palette()
{
    for (decltype(m_ppu->m_palette_snapshot.color_count()) i = 0;
         i < m_ppu->m_palette_snapshot.color_count(); ++i)
    {
        static_assert(lnd::Palette::color_count() == 32,
                      "Might overflow below.");
        Address color_addr = Address(LN_PALETTE_ADDR_HEAD + i);
        Byte color_byte = 0;
        // @NOTE: The address is in VRAM.
        (void)m_ppu->m_memory->get_byte(color_addr, color_byte);

        Color clr = get_palette().to_rgb(color_byte);
        m_ppu->m_palette_snapshot.set_color(i, color_byte, clr.r, clr.g, clr.b);
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
    for (decltype(m_ppu->m_oam_snapshot.get_sprite_count()) i = 0;
         i < m_ppu->m_oam_snapshot.get_sprite_count(); ++i)
    {
        update_oam_sprite(m_ppu->m_oam_snapshot.sprite_of(i), i);
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

    // @TODO: Support 8x16 as well.
    // Assuming 8x8 for now.
    bool tbl_right = get_register(PPU::PPUCTRL) & 0x08;
    for (int fine_y = 0; fine_y < 8; ++fine_y)
    {
        bool upper = false;
        Address sliver_addr =
            Byte(fine_y) | (upper << 3) | (i_tile << 4) | (tbl_right << 12);
        Byte ptn_bit0_byte = 0;
        (void)m_ppu->m_memory->get_byte(sliver_addr, ptn_bit0_byte);

        upper = true;
        sliver_addr =
            Byte(fine_y) | (upper << 3) | (i_tile << 4) | (tbl_right << 12);
        Byte ptn_bit1_byte = 0;
        (void)m_ppu->m_memory->get_byte(sliver_addr, ptn_bit1_byte);

        /* flipping */
        bool flip_y = i_attr & 0x80;
        if (flip_y)
        {
            static_assert(LN_PATTERN_TILE_HEIGHT >= 1,
                          "oops, invalid LN_PATTERN_TILE_HEIGHT.");
            fine_y = (LN_PATTERN_TILE_HEIGHT - 1) - fine_y;
        }
        // @IMPL: We choose to reverse the bits to implement sprite horizontal
        // flipping.
        bool flip_x = i_attr & 0x40;
        if (flip_x)
        {
            byte_reverse(ptn_bit0_byte);
            byte_reverse(ptn_bit1_byte);
        }

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

            static_assert(LN_PALETTE_SIZE == 32, "Rework code below.");
            Color pixel =
                get_palette_color(16 + addr_palette_set_offset * 4 + ptn);
            o_sprite.set_pixel(fine_y, fine_x, pixel);
        }
    }
}

} // namespace ln

#include "pre_render_scanline.hpp"

#include "console/spec.hpp"
#include "console/ppu/pipeline_accessor.hpp"

namespace ln {

PreRenderScanline::PreRenderScanline(PipelineAccessor *io_accessor)
    : Tickable(LN_SCANLINE_CYCLES)
    , m_accessor(io_accessor)
    , m_bg(io_accessor)
    , m_sp(io_accessor, true)
{
}

void
PreRenderScanline::reset()
{
    Tickable::reset();

    m_bg.reset();
    m_sp.reset();
}

Cycle
PreRenderScanline::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_total);

    switch (i_curr)
    {
        case 1:
        {
            /* clear flags */
            // VSO (VBLANK, Sprite 0 hit, Sprite overflow)
            m_accessor->get_register(PPU::PPUSTATUS) &= 0x1F;

            // @QUIRK: Minor corruption
            // https://www.nesdev.org/wiki/PPU_sprite_evaluation#Notes
            // @NOTE: Simply do this near the start of pre-render scanline.
            Byte oam_addr = m_accessor->get_register(PPU::OAMADDR);
            Byte oam_cpy_addr = oam_addr & 0xF8;
            if (oam_cpy_addr)
            {
                // @IMPL: memcpy is ok, memmove is not needed.
                std::memcpy(m_accessor->get_oam_addr(0),
                            m_accessor->get_oam_addr(oam_cpy_addr), 8);
            }
        }
        break;

        case 280:
        case 281:
        case 282:
        case 283:
        case 284:
        case 285:
        case 286:
        case 287:
        case 288:
        case 289:
        case 290:
        case 291:
        case 292:
        case 293:
        case 294:
        case 295:
        case 296:
        case 297:
        case 298:
        case 299:
        case 300:
        case 301:
        case 302:
        case 303:
        case 304:
        {
            if (m_accessor->rendering_enabled())
            {
                /* reload vertical bits */
                Byte2 &v = m_accessor->get_v();
                const Byte2 &t = m_accessor->get_t();
                v = (v & 0x041F) | (t & ~0x041F);
            }
        }
        break;

        default:
            break;
    }

    m_bg.tick();
    m_sp.tick();

    return 1;
}

} // namespace ln

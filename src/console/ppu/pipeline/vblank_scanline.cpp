#include "vblank_scanline.hpp"

#include "console/spec.hpp"
#include "console/ppu/pipeline_accessor.hpp"

namespace ln {

VBlankScanline::VBlankScanline(PipelineAccessor *io_accessor)
    : Tickable(LN_SCANLINE_CYCLES)
    , m_accessor(io_accessor)
{
}

Cycle
VBlankScanline::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_total);

    switch (i_curr)
    {
        case 1:
        {
            if (241 == m_accessor->get_context().scanline_no)
            {
                if (!m_accessor->no_nmi())
                {
                    /* Set NMI_occurred in PPU to true */
                    m_accessor->get_register(PPU::PPUSTATUS) |= 0x80;
                }
            }
        }
        break;

        default:
            break;
    }

    return 1;
}

} // namespace ln

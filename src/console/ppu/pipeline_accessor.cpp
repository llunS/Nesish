#include "pipeline_accessor.hpp"

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
    return m_ppu->m_frame_buf;
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
    m_ppu->m_frame_buf_dirty = true;
}

} // namespace ln

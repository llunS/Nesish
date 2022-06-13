#include "ines.hpp"

#include "common/logger.hpp"
#include "console/cartridge/mapper/norm.hpp"

namespace ln {

static Mapper *
pvt_get_mapper(Byte i_mapper_number, const INES::RomAccessor *i_accessor);

INES::INES()
    : m_prg_rom(nullptr)
    , m_prg_rom_size(0)
    , m_chr_rom(nullptr)
    , m_chr_rom_size(0)
    , m_rom_accessor(this)
{
}

INES::~INES()
{
    if (m_prg_rom)
    {
        delete[] m_prg_rom;
    }
    m_prg_rom = nullptr;
    if (m_chr_rom)
    {
        delete[] m_chr_rom;
    }
    m_chr_rom = nullptr;
}

Error
INES::resolve()
{
    // mapper
    m_mapper_number = (m_header.mapper_higher << 4) + m_header.mapper_lower;

    auto mapper = pvt_get_mapper(m_mapper_number, &m_rom_accessor);
    if (!mapper)
    {
        LN_LOG_ERROR(ln::get_logger(), "iNES unsupported mapper {}",
                     m_mapper_number);
        return Error::UNIMPLEMENTED;
    }
    m_mapper.reset(mapper);

    return Error::OK;
}

Error
INES::validate() const
{
    // 1. header metadata check
    // magic number: NES^Z
    if (!(m_header.nes_magic[0] == 0x4e && m_header.nes_magic[1] == 0x45 &&
          m_header.nes_magic[2] == 0x53 && m_header.nes_magic[3] == 0x1a))
    {
        LN_LOG_ERROR(ln::get_logger(), "iNES invalid magic number.");
        return Error::CORRUPTED;
    }
    if (!m_prg_rom)
    {
        LN_LOG_ERROR(ln::get_logger(), "iNES empty PRG ROM.");
        return Error::CORRUPTED;
    }
    if (m_header.ines2 != 0)
    {
        LN_LOG_ERROR(ln::get_logger(), "iNES invalid version.");
        return Error::CORRUPTED;
    }

    // 2. runtime state check
    if (!m_mapper)
    {
        LN_LOG_ERROR(ln::get_logger(), "iNES mapper uninitialized.");
        return Error::UNINITIALIZED;
    }

    LN_LOG_INFO(ln::get_logger(), "iNES mapper {}.", m_mapper_number);

    return Error::OK;
}

void
INES::map_memory(Memory *i_memory, VideoMemory *i_video_memory) const
{
    m_mapper->map_memory(this, i_memory, i_video_memory);
}

void
INES::unmap_memory(Memory *i_memory, VideoMemory *i_video_memory) const
{
    m_mapper->unmap_memory(this, i_memory, i_video_memory);
}

bool
INES::h_mirror() const
{
    return !m_header.mirror;
}

Mapper *
pvt_get_mapper(Byte i_mapper_number, const INES::RomAccessor *i_accessor)
{
    switch (i_mapper_number)
    {
        case 0:
            return new NORM(i_accessor);
            break;

        default:
            return nullptr;
            break;
    }
}

} // namespace ln

namespace ln {

INES::RomAccessor::RomAccessor(const INES *i_nes)
    : m_ines(i_nes)
{
}

void
INES::RomAccessor::get_prg_rom(Byte **o_addr, std::size_t *o_size) const
{
    if (o_addr)
        *o_addr = m_ines->m_prg_rom;
    if (o_size)
        *o_size = m_ines->m_prg_rom_size;
}

void
INES::RomAccessor::get_chr_rom(Byte **o_addr, std::size_t *o_size) const
{
    if (o_addr)
        *o_addr = m_ines->m_chr_rom;
    if (o_size)
        *o_size = m_ines->m_chr_rom_size;
}

} // namespace ln

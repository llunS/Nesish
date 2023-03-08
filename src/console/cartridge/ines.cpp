#include "ines.hpp"

#include "common/logger.hpp"
#include "console/cartridge/mapper/nrom.hpp"
#include "console/cartridge/mapper/mmc1.hpp"
#include "console/cartridge/mapper/cnrom.hpp"

namespace ln {

static Mapper *
pv_get_mapper(Byte i_mapper_number, const INES::RomAccessor *i_accessor);

INES::INES()
    : m_prg_rom(nullptr)
    , m_prg_rom_size(0)
    , m_chr_rom(nullptr)
    , m_chr_rom_size(0)
    , m_use_chr_ram(false)
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

    auto mapper = pv_get_mapper(m_mapper_number, &m_rom_accessor);
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
        LN_LOG_ERROR(ln::get_logger(), "Support only iNES format for now, {}.",
                     m_header.ines2);
        return Error::CORRUPTED;
    }

    // 2. runtime state check
    if (!m_mapper)
    {
        LN_LOG_ERROR(ln::get_logger(), "iNES mapper uninitialized.");
        return Error::UNINITIALIZED;
    }

    LN_LOG_INFO(ln::get_logger(), "iNES mapper {}.", m_mapper_number);

    auto error = m_mapper->validate();
    if (LN_FAILED(error))
    {
        return error;
    }

    return Error::OK;
}

void
INES::power_up()
{
    m_mapper->power_up();
}

void
INES::reset()
{
    m_mapper->reset();
}

void
INES::map_memory(Memory *o_memory, VideoMemory *o_video_memory)
{
    m_mapper->map_memory(o_memory, o_video_memory);
}

void
INES::unmap_memory(Memory *o_memory, VideoMemory *o_video_memory)
{
    m_mapper->unmap_memory(o_memory, o_video_memory);
}

Mapper *
pv_get_mapper(Byte i_mapper_number, const INES::RomAccessor *i_accessor)
{
    switch (i_mapper_number)
    {
        case 0:
            return new NROM(i_accessor);
            break;

        case 1:
            return new MMC1(i_accessor, MMC1::V_B);
            break;

        case 3:
            return new CNROM(i_accessor);
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

bool
INES::RomAccessor::h_mirror() const
{
    return !m_ines->m_header.mirror;
}

void
INES::RomAccessor::get_prg_rom(Byte **o_addr, std::size_t *o_size) const
{
    if (o_addr)
        *o_addr = m_ines->m_prg_rom;
    if (o_size)
        *o_size = m_ines->m_prg_rom_size;
}

std::size_t
INES::RomAccessor::get_prg_ram_size() const
{
    return m_ines->m_header.prg_ram_size * 8 * 1024;
}

void
INES::RomAccessor::get_chr_rom(Byte **o_addr, std::size_t *o_size) const
{
    if (o_addr)
        *o_addr = m_ines->m_chr_rom;
    if (o_size)
        *o_size = m_ines->m_chr_rom_size;
}

bool
INES::RomAccessor::use_chr_ram() const
{
    return m_ines->m_use_chr_ram;
}

} // namespace ln

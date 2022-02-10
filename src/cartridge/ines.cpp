#include "ines.hpp"

#include "common/logger.hpp"

namespace ln {

INES::INES()
    : m_prg_rom(nullptr)
    , m_chr_rom(nullptr)
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

void
INES::resolve()
{
    m_mapper_number = (m_header.mapper_upper << 4) + m_header.mapper_lower;
}

Error
INES::validate()
{
    // @TODO: Try some test roms.

    // magic number: NES^Z
    if (!(m_header.nes_magic[0] == 0x4e && m_header.nes_magic[1] == 0x45 &&
          m_header.nes_magic[2] == 0x53 && m_header.nes_magic[3] == 0x1a))
    {
        get_logger()->error("iNES invalid magic number.");
        return Error::CORRUPTED;
    }
    if (!m_prg_rom)
    {
        get_logger()->error("iNES empty PRG ROM.");
        return Error::CORRUPTED;
    }
    if (m_header.ines2 != 0)
    {
        get_logger()->error("iNES invalid version.");
        return Error::CORRUPTED;
    }

    get_logger()->debug("iNES mapper {}.", m_mapper_number);

    return Error::OK;
}

} // namespace ln

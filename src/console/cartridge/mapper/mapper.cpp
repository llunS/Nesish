#include "mapper.hpp"

namespace ln {

Mapper::Mapper(const INES::RomAccessor *i_accessor)
    : m_rom_accessor(i_accessor)
{
}

} // namespace ln

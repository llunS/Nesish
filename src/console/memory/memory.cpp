#include "memory.hpp"

#include <cstring>

namespace ln {

constexpr Address Memory::STACK_PAGE_MASK;
constexpr Address Memory::IRQ_VECTOR_ADDR;

Memory::Memory()
    : m_ram{}
    , m_adhoc{}
{
    // Internal RAM space mapping
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            Byte *ram = (Byte *)i_entry->opaque;

            Address addr = i_addr & LN_INTERNAL_RAM_MASK;
            return ram + addr;
        };
        set_mapping(MemoryMappingPoint::INTERNAL_RAM,
                    {LN_RAM_ADDR_HEAD, LN_RAM_ADDR_TAIL, false, decode, m_ram});
    }

    // @TMP: Ad-hoc solution until APU is implemented
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            Byte *ram = (Byte *)i_entry->opaque;

            Address addr = i_addr;
            return ram + (addr - i_entry->begin);
        };
        set_mapping(MemoryMappingPoint::ADHOC,
                    {0x4000, 0x4017, false, decode, m_adhoc});
    }
}

Error
Memory::set_bulk(Address i_begin, Address i_end, Byte i_byte)
{
    if (i_begin >= i_end)
    {
        return Error::INVALID_ARGUMENT;
    }

    // optimize only simple cases for now.
    if (i_begin < LN_INTERNAL_RAM_SIZE && i_end < LN_INTERNAL_RAM_SIZE)
    {
        std::memset(m_ram + i_begin, i_byte, i_end - i_begin);
        return Error::OK;
    }

    // For now, only support consecutive and non-page-crossing range.
    return Error::INVALID_ARGUMENT;
}

} // namespace ln

#include "mmu.hpp"

#include <cstring>
#include <type_traits>
#include <algorithm>
#include <iterator>

#include "common/logger.hpp"

namespace ln {

MappingEntry::MappingEntry(Address i_begin, Address i_end, bool i_readonly,
                           MappingDecodeFunc i_decode, void *i_opaque)
    : begin(i_begin)
    , end(i_end)
    , readonly(i_readonly)
    , decode(i_decode)
    , opaque(i_opaque)
{
}

} // namespace ln

namespace ln {

constexpr Address MMU::STACK_PAGE_MASK;
constexpr Address MMU::IRQ_VECTOR_ADDR;

MMU::MMU()
    : m_ram{}
    , m_adhoc{}
    , m_mapping_registry{}
{
    static_assert(MappingPoint::INVALID == 0,
                  "m_mapping_registry initialization needs other logic if "
                  "invalid value is not 0");

    // Register RAM space mapping
    auto internal_ram_decode = [](const MappingEntry *i_entry,
                                  Address i_addr) -> Byte * {
        Byte *ram = (Byte *)i_entry->opaque;
        Address addr = i_addr & LN_INTERNAL_RAM_MASK;
        return ram + addr;
    };
    set_mapping(MappingPoint::INTERNAL_RAM,
                {LN_RAM_ADDRESS_HEAD, LN_RAM_ADDRESS_TAIL, false,
                 internal_ram_decode, m_ram});

    // @TMP: Ad-hoc solution
    auto adhoc_ram_decode = [](const MappingEntry *i_entry,
                               Address i_addr) -> Byte * {
        Byte *ram = (Byte *)i_entry->opaque;

        Address addr = i_addr;
        return ram + (addr - 0x4000);
    };
    set_mapping(MappingPoint::ADHOC,
                {0x4000, 0x4017, false, adhoc_ram_decode, m_adhoc});
}

void
MMU::set_mapping(MappingPoint i_point, MappingEntry i_entry)
{
    // @TODO: Check for overlapping range with existing mapping.
    // @NOTE: Ensure valid entry value.
    if (i_entry.begin > i_entry.end)
    {
        get_logger()->error("Invalid mapping entry range: {}, {}",
                            i_entry.begin, i_entry.end);
        return;
    }
    if (i_entry.decode == nullptr)
    {
        get_logger()->error("Empty mapping decode func.");
        return;
    }

    // unset, if any.
    unset_mapping(i_point);

    m_mapping_entries.insert({i_point, i_entry});
    unsigned long address_count = (i_entry.end - i_entry.begin + 1);
    std::fill_n(std::begin(m_mapping_registry) + i_entry.begin, address_count,
                i_point);
}

void
MMU::unset_mapping(MappingPoint i_point)
{
    if (m_mapping_entries.find(i_point) == m_mapping_entries.end())
    {
        return;
    }

    MappingEntry entry = m_mapping_entries.at(i_point);

    static_assert(MappingPoint::INVALID == 0,
                  "m_mapping_registry initialization needs other logic if "
                  "invalid value is not 0");
    unsigned long address_count = (entry.end - entry.begin + 1);
    std::memset(
        m_mapping_registry + entry.begin, MappingPoint::INVALID,
        address_count *
            sizeof(std::remove_extent<decltype(m_mapping_registry)>::type));

    m_mapping_entries.erase(i_point);
}

Error
MMU::get_byte(Address i_addr, Byte &o_byte) const
{
    Byte *byte_ptr = decode_addr(i_addr, false);
    if (!byte_ptr)
    {
        return Error::SEGFAULT;
    }

    o_byte = *byte_ptr;
    return Error::OK;
}

Error
MMU::set_byte(Address i_addr, Byte i_byte)
{
    Byte *byte_ptr = decode_addr(i_addr, true);
    if (!byte_ptr)
    {
        return Error::SEGFAULT;
    }

    *byte_ptr = i_byte;
    return Error::OK;
}

Error
MMU::set_bulk(Address i_begin, Address i_end, Byte i_byte)
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

Byte *
MMU::decode_addr(Address i_addr, bool i_write) const
{
    MappingPoint mp = m_mapping_registry[i_addr];
    if (mp == MappingPoint::INVALID)
    {
        // Invalid address point to nothing.
        return nullptr;
    }

    auto it = m_mapping_entries.find(mp);
    if (it == m_mapping_entries.cend())
    {
        get_logger()->error("Programming error: corrupted mapping state.");
        return nullptr;
    }

    MappingEntry entry = it->second;
    if (i_write && entry.readonly)
    {
        // Can not write to read-only memory.
        return nullptr;
    }

    auto byte_ptr = entry.decode(&entry, i_addr);
    if (!byte_ptr)
    {
        get_logger()->error("Registered mapping can not handle address "
                            "decoding: {}, {}, {}, {}",
                            i_addr, mp, entry.begin, entry.end);
    }
    return byte_ptr;
}

} // namespace ln



#include <cstring>
#include <algorithm>
#include <iterator>

#include "common/logger.hpp"

namespace ln {

template <typename EMappingPoint, std::size_t AddressableSize>
MappableMemory<EMappingPoint, AddressableSize>::MappableMemory()
    : m_mapping_registry{}
{
    static_assert(EMappingPoint::INVALID == 0,
                  "m_mapping_registry initialization needs other logic if "
                  "invalid value is not 0");
}

template <typename EMappingPoint, std::size_t AddressableSize>
Error
MappableMemory<EMappingPoint, AddressableSize>::get_byte(Address i_addr,
                                                         Byte &o_byte) const
{
    Byte *byte_ptr = decode_addr(i_addr, false);
    if (!byte_ptr)
    {
        return Error::SEGFAULT;
    }

    o_byte = *byte_ptr;
    return Error::OK;
}

template <typename EMappingPoint, std::size_t AddressableSize>
Error
MappableMemory<EMappingPoint, AddressableSize>::set_byte(Address i_addr,
                                                         Byte i_byte)
{
    Byte *byte_ptr = decode_addr(i_addr, true);
    if (!byte_ptr)
    {
        return Error::SEGFAULT;
    }

    *byte_ptr = i_byte;
    return Error::OK;
}

template <typename EMappingPoint, std::size_t AddressableSize>
void
MappableMemory<EMappingPoint, AddressableSize>::set_mapping(
    EMappingPoint i_point, MappingEntry i_entry)
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

template <typename EMappingPoint, std::size_t AddressableSize>
void
MappableMemory<EMappingPoint, AddressableSize>::unset_mapping(
    EMappingPoint i_point)
{
    if (m_mapping_entries.find(i_point) == m_mapping_entries.end())
    {
        return;
    }

    MappingEntry entry = m_mapping_entries.at(i_point);

    static_assert(EMappingPoint::INVALID == 0,
                  "m_mapping_registry initialization needs other logic if "
                  "invalid value is not 0");
    unsigned long address_count = (entry.end - entry.begin + 1);
    std::memset(m_mapping_registry + entry.begin, EMappingPoint::INVALID,
                address_count * sizeof(EMappingPoint));

    m_mapping_entries.erase(i_point);
}

template <typename EMappingPoint, std::size_t AddressableSize>
Byte *
MappableMemory<EMappingPoint, AddressableSize>::decode_addr(Address i_addr,
                                                            bool i_write) const
{
    EMappingPoint mp = m_mapping_registry[i_addr];
    if (mp == EMappingPoint::INVALID)
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

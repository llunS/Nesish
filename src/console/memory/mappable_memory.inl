

#include <cstring>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <limits>

#include "common/logger.hpp"

namespace ln {

template <typename EMappingPoint, std::size_t AddressableSize>
MappableMemory<EMappingPoint, AddressableSize>::MappableMemory()
    : m_mapping_registry{}
{
    static_assert(std::numeric_limits<Address>::max() + 1 >= AddressableSize,
                  "AddressableSize too large");

    static_assert(std::is_enum<EMappingPoint>::value,
                  "\"EMappingPoint\" must be enum type");
    static_assert(EMappingPoint::INVALID == EMappingPoint(0),
                  "\"m_mapping_registry\" initialization needs other logic if "
                  "invalid value is not 0");
    for (MappingPointIndex_t i = 0;
         i < MappingPointIndex_t(EMappingPoint::SIZE); ++i)
    {
        m_mapping_entries[i].valid = false;
    }
}

template <typename EMappingPoint, std::size_t AddressableSize>
Error
MappableMemory<EMappingPoint, AddressableSize>::get_byte(Address i_addr,
                                                         Byte &o_val) const
{
    auto entry_kv = get_entry_kv(i_addr);
    const auto &entry = entry_kv.v;
    if (!entry)
    {
        LN_LOG_ERROR(ln::get_logger(),
                     "Programming error: corrupted mapping state.");
        return Error::SEGFAULT;
    }

    if (entry->get_byte)
    {
        auto err = entry->get_byte(entry, i_addr, o_val);
        return err;
    }
    else
    {
        Byte *byte_ptr = decode_addr_raw_impl(entry_kv, i_addr);
        if (!byte_ptr)
        {
            return Error::SEGFAULT;
        }

        o_val = *byte_ptr;
        return Error::OK;
    }
}

template <typename EMappingPoint, std::size_t AddressableSize>
Error
MappableMemory<EMappingPoint, AddressableSize>::set_byte(Address i_addr,
                                                         Byte i_val)
{
    auto entry_kv = get_entry_kv(i_addr);
    const auto &entry = entry_kv.v;
    if (!entry)
    {
        LN_LOG_ERROR(ln::get_logger(),
                     "Programming error: corrupted mapping state.");
        return Error::SEGFAULT;
    }

    if (entry->readonly)
    {
        const auto &mp = entry_kv.k;
        // Can not write to read-only memory.
        LN_LOG_ERROR(ln::get_logger(),
                     "Attempted to write to read-only memory: {}, {}", mp,
                     i_addr);
        return Error::SEGFAULT;
    }

    if (entry->set_byte)
    {
        auto err = entry->set_byte(entry, i_addr, i_val);
        return err;
    }
    else
    {
        Byte *byte_ptr = decode_addr_raw_impl(entry_kv, i_addr);
        if (!byte_ptr)
        {
            return Error::SEGFAULT;
        }
        *byte_ptr = i_val;
        return Error::OK;
    }
}

template <typename EMappingPoint, std::size_t AddressableSize>
void
MappableMemory<EMappingPoint, AddressableSize>::set_mapping(
    EMappingPoint i_point, MappingEntry i_entry)
{
    // @TODO: Check for overlapping range with existing mapping?
    // @NOTE: Ensure valid entry value.
    if (i_entry.begin > i_entry.end)
    {
        LN_LOG_ERROR(ln::get_logger(), "Invalid mapping entry range: {}, {}",
                     i_entry.begin, i_entry.end);
        return;
    }
    if (i_entry.begin >= AddressableSize || i_entry.end >= AddressableSize)
    {
        LN_LOG_ERROR(ln::get_logger(),
                     "Invalid mapping entry range: [{}, {}] in {}",
                     i_entry.begin, i_entry.end, AddressableSize);
        return;
    }
    if (i_entry.decode == nullptr &&
        (i_entry.get_byte == nullptr || i_entry.set_byte == nullptr))
    {
        LN_LOG_ERROR(ln::get_logger(), "Empty mapping callbacks.");
        return;
    }

    // unset, if any.
    unset_mapping(i_point);

    auto entry_idx = MappingPointIndex_t(i_point);
    m_mapping_entries[entry_idx].entry = i_entry;
    m_mapping_entries[entry_idx].valid = true;
    unsigned long address_count = (i_entry.end - i_entry.begin + 1);
    std::fill_n(std::begin(m_mapping_registry) + i_entry.begin, address_count,
                i_point);
}

template <typename EMappingPoint, std::size_t AddressableSize>
void
MappableMemory<EMappingPoint, AddressableSize>::unset_mapping(
    EMappingPoint i_point)
{
    auto entry_idx = MappingPointIndex_t(i_point);
    if (!m_mapping_entries[entry_idx].valid)
    {
        return;
    }

    const MappingEntry &entry = m_mapping_entries[entry_idx].entry;

    static_assert(EMappingPoint::INVALID == EMappingPoint(0),
                  "m_mapping_registry initialization needs other logic if "
                  "invalid value is not 0");
    unsigned long address_count = (entry.end - entry.begin + 1);
    std::memset(m_mapping_registry + entry.begin, 0,
                address_count * sizeof(EMappingPoint));

    m_mapping_entries[entry_idx].valid = false;
}

template <typename EMappingPoint, std::size_t AddressableSize>
auto
MappableMemory<EMappingPoint, AddressableSize>::get_entry_kv(
    Address i_addr) const -> EntryKeyValue
{
    const EMappingPoint &mp = m_mapping_registry[i_addr];
    if (mp == EMappingPoint::INVALID)
    {
        // Invalid address points to nothing.
        return {EMappingPoint::INVALID, nullptr};
    }

    auto entry_idx = MappingPointIndex_t(mp);
    if (!m_mapping_entries[entry_idx].valid)
    {
        return {EMappingPoint::INVALID, nullptr};
    }

    return {mp, &m_mapping_entries[entry_idx].entry};
}

template <typename EMappingPoint, std::size_t AddressableSize>
Byte *
MappableMemory<EMappingPoint, AddressableSize>::decode_addr_raw(
    Address i_addr) const
{
    auto entry_kv = get_entry_kv(i_addr);
    return decode_addr_raw_impl(entry_kv, i_addr);
}

template <typename EMappingPoint, std::size_t AddressableSize>
Byte *
MappableMemory<EMappingPoint, AddressableSize>::decode_addr_raw_impl(
    const EntryKeyValue &i_entry_kv, Address i_addr) const
{
    const auto &mp = i_entry_kv.k;
    const auto &entry = i_entry_kv.v;

    if (!entry)
    {
        return nullptr;
    }

    auto byte_ptr = entry->decode(entry, i_addr);
    if (!byte_ptr)
    {
        LN_LOG_ERROR(ln::get_logger(),
                     "Registered mapping can not handle address "
                     "decoding: {}, {}, {}, {}",
                     i_addr, mp, entry->begin, entry->end);
    }
    return byte_ptr;
}

} // namespace ln

#pragma once

#include <cstddef>
#include <type_traits>

#include "common/error.hpp"
#include "console/types.hpp"
#include "console/memory/mapping_entry.hpp"

namespace ln {

template <typename EMappingPoint, std::size_t AddressableSize>
struct MappableMemory {
  public:
    MappableMemory();

    Error
    get_byte(Address i_addr, Byte &o_val) const;
    Error
    set_byte(Address i_addr, Byte i_val);

    void
    set_mapping(EMappingPoint i_point, MappingEntry i_entry);
    void
    unset_mapping(EMappingPoint i_point);

  protected:
    Byte *
    decode_addr_raw(Address i_addr) const;

  private:
    struct EntryKeyValue {
      public:
        EntryKeyValue(EMappingPoint i_k, const MappingEntry *i_v)
            : k(i_k)
            , v(i_v)
        {
        }

        const MappingEntry *v;
        EMappingPoint k;
    };
    EntryKeyValue
    get_entry_kv(Address i_addr) const;
    Byte *
    decode_addr_raw_impl(const EntryKeyValue &i_entry_kv, Address i_addr) const;

  private:
    EMappingPoint m_mapping_registry[AddressableSize];
    struct EntryElement {
        EntryElement()
            : entry(Uninitialized)
            , valid(false)
        {
        }
        MappingEntry entry;
        bool valid;
    };
    typedef
        typename std::underlying_type<EMappingPoint>::type MappingPointIndex_t;
    EntryElement m_mapping_entries[MappingPointIndex_t(EMappingPoint::SIZE)];
};

} // namespace ln

#include "mappable_memory.inl"

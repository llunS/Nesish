#pragma once

#include <type_traits>

#include "nesish/nesish.h"
#include "types.hpp"
#include "memory/mapping_entry.hpp"

namespace nh {

template <typename EMappingPoint, std::size_t AddressableSize>
struct MappableMemory {
  public:
    MappableMemory(NHLogger *i_logger);

    NHErr
    get_byte(Address i_addr, Byte &o_val) const;
    NHErr
    set_byte(Address i_addr, Byte i_val);

    void
    set_mapping(EMappingPoint i_point, MappingEntry i_entry);
    void
    unset_mapping(EMappingPoint i_point);

  protected:
    NHErr
    decode_addr(Address i_addr, Byte *&o_addr) const;

  private:
    struct EntryKeyValue {
      public:
        EntryKeyValue(EMappingPoint i_k, const MappingEntry *i_v)
            : k(i_k)
            , v(i_v)
        {
        }

        EMappingPoint k;
        const MappingEntry *v;
    };
    EntryKeyValue
    get_entry_kv(Address i_addr) const;
    NHErr
    decode_addr_impl(const EntryKeyValue &i_entry_kv, Address i_addr,
                     Byte *&o_addr) const;

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

  protected:
    NHLogger *m_logger;
};

} // namespace nh

#include "mappable_memory.inl"

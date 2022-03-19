#ifndef LN_CONSOLE_MEMORY_MAPPABLEMEMORY_HPP
#define LN_CONSOLE_MEMORY_MAPPABLEMEMORY_HPP

#include <cstddef>
#include <unordered_map>

#include "common/error.hpp"
#include "common/hash.hpp"
#include "console/types.hpp"
#include "console/memory/mapping_entry.hpp"

namespace ln {

template <typename EMappingPoint, std::size_t AddressableSize>
struct MappableMemory {
  public:
    MappableMemory();

    Error
    get_byte(Address i_addr, Byte &o_byte) const;
    Error
    set_byte(Address i_addr, Byte i_byte);

    void
    set_mapping(EMappingPoint i_point, MappingEntry i_entry);
    void
    unset_mapping(EMappingPoint i_point);

  protected:
    Byte *
    decode_addr(Address i_addr, bool i_write) const;

  private:
    EMappingPoint m_mapping_registry[AddressableSize];
    std::unordered_map<EMappingPoint, MappingEntry, EnumHash> m_mapping_entries;
};

} // namespace ln

#include "mappable_memory.inl"

#endif // LN_CONSOLE_MEMORY_MAPPABLEMEMORY_HPP

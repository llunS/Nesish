#ifndef LN_CONSOLE_MEMORY_MAPPINGENTRY_HPP
#define LN_CONSOLE_MEMORY_MAPPINGENTRY_HPP

#include "console/types.hpp"

namespace ln {

struct MappingEntry;
typedef Byte *(*MappingDecodeFunc)(const MappingEntry *i_entry, Address i_addr);
struct MappingEntry {
    Address begin;
    Address end; // inclusive
    bool readonly;

    MappingDecodeFunc decode;
    void *opaque;

    MappingEntry(Address i_begin, Address i_end, bool i_readonly,
                 MappingDecodeFunc i_decode, void *i_opaque);
    MappingEntry(const MappingEntry &) = default; // trivially copyable
};

} // namespace ln

#endif // LN_CONSOLE_MEMORY_MAPPINGENTRY_HPP

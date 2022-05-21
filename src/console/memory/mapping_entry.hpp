#pragma once

#include "console/types.hpp"

#include <functional>

namespace ln {

struct MappingEntry;
typedef std::function<Byte *(const MappingEntry *i_entry, Address i_addr)>
    MappingDecodeFunc;
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

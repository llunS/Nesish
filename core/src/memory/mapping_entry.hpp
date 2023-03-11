#pragma once

#include "nesish/nesish.h"
#include "types.hpp"
#include "nhbase/klass.hpp"

#include <functional>

namespace nh {

struct MappingEntry;
typedef std::function<NHErr(const MappingEntry *i_entry, Address i_addr,
                            Byte *&o_addr)>
    MappingDecodeFunc;
typedef std::function<NHErr(const MappingEntry *i_entry, Address i_addr,
                            Byte &o_val)>
    MappingGetByteFunc;
typedef std::function<NHErr(const MappingEntry *i_entry, Address i_addr,
                            Byte i_val)>
    MappingSetByteFunc;

struct MappingEntry {
  public:
    MappingEntry(Uninitialized_t)
    {
    }
    MappingEntry(Address i_begin, Address i_end, bool i_readonly,
                 MappingDecodeFunc i_decode, void *i_opaque);
    MappingEntry(Address i_begin, Address i_end, bool i_readonly,
                 MappingGetByteFunc i_get_byte, MappingSetByteFunc i_set_byte,
                 void *i_opaque);

    NB_KLZ_DEFAULT_COPY(MappingEntry); // trivially copyable

  private:
    MappingEntry(Address i_begin, Address i_end, bool i_readonly,
                 MappingDecodeFunc i_decode, MappingGetByteFunc i_get_byte,
                 MappingSetByteFunc i_set_byte, void *i_opaque);

  public:
    Address begin;
    Address end; // inclusive

    void *opaque;

  public:
    template <typename EMappingPoint, std::size_t AddressableSize>
    friend struct MappableMemory;

  private:
    bool readonly;

    MappingDecodeFunc decode;

    MappingGetByteFunc get_byte;
    MappingSetByteFunc set_byte;
};

} // namespace nh

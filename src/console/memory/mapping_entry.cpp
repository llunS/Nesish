
#include "mapping_entry.hpp"

namespace nh {

MappingEntry::MappingEntry(Address i_begin, Address i_end, bool i_readonly,
                           MappingDecodeFunc i_decode,
                           MappingGetByteFunc i_get_byte,
                           MappingSetByteFunc i_set_byte, void *i_opaque)
    : begin(i_begin)
    , end(i_end)
    , readonly(i_readonly)
    , decode(i_decode)
    , get_byte(i_get_byte)
    , set_byte(i_set_byte)
    , opaque(i_opaque)
{
}

MappingEntry::MappingEntry(Address i_begin, Address i_end, bool i_readonly,
                           MappingDecodeFunc i_decode, void *i_opaque)
    : MappingEntry(i_begin, i_end, i_readonly, i_decode, nullptr, nullptr,
                   i_opaque)
{
}

MappingEntry::MappingEntry(Address i_begin, Address i_end, bool i_readonly,
                           MappingGetByteFunc i_get_byte,
                           MappingSetByteFunc i_set_byte, void *i_opaque)
    : MappingEntry(i_begin, i_end, i_readonly, nullptr, i_get_byte, i_set_byte,
                   i_opaque)
{
}

} // namespace nh

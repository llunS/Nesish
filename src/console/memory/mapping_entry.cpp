
#include "mapping_entry.hpp"

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
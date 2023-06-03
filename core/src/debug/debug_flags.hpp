#pragma once

#include "nesish/nesish.h"

namespace nhd {

inline bool
is_debug_on(NHDFlag i_flags, NHDFlag i_flag)
{
    return i_flags & i_flag;
}

inline bool
is_debug_off(NHDFlag i_flags)
{
    return i_flags == NHD_DBG_OFF;
}

inline void
debug_on(NHDFlag &o_flags, NHDFlag i_flag)
{
    o_flags = NHDFlag(o_flags | i_flag);
}

inline void
debug_off(NHDFlag &o_flags, NHDFlag i_flag)
{
    o_flags = NHDFlag(o_flags & ~i_flag);
}

} // namespace nhd

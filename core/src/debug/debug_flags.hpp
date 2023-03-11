#pragma once

#include "nesish/nesish.h"

namespace nhd {

inline bool
is_debug_on(NHDFlags i_flags, NHDFlags i_flag)
{
    return i_flags & i_flag;
}

inline bool
is_debug_off(NHDFlags i_flags)
{
    return i_flags == NHD_DBG_OFF;
}

inline void
debug_on(NHDFlags &o_flags, NHDFlags i_flag)
{
    o_flags = NHDFlags(o_flags | i_flag);
}

inline void
debug_off(NHDFlags &o_flags, NHDFlags i_flag)
{
    o_flags = NHDFlags(o_flags & ~i_flag);
}

} // namespace nhd

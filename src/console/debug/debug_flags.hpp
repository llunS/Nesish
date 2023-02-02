#pragma once

namespace lnd {

enum DebugFlags {
    DBG_OFF = 0,
    DBG_PALETTE = 1 << 0,
    DBG_OAM = 1 << 1,
    DBG_PATTERN = 1 << 2,
};

inline bool
is_debug_on(DebugFlags i_flags, DebugFlags i_flag)
{
    return i_flags & i_flag;
}

inline bool
is_debug_off(DebugFlags i_flags)
{
    return i_flags == DBG_OFF;
}

inline void
debug_on(DebugFlags &o_flags, DebugFlags i_flag)
{
    o_flags = DebugFlags(o_flags | i_flag);
}

inline void
debug_off(DebugFlags &o_flags, DebugFlags i_flag)
{
    o_flags = DebugFlags(o_flags & ~i_flag);
}

} // namespace lnd
